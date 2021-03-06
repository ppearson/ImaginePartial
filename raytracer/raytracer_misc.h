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

#ifndef RAYTRACER_MISC_H
#define RAYTRACER_MISC_H

#include <cstdio>

namespace Imagine
{

class Light;

struct LightsAndSamples
{
	LightsAndSamples() : pLight(nullptr), numSamples(0), sampleMultiplier(1), rcpMultiplier(1.0f)
	{
	}

	LightsAndSamples(Light* pLi, unsigned int samples, unsigned int sampleMult) : pLight(pLi), numSamples(samples), sampleMultiplier(sampleMult)
	{
		rcpMultiplier = 1.0f / (float)samples;
	}

	Light*			pLight;
	unsigned int	numSamples;
	unsigned int	sampleMultiplier;
	float			rcpMultiplier;
};

} // namespace Imagine

#endif // RAYTRACER_MISC_H
