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

#include "animation_curve.h"

#include <cstddef>
#include <assert.h>
#include <math.h>

#include "output_context.h"

#include "utils/io/stream.h"
#include "utils/storage_helpers.h"

namespace Imagine
{

AnimationCurve::AnimationCurve() : m_tag(0)
{
}

AnimationCurve::AnimationCurve(const AnimationCurve& rhs) : m_tag(0)
{
	const AnimatedKeys* pRHSKeys = rhs.getPointer();
	if (pRHSKeys)
	{
		AnimatedKeys* pNewKeys = new AnimatedKeys();
		setPointer(pNewKeys);
		*pNewKeys = *pRHSKeys;
	}
	else
	{
		// not animated, so just copy over tag so the constant value float gets updated
		m_tag = rhs.m_tag;
	}
}

AnimationCurve::~AnimationCurve()
{
	AnimatedKeys* pKeys = getPointer();
	if (pKeys)
	{
		delete pKeys;
		setPointer(NULL);
	}
}

AnimationCurve& AnimationCurve::operator=(const AnimationCurve& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	const AnimatedKeys* pRHSKeys = rhs.getPointer();
	AnimatedKeys* pKeys = getPointer();

	if (pRHSKeys)
	{
		// if we're already animated, just copy them over
		if (pKeys)
		{
			*pKeys = *pRHSKeys;
		}
		else
		{
			// otherwise, allocate some more and set tagged pointer
			AnimatedKeys* pNewKeys = new AnimatedKeys();
			setPointer(pNewKeys);
			*pNewKeys = *pRHSKeys;
		}
	}
	else
	{
		// we don't need to be animated any more, so if we are, delete pointer
		if (pKeys)
		{
			delete pKeys;
		}

		// just copy over tag so the constant value float gets updated
		m_tag = rhs.m_tag;
	}

	return *this;
}

float AnimationCurve::getValue(float time) const
{
	const AnimatedKeys* pKeys = getPointer();

	if (!pKeys)
		return m_constantValue;

	const AnimatedKeys& ak = *pKeys;

	size_t keyCount = ak.keys.size();

	if (keyCount == 1) // it's constant, so return that
		return ak.keys.begin()->second;

	float value = 0.0f;

	// see if it exists
	std::map<float, float>::const_iterator itFind = ak.keys.find(time);
	if (itFind != ak.keys.end())
	{
		value = itFind->second;
		return value;
	}

	std::map<float, float>::const_iterator findLower = ak.keys.lower_bound(time);
	std::map<float, float>::const_iterator findUpper = ak.keys.upper_bound(time);

	if (findLower == ak.keys.end())
	{
		// after last frame
		std::map<float, float>::const_iterator itLastKey = --ak.keys.rbegin().base();
		value = itLastKey->second;

		return value;
	}

	if (findUpper == ak.keys.begin())
	{
		// before first frame
		value = ak.keys.begin()->second;
		return value;
	}

	if (findUpper != ak.keys.end())
	{
		std::map<float, float>::const_iterator itPrevKey = findUpper;
		--itPrevKey;

		float lowerTime = (*itPrevKey).first;
		float lowerValue = (*itPrevKey).second;

		float upperTime = (*findUpper).first;
		float upperValue = (*findUpper).second;

		float timeRatio = (time - lowerTime) / (upperTime - lowerTime);

		switch (ak.interpolationType)
		{
			default:
			case eLinearInterpolation:
				value = linearTween(timeRatio, lowerValue, upperValue);
				break;
			case eCubicInterpolation:
				value = cubicTween(timeRatio, lowerValue, upperValue);
				break;
			case eQuadraticInterpolation:
				value = quadraticTween(timeRatio, lowerValue, upperValue);
				break;
		}

		return value;
	}
	else
	{
		assert(false);
	}

	return 0.0f;
}

float AnimationCurve::getValue() const
{
	float time = OutputContext::instance().getFrame();
	return getValue(time);
}

bool AnimationCurve::isAnimated() const
{
	return (getPointer() != NULL);
}

void AnimationCurve::setAnimated(bool animated)
{
	AnimatedKeys* pKeys = getPointer();

	if ((animated && pKeys) || (!animated && !pKeys))
		return;

	if (animated)
	{
		float currentConstantValue = m_constantValue;

		// allocate the AnimatedKeys struct
		AnimatedKeys* pNewKeys = new AnimatedKeys();
		setPointer(pNewKeys);
		// set a key from the constant value
		setValue(currentConstantValue);
	}
	else
	{
		float currentCurveValue = getValue();

		delete pKeys;

		m_tag = 0;

		m_constantValue = currentCurveValue;
	}
}

void AnimationCurve::setValue(float value)
{
	float time = OutputContext::instance().getFrame();
	setValue(value, time);
}

void AnimationCurve::setValue(float value, float time)
{
	AnimatedKeys* pKeys = getPointer();

	if (!pKeys)
	{
		m_constantValue = value;
		return;
	}

	pKeys->keys[time] = value;
}

bool AnimationCurve::isKey() const
{
	const AnimatedKeys* pKeys = getPointer();

	if (!pKeys)
		return false;

	return pKeys->keys.count(OutputContext::instance().getFrame()) > 0;
}

CurveInterpolationType AnimationCurve::getInterpolationType() const
{
	const AnimatedKeys* pKeys = getPointer();

	if (!pKeys)
	{
		return eNotAnimated;
	}
	else
	{
		return pKeys->interpolationType;
	}
}

void AnimationCurve::setInterpolationType(CurveInterpolationType type)
{
	AnimatedKeys* pKeys = getPointer();

	if (pKeys)
	{
		pKeys->interpolationType = type;
	}
}

void AnimationCurve::setKey()
{
	float time = OutputContext::instance().getFrame();
	float currentValue = getValue(time);
	setValue(currentValue, time);
}

void AnimationCurve::setKey(float time)
{
	float currentValue = getValue(time);
	setValue(currentValue, time);
}

void AnimationCurve::deleteKey()
{
	float time = OutputContext::instance().getFrame();
	deleteKey(time);
}

void AnimationCurve::deleteKey(float time)
{
	AnimatedKeys* pKeys = getPointer();

	if (pKeys)
	{
		if (pKeys->keys.size() == 1)
		{
			// if it's the last one, just set it to not be animated
			setAnimated(false);
			return;
		}
		pKeys->keys.erase(time);
	}
}

void AnimationCurve::load(Stream* stream, unsigned int version)
{
	bool animated;
	stream->loadBool(animated);

	AnimatedKeys* pKeys = getPointer();

	// remove any current keys
	if (pKeys)
	{
		delete pKeys;
	}

	m_tag = 0;

	CurveInterpolationType tempInterpolation;

	if (version <= 13)
	{
		tempInterpolation = (CurveInterpolationType)stream->loadEnum();
	}

	if (!animated)
	{
		stream->loadFloat(m_constantValue);
	}
	else
	{
		AnimatedKeys* pNewKeys = new AnimatedKeys();
		setPointer(pNewKeys);

		if (version > 13)
		{
			pNewKeys->interpolationType = (CurveInterpolationType)stream->loadEnum();
		}

		unsigned int numKeyFrames = 0;
		if (version <= 8)
			stream->loadUIntFromUChar(numKeyFrames);
		else
			stream->loadUInt(numKeyFrames);

		for (unsigned int i = 0; i < numKeyFrames; i++)
		{
			float time;
			float value;
			stream->loadFloat(time);
			stream->loadFloat(value);

			pNewKeys->keys[time] = value;
		}
	}
}

void AnimationCurve::store(Stream* stream) const
{
	const AnimatedKeys* pKeys = getPointer();

	bool isAnimated = (pKeys != NULL);
	stream->storeBool(isAnimated);

	if (!isAnimated)
	{
		stream->storeFloat(m_constantValue);
	}
	else
	{
		stream->storeEnum(pKeys->interpolationType);

		unsigned int numKeyFrames = pKeys->keys.size();
		stream->storeUInt(numKeyFrames);

		std::map<float, float>::const_iterator itKeys = pKeys->keys.begin();
		for (; itKeys != pKeys->keys.end(); ++itKeys)
		{
			float time = itKeys->first;
			float value = itKeys->second;

			stream->storeFloat(time);
			stream->storeFloat(value);
		}
	}
}

float AnimationCurve::linearTween(float time, float start, float end)
{
	if (time > 1.0f)
		return end;

	return time * end + (1.0f - time) * start;
}

float AnimationCurve::cubicTween(float time, float start, float end)
{
	float b = start;
	float c = end - start;
	float d = 1.0f;

	if (time > 1.0f)
		return end;
	else if (time < 0.0f)
		return start;

	if ((time /= d / 2.0f) < 1.0f)
		return c / 2.0f * powf(time, 3.0f) + b;

	return c / 2.0f * (powf(time - 2.0f, 3.0f) + 2.0f) + b;
}

float AnimationCurve::quadraticTween(float time, float start, float end)
{
	float b = start;
	float c = end - start;
	float d = 1.0f;

	if ((time /= d / 2.0f) < 1.0f)
		return c / 2.0f * time * time + b;

	--time;
	return -c / 2.0f * ((time) * (time - 2.0f) - 1.0f) + b;
}

} // namespace Imagine
