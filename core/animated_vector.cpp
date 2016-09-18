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

#include "animated_vector.h"

#include "output_context.h"

namespace Imagine
{

AnimatedVector::AnimatedVector()
{
}

Vector AnimatedVector::getVector() const
{
	float time = OutputContext::instance().getFrame();

	float xVal = x.getValue(time);
	float yVal = y.getValue(time);
	float zVal = z.getValue(time);

	Vector newVector(xVal, yVal, zVal);
	return newVector;
}

Vector AnimatedVector::getVectorAt(float time) const
{
	float xVal = x.getValue(time);
	float yVal = y.getValue(time);
	float zVal = z.getValue(time);

	Vector newVector(xVal, yVal, zVal);
	return newVector;
}

void AnimatedVector::setFromVector(const Vector& vector)
{
	float time = OutputContext::instance().getFrame();

	x.setValue(vector.x, time);
	y.setValue(vector.y, time);
	z.setValue(vector.z, time);
}

void AnimatedVector::setFromVectorAt(const Vector& vector, float time)
{
	x.setValue(vector.x, time);
	y.setValue(vector.y, time);
	z.setValue(vector.z, time);
}

void AnimatedVector::add(const Vector& vector)
{
	float time = OutputContext::instance().getFrame();

	float curX = x.getValue(time);
	float curY = y.getValue(time);
	float curZ = z.getValue(time);

	x.setValue(curX + vector.x, time);
	y.setValue(curY + vector.y, time);
	z.setValue(curZ + vector.z, time);
}

void AnimatedVector::addValues(float xVal, float yVal, float zVal)
{
	float time = OutputContext::instance().getFrame();

	float curX = x.getValue(time);
	float curY = y.getValue(time);
	float curZ = z.getValue(time);

	x.setValue(curX + xVal, time);
	y.setValue(curY + yVal, time);
	z.setValue(curZ + zVal, time);
}

bool AnimatedVector::isAnimated() const
{
	// for the moment, all curves have to be animated together
	return x.isAnimated();
}

void AnimatedVector::setAllAnimated(bool animated)
{
	x.setAnimated(animated);
	y.setAnimated(animated);
	z.setAnimated(animated);
}

bool AnimatedVector::isKeyed() const
{
	// for the moment, all curves have to be keyed together
	return x.isKey();
}

void AnimatedVector::setKey()
{
	float time = OutputContext::instance().getFrame();

	x.setKey(time);
	y.setKey(time);
	z.setKey(time);
}

void AnimatedVector::setKey(float time)
{
	x.setKey(time);
	y.setKey(time);
	z.setKey(time);
}

void AnimatedVector::deleteKey()
{
	float time = OutputContext::instance().getFrame();

	x.deleteKey(time);
	y.deleteKey(time);
	z.deleteKey(time);
}

CurveInterpolationType AnimatedVector::getInterpolationType() const
{
	// currently, they're all the same
	return x.getInterpolationType();
}

void AnimatedVector::setInterpolationType(CurveInterpolationType type)
{
	x.setInterpolationType(type);
	y.setInterpolationType(type);
	z.setInterpolationType(type);
}

void AnimatedVector::load(Stream* stream, unsigned int version)
{
	x.load(stream, version);
	y.load(stream, version);
	z.load(stream, version);
}

void AnimatedVector::store(Stream* stream) const
{
	x.store(stream);
	y.store(stream);
	z.store(stream);
}

void AnimatedVector::loadNonAnimated(Stream* stream, unsigned int version)
{
	float xVal;
	float yVal;
	float zVal;

	stream->loadFloat(xVal);
	stream->loadFloat(yVal);
	stream->loadFloat(zVal);

	x.setValue(xVal);
	y.setValue(yVal);
	z.setValue(zVal);
}

void AnimatedVector::storeNonAnimated(Stream* stream) const
{
	float xVal = x.getValue();
	float yVal = y.getValue();
	float zVal = z.getValue();

	stream->storeFloat(xVal);
	stream->storeFloat(yVal);
	stream->storeFloat(zVal);
}

bool AnimatedVector::isNull() const
{
	// if we're animated, we're not zero, as keys have been set
	if (isAnimated())
		return false;

	// get constant values, and check if they're 0.0f

	bool constantIsZero = (x.getValue() == 0.0f && y.getValue() == 0.0f && z.getValue() == 0.0f);
	return constantIsZero;
}

} // namespace Imagine
