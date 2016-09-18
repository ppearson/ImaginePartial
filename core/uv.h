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

#ifndef UV_H
#define UV_H

#include "utils/io/stream.h"

namespace Imagine
{

struct UV
{
	UV()
	{
	}

	UV(float uVal, float vVal) : u(uVal), v(vVal)
	{
	}

	inline UV& operator+=(const UV& rhs)
	{
		u += rhs.u;
		v += rhs.v;
		return *this;
	}

	UV operator+(const UV& rhs) const
	{
		UV tmp(*this);
		tmp.u += rhs.u;
		tmp.v += rhs.v;
		return tmp;
	}

	UV operator-(const UV& rhs) const
	{
		UV tmp(*this);
		tmp.u -= rhs.u;
		tmp.v -= rhs.v;
		return tmp;
	}

	inline UV &operator*=(float scale)
	{
		u *= scale;
		v *= scale;
		return *this;
	}

	inline UV operator*(float scale) const
	{
		UV tmp(*this);
		tmp *= scale;
		return tmp;
	}

	inline UV &operator/=(float scale)
	{
		u /= scale;
		v /= scale;
		return *this;
	}

	inline UV operator/(float scale) const
	{
		UV tmp(*this);
		tmp /= scale;
		return tmp;
	}

	void load(Stream* stream, unsigned int)
	{
		stream->loadFloat(u);
		stream->loadFloat(v);
	}

	void store(Stream* stream) const
	{
		stream->storeFloat(u);
		stream->storeFloat(v);
	}

	float u;
	float v;
};

} // namespace Imagine

#endif // UV_H
