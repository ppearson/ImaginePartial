/*
 Imagine
 Copyright 2011-2019 Peter Pearson.

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

#include "geo_reader_ply.h"


#include <algorithm>
#include <stdio.h>

#include "global_context.h"

#include "utils/string_helpers.h"
#include "utils/io/data_conversion.h"

#include "objects/mesh.h"

#include "geometry/standard_geometry_instance.h"

namespace Imagine
{

GeoReaderPly::GeoReaderPly()
{
}

bool GeoReaderPly::readFile(const std::string& path, const GeoReaderOptions& options)
{
	std::fstream fileStream(path.c_str(), std::ios::in | std::ios::binary);
	
	PlyHeader headerInfo;
	if (!readHeader(fileStream, headerInfo))
	{
		GlobalContext::instance().getLogger().error("Cannot process PLY header of file: %s", path.c_str());
		return false;
	}
	
	m_readOptions = options;
	
	if (headerInfo.type == eASCII)
	{
		return readASCIIFile(fileStream, headerInfo, options);
	}
	else
	{
		return readBinaryFile(fileStream, path, headerInfo, options);
	}
	
	return false;
}

bool GeoReaderPly::readHeader(std::fstream& fileStream, PlyHeader& header) const
{
	char buf[256];
	memset(buf, 0, 256);

	std::string line;
	line.resize(256);

	std::string key;
	std::string value;

	Element newElement;

	while (fileStream.getline(buf, 256))
	{
		if (buf[0] == 0)
			continue;

		line.assign(buf);
		
		// check for trailing \r with line endings
		if (line[line.size() - 1] == '\r')
			line = line.substr(0, line.size() - 1);

		if (line == "end_header")
		{
			if (newElement.count > 0)
			{
				header.elements.push_back(newElement);
			}
			break;
		}

		splitInTwo(line, key, value, " ");

		if (key == "format")
		{
			std::string type;
			std::string version;
			splitInTwo(value, type, version, " ");

			if (type == "ascii")
				header.type = eASCII;
			else if (type == "binary_big_endian")
				header.type = eBinaryBigEndian;
			else if (type == "binary_little_endian")
				header.type = eBinaryLittleEndian;
		}
		else if (key == "element")
		{
			if (newElement.count > 0)
			{
				header.elements.push_back(newElement);
				newElement = Element();
			}

			std::string type;
			std::string count;
			splitInTwo(value, type, count, " ");

			if (type == "vertex")
			{
				newElement.type = Element::eEVertex;
				unsigned int elementCount = atol(count.c_str());
				newElement.count = elementCount;
			}
			else if (type == "face")
			{
				newElement.type = Element::eEFace;
				unsigned int elementCount = atol(count.c_str());
				newElement.count = elementCount;
			}
		}
		else if (key == "property")
		{
			std::string type;
			std::string other;
			splitInTwo(value, type, other, " ");

			Property newProperty;

			if (type == "float")
			{
				newProperty.mainDataType = Property::eFloat;
				std::string propertyName = other;
				newProperty.name = propertyName;

				if (newElement.type == Element::eEVertex)
				{
					if (propertyName == "x")
					{
						newElement.xVIndex = newElement.properties.size();
					}
					else if (propertyName == "y")
					{
						newElement.yVIndex = newElement.properties.size();
					}
					else if (propertyName == "z")
					{
						newElement.zVIndex = newElement.properties.size();
					}
				}

				newElement.properties.push_back(newProperty);
			}
			else if (type == "uchar")
			{
				newProperty.mainDataType = Property::eUChar;
				std::string propertyName = other;
				newProperty.name = propertyName;

				newElement.properties.push_back(newProperty);
			}
			else if (type == "list")
			{
				std::string mainType;
				std::string remainder;
				splitInTwo(other, mainType, remainder, " ");

				// several apps which use rply to save out ply files
				// use the non-standard 'uint8' type, so we need to cope
				// with this...
				if (mainType == "uchar" || mainType == "uint8")
				{
					newProperty.mainDataType = Property::eUChar;
				}

				newProperty.list = true;

				std::string listDataType;
				std::string remainder2;
				splitInTwo(remainder, listDataType, remainder2, " ");

				if (listDataType == "int")
				{
					newProperty.listDataType = Property::eInt;
				}

				newElement.properties.push_back(newProperty);
			}
			else if (type == "int")
			{
				newProperty.mainDataType = Property::eInt;
				std::string propertyName = other;
				newProperty.name = propertyName;

				newElement.properties.push_back(newProperty);
			}
		}
	}

	return header.type != eNone;
}

bool GeoReaderPly::readASCIIFile(std::fstream& fileStream, const PlyHeader& header, const GeoReaderOptions& options)
{
	char buf[256];
	memset(buf, 0, 256);

	std::string line;
	line.resize(256);
	
	Mesh* pNewMesh = new Mesh();
	if (!pNewMesh)
		return false;
	
	Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
	pNewMesh->setMaterial(pDefaultMaterial);
	
	// TODO: StandardGeometryInstance support
	
	std::vector<Point> aPoints;
		
	EditableGeometryInstance* pNewGeoInstance = NULL;

	pNewGeoInstance = new EditableGeometryInstance();

	pNewMesh->setGeometryInstance(pNewGeoInstance);
	
	// TODO: a lot of this string manipulation / copying can be optimised...
	
	std::vector<Element>::const_iterator itEl = header.elements.begin();
	for (; itEl != header.elements.end(); ++itEl)
	{
		const Element& element = *itEl;
		
		if (element.type == Element::eEVertex)
		{
			aPoints.reserve(element.count);
			
			// assume for the moment we just have float types at the beginning
			
			for (unsigned int i = 0; i < element.count; i++)
			{
				fileStream.getline(buf, 256);
				
				// assume for the moment that the first items are x, y, z vertex positions, and just read those in...
				Point newPoint;
				sscanf(buf, "%f %f %f", &newPoint.x, &newPoint.y, &newPoint.z);
				
				aPoints.push_back(newPoint);
			}
		}
		else if (element.type == Element::eEFace)
		{
			std::deque<Face>& faces = pNewGeoInstance->getFaces();

			for (unsigned int i = 0; i < element.count; i++)
			{
				fileStream.getline(buf, 256);

				line.assign(buf);

				std::string count;
				std::string remainder;
				splitInTwo(line, count, remainder, " ");

				unsigned int numVerts = atol(count.c_str());

				Face newFace(numVerts);
//				newFace.reserveUVs(numVerts);

				unsigned int vertices[5];
				if (numVerts == 3)
				{
					sscanf(remainder.c_str(), "%u %u %u", &vertices[0], &vertices[1], &vertices[2]);

					newFace.addVertex(vertices[0]);
					newFace.addVertex(vertices[1]);
					newFace.addVertex(vertices[2]);
				}
				else if (numVerts == 4)
				{
					sscanf(remainder.c_str(), "%u %u %u %u", &vertices[0], &vertices[1], &vertices[2], &vertices[3]);

					newFace.addVertex(vertices[0]);
					newFace.addVertex(vertices[1]);
					newFace.addVertex(vertices[2]);
					newFace.addVertex(vertices[3]);
				}
				else
				{
					// only pay this penalty if we need to, on the assumption it will be rare...
					std::vector<std::string> aValues;
					splitString(remainder, aValues, " ");
					
					std::vector<std::string>::const_iterator itVal = aValues.begin();
					for (; itVal != aValues.end(); ++itVal)
					{
						const std::string& val = *itVal;
						
						unsigned int vertex = atoi(val.c_str());
						newFace.addVertex(vertex);
					}
				}
				
				faces.push_back(newFace);
			}
		}
	}
	
	std::deque<Point>& geoPoints = pNewGeoInstance->getPoints();
	std::copy(aPoints.begin(), aPoints.end(), std::back_inserter(geoPoints));
	
	// now we've copied the points for the EditableGeometryInstance, we need to go through the faces
	// calculating the normals
	std::deque<Face>& faces = pNewGeoInstance->getFaces();
	std::deque<Face>::iterator itFace = faces.begin();
	for (; itFace != faces.end(); ++itFace)
	{
		Face& face = *itFace;
		
		face.calculateNormal(pNewGeoInstance);
	}
	
	fileStream.close();	
	
	m_newObject = pNewMesh;
	
	postProcess();
	
	return true;
}

bool GeoReaderPly::readBinaryFile(std::fstream& fileStream, const std::string& path, const PlyHeader& header, const GeoReaderOptions& options)
{
	Mesh* pNewMesh = new Mesh();
	if (!pNewMesh)
		return false;
	
	Material* pDefaultMaterial = pNewMesh->getMaterialManager().getMaterialFromID(1);
	pNewMesh->setMaterial(pDefaultMaterial);
	
	std::vector<Point> aTempPoints; // for use with EditableGeo
	
	std::vector<Point>* pActualPoints = NULL;
		
	EditableGeometryInstance* pNewEditableGeoInstance = NULL;
	StandardGeometryInstance* pNewStandardGeoInstance = NULL;
	
	if (options.meshType == GeoReaderOptions::eStandardMesh)
	{
		pNewStandardGeoInstance = new StandardGeometryInstance();
		pNewMesh->setGeometryInstance(pNewStandardGeoInstance);
		pActualPoints = &pNewStandardGeoInstance->getPoints();
	}
	else
	{
		pNewEditableGeoInstance = new EditableGeometryInstance();
		pNewMesh->setGeometryInstance(pNewEditableGeoInstance);
		pActualPoints = &aTempPoints;
	}
	
	std::vector<Element>::const_iterator itEl = header.elements.begin();
	for (; itEl != header.elements.end(); ++itEl)
	{
		const Element& element = *itEl;
		
		if (element.type == Element::eEVertex)
		{
			pActualPoints->reserve(element.count);
			
			Matrix4 rotate;
			rotate.setRotationX(-90.0f);
			
			// assumption here is x y z items are floats and at the beginning
			
			size_t skipSize = 0;
			for (unsigned int i = 0; i < element.properties.size(); i++)
			{
				const Property& property = element.properties[i];
				
				if (property.name == "x" || property.name == "y" || property.name == "z")
					continue;

				skipSize += Property::getMemSize(property.mainDataType);
			}
			
			for (unsigned int i = 0; i < element.count; i++)
			{
				// we can ideally just read the values into the point item directly...
				Point newPoint;
				
				fileStream.read((char*)&newPoint.x, sizeof(float) * 3);
				
				if (header.type == eBinaryBigEndian)
				{
					newPoint.x = reverseFloatBytes(newPoint.x);
					newPoint.y = reverseFloatBytes(newPoint.y);
					newPoint.z = reverseFloatBytes(newPoint.z);
				}
				
				if (options.rotate90NegX)
				{
					newPoint = rotate.transformAffine(newPoint);
				}

				pActualPoints->push_back(newPoint);
				
				if (skipSize > 0)
				{
					// skip past stuff we don't care about
					fileStream.seekg(skipSize, std::ios::cur);
				}
			}
		}
		else if (element.type == Element::eEFace)
		{
			if (element.properties.size() == 0)
			{
				GlobalContext::instance().getLogger().error("Cannot import PLY file: %s - no known face properties found.", path.c_str());
				return false;
			}
			
			// find the first list element on the assumption that it's the vertex indices list
			// TODO: this isn't always going to work, but for all files tested so far, does.
			unsigned int vertexListPropertiesIndex = 0;
			
			size_t preSkipSize = 0;
			
			for (unsigned int i = 0; i < element.properties.size(); i++)
			{
				const Property& testProperty = element.properties[i];
				if (testProperty.list)
				{
					vertexListPropertiesIndex = i;
					break;
				}
				else
				{
					preSkipSize += Property::getMemSize(testProperty.mainDataType);
				}
			}
			
			// assume for the moment that the first element property is the vertex indices list
			const Property& property = element.properties[vertexListPropertiesIndex];
			
			if (!property.list)
			{
				GlobalContext::instance().getLogger().error("Cannot import PLY file: %s - unsupported property combination", path.c_str());
				return false;
			}
			
			if (property.mainDataType != Property::eUChar && property.mainDataType != Property::eChar)
			{
				GlobalContext::instance().getLogger().error("Cannot import PLY file: %s - unsupported property combination", path.c_str());
				return false;
			}
			
			// see if there are other
			size_t afterSkipSize = 0;
			if (element.properties.size() > 1)
			{
				for (unsigned int i = vertexListPropertiesIndex + 1; i < element.properties.size(); i++)
				{
					const Property& prop = element.properties[i];
					
					afterSkipSize += Property::getMemSize(prop.mainDataType);
				}
			}
			
			// in theory, except for the first time through, we can possibly get away
			// with combining the pre and after skips together as they'll happen together,
			// but to future-proof this, do them separately.
			
			std::deque<Face>& faces = pNewEditableGeoInstance->getFaces();
			
			std::vector<unsigned int> aNGonVertices;
			
			if (header.type == eBinaryLittleEndian)
			{
				if (options.meshType == GeoReaderOptions::eStandardMesh)
				{
					std::vector<uint32_t>& aPolyOffsets = pNewStandardGeoInstance->getPolygonOffsets();
					std::vector<uint32_t>& aPolyIndices = pNewStandardGeoInstance->getPolygonIndices();
					
					aPolyOffsets.reserve(element.count + 1);
					aPolyIndices.reserve(element.count * 3); // rough estimate...
					
					for (unsigned int i = 0; i < element.count; i++)
					{
						unsigned char numVerts = 0;
						
						if (preSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(preSkipSize, std::ios::cur);
						}
						
						fileStream.read((char*)&numVerts, sizeof(unsigned char));
						
						unsigned int vertices[4];
						if (numVerts == 3)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 3);
							
							aPolyIndices.push_back(vertices[0]);
							aPolyIndices.push_back(vertices[1]);
							aPolyIndices.push_back(vertices[2]);
						}
						else if (numVerts == 4)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 4);
							
							aPolyIndices.push_back(vertices[0]);
							aPolyIndices.push_back(vertices[1]);
							aPolyIndices.push_back(vertices[2]);
							aPolyIndices.push_back(vertices[3]);
						}
						else
						{
							// only pay this penalty if we need to, on the assumption it'll be pretty rare...
							aNGonVertices.resize(numVerts);
							fileStream.read((char*)aNGonVertices.data(), sizeof(unsigned int) * numVerts);
							std::copy(aNGonVertices.begin(), aNGonVertices.end(), std::back_inserter(aPolyIndices));
						}
						
						aPolyOffsets.push_back(aPolyIndices.size());
						
						if (afterSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(afterSkipSize, std::ios::cur);
						}
					}
				}
				else
				{
					for (unsigned int i = 0; i < element.count; i++)
					{
						unsigned char numVerts = 0;
						
						if (preSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(preSkipSize, std::ios::cur);
						}
						
						fileStream.read((char*)&numVerts, sizeof(unsigned char));
						
						Face newFace((unsigned int)numVerts);
		//				newFace.reserveUVs(numVerts);
						
						unsigned int vertices[4];
						if (numVerts == 3)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 3);
							
							newFace.addVertex(vertices[0]);
							newFace.addVertex(vertices[1]);
							newFace.addVertex(vertices[2]);
						}
						else if (numVerts == 4)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 4);
							
							newFace.addVertex(vertices[0]);
							newFace.addVertex(vertices[1]);
							newFace.addVertex(vertices[2]);
							newFace.addVertex(vertices[3]);
						}
						else
						{
							// only pay this penalty if we need to, on the assumption it'll be pretty rare...
							aNGonVertices.resize(numVerts);
							fileStream.read((char*)aNGonVertices.data(), sizeof(unsigned int) * numVerts);
							
							std::vector<unsigned int>::const_iterator itVert = aNGonVertices.begin();
							for (; itVert != aNGonVertices.end(); ++itVert)
							{
								newFace.addVertex(*itVert);
							}
						}
						
						faces.push_back(newFace);
						
						if (afterSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(afterSkipSize, std::ios::cur);
						}
					}
				}
			}
			else
			{
				// we need to convert byte orders around, so
				
				if (options.meshType == GeoReaderOptions::eStandardMesh)
				{
					std::vector<uint32_t>& aPolyOffsets = pNewStandardGeoInstance->getPolygonOffsets();
					std::vector<uint32_t>& aPolyIndices = pNewStandardGeoInstance->getPolygonIndices();
					
					aPolyOffsets.reserve(element.count + 1);
					aPolyIndices.reserve(element.count * 3); // rough estimate...
					
					for (unsigned int i = 0; i < element.count; i++)
					{
						unsigned char numVerts = 0;
						
						if (preSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(preSkipSize, std::ios::cur);
						}
						
						fileStream.read((char*)&numVerts, sizeof(unsigned char));

						unsigned int vertices[4];
						if (numVerts == 3)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 3);
							
							// TODO: this can be more inlined, and it probably aliases too
							//       but doing it this way is useful for debugging, so...
							vertices[0] = reverseUIntBytes(vertices[0]);
							vertices[1] = reverseUIntBytes(vertices[1]);
							vertices[2] = reverseUIntBytes(vertices[2]);
							
							aPolyIndices.push_back(vertices[0]);
							aPolyIndices.push_back(vertices[1]);
							aPolyIndices.push_back(vertices[2]);
						}
						else if (numVerts == 4)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 4);
							
							// TODO: this can be more inlined, and it probably aliases too
							//       but doing it this way is useful for debugging, so...
							vertices[0] = reverseUIntBytes(vertices[0]);
							vertices[1] = reverseUIntBytes(vertices[1]);
							vertices[2] = reverseUIntBytes(vertices[2]);
							vertices[3] = reverseUIntBytes(vertices[3]);
							
							aPolyIndices.push_back(vertices[0]);
							aPolyIndices.push_back(vertices[1]);
							aPolyIndices.push_back(vertices[2]);
							aPolyIndices.push_back(vertices[3]);
						}
						else
						{
							// only pay this penalty if we need to, on the assumption it'll be pretty rare...
							aNGonVertices.resize(numVerts);
							fileStream.read((char*)aNGonVertices.data(), sizeof(unsigned int) * numVerts);
							
							std::vector<unsigned int>::const_iterator itVert = aNGonVertices.begin();
							for (; itVert != aNGonVertices.end(); ++itVert)
							{
								unsigned int vertexIndex = *itVert;
								
								aPolyIndices.push_back(reverseUIntBytes(vertexIndex));
							}
						}
						
						aPolyOffsets.push_back(aPolyIndices.size());
						
						if (afterSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(afterSkipSize, std::ios::cur);
						}
					}
				}
				else
				{
					for (unsigned int i = 0; i < element.count; i++)
					{
						unsigned char numVerts = 0;
						
						if (preSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(preSkipSize, std::ios::cur);
						}
						
						fileStream.read((char*)&numVerts, sizeof(unsigned char));
						
						Face newFace((unsigned int)numVerts);
		//				newFace.reserveUVs(numVerts);
						
						unsigned int vertices[4];
						if (numVerts == 3)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 3);
							
							// TODO: this can be more inlined, and it probably aliases too
							//       but doing it this way is useful for debugging, so...
							vertices[0] = reverseUIntBytes(vertices[0]);
							vertices[1] = reverseUIntBytes(vertices[1]);
							vertices[2] = reverseUIntBytes(vertices[2]);
							
							newFace.addVertex(vertices[0]);
							newFace.addVertex(vertices[1]);
							newFace.addVertex(vertices[2]);
						}
						else if (numVerts == 4)
						{
							fileStream.read((char*)&vertices[0], sizeof(unsigned int) * 4);
							
							// TODO: this can be more inlined, and it probably aliases too
							//       but doing it this way is useful for debugging, so...
							vertices[0] = reverseUIntBytes(vertices[0]);
							vertices[1] = reverseUIntBytes(vertices[1]);
							vertices[2] = reverseUIntBytes(vertices[2]);
							vertices[3] = reverseUIntBytes(vertices[3]);
							
							newFace.addVertex(vertices[0]);
							newFace.addVertex(vertices[1]);
							newFace.addVertex(vertices[2]);
							newFace.addVertex(vertices[3]);
						}
						else
						{
							// only pay this penalty if we need to, on the assumption it'll be pretty rare...
							aNGonVertices.resize(numVerts);
							fileStream.read((char*)aNGonVertices.data(), sizeof(unsigned int) * numVerts);
							
							std::vector<unsigned int>::const_iterator itVert = aNGonVertices.begin();
							for (; itVert != aNGonVertices.end(); ++itVert)
							{
								unsigned int vertexIndex = *itVert;
								
								newFace.addVertex(reverseUIntBytes(vertexIndex));
							}
						}
						
						faces.push_back(newFace);
						
						if (afterSkipSize > 0)
						{
							// skip past stuff we don't care about
							fileStream.seekg(afterSkipSize, std::ios::cur);
						}
					}
				}
			}
		}
	}
	
	if (options.meshType != GeoReaderOptions::eStandardMesh)
	{
		std::deque<Point>& geoPoints = pNewEditableGeoInstance->getPoints();
		std::copy(pActualPoints->begin(), pActualPoints->end(), std::back_inserter(geoPoints));
		
		// now we've copied the points for the EditableGeometryInstance, we need to go through the faces
		// calculating the normals
		std::deque<Face>& faces = pNewEditableGeoInstance->getFaces();
		std::deque<Face>::iterator itFace = faces.begin();
		for (; itFace != faces.end(); ++itFace)
		{
			Face& face = *itFace;
			
			face.calculateNormal(pNewEditableGeoInstance);
		}
	}
	
	fileStream.close();	
	
	m_newObject = pNewMesh;
	
	postProcess();
	
	return true;
}

} // namespace Imagine

namespace
{
	Imagine::GeoReader* createGeoReaderPly()
	{
		return new Imagine::GeoReaderPly();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerGeoReader("ply", createGeoReaderPly);
}
