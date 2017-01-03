/*
 Imagine
 Copyright 2011-2014 Peter Pearson.

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

#include "geo_reader_obj.h"

#include <fstream>
#include <algorithm>

#include "geometry/triangle_geometry_instance.h"
#include "geometry/standard_geometry_instance.h"

#include "utils/string_helpers.h"
#include "utils/file_helpers.h"

#include "materials/standard_material.h"

#include "objects/mesh.h"
#include "objects/triangle_mesh.h"
#include "objects/compound_object.h"

#include "io/geo_helper_obj.h"

namespace Imagine
{

// TODO: lots of duplication here...

GeoReaderObj::GeoReaderObj() : GeoReader()
{
}

bool GeoReaderObj::readFile(const std::string& path, const GeoReaderOptions& options)
{
	if (options.meshType == GeoReaderOptions::eEditable)
	{
		return readFileEditableMesh(path, options);
	}
	else if (options.meshType == GeoReaderOptions::eStandard)
	{
		return readFileStandardMesh(path, options);
	}
	else
	{
		return readFileTriangleMesh(path, options);
	}

	////
}

bool GeoReaderObj::readFileEditableMesh(const std::string& path, const GeoReaderOptions& options)
{
	m_readOptions = options;

	char buf[2048];
	memset(buf, 0, 2048);

	std::fstream fileStream(path.c_str(), std::ios::in);

	std::string line;
	line.resize(2048);

	Mesh* pNewMesh = new Mesh();
	if (!pNewMesh)
		return false;

	std::vector<StringToken> aItems(128);
	std::vector<StringToken> aComponents(3);

	const std::string sep1 = " ";
	const std::string sep2 = "/";

	unsigned int items = 0;
	unsigned int components = 0;

	// global points list
	std::vector<Point> aPoints;
	std::vector<UV> aUVs;

	EditableGeometryInstance* pNewGeoInstance = NULL;

	pNewGeoInstance = new EditableGeometryInstance();

	aPoints.reserve(400000);
	aUVs.reserve(400000);

	pNewMesh->setGeometryInstance(pNewGeoInstance);

	std::set<unsigned int> aVertexIndexesForObject;
	std::set<unsigned int> aUVIndexesForObject;

	Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
	pNewMesh->setMaterial(pDefaultMaterial);

	GeoMaterials materials;
	std::set<Material*> aUsedMaterials;
	std::vector<Object*> subObjects;

	Matrix4 rotate;
	rotate.setRotationX(-90.0f);

	std::string lastName;
	std::string lastMaterialName;
	
	// TODO: need to handle \r\n line ending properly
	while (fileStream.getline(buf, 2048))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		if (buf[0] == 'v' && buf[1] == ' ')
		{
			float x;
			float y;
			float z;

			sscanf(buf, "v %f %f %f", &x, &y, &z);

			Point vertex(x, y, z);

			aPoints.push_back(vertex);
		}
/*		else if (buf[0] == 'v' && buf[1] == 'n')
		{
			float x;
			float y;
			float z;

			sscanf(buf, "vn %f %f %f", &x, &y, &z);

			Normal vertex(x, y, z);

			getSubObjectVertexNormals(pNewMesh).push_back(vertex);
		}
*/		else if (buf[0] == 'v' && buf[1] == 't')
		{
			float u;
			float v;

			sscanf(buf, "vt %f %f", &u, &v);

			UV uv(u, v);

			aUVs.push_back(uv);
		}
		else if ((buf[0] == 'o' || buf[0] == 'g') && buf[1] == ' ')
		{
			// if we don't want compound objects, ignore this so we just get a single mesh
			if (!options.importCompoundObjects)
				continue;
			
			if (buf[0] == 'g')
			{
				// if it's a new group, see if we've got any faces already
				if (pNewGeoInstance->getFaces().empty())
				{
					// if so, don't bother deleting the current object
					continue;
				}
			}

			line.assign(buf);

			// check for trailing \r with line endings which confuses the splitting
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			// new object - allocate a new mesh if the current one's valid
			// if it's valid, add the old one to the list
			if (pNewMesh)
			{
				GeoHelperObj::copyPointsToGeometry(aPoints, aVertexIndexesForObject, pNewGeoInstance);
				GeoHelperObj::copyUVsToGeometry(aUVs, aUVIndexesForObject, pNewGeoInstance);

				if (!getSubObjectPoints(pNewMesh).empty())
				{
					if (options.rotate90NegX)
					{
						applyMatrixToMesh(rotate, pNewMesh, false);
					}
					GeoHelperObj::calculateFaceNormalsForGeometry(pNewGeoInstance);
					pNewGeoInstance->calculateBoundaryBox();
					subObjects.push_back(pNewMesh);
				}
				else
				{
					delete pNewMesh;
					pNewMesh = NULL;
				}
			}

			aVertexIndexesForObject.clear();
			aUVIndexesForObject.clear();

			std::string objectName;
			if (line.size() > 3)
				objectName = line.substr(2);

			lastName = objectName;

			// create the new one
			pNewMesh = new Mesh();
			if (pNewMesh)
			{
				pNewGeoInstance = new EditableGeometryInstance();
				pNewMesh->setGeometryInstance(pNewGeoInstance);
				pNewMesh->setMaterial(pDefaultMaterial);
				pNewMesh->setName(objectName);
			}
		}
		else if (buf[0] == 'f')
		{
			line.assign(buf);

			// check for trailing \r with line endings which confuses the splitting
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			bool overlaps = line[line.size() - 1] == '\\';
			if (overlaps)
			{
				// if it overlaps, read in the next line too
				line = line.substr(0, line.size() - 1);
				fileStream.getline(buf, 256);
				std::string line2(buf);
				line += " ";
				line += line2;
			}

			Face newFace(4);
			newFace.reserveUVs(4);

			items = fastSplit(line, aItems, sep1, 2);

			for (unsigned int i = 0; i < items; i++)
			{
				const StringToken& item = aItems[i];

				const std::string& strItem = line.substr(item.start, item.length);

				components = fastSplitNoEmpties(strItem, aComponents, sep2);

				for (unsigned int j = 0; j < components; j++)
				{
					const StringToken& component = aComponents[j];

					if (component.length == 0)
						continue;

					const std::string& strValue = strItem.substr(component.start, component.length);

					int value = atoi(strValue.c_str());

					switch (j)
					{
						case 0:
						{
							// need to cope with direct indexes (positive, 1-based numbers) and negative indexes
							unsigned int vertexIndex = value > 0 ? value - 1 : (aPoints.size() + value);
							aVertexIndexesForObject.insert(vertexIndex);

							newFace.addVertex(vertexIndex);
							break;
						}
						case 1:
						{
							// uvs

							// need to cope with direct indexes (positive, 1-based numbers) and negative indexes
							unsigned int uvIndex = value > 0 ? value - 1 : (aUVs.size() + value);
							aUVIndexesForObject.insert(uvIndex);

							newFace.addUV(uvIndex);
							break;
						}
						case 2:
						{
							newFace.addNormal(value - 1);
							break;
						}
						default:
							break;
					}
				}
			}

			getSubObjectFaces(pNewMesh).push_back(newFace);
		}
		else if (stringCompare(buf, "mtllib", 6))
		{
			// ignore materials
			if (!options.importMaterials)
				continue;

			std::string basePath = FileHelpers::getFileDirectory(path);

			line.assign(buf);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			std::string mtlPath(line.substr(7));
			GeoHelperObj::readMaterialFile(basePath + mtlPath, materials);
		}
		else if (stringCompare(buf, "usemtl", 6))
		{
			line.assign(buf);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			// apply material (if it exists) to this subobject, overwriting the default one we've set
			std::string mtlName(line.substr(7));

			// new material, if option is enabled, use it to enforce new object, as a different material has started
			// new object - allocate a new mesh if the current one's valid
			if (options.newMaterialBreaksObjectGroup && pNewMesh && !aVertexIndexesForObject.empty() && (mtlName != lastMaterialName))
			{
				GeoHelperObj::copyPointsToGeometry(aPoints, aVertexIndexesForObject, pNewGeoInstance);
				GeoHelperObj::copyUVsToGeometry(aUVs, aUVIndexesForObject, pNewGeoInstance);

				if (!getSubObjectPoints(pNewMesh).empty())
				{
					if (options.rotate90NegX)
					{
						applyMatrixToMesh(rotate, pNewMesh, false);
					}
					GeoHelperObj::calculateFaceNormalsForGeometry(pNewGeoInstance);
					pNewGeoInstance->calculateBoundaryBox();
					subObjects.push_back(pNewMesh);
				}
				else
				{
					delete pNewMesh;
					pNewMesh = NULL;
				}

				aVertexIndexesForObject.clear();
				aUVIndexesForObject.clear();

				// create the new one
				pNewMesh = new Mesh();
				if (pNewMesh)
				{
					pNewGeoInstance = new EditableGeometryInstance();
					pNewMesh->setGeometryInstance(pNewGeoInstance);
					pNewMesh->setMaterial(pDefaultMaterial);
					pNewMesh->setName(lastName);
				}
			}

			lastMaterialName = mtlName;

			// ignore if necessary -- this needs to be done here so that we can break objects apart based on their material (above)
			// but without importing the materials
			if (!options.importMaterials)
				continue;

			if (materials.hasMaterialName(mtlName))
			{
				StandardMaterial* pStandardMaterial = materials.materials[mtlName];

				Material* pMaterial = static_cast<Material*>(pStandardMaterial);
				pNewMesh->setMaterial(pMaterial);

				aUsedMaterials.insert(pMaterial);
			}
		}
	}

	fileStream.close();

	// make sure the last subobject gets added
	if (pNewMesh)
	{
		GeoHelperObj::copyPointsToGeometry(aPoints, aVertexIndexesForObject, pNewGeoInstance);
		if (options.importMaterials)
			GeoHelperObj::copyUVsToGeometry(aUVs, aUVIndexesForObject, pNewGeoInstance);
		if (!getSubObjectPoints(pNewMesh).empty())
		{
			if (options.rotate90NegX)
			{
				applyMatrixToMesh(rotate, pNewMesh, false);
			}
			GeoHelperObj::calculateFaceNormalsForGeometry(pNewGeoInstance);
			pNewGeoInstance->calculateBoundaryBox();
			subObjects.push_back(pNewMesh);
		}
	}

	if (subObjects.empty())
	{
		// we've got nothing

		if (pNewMesh)
			delete pNewMesh;

		return false;
	}
	else if (subObjects.size() == 1)
	{
		// just the one object
		m_newObject = subObjects[0];
//		removeDuplicatePoints(m_newObject);
	}
	else
	{
		CompoundObject* pCO = new CompoundObject();

		std::vector<Object*>::iterator it = subObjects.begin();
		for (; it != subObjects.end(); ++it)
		{
			Object* pObject = *it;
//			removeDuplicatePoints(pObject);
			pCO->addObject(pObject);
		}

		pCO->setType(CompoundObject::eBaked);

		m_newObject = pCO;
	}

	std::vector<Material*> aMaterials;

	// add all the materials
	std::set<Material*>::iterator itMat = aUsedMaterials.begin();
	for (; itMat != aUsedMaterials.end(); ++itMat)
	{
		Material* pMat = *itMat;

		aMaterials.push_back(pMat);

		m_aNewMaterials.push_back(pMat);
	}

	m_newObject->getMaterialManager().addMaterials(aMaterials, true);

	postProcess();

	return true;
}

