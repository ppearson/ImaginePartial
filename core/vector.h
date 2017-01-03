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

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#include "normal.h"

#include "utils/io/stream.h"

namespace Imagine
{

class Point;

class Vector
{
public:
	Vector() : x(0.0f), y(0.0f), z(0.0f)
	{
	}
	Vector(float X, float Y, float Z) : x(X), y(Y), z(Z)
	{
	}

	Vector(const Point& rhs);
	Vector(const Normal& rhs);

	inline float length() const
	{
		return sqrtf((x * x) + (y * y) + (z * z));
	}

	inline float lengthSquared() const
	{
		return (x * x) + (y * y) + (z * z);
	}

	inline float normalise()
	{
		float len = length();
		float inv = 1.0f / len;
		x *= inv;
		y *= inv;
		z *= inv;

		return len;
	}

	inline static Normal cross(const Vector& point1, const Vector& point2)
	{
		return Normal((point1.y * point2.z) - (point1.z * point2.y),
				-((point2.z * point1.x) - (point2.x * point1.z)),
				(point1.x * point2.y) - (point1.y * point2.x));
	}

	inline static float dot(const Vector& point1, const Vector& point2)
	{
		return (point1.x * point2.x + point1.y * point2.y + point1.z * point2.z);
	}

	bool operator==(const Vector& rhs) const
	{
		return (rhs.x == x && rhs.y == y && rhs.z == z);
	}

	inline Vector operator+(const Vector& rhs) const
	{
		return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	inline Vector& operator+=(const Vector& rhs)
	{
		x += rhs.x; y += rhs.y; z += rhs.z;
		return *this;
	}

	inline Vector operator-(const Vector& rhs) const
	{
		return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	inline Vector& operator-=(const Vector& rhs)
	{
		x -= rhs.x; y -= rhs.y; z -= rhs.z;
		return *this;
	}

	inline Vector& operator*=(const Vector& scale)
	{
		x *= scale.x;
		y *= scale.y;
		z *= scale.z;
		return *this;
	}

	inline Vector operator*(float scale) const
	{
		return Vector(x * scale, y * scale, z * scale);
	}

	inline Vector &operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	inline Vector operator/(float scale) const
	{
		float inv = 1.0f / scale;
		return Vector(x * inv, y * inv, z * inv);
	}

	inline Vector &operator/=(float scale)
	{
		float inv = 1.0f / scale;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}

	inline Vector &operator/=(Vector scale)
	{
		x /= scale.x;
		y /= scale.y;
		z /= scale.z;
		return *this;
	}

	// doesn't change current Vector...
	inline Vector operator- () const
	{
		return (Vector(-x, -y, -z));
	}

	static Vector interpolate(const Vector& v1, const Vector& v2, float t)
	{
		float x1 = linearInterpolate(v1.x, v2.x, t);
		float y1 = linearInterpolate(v1.y, v2.y, t);
		float z1 = linearInterpolate(v1.z, v2.z, t);
		return Vector(x1, y1, z1);
	}

	unsigned int biggestAxis() const
	{
		float aX = fabsf(x);
		float aY = fabsf(y);
		float aZ = fabsf(z);

		if (aX > aY)
		{
			if (aZ > aX)
				return 2;
			else
				return 0;
		}
		else
		{
			if (aZ > aY)
				return 2;
			else
				return 1;
		}
	}

	// used for OpenGL viewer
	unsigned int secondBiggestAxis() const
	{
		float aX = fabsf(x);
		float aY = fabsf(y);
		float aZ = fabsf(z);

		if (aX > aY && aX > aZ)
		{
			if (aZ > aY)
				return 2;
			else
				return 1;
		}
		else if (aY > aX && aY > aZ)
		{
			if (aX > aZ)
				return 0;
			else
				return 2;
		}
		else // aZ > aX && aZ > aY ??
		{
			if (aX > aY)
				return 0;
			else
				return 1;
		}
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

	bool isNull() const
	{
		return (x == 0.0f && y == 0.0f && z == 0.0f);
	}

	bool isAllValue(float value) const
	{
		return (x == value && y == value && z == value);
	}

public:
	float x;
	float y;
	float z;
};

} // namespace Imagine

#endif // VECTOR_H
