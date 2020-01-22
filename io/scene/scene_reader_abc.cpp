/*
 Imagine
 Copyright 2012-2013 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

#include "scene_reader_abc.h"

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Util/All.h>

#include <cmath>

#include <set>

#include "geometry/editable_geometry_instance.h"

#include "materials/standard_material.h"

#include "global_context.h"

#include "objects/compound_object.h"
#include "objects/mesh.h"

namespace Imagine
{

using namespace Alembic::AbcGeom;

// TODO: this has got a lot of duplicate code from GeoReaderAbc - inherit both that and this class from a common class
//       with helper static functions maybe?

SceneReaderAbc::SceneReaderAbc()
{
}

bool SceneReaderAbc::readFile(const std::string& path, const SceneReaderOptions& options, SceneReaderResults& results)
{
	Alembic::AbcGeom::IArchive archive(Alembic::AbcCoreHDF5::ReadArchive(), path, Abc::ErrorHandler::kQuietNoopPolicy);

	if (!archive.valid())
	{
		GlobalContext::instance().getLogger().error("Invalid Alembic file: %s.", path.c_str());
		return false;
	}

	IObject geomBase = archive.getTop();

	std::vector<Object*> parentObjects;
	std::vector<Material*> newMaterials;

	chrono_t time = 1.0f;//(float)options.importFrame;

	m_options = options;

	processObjectsInstances(geomBase, parentObjects, newMaterials, time, 0);

	if (parentObjects.empty())
	{
		GlobalContext::instance().getLogger().error("No objects found in Alembic file: %s", path.c_str());
		return false;
	}

	std::vector<Object*>::iterator itObject = parentObjects.begin();
	for (; itObject != parentObjects.end(); ++itObject)
	{
		Object* pObject = *itObject;

		results.objects.emplace_back(pObject);
	}

	std::vector<Material*>::iterator itMaterial = newMaterials.begin();
	for (; itMaterial != newMaterials.end(); ++itMaterial)
	{
		Material* pMaterial = *itMaterial;

		results.materials.emplace_back(pMaterial);
	}

	return true;
}

void SceneReaderAbc::processObjectsInstances(IObject& object, std::vector<Object*>& objects, std::vector<Material*>& materials, chrono_t time, unsigned int currentDepth)
{
	Object* pNewObject = nullptr;
	unsigned int childrenCount = object.getNumChildren();
	for (unsigned int i = 0; i < childrenCount; i++)
	{
		IObject child(object.getChild(i));

		P3fArraySamplePtr pPoints;
		Int32ArraySamplePtr pFaceIndices;
		Int32ArraySamplePtr pFaceCounts;
		IV2fGeomParam uvParams;

		size_t numFaces = 0;
		size_t numIndices = 0;
		size_t numPoints = 0;
		size_t numUVs = 0;

		bool isGeometry = false;
		bool alreadyHaveGeometry = false;

		Hash meshDigest;
		AbcA::ArraySampleKey sampleHashKey;

		if (Alembic::AbcGeom::IPolyMesh::matches(child.getHeader()))
		{
			IPolyMesh meshObj(child, Alembic::Abc::kWrapExisting);
			IPolyMeshSchema mesh = meshObj.getSchema();
			IN3fGeomParam N = mesh.getNormalsParam();
			IV2fGeomParam uv = mesh.getUVsParam();

			IPolyMeshSchema::Sample meshSamp;
			const ISampleSelector ss(time);

			mesh.getPositionsProperty().getKey(sampleHashKey, ss);
			std::string hashDigest = sampleHashKey.digest.str();
			meshDigest.addString(hashDigest);

			HashValue finalHash = meshDigest.getHash();

			std::vector<std::string> faceSetNames;
			if (m_options.processFacesets)
			{
				mesh.getFaceSetNames(faceSetNames);
			}
			if (faceSetNames.empty() || faceSetNames.size() == 1)
			{
				pNewObject = new Mesh();
				if (!pNewObject)
					return;

				// see if we've already got the geometry
				GeoInstanceMap::iterator itFind = m_aGeoInstances.find(finalHash);
				if (itFind != m_aGeoInstances.end())
				{
					// we've got it already
					alreadyHaveGeometry = true;
					GeometryInstanceGathered* pExistingGeoInstance = (*itFind).second;

					pNewObject->setGeometryInstance(pExistingGeoInstance);
				}
				else
				{
					mesh.get(meshSamp, ss);

					pPoints = meshSamp.getPositions();
					pFaceIndices = meshSamp.getFaceIndices();
					pFaceCounts = meshSamp.getFaceCounts();
					uvParams = mesh.getUVsParam();

					if (pFaceCounts->size() < 1 || pFaceIndices->size() < 1 || pPoints->size() < 1)
						continue;

					GeometryInstanceGathered* pNewGeoInstance = new EditableGeometryInstance();

					m_aGeoInstances[finalHash] = pNewGeoInstance;

					addPolyPoints(mesh, meshSamp, reinterpret_cast<EditableGeometryInstance*>(pNewGeoInstance), time);

					pNewObject->setGeometryInstance(pNewGeoInstance);
				}

				isGeometry = true;
			}
			else
			{
				// we've got facesets, so make a compound object
				pNewObject = new CompoundObject();
				CompoundObject* pParentObject = reinterpret_cast<CompoundObject*>(pNewObject);
				if (!pParentObject)
					return;

				// process each faceset
				std::vector<std::string>::const_iterator itFacesetName = faceSetNames.begin();
				for (; itFacesetName != faceSetNames.end(); ++itFacesetName)
				{
					const std::string& facesetName = *itFacesetName;
					IFaceSet faceset = mesh.getFaceSet(facesetName);

					IFaceSetSchema& schema = faceset.getSchema();

					IFaceSetSchema::Sample faceSetSample;
					schema.get(faceSetSample);

					Int32ArraySamplePtr actualFaceIndices = faceSetSample.getFaces();
					const int32_t* pFaces = actualFaceIndices->get();
					unsigned int numFaces = actualFaceIndices->size();

					// create a new subobject
					Object* pSubMesh = new Mesh();
					pSubMesh->setName(facesetName);

					Hash faceSetHash;
					// build up a hash of the faceset indexes
					for (unsigned int i = 0; i < numFaces; i++)
					{
						faceSetHash.addInt(pFaces[i]);
					}

					// add original mesh's hash to it
					faceSetHash.addLongLong(finalHash);

					HashValue faceSetMeshHash = faceSetHash.getHash();

					// now see if we have this hash
					GeoInstanceMap::iterator itFind = m_aGeoInstances.find(faceSetMeshHash);
					if (itFind != m_aGeoInstances.end())
					{
						// we've got it already
						GeometryInstanceGathered* pExistingGeoInstance = (*itFind).second;

						pSubMesh->setGeometryInstance(pExistingGeoInstance);
					}
					else
					{
						// otherwise, create it, but only with the faces (and points and UVs) needed
						mesh.get(meshSamp, ss);

						pPoints = meshSamp.getPositions();
						pFaceIndices = meshSamp.getFaceIndices();
						pFaceCounts = meshSamp.getFaceCounts();
						uvParams = mesh.getUVsParam();

						if (meshSamp.getFaceCounts()->size() < 1 || meshSamp.getFaceIndices()->size() < 1 || meshSamp.getPositions()->size() < 1)
							continue;

						GeometryInstanceGathered* pNewGeoInstance = new EditableGeometryInstance();

						m_aGeoInstances[faceSetMeshHash] = pNewGeoInstance;

						addFacesAndUVsWithFacesets(pPoints, pFaceIndices, pFaceCounts, uvParams, actualFaceIndices, reinterpret_cast<EditableGeometryInstance*>(pNewGeoInstance));

						pSubMesh->setGeometryInstance(pNewGeoInstance);
					}

					Material* pObjectMaterial = nullptr;

					MaterialManager& matManager = pSubMesh->getMaterialManager();

					if (m_options.createMaterialsFromFacesets)
					{
						// see if one exists already
						pObjectMaterial = matManager.getMaterialFromName(facesetName);

						// if not, create one
						if (!pObjectMaterial)
						{
							pObjectMaterial = new StandardMaterial();
							pObjectMaterial->setName(facesetName);

							materials.emplace_back(pObjectMaterial);

							matManager.addMaterial(pObjectMaterial);
						}
					}
					else
					{
						// otherwise, just set the default one...
						pObjectMaterial = matManager.getMaterialFromID(1);
					}

					pSubMesh->setMaterial(pObjectMaterial);

					pParentObject->addObject(pSubMesh);
				}

				// apply default material to mesh - don't really need this...
				Material* pDefaultMaterial = pParentObject->getMaterialManager().getMaterialFromID(1);
				if (pDefaultMaterial)
					pParentObject->setMaterial(pDefaultMaterial);

				pParentObject->setStaticStructure(true);

				pParentObject->updateBoundaryBox();
			}
		}
		else if (Alembic::AbcGeom::ISubD::matches(child.getHeader()))
		{
			ISubD subDObje(child, Alembic::Abc::kWrapExisting);
			ISubDSchema mesh = subDObje.getSchema();

			ISubDSchema::Sample meshSamp;
			const ISampleSelector ss(time);

			mesh.getPositionsProperty().getKey(sampleHashKey, ss);
			std::string hashDigest = sampleHashKey.digest.str();
			meshDigest.addString(hashDigest);

			HashValue finalHash = meshDigest.getHash();

			std::vector<std::string> faceSetNames;
			if (m_options.processFacesets)
			{
				mesh.getFaceSetNames(faceSetNames);
			}
			if (faceSetNames.empty() || faceSetNames.size() == 1)
			{
				pNewObject = new Mesh();
				if (!pNewObject)
					return;

				// see if we've already got the geometry
				GeoInstanceMap::iterator itFind = m_aGeoInstances.find(finalHash);
				if (itFind != m_aGeoInstances.end())
				{
					// we've got it already
					alreadyHaveGeometry = true;
					GeometryInstanceGathered* pExistingGeoInstance = (*itFind).second;

					pNewObject->setGeometryInstance(pExistingGeoInstance);
				}
				else
				{
					mesh.get(meshSamp, ss);

					pPoints = meshSamp.getPositions();
					pFaceIndices = meshSamp.getFaceIndices();
					pFaceCounts = meshSamp.getFaceCounts();
					uvParams = mesh.getUVsParam();

					if (meshSamp.getFaceCounts()->size() < 1 || meshSamp.getFaceIndices()->size() < 1 || meshSamp.getPositions()->size() < 1)
						continue;

					GeometryInstanceGathered* pNewGeoInstance = new EditableGeometryInstance();

					m_aGeoInstances[finalHash] = pNewGeoInstance;

					addSubDPoints(mesh, meshSamp, reinterpret_cast<EditableGeometryInstance*>(pNewGeoInstance), time);

					pNewObject->setGeometryInstance(pNewGeoInstance);
				}

				isGeometry = true;
			}
			else
			{
				// we've got facesets, so make a compound object
				pNewObject = new CompoundObject();
				CompoundObject* pParentObject = reinterpret_cast<CompoundObject*>(pNewObject);
				if (!pParentObject)
					return;

				// process each faceset
				std::vector<std::string>::const_iterator itFacesetName = faceSetNames.begin();
				for (; itFacesetName != faceSetNames.end(); ++itFacesetName)
				{
					const std::string& facesetName = *itFacesetName;
					IFaceSet faceset = mesh.getFaceSet(facesetName);

					IFaceSetSchema& schema = faceset.getSchema();

					IFaceSetSchema::Sample faceSetSample;
					schema.get(faceSetSample);

					Int32ArraySamplePtr actualFaceIndices = faceSetSample.getFaces();
					const int32_t* pFaces = actualFaceIndices->get();
					unsigned int numFaces = actualFaceIndices->size();

					// create a new subobject
					Object* pSubMesh = new Mesh();
					pSubMesh->setName(facesetName);

					Hash faceSetHash;
					// build up a hash of the faceset indexes
					for (unsigned int i = 0; i < numFaces; i++)
					{
						faceSetHash.addInt(pFaces[i]);
					}

					// add original mesh's hash to it
					faceSetHash.addLongLong(finalHash);

					HashValue faceSetMeshHash = faceSetHash.getHash();

					// now see if we have this hash
					GeoInstanceMap::iterator itFind = m_aGeoInstances.find(faceSetMeshHash);
					if (itFind != m_aGeoInstances.end())
					{
						// we've got it already
						GeometryInstanceGathered* pExistingGeoInstance = (*itFind).second;

						pSubMesh->setGeometryInstance(pExistingGeoInstance);
					}
					else
					{
						// otherwise, create it, but only with the faces (and points and UVs) needed
						mesh.get(meshSamp, ss);

						pPoints = meshSamp.getPositions();
						pFaceIndices = meshSamp.getFaceIndices();
						pFaceCounts = meshSamp.getFaceCounts();
						uvParams = mesh.getUVsParam();

						if (meshSamp.getFaceCounts()->size() < 1 || meshSamp.getFaceIndices()->size() < 1 || meshSamp.getPositions()->size() < 1)
							continue;

						GeometryInstanceGathered* pNewGeoInstance = new EditableGeometryInstance();

						m_aGeoInstances[faceSetMeshHash] = pNewGeoInstance;

						addFacesAndUVsWithFacesets(pPoints, pFaceIndices, pFaceCounts, uvParams, actualFaceIndices, reinterpret_cast<EditableGeometryInstance*>(pNewGeoInstance));

						pSubMesh->setGeometryInstance(pNewGeoInstance);
					}

					Material* pObjectMaterial = nullptr;

					MaterialManager& matManager = pSubMesh->getMaterialManager();

					if (m_options.createMaterialsFromFacesets)
					{
						// see if one exists already
						pObjectMaterial = matManager.getMaterialFromName(facesetName);

						// if not, create one
						if (!pObjectMaterial)
						{
							pObjectMaterial = new StandardMaterial();
							pObjectMaterial->setName(facesetName);

							materials.emplace_back(pObjectMaterial);
							matManager.addMaterial(pObjectMaterial);
						}
					}
					else
					{
						// otherwise, just set the default one...
						pObjectMaterial = matManager.getMaterialFromID(1);
					}

					pSubMesh->setMaterial(pObjectMaterial);

					pParentObject->addObject(pSubMesh);
				}

				pParentObject->updateBoundaryBox();

				// TODO: We shouldn't need this...
				// apply default material to mesh
				Material* pDefaultMaterial = pParentObject->getMaterialManager().getMaterialFromID(1);
				if (pDefaultMaterial)
					pParentObject->setMaterial(pDefaultMaterial);

				pParentObject->setStaticStructure(true);
			}
		}
		else
		{
			// recurse down, as this object hasn't got any geometry...
			processObjectsInstances(child, objects, materials, time, currentDepth + 1);
			continue;
		}

		if (m_options.setItemsAsBBox)
		{
			pNewObject->setDisplayType(eBoundaryBox);
		}

		pNewObject->setName(child.getName());

		// work out the object transform
		Vector position;
		Vector rotation;
		Vector scale;

		getObjectTransform(child, time, position, rotation, scale);

		pNewObject->transform().position().setFromVector(position);
		pNewObject->transform().rotation().setFromVector(rotation);
		pNewObject->transform().setUniformScale(scale.x);

		if (isGeometry)
		{
			// if we're geometry, we weren't a faceset, so apply default material to mesh
			Material* pDefaultMaterial = pNewObject->getMaterialManager().getMaterialFromID(1);
			if (pDefaultMaterial)
				pNewObject->setMaterial(pDefaultMaterial);

			if (!alreadyHaveGeometry)
			{
				EditableGeometryInstance* pGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pNewObject->getGeometryInstance());
				addFacesAndUVs(pPoints, pFaceIndices, pFaceCounts, uvParams, pGeoInstance);

				if (pNewObject)
				{
					if (!pGeoInstance->getPoints().empty())
					{
						objects.emplace_back(pNewObject);
					}
				}
			}
			else
			{
				// we've got the GeoInstance already...
				if (pNewObject)
				{
					objects.emplace_back(pNewObject);
				}
			}
		}
		else
		{
			// it was from a faceset, so add the CompoundObject which itself should be complete
			objects.emplace_back(pNewObject);
		}
	}
}

void SceneReaderAbc::addPolyPoints(IPolyMeshSchema& meshSchema, IPolyMeshSchema::Sample& meshSample, EditableGeometryInstance* pGeoInstance, chrono_t time)
{
	unsigned int pointCount = meshSample.getPositions()->size();

	std::deque<Point>& meshPoints = pGeoInstance->getPoints();

	for (unsigned int i = 0; i < pointCount; i++)
	{
		V3d point = (*(meshSample.getPositions()))[i];

		meshPoints.emplace_back(Point(point.x, point.y, point.z));
	}
}

void SceneReaderAbc::addSubDPoints(ISubDSchema& subDSchema, ISubDSchema::Sample& subDSample, EditableGeometryInstance* pGeoInstance, chrono_t time)
{
	unsigned int pointCount = subDSample.getPositions()->size();

	std::deque<Point>& meshPoints = pGeoInstance->getPoints();

	for (unsigned int i = 0; i < pointCount; i++)
	{
		V3d point = (*(subDSample.getPositions()))[i];

		meshPoints.emplace_back(Point(point.x, point.y, point.z));
	}
}

void SceneReaderAbc::addFacesAndUVs(P3fArraySamplePtr pPoints, Int32ArraySamplePtr pFaceIndices, Int32ArraySamplePtr pFaceCounts, IV2fGeomParam uvParams,
					EditableGeometryInstance* pGeoInstance)
{
	unsigned int numFaces = pFaceCounts->size();
	unsigned int numIndices = pFaceIndices->size();
	unsigned int numPoints = pPoints->size();
	unsigned int numUVs = 0;

	Alembic::AbcGeom::V2fArraySamplePtr uvValues;
	Alembic::Abc::UInt32ArraySamplePtr uvIndices;

	// check if we've got UVs
	if (uvParams.valid())
	{
		Alembic::AbcGeom::IV2fGeomParam::Sample samples = uvParams.getIndexedValue();
		uvValues = samples.getVals();
		uvIndices = samples.getIndices();

		numUVs = uvIndices->size();
	}

	unsigned int indexCount = 0;
	unsigned int uvIndex = 0;
	bool addUVs = numUVs > 0;

	std::deque<Face>& geoInstanceFaces = pGeoInstance->getFaces();
	std::deque<Point>& geoInstancePoints = pGeoInstance->getPoints();
	std::deque<UV>& geoInstanceUVs = pGeoInstance->getUVs();

	// per polygon, per vertex UVs
	if (numIndices == numUVs)
	{
		for (unsigned int faceIndex = 0; faceIndex < numFaces; faceIndex++)
		{
			unsigned int numVertices = pFaceCounts->get()[faceIndex];

			Face newFace(numVertices);

			unsigned int startUVIndex = uvIndex + numVertices - 1;

			for (unsigned int j = 0; j < numVertices; j++)
			{
				unsigned int vertexIndex = indexCount + j;
				unsigned int vertex = (*pFaceIndices)[vertexIndex];

				if (addUVs)
				{
					unsigned int thisUVIndex = startUVIndex - j;
					V2f uvValue = (*uvValues)[(*uvIndices)[thisUVIndex]];
					geoInstanceUVs.emplace_back(UV(uvValue[0], uvValue[1]));
					newFace.addUV(uvIndex++);
				}

				newFace.addVertex(vertex);
			}

			newFace.calculateNormal(pGeoInstance);
			newFace.reverse(true);

			geoInstanceFaces.emplace_back(newFace);

			indexCount += numVertices;
		}
	}
	else
	{
		for (unsigned int faceIndex = 0; faceIndex < numFaces; faceIndex++)
		{
			unsigned int numVertices = pFaceCounts->get()[faceIndex];

			Face newFace(numVertices);

			unsigned int startUVIndex = uvIndex + numVertices - 1;

			for (unsigned int j = 0; j < numVertices; j++)
			{
				unsigned int vertexIndex = indexCount + j;
				unsigned int vertex = (*pFaceIndices)[vertexIndex];

				if (addUVs)
				{
					unsigned int thisUVIndex = startUVIndex - j;
					// use this index into face vertices index
					unsigned int finalUVIndex = (*pFaceIndices)[thisUVIndex];

					V2f uvValue = (*uvValues)[(*uvIndices)[finalUVIndex]];
					geoInstanceUVs.emplace_back(UV(uvValue[0], uvValue[1]));
					newFace.addUV(uvIndex++);
				}

				newFace.addVertex(vertex);
			}

			newFace.calculateNormal(pGeoInstance);
			newFace.reverse(true);

			geoInstanceFaces.emplace_back(newFace);

			indexCount += numVertices;
		}
	}

	if (addUVs)
	{
		pGeoInstance->setHasPerVertexUVs(true);
	}

	if (!geoInstancePoints.empty())
	{
		pGeoInstance->calculateBoundaryBox();
	}
}

void SceneReaderAbc::addFacesAndUVsWithFacesets(P3fArraySamplePtr pPoints, Int32ArraySamplePtr pFaceIndices, Int32ArraySamplePtr pFaceCounts, IV2fGeomParam uvParams,
					Int32ArraySamplePtr pFaceSetIndices, EditableGeometryInstance* pGeoInstance)
{
	unsigned int numFaces = pFaceCounts->size();
	unsigned int numIndices = pFaceIndices->size();
	unsigned int numPoints = pPoints->size();
	unsigned int numUVs = 0;

	Alembic::AbcGeom::V2fArraySamplePtr uvValues;
	Alembic::Abc::UInt32ArraySamplePtr uvIndices;

	// check if we've got UVs
	if (uvParams.valid())
	{
		Alembic::AbcGeom::IV2fGeomParam::Sample samples = uvParams.getIndexedValue();
		uvValues = samples.getVals();
		uvIndices = samples.getIndices();

		numUVs = uvIndices->size();
	}

	unsigned int indexCount = 0;
	unsigned int uvIndex = 0;
	bool addUVs = numUVs > 0;

	const int32_t* pFacesetFaces = pFaceSetIndices->get();
	unsigned int numFacesetFaces = pFaceSetIndices->size();

	// build a list of faces we want
	std::set<unsigned int> aWantedFaces;
	for (unsigned int i = 0; i < numFacesetFaces; i++)
	{
		unsigned int face = pFacesetFaces[i];

		aWantedFaces.insert(face);
	}

	std::deque<Face>& geoInstanceFaces = pGeoInstance->getFaces();
	std::deque<Point>& geoInstancePoints = pGeoInstance->getPoints();
	std::deque<UV>& geoInstanceUVs = pGeoInstance->getUVs();

	unsigned int newPointIndex = 0;
	unsigned int newUVIndex = 0;

	// per polygon, per vertex UVs
	if (numIndices == numUVs)
	{
		for (unsigned int faceIndex = 0; faceIndex < numFaces; faceIndex++)
		{
			unsigned int numVertices = pFaceCounts->get()[faceIndex];

			unsigned int startUVIndex = uvIndex + numVertices - 1;

			bool addFace = aWantedFaces.count(faceIndex) > 0;

			Face newFace(numVertices);

			for (unsigned int j = 0; j < numVertices; j++)
			{
				unsigned int vertexIndex = indexCount + j;
				unsigned int vertex = (*pFaceIndices)[vertexIndex];

				if (addFace)
				{
					V3d point = (*pPoints)[vertex];
					geoInstancePoints.emplace_back(Point(point.x, point.y, point.z));
					newFace.addVertex(newPointIndex++);

					if (addUVs)
					{
						unsigned int thisUVIndex = startUVIndex - j;
						V2f uvValue = (*uvValues)[(*uvIndices)[thisUVIndex]];

						geoInstanceUVs.emplace_back(UV(uvValue[0], uvValue[1]));

						newFace.addUV(newUVIndex++);
					}
				}

				uvIndex++;
			}

			if (addFace)
			{
				newFace.calculateNormal(pGeoInstance);
				newFace.reverse(true);

				geoInstanceFaces.emplace_back(newFace);
			}

			indexCount += numVertices;
		}
	}
	else
	{
/*
		for (unsigned int faceIndex = 0; faceIndex < numFaces; faceIndex++)
		{
			unsigned int numVertices = pFaceCounts->get()[faceIndex];

			unsigned int startUVIndex = uvIndex + numVertices - 1;

			bool addFace = aWantedFaces.count(faceIndex) > 0;

			Face newFace(numVertices);

			for (unsigned int j = 0; j < numVertices; j++)
			{
				unsigned int vertexIndex = indexCount + j;
				unsigned int vertex = (*pFaceIndices)[vertexIndex];

				if (addFace)
				{
					V3d point = (*pPoints)[vertex];
					geoInstancePoints.emplace_back(Point(point.x, point.y, point.z));
					newFace.addVertex(newPointIndex++);

				}

				if (addUVs)
				{
					unsigned int thisUVIndex = startUVIndex - j;
					// use this index into face vertices index
					unsigned int finalUVIndex = (*pFaceIndices)[thisUVIndex];

					V2f uvValue = (*uvValues)[(*uvIndices)[finalUVIndex]];
					geoInstanceUVs.emplace_back(UV(uvValue[0], uvValue[1]));

					if (addFace)
					{
						newFace.addUV(uvIndex);
					}

					uvIndex++;
				}

				if (addFace)
				{
					newFace.addVertex(vertex);
				}
			}

			if (addFace)
			{
				newFace.calculateNormal(pGeoInstance);
				newFace.reverse(true);

				geoInstanceFaces.emplace_back(newFace);
			}

			indexCount += numVertices;
		}

		*/
	}

	if (addUVs)
	{
		pGeoInstance->setHasPerVertexUVs(true);
	}

	if (!geoInstancePoints.empty())
	{
		pGeoInstance->calculateBoundaryBox();
	}
}

