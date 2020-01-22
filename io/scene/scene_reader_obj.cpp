/*
 Imagine
 Copyright 2013 Peter Pearson.

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

#include "scene_reader_obj.h"

#include <fstream>
#include <algorithm>

#include "geometry/geometry_instance.h"

#include "utils/string_helpers.h"
#include "utils/file_helpers.h"

#include "materials/standard_material.h"

#include "objects/mesh.h"
#include "objects/compound_object.h"

#include "io/geo_helper_obj.h"

namespace Imagine
{

SceneReaderObj::SceneReaderObj()
{
}

bool SceneReaderObj::readFile(const std::string& path, const SceneReaderOptions& options, SceneReaderResults& results)
{
	char buf[2048];
	memset(buf, 0, 2048);

	std::fstream fileStream(path.c_str(), std::ios::in);

	std::string line;
	line.resize(2048);

	Mesh* pNewMesh = new Mesh();

	EditableGeometryInstance* pNewGeoInstance = new EditableGeometryInstance();
	pNewMesh->setGeometryInstance(pNewGeoInstance);

	std::vector<StringToken> aItems(128);
	std::vector<StringToken> aComponents(3);

	const std::string sep1 = " ";
	const std::string sep2 = "/";

	unsigned int items = 0;
	unsigned int components = 0;

	// global points list
	std::vector<Point> aPoints;
	aPoints.reserve(400000);
	std::vector<UV> aUVs;
	aUVs.reserve(400000);

	std::set<unsigned int> aVertexIndexesForObject;
	std::set<unsigned int> aUVIndexesForObject;

	Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
	pNewMesh->setMaterial(pDefaultMaterial);

	GeoMaterials materials;
	std::set<Material*> aUsedMaterials;
	std::vector<Object*> subObjects;
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

			aPoints.emplace_back(vertex);
		}
/*		else if (buf[0] == 'v' && buf[1] == 'n')
		{
			float x;
			float y;
			float z;

			sscanf(buf, "vn %f %f %f", &x, &y, &z);

			Normal vertex(x, y, z);

			getSubObjectVertexNormals(pNewMesh).emplace_back(vertex);
		}
*/		else if (buf[0] == 'v' && buf[1] == 't')
		{
			float u;
			float v;

			sscanf(buf, "vt %f %f", &u, &v);

			UV uv(u, v);

			aUVs.emplace_back(uv);
		}
		else if ((buf[0] == 'o' || buf[0] == 'g') && buf[1] == ' ')
		{
			// if we don't want compound objects, ignore this so we just get a single mesh
//			if (!options.importCompoundObjects)
//				continue;

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

				if (!pNewGeoInstance->getPoints().empty())
				{
					GeoHelperObj::calculateFaceNormalsForGeometry(pNewGeoInstance);
					pNewGeoInstance->calculateBoundaryBox();
					subObjects.emplace_back(pNewMesh);
				}
				else
				{
					delete pNewMesh;
					pNewMesh = nullptr;
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
			pNewGeoInstance = new EditableGeometryInstance();
			pNewMesh->setGeometryInstance(pNewGeoInstance);
			pNewMesh->setMaterial(pDefaultMaterial);
			pNewMesh->setName(objectName);
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

			items = fastStringSplit(line, aItems, sep1, 2);

			for (unsigned int i = 0; i < items; i++)
			{
				const StringToken& item = aItems[i];

				const std::string strItem = line.substr(item.start, item.length);

				components = fastSplitNoEmpties(strItem, aComponents, sep2);

				for (unsigned int j = 0; j < components; j++)
				{
					const StringToken& component = aComponents[j];

					if (component.length == 0)
						continue;

					const std::string strValue = strItem.substr(component.start, component.length);

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

			pNewGeoInstance->getFaces().emplace_back(newFace);
		}
		else if (stringCompare(buf, "mtllib", 6))
		{
			// ignore if necessary
			if (!options.importMaterials)
				continue;

			std::string basePath = FileHelpers::getFileDirectory(path);

			line.assign(buf);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			std::string mtlPath(line.substr(7));
			GeoHelperObj::readMaterialFile(basePath + mtlPath, true, "", materials);
		}
		else if (stringCompare(buf, "usemtl", 6))
		{
			// ignore if necessary
			if (!options.importMaterials)
				continue;

			line.assign(buf);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			// apply material (if it exists) to this subobject, overwriting the default one we've set
			std::string mtlName(line.substr(7));

			bool newMaterialBreaksObjectGroup = false;

			// new material, if option is enabled, use it to enforce new object, as a different material has started
			// new object - allocate a new mesh if the current one's valid
			if (newMaterialBreaksObjectGroup && pNewMesh && !aVertexIndexesForObject.empty() && (mtlName != lastMaterialName))
			{
				GeoHelperObj::copyPointsToGeometry(aPoints, aVertexIndexesForObject, pNewGeoInstance);
				GeoHelperObj::copyUVsToGeometry(aUVs, aUVIndexesForObject, pNewGeoInstance);

				if (!pNewGeoInstance->getPoints().empty())
				{
					GeoHelperObj::calculateFaceNormalsForGeometry(pNewGeoInstance);
					pNewGeoInstance->calculateBoundaryBox();
					subObjects.emplace_back(pNewMesh);
				}
				else
				{
					delete pNewMesh;
					pNewMesh = nullptr;
				}

				aVertexIndexesForObject.clear();
				aUVIndexesForObject.clear();

				// create the new one
				pNewMesh = new Mesh();
				pNewGeoInstance = new EditableGeometryInstance();
				pNewMesh->setGeometryInstance(pNewGeoInstance);
				pNewMesh->setMaterial(pDefaultMaterial);
				pNewMesh->setName(lastName);
			}

			lastMaterialName = mtlName;
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
		if (!pNewGeoInstance->getPoints().empty())
		{
			GeoHelperObj::calculateFaceNormalsForGeometry(pNewGeoInstance);
			pNewGeoInstance->calculateBoundaryBox();
			subObjects.emplace_back(pNewMesh);
		}
	}

	if (subObjects.empty())
	{
		// we've got nothing

		if (pNewMesh)
			delete pNewMesh;

		return false;
	}


//	CompoundObject* pCO = new CompoundObject();

	std::vector<Object*>::iterator it = subObjects.begin();
	for (; it != subObjects.end(); ++it)
	{
		Object* pObject = *it;

		results.objects.emplace_back(pObject);

//			pCO->addObject(pObject);
	}

	std::vector<Material*> aMaterials;

	// add all the materials
	std::set<Material*>::iterator itMat = aUsedMaterials.begin();
	for (; itMat != aUsedMaterials.end(); ++itMat)
	{
		Material* pMat = *itMat;

		aMaterials.emplace_back(pMat);

		results.materials.emplace_back(pMat);
	}

	pNewMesh->getMaterialManager().addMaterials(aMaterials, true);

	return true;
}


} // namespace Imagine

namespace
{
	Imagine::SceneReader* createSceneReaderObj()
	{
		return new Imagine::SceneReaderObj();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerSceneReader("obj", createSceneReaderObj);
}
