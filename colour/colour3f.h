/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#ifndef COLOUR3F_H
#define COLOUR3F_H

#include "utils/io/stream.h"

#include "utils/maths/maths.h"

namespace Imagine
{

class Colour3f
{
public:
	Colour3f() : r(0.0f), g(0.0f), b(0.0f)
	{
	}

	Colour3f(float colour) : r(colour), g(colour), b(colour)
	{
	}

	Colour3f(float red, float green, float blue) : r(red), g(green), b(blue)
	{
	}

	bool operator==(const Colour3f& rhs) const
	{
		return (rhs.r == r && rhs.g == g && rhs.b == b);
	}

	bool operator!=(const Colour3f& rhs) const
	{
		return (rhs.r != r && rhs.g != g && rhs.b != b);
	}

	Colour3f& operator/=(float scale)
	{
		float inv = 1.0f / scale;
		r *= inv;
		g *= inv;
		b *= inv;

		return *this;
	}

	Colour3f operator*(float scale) const
	{
		return Colour3f(scale * r, scale * g, scale * b);
	}

	Colour3f operator/(float scale) const
	{
		float inv = 1.0f / scale;
		return Colour3f(r * inv, g * inv, b * inv);
	}

	Colour3f operator-(const Colour3f& rhs) const
	{
		return Colour3f(r - rhs.r, g - rhs.g, b - rhs.b);
	}

	Colour3f operator+(const Colour3f& rhs) const
	{
		return Colour3f(r + rhs.r, g + rhs.g, b + rhs.b);
	}

	Colour3f operator*(const Colour3f& rhs) const
	{
		return Colour3f(r * rhs.r, g * rhs.g, b * rhs.b);
	}

	Colour3f& operator*=(float scale)
	{
		r *= scale;
		g *= scale;
		b *= scale;

		return *this;
	}

	Colour3f& operator+=(const Colour3f& rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;

		return *this;
	}

	Colour3f& operator*=(const Colour3f& rhs)
	{
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;

		return *this;
	}

	void load(Stream* stream, unsigned int)
	{
		stream->loadFloat(r);
		stream->loadFloat(g);
		stream->loadFloat(b);
	}

	void store(Stream* stream) const
	{
		stream->storeFloat(r);
		stream->storeFloat(g);
		stream->storeFloat(b);
	}

	float brightness() const
	{
		return (0.2126f * r) + (0.7152f * g) + (0.0722f * b);
	}

	float average() const
	{
		return (r + g + b) * 0.3333333f;
	}

	bool black() const
	{
		return r == 0.0f && g == 0.0f && b == 0.0f;
	}

	float max() const
	{
		if (r > g)
		{
			if (b > r)
				return b;
			else
				return r;
		}
		else
		{
			if (b > g)
				return b;
			else
				return g;
		}
	}

	inline void clamp(float max = 1.0f)
	{
		r = std::min(r, max);
		g = std::min(g, max);
		b = std::min(b, max);
	}

	inline void fullClamp(float max = 1.0f)
	{
		r = std::max(0.0f, r);
		g = std::max(0.0f, g);
		b = std::max(0.0f, b);

		r = std::min(r, max);
		g = std::min(g, max);
		b = std::min(b, max);
	}

	void exp()
	{
		r = std::exp(r);
		g = std::exp(g);
		b = std::exp(b);
	}

	Colour3f pow(float value) const
	{
		float r2 = expf(logf(std::max(0.000000001f, r)) * value);
		float g2 = expf(logf(std::max(0.000000001f, g)) * value);
		float b2 = expf(logf(std::max(0.000000001f, b)) * value);

		return Colour3f(r2, g2, b2);
	}

	///

public:
	float r;
	float g;
	float b;
};

} // namespace Imagine

#endif // COLOUR3F_H
