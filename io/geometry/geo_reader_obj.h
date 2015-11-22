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

#ifndef OBJ_READER_H
#define OBJ_READER_H

#include "io/geo_reader.h"

#include <set>

class GeoReaderObj : public GeoReader
{
public:
	GeoReaderObj();
	virtual ~GeoReaderObj()
	{
	}

	virtual bool readFile(const std::string& path, const GeoReaderOptions& options);

	// EditableGeometryInstance
	bool readFileEditableMesh(const std::string& path, const GeoReaderOptions& options);

	// StandardGeometryInstance
	bool readFileStandardMesh(const std::string& path, const GeoReaderOptions& options);

	// TriangleGeometryInstance
	bool readFileTriangleMesh(const std::string& path, const GeoReaderOptions& options);
};

#endif // OBJ_READER_H