void SceneReaderAbc::getObjectTransform(IObject& object, chrono_t time, Vector& translate, Vector& rotation, Vector& scale)
{
	Imath::M44d matrix = getOverallTransform(object, time);

	getMatrixTransformComponents(matrix, translate, rotation, scale);
}

void SceneReaderAbc::concatenateTransform(Imath::M44d& transform, IObject& object, chrono_t time)
{
	if (!IXform::matches(object.getHeader()))
		return;

	Imath::M44d objectTransform;
	IXform localTransform(object, kWrapExisting);
	XformSample transformSample;
	localTransform.getSchema().get(transformSample);

	if (!localTransform.getSchema().isConstant())
	{
		objectTransform = transformSample.getMatrix();
	}
	else
	{
		ISampleSelector ss(time);
		transformSample = localTransform.getSchema().getValue(ss);
		objectTransform = transformSample.getMatrix();
	}

	transform *= objectTransform;
}

Imath::M44d SceneReaderAbc::getOverallTransform(IObject& object, chrono_t time)
{
	Imath::M44d matrix;
	IObject parentObject = object.getParent();

	while (parentObject)
	{
		concatenateTransform(matrix, parentObject, time);
		parentObject = parentObject.getParent();
	}

	return matrix;
}

void SceneReaderAbc::getMatrixTransformComponents(const Imath::M44d& matrix, Vector& translate, Vector& rotation, Vector& scale)
{
	Imath::M44d localMatrix(matrix);

	Imath::V3d scaleRaw;
	Imath::V3d shear;
	Imath::extractAndRemoveScalingAndShear(localMatrix, scaleRaw, shear);

	scale.x = scaleRaw.x;
	scale.y = scaleRaw.y;
	scale.z = scaleRaw.z;

	translate.x = localMatrix[3][0];
	translate.y = localMatrix[3][1];
	translate.z = localMatrix[3][2];

	// TODO: This still isn't right, but it gives much better results with sample production abc models -

	Matrix4 tempMatrix;
	tempMatrix.setFromArray(localMatrix.getValue(), false);

	rotation = tempMatrix.getRotationXYZ();

	// weirdly needed for some models...
	rotation.z = -rotation.z;
/*
	Imath::Quatd qRotation;
	qRotation = extractQuat(localMatrix);

	rotation = quaternionToEuler(qRotation);
*/
}

