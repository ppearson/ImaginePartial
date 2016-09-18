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

#include "geo_helper_obj.h"

#include <fstream>
#include <algorithm>

#include "geometry/geometry_instance.h"
#include "geometry/standard_geometry_instance.h"
#include "geometry/standard_geometry_operations.h"

#include "utils/string_helpers.h"
#include "utils/file_helpers.h"
#include "utils/maths/maths.h"

#include "materials/standard_material.h"

namespace Imagine
{

GeoHelperObj::GeoHelperObj()
{
}

bool GeoHelperObj::readMaterialFile(const std::string& mtlPath, GeoMaterials& materials)
{
	char buf[256];
	memset(buf, 0, 256);

	std::fstream fileStream(mtlPath.c_str(), std::ios::in);

	std::string line;
	line.resize(256);

	StandardMaterial* pNewMaterial = NULL;

	float r;
	float g;
	float b;

	while (fileStream.getline(buf, 256))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		unsigned int startIndex = 0;
		// account of some .mtl files having tabs/spaces at the beginning for material values
		while (buf[startIndex] == '\t' || buf[startIndex] == ' ')
			startIndex ++;

		if (stringCompare(buf, "newmtl", 6, startIndex))
		{
			// if it's valid, add the old one to the list
			if (pNewMaterial && !pNewMaterial->getName().empty())
			{
				materials.materials[pNewMaterial->getName()] = pNewMaterial;
			}
			// create a new one
			pNewMaterial = new StandardMaterial();

			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			// set the name
			std::string newName;
			newName.assign(line.substr(7));
			pNewMaterial->setName(newName);
		}
		else if (stringCompare(buf, "map_Kd", 6, startIndex))
		{
			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			std::string diffuseTextureMapFile;
			diffuseTextureMapFile.assign(line.substr(7));

			if (!diffuseTextureMapFile.empty())
			{
				if (diffuseTextureMapFile.find(".") != std::string::npos)
				{
					std::string basePath = FileHelpers::getFileDirectory(mtlPath);

					std::string diffuseTextureMapFullPath = basePath + diffuseTextureMapFile;
					pNewMaterial->setDiffuseTextureMapPath(diffuseTextureMapFullPath, false);
				}
			}
		}
		else if (stringCompare(buf, "bump", 4, startIndex) || stringCompare(buf, "map_bump", 7, startIndex))
		{
			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			size_t spacePos = line.find(" ");

			std::string bumpTextureMapFile;
			bumpTextureMapFile.assign(line.substr(spacePos + 1));

			if (!bumpTextureMapFile.empty())
			{
				if (bumpTextureMapFile.find(".") != std::string::npos)
				{
					std::string basePath = FileHelpers::getFileDirectory(mtlPath);

					std::string bumpTextureMapFullPath = basePath + bumpTextureMapFile;
					pNewMaterial->setBumpTextureMapPath(bumpTextureMapFullPath, false);
				}
			}
		}
		else if (stringCompare(buf, "map_d", 5, startIndex)) // alpha texture
		{
			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			size_t spacePos = line.find(" ");

			std::string alphaTextureMapFile;
			alphaTextureMapFile.assign(line.substr(spacePos + 1));

			if (!alphaTextureMapFile.empty())
			{
				if (alphaTextureMapFile.find(".") != std::string::npos)
				{
					std::string basePath = FileHelpers::getFileDirectory(mtlPath);

					std::string alphaTextureMapFullPath = basePath + alphaTextureMapFile;
					pNewMaterial->setAlphaTextureMapPath(alphaTextureMapFullPath, false);
				}
			}
		}
		else if (buf[startIndex] == 'K' && buf[startIndex + 1] == 'd')
		{
			sscanf(buf + startIndex, "Kd %f %f %f", &r, &g, &b);
			pNewMaterial->setDiffuseColour(Colour3f(r, g, b));
		}
		else if (buf[startIndex] == 'K' && buf[startIndex + 1] == 's')
		{
			sscanf(buf + startIndex, "Ks %f %f %f", &r, &g, &b);
			pNewMaterial->setSpecularColour(Colour3f(r, g, b));
		}
		else if (buf[startIndex] == 'N' && buf[startIndex + 1] == 's')
		{
			float shininess;
			sscanf(buf + startIndex, "Ns %f", &shininess);

			// convert back to our range
			float roughnessValue = fit(shininess, 0.0f, 128.0f, 0.0f, 1.0f);
			pNewMaterial->setSpecularRoughness(1.0f - roughnessValue);
		}
		else if (buf[startIndex] == 'd' && buf[startIndex + 1] == ' ')
		{
			float transparency;
			sscanf(buf + startIndex, "d %f", &transparency);
			pNewMaterial->setTransparancy(1.0f - transparency);
		}
/*		else if (buf[startIndex] == 'T' && buf[startIndex + 1] == 'r')
		{
			float transparency;
			sscanf(buf + startIndex, "Tr %f", &transparency);
			pNewMaterial->setTransparancy(1.0f - transparency);
		}
*/
	}

	fileStream.close();

	// make sure the last material gets added
	if (pNewMaterial && !pNewMaterial->getName().empty())
	{
		materials.materials[pNewMaterial->getName()] = pNewMaterial;
	}

	if (materials.materials.empty())
		return false;

	return true;
}

