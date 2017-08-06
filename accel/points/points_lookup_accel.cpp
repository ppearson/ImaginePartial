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

#include "points_lookup_accel.h"

#include <fstream>
#include <string.h>

namespace Imagine
{

// TODO: some of this stuff should be moved out of here at some point

PointsLookupAccel::PointsLookupAccel()
{

}

bool PointsLookupAccel::readPointCloudFile(const std::string& filePath, std::vector<PointColour3f>& points)
{
	std::fstream fileStream(filePath.c_str(), std::ios::in);

	char buf[256];
	memset(buf, 0, 256);

	points.reserve(100000);

	while (fileStream.getline(buf, 256))
	{
		if (buf[0] == 0)
			continue;

		PointColour3f newPoint;

		unsigned char colR;
		unsigned char colG;
		unsigned char colB;

		sscanf(buf, "%f %f %f %hhu %hhu %hhu", &newPoint.position.x, &newPoint.position.y, &newPoint.position.z,
								&colR, &colG, &colB);


		newPoint.colour.r = (float)colR / 255.0f;
		newPoint.colour.g = (float)colG / 255.0f;
		newPoint.colour.b = (float)colB / 255.0f;

		points.push_back(newPoint);
	}


	fileStream.close();

	return true;
}

} // namespace Imagine
