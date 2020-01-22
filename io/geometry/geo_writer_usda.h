/*
 Imagine
 Copyright 2019-2020 Peter Pearson.

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

#ifndef GEO_WRITER_USDA_H
#define GEO_WRITER_USDA_H

#include "io/geo_writer.h"

#include <fstream>

#include <set>

namespace Imagine
{

class Matrix4;
class Vector;
class Point;
class UV;

class GeoWriterUSDA : public GeoWriter
{
public:
	GeoWriterUSDA();
	
	virtual bool writeFile(Object* pObject, const std::string& path, const GeoWriterOptions& options);
	
protected:

	struct ObjectTypeNameCounts
	{
		unsigned int	groupCount = 0;
		unsigned int	meshCount = 0;
		unsigned int	sphereCount = 0;
		unsigned int	existingCount = 0;
	};

	struct ProcessContext
	{
		ProcessContext(std::fstream& stream, const GeoWriterOptions& options) : stream(stream), options(options)
		{

		}

		std::fstream&				stream;
		const GeoWriterOptions&		options;

		ObjectTypeNameCounts		counts;
	};
	
	bool writeObject(ProcessContext& processContext, const Object* pObject, unsigned int nestLevel);
	
	void writeMesh(ProcessContext& processContext, const Object* pObject, unsigned int nestLevel);

	static void writeObjectState(std::fstream& stream, const Object* pObject, const GeoWriterOptions& options, unsigned int nestLevel);
	
	static void writeStringLine(std::fstream& stream, unsigned int nestLevel, const std::string& stringVal);
	
	static std::string vectorValsToString(const Vector& vec);
	static std::string pointValsToString(const Point& point);
	static std::string uvValsToString(const UV& uv);

	enum ObjectType
	{
		eObjTypeNone,
		eObjTypeSphere,
		eObjTypeMesh,
		eObjTypeGroup
	};

	// USD has some criteria for valid names, i.e. that they're unique (within scope), and can't start with numbers...
	std::string generateValidObjectName(const std::string& originalName, ObjectTypeNameCounts& nameCounts, ObjectType objectType);

	// only generates the raw name (concats type plus number), but increments the object type count...
	static std::string buildName(ObjectTypeNameCounts& nameCounts, ObjectType objectType);

	static std::string prependObjectTypeName(const std::string& originalName, ObjectType objectType);

protected:
	std::set<std::string>		m_aUsedNames;
};

} // namespace Imagine

#endif // GEO_WRITER_USDA_H
