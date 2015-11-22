/*
 Imagine
 Copyright 2011-2012 Peter Pearson.

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

#include "geo_reader_3ds.h"

#include <algorithm>
#include <fstream>

#include "utils/string_helpers.h"
#include "utils/file_helpers.h"

#include "objects/mesh.h"
#include "objects/compound_object.h"
#include "materials/standard_material.h"

GeoReader3ds::GeoReader3ds() : GeoReader()
{
}

bool GeoReader3ds::readFile(const std::string& path, const GeoReaderOptions& options)
{
	m_readOptions = options;

	std::fstream fileStream(path.c_str(), std::ios::in | std::ios::binary);

	if (!fileStream)
		return false;

	m_originalPath = path;

	m_pNewMesh = new Mesh();
	EditableGeometryInstance* pNewGeoInstance = new EditableGeometryInstance();
	m_pNewMesh->setGeometryInstance(pNewGeoInstance);

	processChunks(fileStream, options);

	fileStream.close();

	// make sure the last subobject gets added
	if (m_pNewMesh)
	{
		if (!getSubObjectPoints(m_pNewMesh).empty())
		{
			m_pNewMesh->getGeometryInstance()->calculateBoundaryBox();
			m_aSubObjects.push_back(m_pNewMesh);
		}
	}

	if (m_aSubObjects.empty())
	{
		// we've got nothing

		if (m_pNewMesh)
			delete m_pNewMesh;

		return false;
	}

	// rotate 90 deg on x axis
	Matrix4 rotate;
	rotate.setRotationX(-90.0f);

	if (m_aSubObjects.size() == 1)
	{
		// just the one object
		m_newObject = m_aSubObjects[0];

		Mesh* pMeshObj = dynamic_cast<Mesh*>(m_newObject);
		EditableGeometryInstance* pGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pMeshObj->getGeometryInstance());

		if (options.rotate90NegX)
		{
			applyMatrixToMesh(rotate, pMeshObj, true);
		}
		else
		{
			std::deque<Face>::iterator itFace = pGeoInstance->getFaces().begin();
			for (; itFace != pGeoInstance->getFaces().end(); ++itFace)
			{
				Face& face = *itFace;
				face.calculateNormal(pGeoInstance);
			}
		}
	}
	else
	{
		CompoundObject* pCO = new CompoundObject();

		std::vector<Object*>::iterator it = m_aSubObjects.begin();
		for (; it != m_aSubObjects.end(); ++it)
		{
			Object* pObject = *it;

			Mesh* pMeshObj = dynamic_cast<Mesh*>(pObject);
			EditableGeometryInstance* pGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pMeshObj->getGeometryInstance());

			if (options.rotate90NegX)
			{
				applyMatrixToMesh(rotate, pMeshObj, true);
			}
			else
			{
				std::deque<Face>::iterator itFace = pGeoInstance->getFaces().begin();
				for (; itFace != pGeoInstance->getFaces().end(); ++itFace)
				{
					Face& face = *itFace;
					face.calculateNormal(pGeoInstance);
				}
			}

			pCO->addObject(pMeshObj);
		}

		m_newObject = pCO;
	}

	postProcess();

	MaterialManager& mm = m_newObject->getMaterialManager();

	// add materials
	std::map<std::string, StandardMaterial*>::iterator itMat = m_materials.materials.begin();
	for (; itMat != m_materials.materials.end(); ++itMat)
	{
		StandardMaterial* pBMaterial = (*itMat).second;

		Material* pMaterial = static_cast<Material*>(pBMaterial);

		mm.addMaterial(pMaterial, true);

		m_aNewMaterials.push_back(pMaterial);
	}

	return true;
}

bool GeoReader3ds::processChunks(std::fstream& stream, const GeoReaderOptions& options)
{
	unsigned int vertexCount = 0;
	unsigned int lastVertexCount = 0;
	while (true)
	{
		unsigned short chunkID = readShort(stream);
		uint32_t chunkLength = readLong(stream);

		switch (chunkID)
		{
			case 0x4D4D: // main chunk
				break;
			case 0x3d3d: // 3d editor
				break;
			case 0x4000: // object
			{
				// read in name
				std::string name;
				char cName;
				unsigned int count = 0;
				do
				{
					stream.read((char *) &cName, sizeof(char));
					count ++;
					if (cName != 0)
						name += cName;
				}
				while (cName != 0 && count < 20);

				if (m_pNewMesh)
				{
					m_pNewMesh->setName(name);
				}

				break;
			}
			case 0x4100: // triangle mesh
			{
				// if we don't want compound objects, ignore this so we just get a single mesh
				if (!options.importCompoundObjects)
					continue;

				// new object - allocate a new mesh if the current one's valid
				// if it's valid, add the old one to the list
				if (m_pNewMesh)
				{
					if (!getSubObjectPoints(m_pNewMesh).empty())
					{
						m_pNewMesh->getGeometryInstance()->calculateBoundaryBox();
						m_aSubObjects.push_back(m_pNewMesh);
					}
				}
				// create the new one
				m_pNewMesh = new Mesh();
				EditableGeometryInstance* pNewGeoInstance = new EditableGeometryInstance();
				m_pNewMesh->setGeometryInstance(pNewGeoInstance);
				m_pNewMesh->setMaterial(m_pNewMesh->getMaterialManager().getMaterialFromID(1));

				break;
			}
			case 0x4110: // vertices
			{
				unsigned short numVertices = readShort(stream);

				for (unsigned int i = 0; i < numVertices; i++)
				{
					float x = readFloat(stream);
					float y = readFloat(stream);
					float z = readFloat(stream);

					Point newPoint(x, y, z);

					getSubObjectPoints(m_pNewMesh).push_back(newPoint);
				}

				if (options.importCompoundObjects)
					continue;

				lastVertexCount = vertexCount;
				vertexCount += numVertices;

				break;
			}
			case 0x4120: // faces
			{
				unsigned int vertexOffset = lastVertexCount;
				unsigned short numFaces = readShort(stream);

				bool haveUVs = !getSubObjectVertexUVs(m_pNewMesh).empty();

				for (unsigned int i = 0; i < numFaces; i++)
				{
					unsigned int vert1 = readShort(stream);
					unsigned int vert2 = readShort(stream);
					unsigned int vert3 = readShort(stream);

					// read in the flags
					readShort(stream);

					Face newFace(vert1 + vertexOffset, vert2 + vertexOffset, vert3 + vertexOffset);

					if (haveUVs)
					{
						newFace.addUV(vert1 + vertexOffset);
						newFace.addUV(vert2 + vertexOffset);
						newFace.addUV(vert3 + vertexOffset);
					}

					getSubObjectFaces(m_pNewMesh).push_back(newFace);
				}

				if (haveUVs)
					m_pNewMesh->getGeometryInstance()->setHasPerVertexUVs(true);

				break;
			}
			case 0x4130: // faces material
			{
				// read in name
				std::string materialName;

				char cName;
				unsigned int count = 0;
				do
				{
					stream.read((char *) &cName, sizeof(char));
					count ++;
					if (cName != 0)
						materialName += cName;
				}
				while (cName != 0 && count < 80);

				if (options.importMaterials && m_materials.hasMaterialName(materialName))
				{
					StandardMaterial* pStandardMaterial = m_materials.materials[materialName];

					Material* pMaterial = static_cast<Material*>(pStandardMaterial);
					m_pNewMesh->setMaterial(pMaterial);
				}

				unsigned short numFaces = readShort(stream);

				for (unsigned int i = 0; i < numFaces; i++)
				{
					readShort(stream);
				}

				break;
			}
			case 0x4140: // UV co-ords
			{
				unsigned short numUVs = readShort(stream);

				for (unsigned int i = 0; i < numUVs; i++)
				{
					float u = readFloat(stream);
					float v = readFloat(stream);

					UV newUV(u, v);

					getSubObjectVertexUVs(m_pNewMesh).push_back(newUV);
				}
				break;
			}
			case 0x4160: // local coords
			{
				Matrix4 localMatrix;
				processMeshMatrix(stream, localMatrix);

	//			std::deque<Point>& points = getSubObjectPoints(m_pNewMesh);
	//			applyMatrixToPoints(points, localMatrix);

				break;
			}
			case 0xAFFF: // materials
			{
				processMaterialChunks(stream);

				break;
			}
			case 0xB011: // instance
			{
				break;
			}
			case 0xb013: // pivot
			{
				Vector pivotPos;
				pivotPos.x = readFloat(stream);
				pivotPos.y = readFloat(stream);
				pivotPos.z = readFloat(stream);
				break;
			}
			case 0xB020: // position animation track (for initial position)
			{
				Vector finalPosition;
				processPositionChunks(stream, finalPosition);

		//		Point finalPos(finalPosition);

//				m_pNewMesh->setPosition(finalPos);

				break;
			}
			default:
			{
				stream.seekg(chunkLength - 6, std::ios_base::cur);
				break;
			}
		}

		if (stream.eof() || stream.fail())
			break;
	}

	return true;
}

bool GeoReader3ds::processMaterialChunks(std::fstream& stream)
{
	StandardMaterial* pMaterial = new StandardMaterial();

	bool process = true;

	int lastColour = -1;

	while (process)
	{
		unsigned short chunkID = readShort(stream);
		uint32_t chunkLength = readLong(stream);

		switch (chunkID)
		{
			case 0xA000: // material name
			{
//				stream.seekg(6, std::ios_base::cur);
				std::string name;
				// read in name
				char cName;
				unsigned int count = 0;
				do
				{
					stream.read((char *) &cName, sizeof(char));
					count ++;
					if (cName != 0)
						name += cName;
				}
				while (cName != 0 && count < 80);

				std::string newName = name;
				pMaterial->setName(newName);

				break;
			}
			case 0x0010: // float colour
			{
				float r = readFloat(stream);
				float g = readFloat(stream);
				float b = readFloat(stream);

				if (lastColour != -1)
				{
					Colour3f colour(r, g, b);
					if (lastColour == 1)
						pMaterial->setDiffuseColour(colour);
					else if (lastColour == 2)
						pMaterial->setSpecularColour(colour);
				}

				break;
			}
			case 0x0011: // 24-bit colour
			{
				unsigned char r = readChar(stream);
				unsigned char g = readChar(stream);
				unsigned char b = readChar(stream);

				if (lastColour != -1)
				{
					Colour3f colour(r / 255.0f, g / 255.0f, b / 255.0f);
					if (lastColour == 1)
						pMaterial->setDiffuseColour(colour);
					else if (lastColour == 2)
						pMaterial->setSpecularColour(colour);
				}

				break;
			}
			case 0xA010: // ambient colour
			{
//				lastColour = 0;

				break;
			}
			case 0xA020: // diffuse colour
			{
				lastColour = 1;

				break;
			}
			case 0xA030: // specular colour
			{
				lastColour = 2;

				break;
			}
			case 0xA040: // shininess
			{
//				lastColour = -1;
				break;
			}
			case 0xA041: // shine strength
			{
//				lastColour = -1;
				break;
			}
			case 0xA050: // transparency
			{
//				lastColour = -1;
				break;
			}
			case 0xA200: // texture map
			{
				std::string name;
				// read in name
				char cName;
				unsigned int count = 0;
				do
				{
					stream.read((char *) &cName, sizeof(char));
					count ++;
					if (cName != 0)
						name += cName;
				}
				while (cName != 0 && count < 80);

				std::string diffuseTextureMapFile = name;

				if (!diffuseTextureMapFile.empty())
				{
					if (diffuseTextureMapFile.find(".") != std::string::npos)
					{
						std::string basePath = FileHelpers::getFileDirectory(m_originalPath);

						std::string diffuseTextureMapFullPath = basePath + diffuseTextureMapFile;
						pMaterial->setDiffuseTextureMapPath(diffuseTextureMapFullPath, false);
					}
				}
				break;
			}
			default:
			{
				process = false;
				stream.seekg(chunkLength - 6, std::ios_base::cur);
				break;
			}
		}

		if (stream.eof() || stream.fail())
			break;
	}

	// if it's valid, add the old one to the list
	if (!pMaterial->getName().empty())
	{
		m_materials.materials[pMaterial->getName()] = pMaterial;
	}

	return true;
}

bool GeoReader3ds::processMeshMatrix(std::fstream& stream, Matrix4& matrix)
{
	Matrix4 localMatrix;

	for (unsigned int y = 0; y < 4; y++)
	{
		for (unsigned int x = 0; x < 3; x++)
		{
			localMatrix.at(y, x) = readFloat(stream);
		}
	}

	localMatrix.at(3, 3) = 1.0f;
/*
	float transY = localMatrix.m[1][3];
	float transZ = localMatrix.m[2][3];

	localMatrix.m[1][3] = transZ;
	localMatrix.m[2][3] = -transY;
*/
//	localMatrix.transpose();
//	localMatrix = localMatrix.inverse();