Vector SceneReaderAbc::quaternionToEuler(const Imath::Quatd& quat)
{
	Imath::Vec3<double> axis = quat.axis();

	double angle = quat.angle();

	double sqrW = angle * angle;
	double sqrX = axis.x * axis.x;
	double sqrY = axis.y * axis.y;
	double sqrZ = axis.z * axis.z;

	float x = atan2(0.5f * (axis.y * axis.z + axis.x * angle), -sqrX - sqrY + sqrZ + sqrW);

	float y = asin(-0.5f * axis.x * axis.z - axis.y * angle);

	float z = atan2(0.5f * (axis.x * axis.y + axis.z * angle), sqrX - sqrY - sqrZ + sqrW);

	Vector endRotation(degrees(x), degrees(y), degrees(z));


	/*
	Imath::Vec3<double> axis = quat.axis();
	float x = atan2(2.0f * axis.y * quat.angle() - 2 * axis.x * axis.z, 1.0f - 2.0f * powf(axis.y, 2.0f) - 2.0f * powf(axis.z, 2.0f));

	float z = asin(2.0f * axis.x * axis.y + 2.0f * axis.z * quat.angle());

	float y = atan2(2.0f * axis.x * quat.angle() - 2.0f * axis.y * axis.z, 1.0f - 2.0f * powf(axis.x, 2.0f) - 2.0f * powf(axis.z, 2.0f));

	if (axis.x * axis.y + axis.z * quat.angle() == 0.5f)
	{
		x = 2.0f * atan2(axis.x, quat.angle());
		y = 0.0f;
	}
	else if (axis.x * axis.y + axis.z * quat.angle() == -0.5f)
	{
		x =-2.0f * atan2(axis.x, quat.angle());
		y = 0.0f;
	}

	Vector endRotation(degrees(x), degrees(y), degrees(z));
*/
	return endRotation;
}


} // namespace Imagine

namespace
{
	SceneReader* createSceneReaderAbc()
	{
		return new SceneReaderAbc();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerSceneReader("abc", createSceneReaderAbc);
}
