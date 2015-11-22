/*
 Imagine
 Copyright 2011-2013 Peter Pearson.

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

#include "geo_reader_abc.h"

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Util/All.h>

#include <math.h>

#include "objects/compound_object.h"
#include "objects/mesh.h"

using namespace Alembic::AbcGeom;

GeoReaderAbc::GeoReaderAbc() : GeoReader()
{
}

bool GeoReaderAbc::readFile(const std::string& path, const GeoReaderOptions& options)
{
	m_readOptions = options;

	Alembic::AbcGeom::IArchive archive(Alembic::AbcCoreHDF5::ReadArchive(), path, Abc::ErrorHandler::kQuietNoopPolicy);

	if (!archive.valid())
		return false;

	IObject geomBase = archive.getTop();

	std::vector<Object*> subObjects;

	chrono_t time = (float)options.importFrame;

	if (options.useInstances)
	{
		processObjectsInstances(geomBase, subObjects, time);
	}
	else
	{
		processObjectsBaked(geomBase, subObjects, time);
	}

	if (subObjects.empty())
		return false;

	if (subObjects.size() == 1)
	{
		m_newObject = subObjects[0];
	}
	else
	{
		CompoundObject* pCO = new CompoundObject();

		std::vector<Object*>::iterator it = subObjects.begin();
		for (; it != subObjects.end(); ++it)
		{
			pCO->addObject(*it);
		}

		m_newObject = pCO;
	}

	postProcess();

	return true;
}

void GeoReaderAbc::processObjectsBaked(IObject& object, std::vector<Object*>& objects, chrono_t time)
{
	Mesh* pNewMesh = NULL;
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

		EditableGeometryInstance* pNewGeoInstance = NULL;

		if (Alembic::AbcGeom::IPolyMesh::matches(child.getHeader()))
		{
			IPolyMesh meshObj(child, Alembic::Abc::kWrapExisting);
			IPolyMeshSchema mesh = meshObj.getSchema();
			IN3fGeomParam N = mesh.getNormalsParam();
			IV2fGeomParam uv = mesh.getUVsParam();

			IPolyMeshSchema::Sample meshSamp;
			const ISampleSelector ss(time);
			mesh.get(meshSamp, ss);

			pPoints = meshSamp.getPositions();
			pFaceIndices = meshSamp.getFaceIndices();
			pFaceCounts = meshSamp.getFaceCounts();
			uvParams = mesh.getUVsParam();

			if (pFaceCounts->size() < 1 || pFaceIndices->size() < 1 || pPoints->size() < 1)
				continue;

			pNewMesh = new Mesh();
			if (!pNewMesh)
				return;

			pNewGeoInstance = new EditableGeometryInstance();
			pNewMesh->setGeometryInstance(pNewGeoInstance);

			addTransformedPolyPoints(mesh, meshSamp, pNewMesh, time);
		}
		else if (Alembic::AbcGeom::ISubD::matches(child.getHeader()))
		{
			ISubD subDObje(child, Alembic::Abc::kWrapExisting);
			ISubDSchema mesh = subDObje.getSchema();

			ISubDSchema::Sample meshSamp;
			const ISampleSelector ss(time);
			mesh.get(meshSamp, ss);

			pPoints = meshSamp.getPositions();
			pFaceIndices = meshSamp.getFaceIndices();
			pFaceCounts = meshSamp.getFaceCounts();
			uvParams = mesh.getUVsParam();

			if (meshSamp.getFaceCounts()->size() < 1 || meshSamp.getFaceIndices()->size() < 1 || meshSamp.getPositions()->size() < 1)
				continue;

			pNewMesh = new Mesh();
			if (!pNewMesh)
				return;

			pNewGeoInstance = new EditableGeometryInstance();
			pNewMesh->setGeometryInstance(pNewGeoInstance);

			addTransformedSubDPoints(mesh, meshSamp, pNewMesh, time);
		}
		else
		{
			// recurse down, as this object hasn't got any geometry...
			processObjectsBaked(child, objects, time);
			continue;
		}

		pNewMesh->setName(child.getName());

		// apply default material to mesh
		Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
		if (pDefaultMaterial)
			pNewMesh->setMaterial(pDefaultMaterial);

		numFaces = pFaceCounts->size();
		numIndices = pFaceIndices->size();
		numPoints = pPoints->size();

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

		EditableGeometryInstance* pGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pNewMesh->getGeometryInstance());

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
						geoInstanceUVs.push_back(UV(uvValue[0], uvValue[1]));
						newFace.addUV(uvIndex++);
					}

					newFace.addVertex(vertex);
				}

				newFace.calculateNormal(pGeoInstance);
				newFace.reverse(true);

				geoInstanceFaces.push_back(newFace);

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
						geoInstanceUVs.push_back(UV(uvValue[0], uvValue[1]));
						newFace.addUV(uvIndex++);
					}

					newFace.addVertex(vertex);
				}

				newFace.calculateNormal(pGeoInstance);
				newFace.reverse(true);

				geoInstanceFaces.push_back(newFace);

				indexCount += numVertices;
			}
		}

		if (addUVs)
			pNewMesh->getGeometryInstance()->setHasPerVertexUVs(true);

		if (pNewMesh)
		{
			if (!geoInstancePoints.empty())
			{
				pNewMesh->getGeometryInstance()->calculateBoundaryBox();
				objects.push_back(pNewMesh);
			}
		}
	}
}

void GeoReaderAbc::processObjectsInstances(IObject& object, std::vector<Object*>& objects, chrono_t time)
{
	Mesh* pNewMesh = NULL;
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

		bool hasUVs = false;
		bool alreadyHaveGeometry = false;

		Hash meshDigest;
		AbcA::ArraySampleKey sampleHashKey;

		if (Alembic::AbcGeom::IPolyMesh::matches(child.getHeader()))
		{
			IPolyMesh meshObj(child, Alembic::Abc::kWrapExisting);
			IPolyMeshSchema mesh = meshObj.getSchema();
			IN3fGeomParam N = mesh.getNormalsParam();
			IV2fGeomParam uv = mesh.getUVsParam();

			pNewMesh = new Mesh();
			if (!pNewMesh)
				return;

			IPolyMeshSchema::Sample meshSamp;
			const ISampleSelector ss(time);

			mesh.getPositionsProperty().getKey(sampleHashKey, ss);
			std::string hashDigest = sampleHashKey.digest.str();
			meshDigest.addString(hashDigest);

			HashValue finalHash = meshDigest.getHash();

			// see if we've already got the geometry
			GeoInstanceMap::iterator itFind = m_aGeoInstances.find(finalHash);
			if (itFind != m_aGeoInstances.end())
			{
				// we've got it already
				alreadyHaveGeometry = true;
				EditableGeometryInstance* pExistingGeoInstance = (*itFind).second;

				pNewMesh->setGeometryInstance(pExistingGeoInstance);
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

				EditableGeometryInstance* pNewGeoInstance = new EditableGeometryInstance();

				m_aGeoInstances[finalHash] = pNewGeoInstance;

				addPolyPoints(mesh, meshSamp, pNewGeoInstance, time);

				pNewMesh->setGeometryInstance(pNewGeoInstance);
			}
		}
		else if (Alembic::AbcGeom::ISubD::matches(child.getHeader()))
		{
			ISubD subDObje(child, Alembic::Abc::kWrapExisting);
			ISubDSchema mesh = subDObje.getSchema();

			pNewMesh = new Mesh();
			if (!pNewMesh)
				return;

			ISubDSchema::Sample meshSamp;
			const ISampleSelector ss(time);

			mesh.getPositionsProperty().getKey(sampleHashKey, ss);
			std::string hashDigest = sampleHashKey.digest.str();
			meshDigest.addString(hashDigest);

			HashValue finalHash = meshDigest.getHash();

			// see if we've already got the geometry
			GeoInstanceMap::iterator itFind = m_aGeoInstances.find(finalHash);
			if (itFind != m_aGeoInstances.end())
			{
				// we've got it already
				alreadyHaveGeometry = true;
				EditableGeometryInstance* pExistingGeoInstance = (*itFind).second;

				pNewMesh->setGeometryInstance(pExistingGeoInstance);
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

				EditableGeometryInstance* pNewGeoInstance = new EditableGeometryInstance();

				m_aGeoInstances[finalHash] = pNewGeoInstance;

				addSubDPoints(mesh, meshSamp, pNewGeoInstance, time);

				pNewMesh->setGeometryInstance(pNewGeoInstance);
			}
		}
		else
		{
			// recurse down, as this object hasn't got any geometry...
			processObjectsInstances(child, objects, time);
			continue;
		}

		pNewMesh->setName(child.getName());

		// work out the object transform
		Vector position;
		Vector rotation;

		getObjectTransform(child, time, position, rotation);

		pNewMesh->transform().position().setFromVector(position);
		pNewMesh->transform().rotation().setFromVector(rotation);

		// apply default material to mesh
		Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
		if (pDefaultMaterial)
			pNewMesh->setMaterial(pDefaultMaterial);

		if (!alreadyHaveGeometry)
		{
			numFaces = pFaceCounts->size();
			numIndices = pFaceIndices->size();
			numPoints = pPoints->size();

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
			unsigned int uvCount = 0;
			bool addUVs = numUVs > 0;

			EditableGeometryInstance* pGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pNewMesh->getGeometryInstance());

			for (unsigned int faceIndex = 0; faceIndex < numFaces; faceIndex++)
			{
				unsigned int numVertices = pFaceCounts->get()[faceIndex];

				Face newFace(numVertices);

				for (unsigned int j = 0; j < numVertices; j++)
				{
					unsigned int vertexIndex = indexCount + j;
					unsigned int vertex = (*pFaceIndices)[vertexIndex];

					if (addUVs)
					{
						unsigned int uvIndex = uvCount;// + numVertices - 1; // reverse winding order
						V2f uvValue = (*uvValues)[(*uvIndices)[uvIndex/* - j*/]];
						getSubObjectVertexUVs(pNewMesh).push_back(UV(uvValue[0], uvValue[1]));
						newFace.addUV(uvCount++);
					}

					newFace.addVertex(vertex);
				}

				newFace.calculateNormal(pGeoInstance);
				newFace.reverse(true);

				getSubObjectFaces(pNewMesh).push_back(newFace);

				indexCount += numVertices;
			}

			if (addUVs)
				pNewMesh->getGeometryInstance()->setHasPerVertexUVs(true);

			if (pNewMesh)
			{
				if (!getSubObjectPoints(pNewMesh).empty())
				{
					pNewMesh->getGeometryInstance()->calculateBoundaryBox();
					objects.push_back(pNewMesh);
				}
			}
		}
		else
		{
			// we've got the GeoInstance already...

			if (pNewMesh)
			{
				objects.push_back(pNewMesh);
			}
		}
	}
}

