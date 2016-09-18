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

#ifndef COLOUR4F_H
#define COLOUR4F_H

#include "utils/io/stream.h"

#include "colour3f.h"

namespace Imagine
{

class Colour4f
{
public:
	Colour4f() : r(0.0f), g(0.0f), b(0.0f), a(0.0f)
	{
	}

	Colour4f(float red, float green, float blue, float alpha = 0.0f) : r(red), g(green), b(blue), a(alpha)
	{
	}

	Colour4f(const Colour3f& rhs) : r(rhs.r), g(rhs.g), b(rhs.b), a(0.0f)
	{
	}

	void merge(const Colour4f& rhs, float ratio = 0.5f)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		a += rhs.a;

		*this *= ratio;
	}

	Colour4f& operator/=(float scale)
	{
		float inv = 1.0f / scale;
		r *= inv;
		g *= inv;
		b *= inv;
		a *= inv;

		return *this;
	}

	Colour4f& operator*=(float scale)
	{
		r *= scale;
		g *= scale;
		b *= scale;
		a *= scale;

		return *this;
	}

	Colour4f operator*(float scale) const
	{
		return Colour4f(scale * r, scale * g, scale * b, scale * a);
	}

	Colour4f& operator*=(const Colour4f& rhs)
	{
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;
		a *= rhs.a;

		return *this;
	}

	Colour4f& operator*=(const Colour3f& rhs)
	{
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;

		return *this;
	}

	Colour4f& operator+=(const Colour4f& rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		a += rhs.a;

		return *this;
	}

	Colour4f& operator+=(const Colour3f& rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;

		return *this;
	}

	Colour4f operator-(const Colour4f& rhs) const
	{
		return Colour4f(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a);
	}

	Colour4f operator+(const Colour4f& rhs) const
	{
		return Colour4f(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
	}

	Colour4f operator/(float scale) const
	{
		float inv = 1.0f / scale;
		return Colour4f(r * inv, g * inv, b * inv, a * inv);
	}

	Colour4f operator*(const Colour4f& rhs) const
	{
		return Colour4f(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
	}

	float brightness() const
	{
		return (0.2126f * r) + (0.7152f * g) + (0.0722f * b);
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

	const float& operator[](unsigned int i) const
	{
		return (&r)[i];
	}

	float& operator[](unsigned int i)
	{
		return (&r)[i];
	}

	void load(Stream* stream, unsigned int)
	{
		stream->loadFloat(r);
		stream->loadFloat(g);
		stream->loadFloat(b);
		stream->loadFloat(a);
	}

	void store(Stream* stream) const
	{
		stream->storeFloat(r);
		stream->storeFloat(g);
		stream->storeFloat(b);
		stream->storeFloat(a);
	}

public:
	float r;
	float g;
	float b;
	float a;
};

} // namespace Imagine

#endif // COLOUR4F_H
