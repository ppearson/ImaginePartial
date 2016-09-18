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

#ifndef DIRECT_ILLUMINATION_H
#define DIRECT_ILLUMINATION_H

#include "renderer.h"

#include "raytracer_common.h"
#include "raytracer_ambient_occlusion.h"

#include "sampling/sampler_common.h"

namespace Imagine
{

class RNG;
class Params;
class RenderThreadContext;

template <typename Accumulator>
class DirectIllumination : public Renderer
{
public:
	DirectIllumination(Raytracer& rt, const Params& settings);
	virtual ~DirectIllumination()
	{
	}

	virtual void initialise();

	virtual bool processTask(RenderTask* pRTask, unsigned int threadID);

	bool doProgressiveTask(RenderTask* pTask, unsigned int threadID);
	bool doFullTask(RenderTask* pTask, unsigned int threadID);

	Colour4f processRayRecurse(RenderThreadContext& rtc, ShadingContext& shadingContext, const Ray& ray, unsigned int flags, PathState& pathState,
								  RNG& rng, SampleBundle& samples, unsigned int sampleIndex);

	virtual float calculateProgress() const;

protected:
	Accumulator						m_accumulator;
	RaytracerAmbientOcclusion		m_rtAmbientOcclusion;

	bool				m_ambientOnly;

	unsigned int		m_antiAliasing;
	float				m_antiAliasingSamples;

	bool				m_ambientOcclusion;
	unsigned int		m_ambientOcclusionSamples;

	bool				m_diffuseReflection;
	unsigned int		m_diffuseReflectionSamples;
	float				m_invDiffReflectionSamples;
	float				m_invTotalDiffReflectionSamples;
	std::vector<float>	m_cachedSampleIncrements;

	Sample2DPacket		m_cameraSamples;

	unsigned int		m_samplesPerPixel;
	float				m_invSamplesPerIt;

	unsigned int		m_totalTasks;
	mutable unsigned int		m_tasksDone;

	SampleBundle		m_roughSampleBundle;
};

} // namespace Imagine

#endif // DIRECT_ILLUMINATION_H
