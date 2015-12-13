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

#include "raytracer/light_sampler.h"

#include "image/image_texture_cache.h"

class ImageTextureCache;

class RenderThreadContext
{
public:
	RenderThreadContext(unsigned int threadID) : m_threadID(threadID), m_pLightSampler(NULL), m_pTimeCounter(NULL),
					m_pMainImageTextureCache(NULL)
	{
		if (m_pLightSampler)
		{
			delete m_pLightSampler;
			m_pLightSampler = NULL;
		}

		if (m_pTimeCounter)
		{
			delete m_pTimeCounter;
			m_pTimeCounter = NULL;
		}
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

protected:
	// not really sure we need this, but...
	unsigned int		m_threadID;

	RenderStatistics	m_statistics;

	// we own this
	LightSampler*		m_pLightSampler;

	// we own this
	ThreadTimeCounter*	m_pTimeCounter;

	// we don't own this
	ImageTextureCache*	m_pMainImageTextureCache;

	ImageTextureCache::Microcache	m_textureMicrocache;
};

#endif // RENDER_THREAD_CONTEXT_H