void GeoHelperObj::copyPointsToGeometry(std::vector<Point>& points, std::set<unsigned int>& pointIndexesRequired, EditableGeometryInstance* pGeoInstance)
{
	std::deque<Point>& geoPoints = pGeoInstance->getPoints();
	std::map<unsigned int, unsigned int> aMapToFaceVertices;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = pointIndexesRequired.begin();
	for (; it != pointIndexesRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoPoints.push_back(points[index]);

		aMapToFaceVertices[index] = localIndex;

		localIndex++;
	}

	// fix up faces so they only
	std::deque<Face>& geoFaces = pGeoInstance->getFaces();

	std::deque<Face>::iterator itFace = geoFaces.begin();
	for (; itFace != geoFaces.end(); ++itFace)
	{
		Face& face = *itFace;

		std::vector<unsigned int> aNewFaceIndexes;

		unsigned int vertexCount = face.getVertexCount();
		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int vertexIndex = face.getVertexPosition(i);

			std::map<unsigned int, unsigned int>::iterator it1 = aMapToFaceVertices.find(vertexIndex);

			unsigned int newIndex = aMapToFaceVertices[vertexIndex];

			aNewFaceIndexes.push_back(newIndex);
		}

		face.clear();

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int vertexIndex = aNewFaceIndexes[i];
			face.addVertex(vertexIndex);
		}

		face.calculateNormal(pGeoInstance);
	}
}

void GeoHelperObj::copyUVsToGeometry(std::vector<UV>& uvs, std::set<unsigned int>& vertexUVsRequired, EditableGeometryInstance* pGeoInstance)
{
	if (vertexUVsRequired.empty())
		return;

	std::deque<UV>& geoUVs = pGeoInstance->getUVs();
	std::map<unsigned int, unsigned int> aMapToFaceUVs;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = vertexUVsRequired.begin();
	for (; it != vertexUVsRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoUVs.push_back(uvs[index]);

		aMapToFaceUVs[index] = localIndex;

		localIndex++;
	}

	if (!geoUVs.empty())
		pGeoInstance->setHasPerVertexUVs(true);

	// fix up faces so they only
	std::deque<Face>& geoFaces = pGeoInstance->getFaces();

	std::deque<Face>::iterator itFace = geoFaces.begin();
	for (; itFace != geoFaces.end(); ++itFace)
	{
		Face& face = *itFace;

		std::vector<unsigned int> aNewFaceIndexes;

		unsigned int vertexCount = face.getVertexCount();
		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int UVIndex = face.getVertexUV(i);

			std::map<unsigned int, unsigned int>::iterator it1 = aMapToFaceUVs.find(UVIndex);
//			assert(it1 != aMapToFaceUVs.end());

			unsigned int newIndex = aMapToFaceUVs[UVIndex];

			aNewFaceIndexes.push_back(newIndex);
		}

		face.clearUVs();

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int vertexIndex = aNewFaceIndexes[i];
			face.addUV(vertexIndex);
		}
	}
}

void GeoHelperObj::calculateFaceNormalsForGeometry(EditableGeometryInstance* pGeoInstance)
{
	std::deque<Face>& geoFaces = pGeoInstance->getFaces();

	std::deque<Face>::iterator it = geoFaces.begin();
	for (; it != geoFaces.end(); ++it)
	{
		Face& face = *it;
		face.calculateNormal(pGeoInstance);
	}
}

void GeoHelperObj::copyPointItemsToGeometry(std::vector<Point>& points, std::set<unsigned int>& pointIndexesRequired,
									StandardGeometryInstance* pGeoInstance)
{
	// points

	std::vector<Point>& geoPoints = pGeoInstance->getPoints();
	std::map<unsigned int, unsigned int> aMapToFaceVertices;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = pointIndexesRequired.begin();
	for (; it != pointIndexesRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoPoints.push_back(points[index]);

		aMapToFaceVertices[index] = localIndex;

		localIndex++;
	}

	std::vector<uint32_t>::iterator itPolyIndices = pGeoInstance->getPolygonIndices().begin();
	for (; itPolyIndices != pGeoInstance->getPolygonIndices().end(); ++itPolyIndices)
	{
		uint32_t& vertexIndex = *itPolyIndices;

		unsigned int newIndex = aMapToFaceVertices[vertexIndex];

		vertexIndex = newIndex;
	}
}

void GeoHelperObj::copyUVItemsToGeometry(std::vector<UV>& uvs, std::set<unsigned int>& vertexUVsRequired, StandardGeometryInstance* pGeoInstance)
{
	std::vector<UV>& geoUVs = pGeoInstance->getUVs();
	std::map<unsigned int, unsigned int> aMapToFaceVerticesUV;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = vertexUVsRequired.begin();
	for (; it != vertexUVsRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoUVs.push_back(uvs[index]);

		aMapToFaceVerticesUV[index] = localIndex;

		localIndex++;
	}

	unsigned int numUVIndices = pGeoInstance->getUVIndicesCount();
	uint32_t* pUVI = const_cast<uint32_t*>(pGeoInstance->m_pUVIndices);
	for (unsigned int i = 0; i < numUVIndices; i++)
	{
		uint32_t& vertexIndex = *pUVI;

		unsigned int newIndex = aMapToFaceVerticesUV[vertexIndex];

		vertexIndex = newIndex;

		pUVI++;
	}
}

} // namespace Imagine
