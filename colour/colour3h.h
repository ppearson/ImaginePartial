/*
 Imagine
 Copyright 2014 Peter Pearson.

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

#ifndef COLOUR3H_H
#define COLOUR3H_H

#include <half.h>

#include "utils/maths/maths.h"

// we can't really use a templated class for this, as special-casing is needed
// for some of the functions in order to make sure we stay within half precision/range bounds...

class Colour3h
{
public:
	Colour3h() : r(0.0f), g(0.0f), b(0.0f)
	{
	}

	Colour3h(half rV, half gV, half bV) : r(rV), g(gV), b(bV)
	{
	}

	Colour3h& operator*=(float scale)
	{
		r *= scale;
		g *= scale;
		b *= scale;

		return *this;
	}

	const half* getStartPointer() const
	{
		return &r;
	}

	Colour3h operator*(float scale) const
	{
		return Colour3h(scale * r, scale * g, scale * b);
	}

	Colour3h& operator*=(Colour3h rhs)
	{
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;

		return *this;
	}

	Colour3h& operator+=(const Colour3h& rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;

		return *this;
	}

	Colour3h operator+(const Colour3h& rhs) const
	{
		return Colour3h(r + rhs.r, g + rhs.g, b + rhs.b);
	}

	half brightness() const
	{
		// because of limited precision/range, we need to do things this way...
		half red = r * 0.2126f;
		half green = g * 0.7152f;
		half blue = b * 0.0722f;

		half total = red + green + blue;
		return total;
	}

	half average() const
	{
		// because of limited precision/range, we need to do things this way...
		half red = r * 0.333333f;
		half green = g * 0.333333f;
		half blue = b * 0.333333f;

		half total = red + green + blue;
		return total;
	}

	bool black() const
	{
		return r == 0.0f && g == 0.0f && b == 0.0f;
	}

	inline void clamp(half max = 1.0f)
	{
		r = std::min(r, max);
		g = std::min(g, max);
		b = std::min(b, max);
	}

	inline void fullClamp(half max = 1.0f)
	{
		r = std::max((half)0.0f, r);
		g = std::max((half)0.0f, g);
		b = std::max((half)0.0f, b);

		r = std::min(r, max);
		g = std::min(g, max);
		b = std::min(b, max);
	}

public:
	half	r;
	half	g;
	half	b;
};

#endif // COLOUR3H_H
