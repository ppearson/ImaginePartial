/*
 Imagine
 Copyright 2012-2014 Peter Pearson.

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

#ifndef COLOUR3B_H
#define COLOUR3B_H

class Colour3b
{
public:
	Colour3b() : r(0), g(0), b(0)
	{
	}

	Colour3b(unsigned char rV, unsigned char gV, unsigned char bV) :
		r(rV), g(gV), b(bV)
	{
	}

	Colour3b& operator*=(float scale)
	{
		r *= scale;
		g *= scale;
		b *= scale;

		return *this;
	}

	const unsigned char* getStartPointer() const
	{
		return &r;
	}

	Colour3b operator*(float scale) const
	{
		return Colour3b(scale * r, scale * g, scale * b);
	}

	Colour3b& operator*=(Colour3b rhs)
	{
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;

		return *this;
	}

	Colour3b& operator+=(const Colour3b& rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;

		return *this;
	}

	Colour3b operator+(const Colour3b& rhs) const
	{
		return Colour3b(r + rhs.r, g + rhs.g, b + rhs.b);
	}

	unsigned char brightness() const
	{
		// TODO: this isn't great...
		const float invf = 1.0f / 255.0f;

		float fR = (float)r * invf;
		float fG = (float)g * invf;
		float fB = (float)b * invf;

		float temp = (0.2126f * fR) + (0.7152f * fG) + (0.0722f * fB);
		return (unsigned char)(temp * 255.0f);
	}

	unsigned char average() const
	{
		return (unsigned char)((unsigned short)(r + g + b) * 0.333333f);
	}

	bool black() const
	{
		return r == 0 && g == 0 && b == 0;
	}

public:
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
};


#endif // COLOUR3B_H
