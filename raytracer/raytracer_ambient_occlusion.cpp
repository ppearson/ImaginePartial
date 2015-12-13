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

#include "raytracer_ambient_occlusion.h"

#include "core/ray.h"
#include "core/normal.h"
#include "core/point.h"

#include "sampling/geometry_sampler.h"

#include "utils/maths/rng.h"

#include "scene_interface.h"

RaytracerAmbientOcclusion::RaytracerAmbientOcclusion(const SceneInterface& scene) : m_scene(scene)
{
	m_numSamples = 64;
	m_distanceAttenuation = 0.25f;
}

float RaytracerAmbientOcclusion::getOcclusionAtPoint(const Point& point, const Normal& normal, RNG& rng)
{
	std::vector<Sample2D> aHemisphereSamples;
	m_sampler.generate2DSamples(aHemisphereSamples, rng);

	unsigned int numObstructed = 0;
	float fNumObstructed = 0.0f;

	for (unsigned int i = 0; i < m_totalSamples; i++)
	{
		const Sample2D& sample = aHemisphereSamples[i];

		const Normal sampleNormal = uniformSampleHemisphereN(sample.x, sample.y, normal);

		Ray occlusionRay(point, sampleNormal, RAY_ALL);
		occlusionRay.tMin = 0.0001f;
		// Ray needs inverse direction for BBox testing
		occlusionRay.calculateInverseDirection();

		if (m_scene.doesOcclude(occlusionRay))
		{
			numObstructed ++;

			fNumObstructed += 1.0f;
		}
	}

	if (numObstructed == 0)
		return 0.0f;

	float occVal = fNumObstructed * m_fInvTotalSamples;
	return occVal;
}

float RaytracerAmbientOcclusion::getOcclusionAtPointExistingSamples(const Point& point, const Normal& normal,
																	SampleBundle& samples, unsigned int sampleIndex) const
{
	unsigned int aoSampleIndexStart = sampleIndex * m_totalSamples;

	unsigned int numObstructed = 0;
	float fNumObstructed = 0.0f;

	unsigned int aoSampleIndex = aoSampleIndexStart;

	for (unsigned int i = 0; i < m_totalSamples; i++)
	{
		const Sample2D& sample = samples.getDirectionSample(aoSampleIndex++);

		const Normal sampleNormal = uniformSampleHemisphereN(sample.x, sample.y, normal);

		Ray occlusionRay(point, sampleNormal, RAY_ALL);
		occlusionRay.tMin = 0.0001f;
		// Ray needs inverse direction for BBox testing
		occlusionRay.calculateInverseDirection();

		if (m_scene.doesOcclude(occlusionRay))
		{
			numObstructed ++;

			fNumObstructed += 1.0f;
		}
	}

	if (numObstructed == 0)
		return 0.0f;

	float occVal = fNumObstructed * m_fInvTotalSamples;
	return occVal;
}

void RaytracerAmbientOcclusion::setSampleCount(unsigned int samples)
{
	m_numSamples = samples;
	m_totalSamples = samples * samples;
	m_fInvTotalSamples = 1.0f / float(m_totalSamples);

	m_jitterDelta = 1.0f / float(m_numSamples);

	m_sampler.generateCache(m_totalSamples);
}
