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

#ifndef ANIMATION_CURVE_H
#define ANIMATION_CURVE_H

#include <map>
#include <cstdio>		// needed for NULL
#include <stdint.h>		// needed for uintptr_t

class Stream;

enum CurveInterpolationType
{
	eLinearInterpolation,
	eQuadraticInterpolation,
	eCubicInterpolation,

	eNotAnimated
};

enum KeyInterpolationType
{
	eLinearKeyInterpolation = 1 << 1,
	eQuadraticKeyInterpolation = 1 << 2,
	eCubicKeyInterpolation = 1 << 3
};

struct AnimatedKey
{
	AnimatedKey(float val)
	{
	}

	float			value;
	unsigned char	preInterpolationType;
	unsigned char	postInterpolationType;
};

struct AnimatedKeys
{
	AnimatedKeys() : interpolationType(eLinearInterpolation)
	{
	}

	std::map<float, float>	keys;
	CurveInterpolationType	interpolationType;
};

class AnimationCurve
{
public:
	AnimationCurve();
	AnimationCurve(const AnimationCurve& rhs);

	~AnimationCurve();

	AnimationCurve& operator=(const AnimationCurve& rhs);

	bool isAnimated() const;
	void setAnimated(bool animated);

	float getValue(float time) const;
	float getValue() const;

	void setValue(float value);
	void setValue(float value, float time);

	bool isKey() const;

	CurveInterpolationType getInterpolationType() const;
	void setInterpolationType(CurveInterpolationType type);

	void setKey();
	void setKey(float time);

	void deleteKey();
	void deleteKey(float time);

	void load(Stream* stream, unsigned int version);
	void store(Stream* stream) const;

protected:
	static float linearTween(float time, float start, float end);
	static float cubicTween(float time, float start, float end);
	static float quadraticTween(float time, float start, float end);

	// for internal Tagged-pointer stuff

	static const uintptr_t kTagMask = 1;
	static const uintptr_t kPointerMask = ~kTagMask;

	inline AnimatedKeys* getPointer()
	{
		// if tag is set
		if (m_tag & kTagMask)
		{
			return reinterpret_cast<AnimatedKeys*>(m_tag & kPointerMask);
		}
		else
		{
			return NULL;
		}
	}

	inline const AnimatedKeys* getPointer() const
	{
		// if tag is set
		if (m_tag & kTagMask)
		{
			return reinterpret_cast<const AnimatedKeys*>(m_tag & kPointerMask);
		}
		else
		{
			return NULL;
		}
	}

	inline void setPointer(AnimatedKeys* pKeys)
	{
		if (pKeys)
		{
			m_pKeys = pKeys;
			m_tag |= 1;
		}
		else
		{
			m_tag = 0;
		}
	}

protected:
	//! NOTE: we use a tagged pointer type of thing here to save space
	//!       if the right-most bit is set, the masked pointer is valid and non-NULL
	//!       otherwise, m_constantValue holds the value
	union
	{
		//! this is only allocated and valid if the curve is animated
		AnimatedKeys*		m_pKeys;

		struct
		{
			//! this is the value used if the above pointer is NULL, and so the value isn't animated
			//! these need to be this way around, so that setting m_constantValue doesn't corrupt
			//! the tag bit
			unsigned int	m_padding;
			float			m_constantValue;
		};

		// full uint representation of the tag pointer union
		uintptr_t			m_tag;
	};
};

#endif // ANIMATION_CURVE_H