bool GeoReaderObj::readFileStandardMesh(const std::string& path, const GeoReaderOptions& options)
{
	m_readOptions = options;

	char buf[2048];
	memset(buf, 0, 2048);

	std::fstream fileStream(path.c_str(), std::ios::in);

	std::string line;
	line.resize(2048);

	Mesh* pNewMesh = new Mesh();
	if (!pNewMesh)
		return false;

	std::vector<StringToken> aItems(128);
	std::vector<StringToken> aComponents(3);

	const std::string sep1 = " ";
	const std::string sep2 = "/";

	unsigned int items = 0;
	unsigned int components = 0;

	// global points list
	std::vector<Point> aPoints;
	std::vector<UV> aUVs;

	StandardGeometryInstance* pNewGeoInstance = NULL;

	pNewGeoInstance = new StandardGeometryInstance();

	aPoints.reserve(400000);
	aUVs.reserve(400000);

	pNewMesh->setGeometryInstance(pNewGeoInstance);

	std::set<unsigned int> aVertexIndexesForObject;
	std::set<unsigned int> aUVIndexesForObject;

	Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
	pNewMesh->setMaterial(pDefaultMaterial);

	GeoMaterials materials;
	std::set<Material*> aUsedMaterials;
	std::vector<Object*> subObjects;

	Matrix4 rotate;
	rotate.setRotationX(-90.0f);

	std::string lastName;
	std::string lastMaterialName;

	std::vector<uint32_t> aPolyOffsets;
	std::vector<uint32_t> aPolyIndices;

	std::vector<uint32_t> aUVIndices;

	// TODO: need to handle \r\n line ending properly
	while (fileStream.getline(buf, 2048))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		if (buf[0] == 'v' && buf[1] == ' ')
		{
			float x;
			float y;
			float z;

			sscanf(buf, "v %f %f %f", &x, &y, &z);

			Point vertex(x, y, z);

			aPoints.push_back(vertex);
		}
/*		else if (buf[0] == 'v' && buf[1] == 'n')
		{
			float x;
			float y;
			float z;

			sscanf(buf, "vn %f %f %f", &x, &y, &z);

			Normal vertex(x, y, z);

			getSubObjectVertexNormals(pNewMesh).push_back(vertex);
		}
*/		else if (buf[0] == 'v' && buf[1] == 't')
		{
			float u;
			float v;

			sscanf(buf, "vt %f %f", &u, &v);

			UV uv(u, v);

			aUVs.push_back(uv);
		}
		else if ((buf[0] == 'o' || buf[0] == 'g') && buf[1] == ' ')
		{
			// if we don't want compound objects, ignore this so we just get a single mesh
			if (!options.importCompoundObjects)
				continue;
			
			if (buf[0] == 'g')
			{
				// if it's a new group, see if we've got any faces already
				if (pNewGeoInstance->getPolygonOffsets().empty())
				{
					// if so, don't bother deleting the current object
					continue;
				}
			}

			line.assign(buf);

			// check for trailing \r with line endings which confuses the splitting
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			// new object - allocate a new mesh if the current one's valid
			// if it's valid, add the old one to the list
			if (pNewMesh)
			{
				if (!aPolyOffsets.empty())
				{
					pNewGeoInstance->getPolygonOffsets() = aPolyOffsets;
					pNewGeoInstance->getPolygonIndices() = aPolyIndices;

					GeoHelperObj::copyPointItemsToGeometry(aPoints, aVertexIndexesForObject, pNewGeoInstance);

					if (options.rotate90NegX)
					{
						applyMatrixToMesh(rotate, pNewMesh, false);
					}

					if (!aUVIndices.empty())
					{
						pNewGeoInstance->setUVIndices(aUVIndices);

						GeoHelperObj::copyUVItemsToGeometry(aUVs, aUVIndexesForObject, pNewGeoInstance);
					}

					pNewGeoInstance->calculateBoundaryBox();
					subObjects.push_back(pNewMesh);
				}
				else
				{
					delete pNewMesh;
					pNewMesh = NULL;
				}
			}

			aVertexIndexesForObject.clear();
			aUVIndexesForObject.clear();

			aPolyOffsets.clear();
			aPolyIndices.clear();

			aUVIndices.clear();

			std::string objectName;
			if (line.size() > 3)
				objectName = line.substr(2);

			lastName = objectName;

			// create the new one
			pNewMesh = new Mesh();
			if (pNewMesh)
			{
				pNewGeoInstance = new StandardGeometryInstance();
				pNewMesh->setGeometryInstance(pNewGeoInstance);
				pNewMesh->setMaterial(pDefaultMaterial);
				pNewMesh->setName(objectName);
			}
		}
		else if (buf[0] == 'f')
		{
			line.assign(buf);

			// check for trailing \r with line endings which confuses the splitting
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			bool overlaps = line[line.size() - 1] == '\\';
			if (overlaps)
			{
				// if it overlaps, read in the next line too
				line = line.substr(0, line.size() - 1);
				fileStream.getline(buf, 256);
				std::string line2(buf);
				line += " ";
				line += line2;
			}

			Face newFace(4);
			newFace.reserveUVs(4);

			items = fastSplit(line, aItems, sep1, 2);

			for (unsigned int i = 0; i < items; i++)
			{
				const StringToken& item = aItems[i];

				const std::string& strItem = line.substr(item.start, item.length);

				components = fastSplitNoEmpties(strItem, aComponents, sep2);

				for (unsigned int j = 0; j < components; j++)
				{
					const StringToken& component = aComponents[j];

					if (component.length == 0)
						continue;

					const std::string& strValue = strItem.substr(component.start, component.length);

					int value = atoi(strValue.c_str());

					switch (j)
					{
						case 0:
						{
							// need to cope with direct indexes (positive, 1-based numbers) and negative indexes
							unsigned int vertexIndex = value > 0 ? value - 1 : (aPoints.size() + value);
							aVertexIndexesForObject.insert(vertexIndex);

							aPolyIndices.push_back(vertexIndex);
							break;
						}
						case 1:
						{
							// uvs

							// need to cope with direct indexes (positive, 1-based numbers) and negative indexes
							unsigned int uvIndex = value > 0 ? value - 1 : (aUVs.size() + value);
							aUVIndexesForObject.insert(uvIndex);

							aUVIndices.push_back(uvIndex);
							break;
						}
/*						case 2:
						{
							newFace.addNormal(value - 1);
							break;
						}
*/						default:
							break;
					}
				}
			}

			aPolyOffsets.push_back(aPolyIndices.size());
		}
		else if (stringCompare(buf, "mtllib", 6))
		{
			// ignore materials
			if (!options.importMaterials)
				continue;

			std::string basePath = FileHelpers::getFileDirectory(path);

			line.assign(buf);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			std::string mtlPath(line.substr(7));
			GeoHelperObj::readMaterialFile(basePath + mtlPath, materials);
		}
		else if (stringCompare(buf, "usemtl", 6))
		{
			line.assign(buf);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			// apply material (if it exists) to this subobject, overwriting the default one we've set
			std::string mtlName(line.substr(7));

			// new material, if option is enabled, use it to enforce new object, as a different material has started
			// new object - allocate a new mesh if the current one's valid
			if (options.newMaterialBreaksObjectGroup && pNewMesh && !aVertexIndexesForObject.empty() && (mtlName != lastMaterialName))
			{
				if (!aPolyOffsets.empty())
				{
					pNewGeoInstance->getPolygonOffsets() = aPolyOffsets;
					pNewGeoInstance->getPolygonIndices() = aPolyIndices;

					GeoHelperObj::copyPointItemsToGeometry(aPoints, aVertexIndexesForObject, pNewGeoInstance);

					if (options.rotate90NegX)
					{
						applyMatrixToMesh(rotate, pNewMesh, false);
					}

					if (!aUVIndices.empty())
					{
						pNewGeoInstance->setUVIndices(aUVIndices);

						GeoHelperObj::copyUVItemsToGeometry(aUVs, aUVIndexesForObject, pNewGeoInstance);
					}

					pNewGeoInstance->calculateBoundaryBox();
					subObjects.push_back(pNewMesh);
				}
				else
				{
					delete pNewMesh;
					pNewMesh = NULL;
				}

				aVertexIndexesForObject.clear();
				aUVIndexesForObject.clear();

				aPolyOffsets.clear();
				aPolyIndices.clear();

				aUVIndices.clear();

				// create the new one
				pNewMesh = new Mesh();
				if (pNewMesh)
				{
					pNewGeoInstance = new StandardGeometryInstance();
					pNewMesh->setGeometryInstance(pNewGeoInstance);
					pNewMesh->setMaterial(pDefaultMaterial);
					pNewMesh->setName(lastName);
				}
			}

			lastMaterialName = mtlName;

			// ignore if necessary -- this needs to be done here so that we can break objects apart based on their material (above)
			// but without importing the materials
			if (!options.importMaterials)
				continue;

			if (materials.hasMaterialName(mtlName))
			{
				StandardMaterial* pStandardMaterial = materials.materials[mtlName];

				Material* pMaterial = static_cast<Material*>(pStandardMaterial);
				pNewMesh->setMaterial(pMaterial);

				aUsedMaterials.insert(pMaterial);
			}
		}
	}

	fileStream.close();

	// make sure the last subobject gets added
	if (pNewMesh)
	{
		if (!aPolyOffsets.empty())
		{
			pNewGeoInstance->getPolygonOffsets() = aPolyOffsets;
			pNewGeoInstance->getPolygonIndices() = aPolyIndices;

			GeoHelperObj::copyPointItemsToGeometry(aPoints, aVertexIndexesForObject, pNewGeoInstance);

			if (options.rotate90NegX)
			{
				applyMatrixToMesh(rotate, pNewMesh, false);
			}

			if (!aUVIndices.empty())
			{
				pNewGeoInstance->setUVIndices(aUVIndices);

				GeoHelperObj::copyUVItemsToGeometry(aUVs, aUVIndexesForObject, pNewGeoInstance);
			}

			pNewGeoInstance->calculateBoundaryBox();
			subObjects.push_back(pNewMesh);
		}
	}

	if (subObjects.empty())
	{
		// we've got nothing

		if (pNewMesh)
			delete pNewMesh;

		return false;
	}
	else if (subObjects.size() == 1)
	{
		// just the one object
		m_newObject = subObjects[0];
//		removeDuplicatePoints(m_newObject);
	}
	else
	{
		CompoundObject* pCO = new CompoundObject();

		std::vector<Object*>::iterator it = subObjects.begin();
		for (; it != subObjects.end(); ++it)
		{
			Object* pObject = *it;
//			removeDuplicatePoints(pObject);
			pCO->addObject(pObject);
		}

		pCO->setType(CompoundObject::eBaked);

		m_newObject = pCO;
	}

	std::vector<Material*> aMaterials;

	// add all the materials
	std::set<Material*>::iterator itMat = aUsedMaterials.begin();
	for (; itMat != aUsedMaterials.end(); ++itMat)
	{
		Material* pMat = *itMat;

		aMaterials.push_back(pMat);

		m_aNewMaterials.push_back(pMat);
	}

	m_newObject->getMaterialManager().addMaterials(aMaterials, true);

	postProcess();

	return true;
}

