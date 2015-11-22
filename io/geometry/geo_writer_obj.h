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

#ifndef GEO_WRITER_OBJ_H
#define GEO_WRITER_OBJ_H

#include "io/geo_writer.h"

class Matrix4;

class GeoWriterObj : public GeoWriter
{
public:
	GeoWriterObj();

	virtual bool writeFile(Object* pObject, const std::string& path, const GeoWriterOptions& options);

	unsigned int writeGeoInstancePoints(EditableGeometryInstance* pGeoInstance, std::fstream& stream, Matrix4* pMatrix);
	unsigned int writeGeoInstanceUVs(EditableGeometryInstance* pGeoInstance, std::fstream& stream);

	void writeGeoInstanceFaces(EditableGeometryInstance* pGeoInstance, std::fstream& stream, unsigned int pointOffset,
							   unsigned int uvOffset, bool writeUVs);
};

#endif // GEO_WRITER_OBJ_H
