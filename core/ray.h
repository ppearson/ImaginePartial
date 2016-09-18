/*
 Imagine
 Copyright 2011-2016 Peter Pearson.

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

#ifndef RAY_H
#define RAY_H

#include <limits>

#include "point.h"
#include "normal.h"

#include "utils/hints.h"

namespace Imagine
{

// done as bitmask so we can quickly check if they match
enum RayType
{
	RAY_UNDEFINED		= 0,
	RAY_CAMERA			= 1 << 0,
	RAY_SHADOW			= 1 << 1,
	RAY_REFLECTION		= 1 << 2,
	RAY_REFRACTION		= 1 << 3,
	RAY_DIFFUSE			= 1 << 4,
	RAY_GLOSSY			= 1 << 5,

	RAY_ALL				= RAY_CAMERA | RAY_SHADOW | RAY_REFLECTION | RAY_REFRACTION | RAY_DIFFUSE | RAY_GLOSSY
};

class Ray
{
public:
	__finline Ray() : type(RAY_UNDEFINED), flags(0), importance(1.0f), width(0.0f), time(0.0f), tMin(0.0f),
		tMax(std::numeric_limits<float>::max()), pCustPayload1(NULL), custValue1(0.0f)
	{
	}

	__finline Ray(const Point& startPos, const Normal& dir, const RayType& rayType)
		: type(rayType), startPosition(startPos), direction(dir), flags(0), importance(1.0f), width(0.0f), time(0.0f),
			tMin(0.0f), tMax(std::numeric_limits<float>::max()),
			pCustPayload1(NULL), custValue1(0.0f)
	{
	}

	__finline Ray(const Point& startPos, const Normal& dir, float timet, const RayType& rayType)
		: type(rayType), startPosition(startPos), direction(dir), flags(0), importance(1.0f), width(0.0f), time(timet),
			tMin(0.0f), tMax(std::numeric_limits<float>::max()), pCustPayload1(NULL), custValue1(0.0f)
	{
	}

	__finline Ray(const Ray& ray) : type(ray.type), startPosition(ray.startPosition), direction(ray.direction), inverseDirection(ray.inverseDirection),
			diffXStartPos(ray.diffXStartPos), diffYStartPos(ray.diffYStartPos), diffXDirection(ray.diffXDirection), diffYDirection(ray.diffYDirection),
			flags(ray.flags), importance(ray.importance), width(ray.width), time(ray.time),
			tMin(ray.tMin), tMax(ray.tMax), pCustPayload1(NULL), custValue1(0.0f)
	{
	}

	enum RayFlags
	{
		RAY_HAS_DIFFERENTIALS	= 1 << 0,
		RAY_PARALLEL_DIRECTION	= 1 << 1
	};

	void setRayDifferentials(const Point& diffXOrigin, const Point& diffYOrigin, const Normal& diffXDir, const Normal& diffYDir)
	{
		diffXStartPos = diffXOrigin;
		diffYStartPos = diffYOrigin;

		diffXDirection = diffXDir;
		diffYDirection = diffYDir;

		flags |= RAY_HAS_DIFFERENTIALS;
	}

	void calculateInverseDirection()
	{
/*		inverseDirection = direction;
		if (fabsf(inverseDirection.x) > 0.000000001f)
			inverseDirection.x = 1.0f / direction.x;

		if (fabsf(inverseDirection.y) > 0.000000001f)
			inverseDirection.y = 1.0f / direction.y;

		if (fabsf(inverseDirection.z) > 0.000000001f)
			inverseDirection.z = 1.0f / direction.z;
*/
		// BVH bbox method and SSE'd BVH traversal depend on sign of inf values after div-by-0 for
		// axis-aligned rays...
		inverseDirection = Normal(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);
	}

	Point pointAt(float t) const
	{
		Point point = startPosition;
		point += direction * t;

		return point;
	}

public:
	RayType			type;

	Point			startPosition;
	Normal			direction;
	Normal			inverseDirection;

	// ray differentials
	Point			diffXStartPos;
	Point			diffYStartPos;
	Normal			diffXDirection;
	Normal			diffYDirection;

	unsigned char	flags;

	float			importance;

	// basically a roughness, 0.0f - 1.0f - camera rays are 0.0f
	float			width;

	float			time;

	float			tMin;
	float			tMax;

	void*			pCustPayload1;
	float			custValue1;
};

} // namespace Imagine

#endif // RAY_H
