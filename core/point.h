/*
 Imagine
 Copyright 2011-2015 Peter Pearson.

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

#ifndef POINT_H
#define POINT_H

#include "normal.h"
#include "vector.h"

#include "utils/maths/maths.h"

#include "utils/io/stream.h"

namespace Imagine
{

class Point
{
public:
	Point() : x(0.0f), y(0.0f), z(0.0f)
	{
	}
	Point(float X, float Y, float Z) : x(X), y(Y), z(Z)
	{
	}
	
	Point(const Point& rhs) : x(rhs.x), y(rhs.y), z(rhs.z)
	{

	}

	Point(const Vector& rhs) : x(rhs.x), y(rhs.y), z(rhs.z)
	{

	}

	inline Point& operator=(const Point& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}
	
	inline Point& operator=(const Vector& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}

	bool operator==(const Point& rhs) const
	{
		return (x == rhs.x && y == rhs.y && z == rhs.z);
	}

	inline Point& operator/=(float scale)
	{
		float inv = 1.0f / scale;
		x *= inv;
		y *= inv;
		z *= inv;
		return *this;
	}

	inline Point &operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	inline Point operator/(float scale) const
	{
		Point tmp(*this);
		tmp /= scale;
		return tmp;
	}

	inline Point operator*(float scale) const
	{
		Point tmp(*this);
		tmp *= scale;
		return tmp;
	}

	inline float operator*(const Point& rhs) const
	{
		return (x * rhs.x + y * rhs.y + z * rhs.z);
	}

	inline float operator*(const Normal& rhs) const
	{
		return (x * rhs.x + y * rhs.y + z * rhs.z);
	}

	static float distanceSquared(const Point& point1, const Point& point2)
	{
		return ((point1.x - point2.x) * (point1.x - point2.x))
				+ ((point1.y - point2.y) * (point1.y - point2.y))
				+ ((point1.z - point2.z) * (point1.z - point2.z));
	}

	static float distance(const Point& point1, const Point& point2)
	{
		return sqrtf(distanceSquared(point1, point2));
	}

	static float distanceSquaredXZ(const Point& point1, const Point& point2)
	{
		return ((point1.x - point2.x) * (point1.x - point2.x))
				+ ((point1.z - point2.z) * (point1.z - point2.z));
	}

	static float distanceXZ(const Point& point1, const Point& point2)
	{
		return sqrtf(distanceSquaredXZ(point1, point2));
	}

	inline float maxValue() const
	{
		float aX = fabsf(x);
		float aY = fabsf(y);
		float aZ = fabsf(z);

		float ret;

		if (aX > aY)
		{
			ret = aX > aZ ? aX : aZ;
		}
		else
		{
			ret = aY > aZ ? aY : aZ;
		}

		return ret;
	}
	
	Point asAbs() const
	{
		float aX = fabsf(x);
		float aY = fabsf(y);
		float aZ = fabsf(z);
		
		return Point(aX, aY, aZ);
	}

	inline Point& operator+=(const Point& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	inline Point& operator-=(const Vector& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	inline Point& operator+=(const Vector& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	inline Point& operator-=(const Point& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	inline Point& operator-=(float value)
	{
		x -= value;
		y -= value;
		z -= value;
		return *this;
	}

	inline Point operator-()
	{
		return Point(-x, -y, -z);
	}

	inline Point operator-() const
	{
		return Point(-x, -y, -z);
	}

	Point operator+(const Point& rhs) const
	{
		return Point(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Point operator+(const Normal& rhs) const
	{
		return Point(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Point operator-(const Point& rhs) const
	{
		return Point(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	static Point interpolate(const Point& point0, const Point& point1, float delta)
	{
		float interX = linearInterpolate(point0.x, point1.x, delta);
		float interY = linearInterpolate(point0.y, point1.y, delta);
		float interZ = linearInterpolate(point0.z, point1.z, delta);

		return Point(interX, interY, interZ);
	}

	Point& position()
	{
		return *this;
	}

	inline float& operator[](unsigned int i)
	{
		return (&x)[i];
	}

	inline const float& operator[](unsigned int i) const
	{
		return (&x)[i];
	}

	void load(Stream* stream, unsigned int)
	{
		stream->loadFloat(x);
		stream->loadFloat(y);
		stream->loadFloat(z);
	}

	void store(Stream* stream) const
	{
		stream->storeFloat(x);
		stream->storeFloat(y);
		stream->storeFloat(z);
	}

public:
	float x;
	float y;
	float z;
};

// used for clipping Triangles
class PointDouble
{
public:
	PointDouble() : x(0.0), y(0.0), z(0.0)
	{
	}

	PointDouble(double xVal, double yVal, double zVal) : x(xVal), y(yVal), z(zVal)
	{
	}

	PointDouble operator-(const PointDouble& rhs) const
	{
		return PointDouble(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	PointDouble operator+(const PointDouble& rhs) const
	{
		return PointDouble(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	PointDouble operator*(double scale) const
	{
		return PointDouble(x * scale, y * scale, z * scale);
	}

	inline PointDouble &operator*=(double scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	inline double& operator[](unsigned int i)
	{
		return (&x)[i];
	}

	inline const double& operator[](unsigned int i) const
	{
		return (&x)[i];
	}


public:
	double x;
	double y;
	double z;
};


} // namespace Imagine

#endif // POINT_H
