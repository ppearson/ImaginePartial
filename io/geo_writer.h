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

#ifndef GEO_WRITER_H
#define GEO_WRITER_H

#include <string>

#include "file_io_registry.h"

class Object;
class EditableGeometryInstance;

struct GeoWriterOptions
{
	GeoWriterOptions() : exportMaterials(true), exportSubObjects(true), applyTransform(true)
	{
	}

	bool exportMaterials;
	bool exportSubObjects;
	bool applyTransform;
};

class GeoWriter
{
public:
	GeoWriter();
	virtual ~GeoWriter();

	virtual bool writeFile(Object* pObject, const std::string& path, const GeoWriterOptions& options) = 0;

	void preProcess();

};

#endif // GEO_WRITER_H