//	matrix = localMatrix;

	return true;
}

bool GeoReader3ds::processPositionChunks(std::fstream& stream, Vector& finalPosition)
{
	unsigned short dummy;
	unsigned short keys;

	dummy = readShort(stream);
	dummy = readShort(stream);
	dummy = readShort(stream);
	dummy = readShort(stream);
	dummy = readShort(stream);
	keys = readShort(stream);
	dummy = readShort(stream);

	// the last key is the final position
	for (unsigned short i = 0; i < keys; i++)
	{
		dummy = readShort(stream);
		unsigned int iDummy = readLong(stream);

		float x = readFloat(stream);
		float y = readFloat(stream);
		float z = readFloat(stream);

		finalPosition = Vector(x, y, z);
	}

	return true;
}

void GeoReader3ds::copyVerticeIndexesToUVIndexes(Mesh* pMesh)
{

}

float GeoReader3ds::readFloat(std::fstream& stream)
{
	float value;
	stream.read((char *) &value, sizeof(float));
	return value;
}

unsigned short GeoReader3ds::readShort(std::fstream& stream)
{
	unsigned short value;
	stream.read((char *) &value, sizeof(unsigned short));
	return value;
}

unsigned long GeoReader3ds::readLong(std::fstream& stream)
{
	uint32_t value;
	stream.read((char *) &value, sizeof(uint32_t));
	return (unsigned long)value;
}

unsigned char GeoReader3ds::readChar(std::fstream& stream)
{
	unsigned char value;
	stream.read((char *) &value, sizeof(unsigned char));
	return value;
}

namespace
{
	GeoReader* createGeoReader3ds()
	{
		return new GeoReader3ds();
	}

	const bool registered = FileIORegistry::instance().registerGeoReader("3ds", createGeoReader3ds);
}