void GeoReaderAbc::addTransformedPolyPoints(IPolyMeshSchema& meshSchema, IPolyMeshSchema::Sample& meshSample, Mesh* pMesh, chrono_t time)
{
	IObject object = meshSchema.getObject();
	Imath::M44d transform = getOverallTransform(object, time);

	unsigned int pointCount = meshSample.getPositions()->size();
	for (unsigned int i = 0; i < pointCount; i++)
	{
		V3d point = (*(meshSample.getPositions()))[i];

		V3d transformedPoint = point * transform;

		getSubObjectPoints(pMesh).push_back(Point(transformedPoint.x, transformedPoint.y, transformedPoint.z));
	}
}

void GeoReaderAbc::addTransformedSubDPoints(ISubDSchema& subDSchema, ISubDSchema::Sample& subDSample, Mesh* pMesh, chrono_t time)
{
	IObject object = subDSchema.getObject();
	Imath::M44d transform = getOverallTransform(object, time);

	unsigned int pointCount = subDSample.getPositions()->size();
	for (unsigned int i = 0; i < pointCount; i++)
	{
		V3f point = (*(subDSample.getPositions()))[i];

		V3d transformedPoint = point * transform;

		getSubObjectPoints(pMesh).push_back(Point(transformedPoint.x, transformedPoint.y, transformedPoint.z));
	}
}

