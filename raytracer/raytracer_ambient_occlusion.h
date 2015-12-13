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

#ifndef RAYTRACER_AMBIENT_OCCLUSION_H
#define RAYTRACER_AMBIENT_OCCLUSION_H

#include <vector>

#include "sampling/cached_sampler.h"

class Normal;
class Point;
class SceneInterface;
class RNG;

class RaytracerAmbientOcclusion
{
public:
	RaytracerAmbientOcclusion(const SceneInterface& scene);

	float getOcclusionAtPoint(const Point& point, const Normal& normal, RNG& rng);
	float getOcclusionAtPointExistingSamples(const Point& point, const Normal& normal, SampleBundle& samples, unsigned int sampleIndex) const;

	void setSampleCount(unsigned int samples);
	void setDistanceAttenuation(float distAttenuation) { m_distanceAttenuation = distAttenuation; }


protected:
	const SceneInterface&	m_scene;

	CachedSampler			m_sampler;

	unsigned int			m_numSamples;
	unsigned int			m_totalSamples;
	float					m_fInvTotalSamples;

	float					m_distanceAttenuation;

	float					m_jitterDelta;

	std::vector<float>		m_aXSamples;
	std::vector<float>		m_aYSamples;
};

#endif // RAYTRACER_AMBIENT_OCCLUSION_H
