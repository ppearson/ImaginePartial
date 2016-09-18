/*
 Imagine
 Copyright 2011-2016 Peter Pearson.

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
#include "render_thread_context.h"
#include "raytracer.h"

#include "core/ray.h"
#include "core/normal.h"
#include "core/point.h"

#include "sampling/geometry_sampler.h"

#include "utils/maths/rng.h"

#include "scene_interface.h"

namespace Imagine
{

RaytracerAmbientOcclusion::RaytracerAmbientOcclusion() : m_pScene(NULL), m_pRaytracer(NULL)
{
	m_distanceAttenuation = 0.25f;
}

RaytracerAmbientOcclusion::RaytracerAmbientOcclusion(const SceneInterface* scene, const Raytracer* rt) : m_pScene(scene), m_pRaytracer(rt)
{
	m_numSamples = 64;
	m_distanceAttenuation = 0.25f;
}

float RaytracerAmbientOcclusion::getOcclusionAtPoint(const HitResult& hitResult, RNG& rng)
{
	std::vector<Sample2D> aHemisphereSamples;
	m_sampler.generate2DSamples(aHemisphereSamples, rng);

	unsigned int numObstructed = 0;
	float fNumObstructed = 0.0f;

	const float tMin = hitResult.intersectionError * m_pRaytracer->getRayEpsilon();

	for (unsigned int i = 0; i < m_totalSamples; i++)
	{
		const Sample2D& sample = aHemisphereSamples[i];

		const Normal sampleNormal = uniformSampleHemisphereN(sample.x, sample.y, hitResult.shaderNormal);

		Ray occlusionRay(hitResult.hitPoint, sampleNormal, RAY_ALL);
		occlusionRay.tMin = tMin;
		// Ray needs inverse direction for BBox testing
		occlusionRay.calculateInverseDirection();

		if (m_pScene->doesOcclude(occlusionRay))
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

float RaytracerAmbientOcclusion::getOcclusionAtPointExistingSamples(const HitResult& hitResult,
																	SampleBundle& samples, unsigned int sampleIndex) const
{
	unsigned int aoSampleIndexStart = sampleIndex * m_totalSamples;

	unsigned int numObstructed = 0;
	float fNumObstructed = 0.0f;

	unsigned int aoSampleIndex = aoSampleIndexStart;

	const float tMin = hitResult.intersectionError * m_pRaytracer->getRayEpsilon();

	for (unsigned int i = 0; i < m_totalSamples; i++)
	{
		const Sample2D& sample = samples.getDirectionSample(aoSampleIndex++);

		const Normal sampleNormal = uniformSampleHemisphereN(sample.x, sample.y, hitResult.shaderNormal);

		Ray occlusionRay(hitResult.hitPoint, sampleNormal, RAY_ALL);
		occlusionRay.tMin = tMin;
		// Ray needs inverse direction for BBox testing
		occlusionRay.calculateInverseDirection();

		if (m_pScene->doesOcclude(occlusionRay))
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

float RaytracerAmbientOcclusion::getOcclusionAtPointStandAlone(const HitResult& hitResult, RNG& rng) const
{
	std::vector<Sample2D> aHemisphereSamples;
	m_sampler.generate2DSamples(aHemisphereSamples, rng);

	unsigned int numObstructed = 0;
	float fNumObstructed = 0.0f;

	const Raytracer* pRT = hitResult.getShadingContext()->getRenderThreadContext()->getRaytracer();
	const SceneInterface* pSI = hitResult.getShadingContext()->getRenderThreadContext()->getSceneInterface();

	const float tMin = hitResult.intersectionError * pRT->getRayEpsilon();

	for (unsigned int i = 0; i < m_totalSamples; i++)
	{
		const Sample2D& sample = aHemisphereSamples[i];

		const Normal sampleNormal = uniformSampleHemisphereN(sample.x, sample.y, hitResult.shaderNormal);

		Ray occlusionRay(hitResult.hitPoint, sampleNormal, RAY_ALL);
		occlusionRay.tMin = tMin;
		// Ray needs inverse direction for BBox testing
		occlusionRay.calculateInverseDirection();

		if (pSI->doesOcclude(occlusionRay))
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

} // namespace Imagine
