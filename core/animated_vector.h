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

#ifndef ANIMATED_VECTOR_H
#define ANIMATED_VECTOR_H

#include "animation_curve.h"
#include "vector.h"

class AnimatedVector
{
public:
	AnimatedVector();

	Vector getVector() const;
	Vector getVectorAt(float time) const;
	void setFromVector(const Vector& vector);
	void setFromVectorAt(const Vector& vector, float time);

	void add(const Vector& vector);
	void addValues(float xVal, float yVal, float zVal);

	bool isAnimated() const;
	void setAllAnimated(bool animated);

	bool isKeyed() const;

	void setKey();
	void setKey(float time);
	void deleteKey();

	CurveInterpolationType getInterpolationType() const;
	void setInterpolationType(CurveInterpolationType type);

	void load(Stream* stream, unsigned int version);
	void store(Stream* stream) const;

	// static versions which only load/store non-animated versions
	void loadNonAnimated(Stream* stream, unsigned int version);
	void storeNonAnimated(Stream* stream) const;

	// work out if any values are keyed or the non-keyed values are non-zero
	bool isNull() const;

protected:
	AnimationCurve		x;
	AnimationCurve		y;
	AnimationCurve		z;
};

#endif // ANIMATED_VECTOR_H
