/*
 Imagine
 Copyright 2011-2013 Peter Pearson.

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

#ifndef VECTOR2_H
#define VECTOR2_H

#include <math.h>

namespace Imagine
{

class Vector2
{
public:
	Vector2(float X = 0.0f, float Y = 0.0f) : x(X), y(Y) { }

	static float cross(const Vector2& point1, const Vector2& point2)
	{
		return ((point1.x * point2.y) - (point1.y * point2.x));
	}

	float dot(const Vector2& rhs) const
	{
		return x * rhs.x + y * rhs.y;
	}

	Vector2 getUnit() const
	{
		float len = length();
		return Vector2(x / len, y / len);
	}

	float length() const
	{
		float len = (x * x) + (y * y);
		return sqrtf(len);
	}

	static float angleBetween(const Vector2& point1, const Vector2& point2)
	{
		float distance = point1.length() * point1.length();

		if (distance == 0.0f)
			return 0.0f;

		float angle = point1.dot(point2) / distance;

		if (angle > 1.0f)
			angle = 1.0f;
		else if (angle < -1.0f)
			angle = -1.0f;

		return (float)acos(angle);
	}


	Vector2& operator+=(const Vector2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	Vector2& operator-=(const Vector2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	Vector2 operator-()
	{
		return Vector2(-x, -y);
	}

	Vector2 operator+(const Vector2& rhs) const
	{
		Vector2 tmp(*this);
		tmp += rhs;
		return tmp;
	}

	Vector2 operator-(const Vector2& rhs) const
	{
		Vector2 tmp(*this);
		tmp -= rhs;
		return tmp;
	}

	Vector2& operator*=(float scale) { x *= scale, y *= scale; return *this; }
	Vector2 operator*(float scale) const { Vector2 tmp(*this); tmp *= scale; return tmp; }
	Vector2& operator/=(float scale) { x /= scale, y /= scale; return *this; }
	Vector2 operator/(float scale) const { Vector2 tmp(*this); tmp /= scale; return tmp; }

	float x;
	float y;
};

} // namespace Imagine

#endif // VECTOR2_H
