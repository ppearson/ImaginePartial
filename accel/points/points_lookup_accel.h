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

#ifndef POINTS_LOOKUP_ACCEL_H
#define POINTS_LOOKUP_ACCEL_H

#include <vector>

#include "core/point.h"
#include "colour/colour3f.h"

namespace Imagine
{

class PointsLookupAccel
{
public:
	PointsLookupAccel();
	virtual ~PointsLookupAccel()
	{

	}

	struct PointColour3f
	{
		PointColour3f()
		{
		}

		PointColour3f(const Point& pos, const Colour3f& col) : position(pos), colour(col)
		{
		}

		Point		position;
		Colour3f	colour;
	};

	enum PointCloudFileType
	{
		ePCFXYZrgb
	};

	static bool readPointCloudFile(const std::string& filePath, std::vector<PointColour3f>& points);


	virtual void preRender()
	{

	}

	virtual void build(const std::vector<PointColour3f>& points, float lookupSize) = 0;

	virtual Colour3f lookupColour(const Point& worldSpacePosition, float filterRadius) const = 0;


	void setMissingColour(const Colour3f& missingColour)
	{
		m_missingColour = missingColour;
	}

protected:
	Colour3f		m_missingColour;
};

} // namespace Imagine

#endif // POINTS_LOOKUP_ACCEL_H

