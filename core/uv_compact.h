/*
 Imagine
 Copyright 2014-2016 Peter Pearson.

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

#ifndef UV_COMPACT_H
#define UV_COMPACT_H

#include "uv.h"

//#include <half.h>

namespace Imagine
{

/*
// only for reference - is close to useless with UDIM use, as the precision is really bad
class UVCompactHalf
{
public:
	inline UVCompactHalf()
	{
	}

	explicit UVCompactHalf(const UV& rhs)
	{
		u = (half)rhs.u;
		v = (half)rhs.v;
	}

	UV toUV() const
	{
		UV fullUV((float)u, (float)v);
		return fullUV;
	}

public:
	half	u;
	half	v;
};
*/


// can store UVs in range 0.0 -> 10.0 on U, and 0.0 -> 20.0 on V, corresponding to
// UDIMs from 1001 to 1201
class UVCompactPacked
{
public:
	inline UVCompactPacked() : m_packed(0)
	{
	}

	explicit UVCompactPacked(const UV& rhs) : m_packed(0)
	{
		float scaledValueU = rhs.u * 3276.8f; // scale by 3276.8 to use 15 bytes 0 -> 32767
		float scaledValueV = rhs.v * 3276.8f; // scale by 3276.8 to use 16 bytes 0 -> 65535

		unsigned short uValuePacked = (unsigned short)(std::floor(scaledValueU));
		unsigned short vValuePacked = (unsigned short)(std::floor(scaledValueV));

		m_packed |= (uValuePacked << 17) & kUValueMask;
		m_packed |= vValuePacked & kVValueMask;
	}

	UV toUV() const
	{
		unsigned short uValuePacked = (m_packed & kUValueMask) >> 17;
		unsigned short vValuePacked = (m_packed & kVValueMask);

		// using different multipliers (constructor) / dividers (here) gives better accuracy distribution on average...

		float finalU = (float)uValuePacked * (1.0f / 3276.7f);
		float finalV = (float)vValuePacked * (1.0f / 3276.7f);

		return UV(finalU, finalV);
	}

	enum
	{
		kUValueMask		= ((1 << 15) - 1) << 17, // 15 bits (32767.0) shifted left-most (shift left 17)
		kVValueMask		= (1 << 17) - 1 // 16 bits (65535.0), right-most bits
		// 1 bit is unused - we *could* give U more accuracy, but probably better both dimensions
		// have the same, so...
	};

protected:
	unsigned int	m_packed;
};

/*
// can store UVs in range -10.0 - 10.0, both U and V
class UVCompactPackedSigned
{
public:
	inline UVCompactPackedSigned() : m_packed(0)
	{
	}

	explicit UVCompactPackedSigned(const UV& rhs) : m_packed(0)
	{
		float absUValue = fabsf(rhs.u);
		float absVValue = fabsf(rhs.v);

		float scaledValueU = absUValue * 3276.7f;
		float scaledValueV = absVValue * 3276.7f;

		scaledValueU += 0.5f;
		scaledValueV += 0.5f;


//		float temp;
//		float testValue = modff(scaledValueU, &temp);
//		if (temp != 0.0f)
//		{
//			scaledValueU += 0.5f;
//		}

//		testValue = modff(scaledValueV, &temp);
//		if (temp != 0.0f)
//		{
//			scaledValueV += 0.5f;
//		}

		// need to use clipped 3267.0 value (without .8) due to int conversion
		unsigned short uValuePacked = (unsigned short)(std::floor(scaledValueU));
		unsigned short vValuePacked = (unsigned short)(std::floor(scaledValueV));

//		unsigned short uValuePacked = (unsigned short)(scaledValueU);
//		unsigned short vValuePacked = (unsigned short)(scaledValueV);

		if (rhs.u < 0.0f)
		{
			m_packed |= kUSignMask;
		}

		if (rhs.v < 0.0f)
		{
			m_packed |= kVSignMask;
		}

		m_packed |= (uValuePacked << 15) & kUValueMask;
		m_packed |= vValuePacked & kVValueMask;
	}

	UV toUV() const
	{
		unsigned short uValuePacked = (m_packed & kUValueMask) >> 15;
		unsigned short vValuePacked = (m_packed & kVValueMask);

		UV final;
		final.u = (float)uValuePacked * (1.0f / 3276.7f); // using 3276.8f means we don't get original whole values...
		final.v = (float)vValuePacked * (1.0f / 3276.7f);

		if (m_packed & kUSignMask)
		{
			final.u = -final.u;
		}

		if (m_packed & kVSignMask)
		{
			final.v = -final.v;
		}

		return final;
	}

	enum
	{
		kUSignMask		= 1 << 31,
		kVSignMask		= 1 << 30,

		kUValueMask		= ((1 << 15) - 1) << 15,
		kVValueMask		= (1 << 15) - 1
	};

protected:
	unsigned int	m_packed;
};
*/

} // namespace Imagine

#endif // UV_COMPACT_H
