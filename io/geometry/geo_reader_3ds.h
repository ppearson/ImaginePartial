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

#ifndef GEO_READER_3DS_H
#define GEO_READER_3DS_H

#include "io/geo_reader.h"

#include <stdio.h>

namespace Imagine
{

class Mesh;

class GeoReader3ds : public GeoReader
{
public:
	GeoReader3ds();
	virtual ~GeoReader3ds() { }

	virtual bool readFile(const std::string& path, const GeoReaderOptions& options);

	bool processChunks(std::fstream& stream, const GeoReaderOptions& options);
	bool processMaterialChunks(std::fstream& stream);

	bool processMeshMatrix(std::fstream& stream, Matrix4& matrix);
	bool processPositionChunks(std::fstream& stream, Vector& finalPosition);

	void copyVerticeIndexesToUVIndexes(Mesh* pMesh);

	float readFloat(std::fstream& stream);
	unsigned short readShort(std::fstream& stream);
	unsigned long readLong(std::fstream& stream);
	unsigned char readChar(std::fstream& stream);

protected:
	GeoMaterials	m_materials;
	Mesh*			m_pNewMesh;
	std::vector<Object*> m_aSubObjects;
};

} // namespace Imagine

#endif // GEO_READER_3DS_H
