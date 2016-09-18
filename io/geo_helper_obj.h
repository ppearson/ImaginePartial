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

#ifndef GEO_HELPER_OBJ_H
#define GEO_HELPER_OBJ_H

#include <set>
#include <vector>

#include "utils/string_helpers.h"

#include "io/geo_reader.h"

namespace Imagine
{

class TriangleGeometryInstance;
class StandardGeometryInstance;

class GeoHelperObj
{
public:
	GeoHelperObj();

	static bool readMaterialFile(const std::string& mtlPath, GeoMaterials& materials);

	struct FaceLineResults
	{
		FaceLineResults() : pointIndex(-1u), uvIndex(-1u), normalIndex(-1u)
		{
		}

		bool haveUV() const { return uvIndex != -1u; }
		bool haveNormal() const { return normalIndex != -1u; }

		unsigned int	pointIndex;
		unsigned int	uvIndex;
		unsigned int	normalIndex;
	};

	// for EditableGeometryInstances
	static void copyPointsToGeometry(std::vector<Point>& points, std::set<unsigned int>& pointIndexesRequired, EditableGeometryInstance* pGeoInstance);
	static void copyUVsToGeometry(std::vector<UV>& uvs, std::set<unsigned int>& vertexUVsRequired, EditableGeometryInstance* pGeoInstance);

	static void calculateFaceNormalsForGeometry(EditableGeometryInstance* pGeoInstance);

	// for StandardGeometryInstances
	static void copyPointItemsToGeometry(std::vector<Point>& points, std::set<unsigned int>& pointIndexesRequired,
									StandardGeometryInstance* pGeoInstance);

	static void copyUVItemsToGeometry(std::vector<UV>& uvs, std::set<unsigned int>& vertexUVsRequired, StandardGeometryInstance* pGeoInstance);
};

} // namespace Imagine

#endif // GEO_HELPER_OBJ_H
