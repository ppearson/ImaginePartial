/*
 Imagine
 Copyright 2015 Peter Pearson.

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

#ifndef RENDER_THREAD_CONTEXT_H
#define RENDER_THREAD_CONTEXT_H

#include "render_statistics.h"

#include "utils/time_counter.h"
#include "utils/maths/rng.h"

#include "sampling/sampler_common.h"
#include "sampling/sample_generator.h"

#include "raytracer/light_sampler.h"

#include "image/image_texture_cache.h"

namespace Imagine
{

class Raytracer;
class SceneInterface;
class RNG;

class RenderThreadContext
{
public:
	RenderThreadContext(const Raytracer* rt, const SceneInterface* sceneInterface, unsigned int threadID)
		: m_pRaytracer(rt), m_pSceneInterface(sceneInterface),
		  m_threadID(threadID), m_pLightSampler(nullptr), m_pIntegratorTimeCounter(nullptr),
					m_pTextureTimeCounter(nullptr), m_pMainImageTextureCache(nullptr),
					m_pRNG(nullptr), m_pSampleBundleReuse(nullptr), m_pSampleGenerator(nullptr)
	{

	}

	~RenderThreadContext()
	{
		if (m_pLightSampler)
		{
			delete m_pLightSampler;
			m_pLightSampler = nullptr;
		}

		if (m_pIntegratorTimeCounter)
		{
			delete m_pIntegratorTimeCounter;
			m_pIntegratorTimeCounter = nullptr;
		}

		if (m_pTextureTimeCounter)
		{
			delete m_pTextureTimeCounter;
			m_pTextureTimeCounter = nullptr;
		}
		
		if (m_pSampleBundleReuse)
		{
			delete m_pSampleBundleReuse;
			m_pSampleBundleReuse = nullptr;
		}
		
		if (m_pSampleGenerator)
		{
			delete m_pSampleGenerator;
			m_pSampleGenerator = nullptr;
		}
	}

	unsigned int getThreadID() const
	{
		return m_threadID;
	}

	inline RenderStatistics& getRenderStatistics()
	{
		return m_statistics;
	}

	inline const RenderStatistics& getRenderStatistics() const
	{
		return m_statistics;
	}

	void setLightSampler(LightSampler* pLightSampler)
	{
		m_pLightSampler = pLightSampler;
	}

	inline LightSampler* getLightSampler()
	{
		return m_pLightSampler;
	}

	void setIntegratorTimeCounter(ThreadTimeCounter* pTimeCounter)
	{
		m_pIntegratorTimeCounter = pTimeCounter;
	}

	inline ThreadTimeCounter* getIntegratorTimeCounter()
	{
		return m_pIntegratorTimeCounter;
	}

	void setTextureTimeCounter(ThreadTimeCounter* pTimeCounter)
	{
		m_pTextureTimeCounter = pTimeCounter;
	}

	inline ThreadTimeCounter* getTextureTimeCounter()
	{
		return m_pTextureTimeCounter;
	}

	void setMainImageTextureCache(ImageTextureCache* pMainImageTextureCache)
	{
		m_pMainImageTextureCache = pMainImageTextureCache;
	}

	inline ImageTextureCache* getMainImageTextureCache() const
	{
		return m_pMainImageTextureCache;
	}

	ImageTextureCache::Microcache& getTextureMicrocache()
	{
		return m_textureMicrocache;
	}
	
	void setRandomNumberGenerator(RNG* pRNG)
	{
		m_pRNG = pRNG;
	}
	
	RNG* getRandomNumberGenerator() const
	{
		return m_pRNG;
	}
	
	void setSampleBundleReuse(SampleBundleReuse* pSBR)
	{
		m_pSampleBundleReuse = pSBR;
	}
	
	SampleBundleReuse* getSampleBundleReuse() const
	{
		return m_pSampleBundleReuse;
	}
	
	void setSampleGenerator(SampleGenerator* pSampleGenerator)
	{
		m_pSampleGenerator = pSampleGenerator;
	}
	
	SampleGenerator* getSampleGenerator() const
	{
		return m_pSampleGenerator;
	}

	const Raytracer* getRaytracer() const
	{
		return m_pRaytracer;
	}

	const SceneInterface* getSceneInterface() const
	{
		return m_pSceneInterface;
	}

protected:
	ImageTextureCache::Microcache	m_textureMicrocache;
	
	const Raytracer*		m_pRaytracer;
	const SceneInterface*	m_pSceneInterface;
	// not really sure we need this, but...
	unsigned int			m_threadID;

	RenderStatistics		m_statistics;

	// we own this
	LightSampler*			m_pLightSampler;

	// we own these
	ThreadTimeCounter*		m_pIntegratorTimeCounter;
	ThreadTimeCounter*		m_pTextureTimeCounter;

	// we don't own this
	ImageTextureCache*		m_pMainImageTextureCache;

	RNG*					m_pRNG; // we don't own this
	
	// optional - preview renderer can use this
	// we take owndership of this
	SampleBundleReuse*		m_pSampleBundleReuse;
	
	// optional - preview renderer can use this
	// we take ownership of this
	SampleGenerator*		m_pSampleGenerator;
	
};

} // namespace Imagine

#endif // RENDER_THREAD_CONTEXT_H