void GeoReaderAbc::addPolyPoints(IPolyMeshSchema& meshSchema, IPolyMeshSchema::Sample& meshSample, EditableGeometryInstance* pGeoInstace, chrono_t time)
{
	unsigned int pointCount = meshSample.getPositions()->size();

	std::deque<Point>& meshPoints = pGeoInstace->getPoints();

	for (unsigned int i = 0; i < pointCount; i++)
	{
		V3d point = (*(meshSample.getPositions()))[i];

		meshPoints.push_back(Point(point.x, point.y, point.z));
	}
}

void GeoReaderAbc::addSubDPoints(ISubDSchema& subDSchema, ISubDSchema::Sample& subDSample, EditableGeometryInstance* pGeoInstace, chrono_t time)
{
	unsigned int pointCount = subDSample.getPositions()->size();

	std::deque<Point>& meshPoints = pGeoInstace->getPoints();

	for (unsigned int i = 0; i < pointCount; i++)
	{
		V3d point = (*(subDSample.getPositions()))[i];

		meshPoints.push_back(Point(point.x, point.y, point.z));
	}
}

void GeoReaderAbc::getObjectTransform(IObject& object, chrono_t time, Vector& translate, Vector& rotation)
{
	Imath::M44d matrix = getOverallTransform(object, time);

	getMatrixTransformComponents(matrix, translate, rotation);
}

void GeoReaderAbc::concatenateTransform(Imath::M44d& transform, IObject& object, chrono_t time)
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

Imath::M44d GeoReaderAbc::getOverallTransform(IObject& object, chrono_t time)
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

void GeoReaderAbc::getMatrixTransformComponents(const Imath::M44d& matrix, Vector& translate, Vector& rotation)
{
	Imath::M44d localMatrix(matrix);

	Imath::V3d scale;
	Imath::V3d shear;
	Imath::extractAndRemoveScalingAndShear(localMatrix, scale, shear);

	translate.x = localMatrix[3][0];
	translate.y = localMatrix[3][1];
	translate.z = localMatrix[3][2];

	// TODO: This still isn't right, but it gives much better results with sample production abc models -

	Matrix4 tempMatrix;
	tempMatrix.setFromArray(localMatrix.getValue(), false);

	rotation = tempMatrix.getRotationXYZ();

	// weirdly needed for some models...
	rotation.z = -rotation.z;

//	Imath::Quatd qRotation;
//	qRotation = extractQuat(localMatrix);

//	rotation = quaternionToEuler(qRotation);
}

Vector GeoReaderAbc::quaternionToEuler(const Imath::Quatd& quat)
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

namespace
{
	GeoReader* createGeoReaderAbc()
	{
		return new GeoReaderAbc();
	}

	const bool registered = FileIORegistry::instance().registerGeoReader("abc", createGeoReaderAbc);
}
