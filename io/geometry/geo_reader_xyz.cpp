/*
 Imagine
 Copyright 2017 Peter Pearson.

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

#include "geo_reader_xyz.h"

#include <algorithm>
#include <cstdio>

#include "global_context.h"

#include "objects/compound_static_spheres.h"

namespace Imagine
{

GeoReaderXYZ::GeoReaderXYZ()
{
	
}

GeoReaderXYZ::~GeoReaderXYZ()
{
	
}

bool GeoReaderXYZ::readFile(const std::string& path, const GeoReaderOptions& options)
{
	m_readOptions = options;
	
	std::fstream fileStream(path.c_str(), std::ios::in);
	
	char buf[256];
	memset(buf, 0, 256);

	std::string line;
	line.resize(256);
	
	std::vector<Point> aPointPositions;

	aPointPositions.reserve(400000);

	while (fileStream.getline(buf, 256))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;
		
		float x;
		float y;
		float z;
		
		sscanf(buf, "%f %f %f", &x, &y, &z);

		Point vertex(x, y, z);

		aPointPositions.emplace_back(vertex);
	}
	
	fileStream.close();
	
	CompoundStaticSpheres* pCompoundSpheres = new CompoundStaticSpheres();
	if (!pCompoundSpheres)
		return false;
	
	pCompoundSpheres->buildFromPositions(aPointPositions, options.pointSize);

	pCompoundSpheres->setName("CompoundSpheresShape");

	m_newObject = pCompoundSpheres;

	postProcess();
	
	return true;
}


} // namespace Imagine

namespace
{
	Imagine::GeoReader* createGeoReaderXyz()
	{
		return new Imagine::GeoReaderXYZ();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerGeoReader("xyz", createGeoReaderXyz);
}