// this method assumes only one large single mesh is going to be read in (no subobjects, or groups)
bool GeoReaderObj::readFileTriangleMesh(const std::string& path, const GeoReaderOptions& options)
{
	m_readOptions = options;

	char buf[1024];
	memset(buf, 0, 1024);

	std::fstream fileStream(path.c_str(), std::ios::in);

	std::string line;
	line.resize(1024);

	TriangleMesh* pNewMesh = new TriangleMesh();
	if (!pNewMesh)
		return false;

	std::vector<StringToken> aItems(16);
	std::vector<StringToken> aComponents(3);

	const std::string sep1 = " ";
	const std::string sep2 = "/";

	unsigned int items = 0;
	unsigned int components = 0;

	TriangleGeometryInstance* pNewGeoInstance = new TriangleGeometryInstance();

	std::vector<Point>& aPoints = pNewGeoInstance->getPoints();
	std::vector<UV>& aUVs = pNewGeoInstance->getUVs();

	aPoints.reserve(400000);
	aUVs.reserve(100000);

	pNewMesh->setGeometryInstance(pNewGeoInstance);

	Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
	pNewMesh->setMaterial(pDefaultMaterial);

	Matrix4 rotate;
	rotate.setRotationX(-90.0f);

	uint32_t triangleVerticies[3];
	unsigned int triangleIndex = 0;

	// TODO: need to handle \r\n line ending properly
	while (fileStream.getline(buf, 1024))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		if (buf[0] == 'v' && buf[1] == ' ')
		{
			float x;
			float y;
			float z;

			sscanf(buf, "v %f %f %f", &x, &y, &z);

			Point vertex(x, y, z);

			aPoints.push_back(vertex);
		}
/*		else if (buf[0] == 'v' && buf[1] == 't')
		{
			float u;
			float v;

			sscanf(buf, "vt %f %f", &u, &v);

			UV uv(u, v);

			pUVsList->push_back(uv);
		}
*/		else if (buf[0] == 'f')
		{
			line.assign(buf);

			// check for trailing \r with line endings which confuses the splitting
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			bool overlaps = line[line.size() - 1] == '\\';
			if (overlaps)
			{
				// if it overlaps, read in the next line too
				line = line.substr(0, line.size() - 1);
				fileStream.getline(buf, 256);
				std::string line2(buf);
				line += " ";
				line += line2;
			}

			items = fastSplit(line, aItems, sep1, 2);

//			assert(items == 3);

			for (unsigned int i = 0; i < items; i++)
			{
				const StringToken& item = aItems[i];

				const std::string& strItem = line.substr(item.start, item.length);

				components = fastSplitNoEmpties(strItem, aComponents, sep2);

				for (unsigned int j = 0; j < components; j++)
				{
					const StringToken& component = aComponents[j];

					if (component.length == 0)
						continue;

					const std::string& strValue = strItem.substr(component.start, component.length);

					int value = atoi(strValue.c_str());

					switch (j)
					{
						case 0:
						{
							// need to cope with direct indexes (positive, 1-based numbers) and negative indexes
							unsigned int vertexIndex = value > 0 ? value - 1 : (aPoints.size() + value);
							triangleVerticies[i] = vertexIndex;

							break;
						}
						case 1:
						{
							// uvs
/*
							// need to cope with direct indexes (positive, 1-based numbers) and negative indexes
							unsigned int uvIndex = value > 0 ? value - 1 : (aUVs.size() + value);
							newFace.addUV(uvIndex);
*/							break;
						}
						case 2:
						{
//							newFace.addNormal(value - 1);
							break;
						}
						default:
							break;
					}
				}
			}

			// only do the TriangleIndicesUniform here, as we'll probably be doing a post-import process to centre or stand on a plane
			// the geometry, which will move the points, so we can't create the Triangles here...

			TriangleIndicesUniform newIndices(triangleVerticies[0], triangleVerticies[1], triangleVerticies[2], triangleIndex++);

			pNewGeoInstance->getTriangleIndices().push_back(newIndices);
		}
	}

//	fprintf(stderr, "Final triangle index: %u\n", triangleIndex);

	fileStream.close();

	// make sure the last subobject gets added
	if (pNewMesh)
	{
		if (options.rotate90NegX)
		{
//			applyMatrixToMesh(rotate, pNewMesh, false);
		}
		pNewGeoInstance->calculateBoundaryBox();
	}

	m_newObject = pNewMesh;

	postProcess();

	return true;
}


} // namespace Imagine

namespace
{
	Imagine::GeoReader* createGeoReaderObj()
	{
		return new Imagine::GeoReaderObj();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerGeoReader("obj", createGeoReaderObj);
}
