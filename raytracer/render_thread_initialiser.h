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

#ifndef RENDER_THREAD_INITIALISER_H
#define RENDER_THREAD_INITIALISER_H

#include <map>

#include "image/output_image_tile.h"
#include "raytracer/render_thread_context.h"
#include "raytracer/raytracer.h"

#include "utils/threads/thread_pool.h"

#include "filters/filter.h"

#include "scene_interface.h"

namespace Imagine
{

// Stand-alone thread pool which emulates the thread count / layout of Raytracer, the purpose being to allocate and initialise per-thread
// data on the particular processor core a thread will be running on, the aim being to ensure data is initialised by the processor
// core it will be assigned to on multi socket systems in order to reduce needless cross-socket bandwidth with memory for items
// being almost exclusively used by particular cores having to be fetched from the memory of different sockets...

// Note: this stuff has to match what Raytracer is doing exactly in terms of threads and affinity otherwise it won't do anything useful...

class RenderThreadInitHelper : public ThreadPool
{
public:
	RenderThreadInitHelper(unsigned int numThreads, bool setAffinity, const Raytracer* pRaytracer, const SceneInterface* pSceneInterface)
		: ThreadPool(numThreads, false),
		  m_pRaytracer(pRaytracer), m_pSceneInterface(pSceneInterface)
	{
	}

	struct RenderThreadInitResult
	{
		RenderThreadInitResult() : pImageTile(NULL), pRenderThreadContext(NULL)
		{

		}

		OutputImageTile*			pImageTile;
		RenderThreadContext*		pRenderThreadContext;
	};

	class RenderThreadInitTask1 : public ThreadPoolTask
	{
	public:
		RenderThreadInitTask1(unsigned int tileSizeX, unsigned int tileSizeY, unsigned int imageComponents, Filter* pFilter, unsigned int threadIndex) :
			m_tileSizeX(tileSizeX), m_tileSizeY(tileSizeY), m_imageComponents(imageComponents), m_pFilter(pFilter), m_threadIndex(threadIndex)
		{

		}

		unsigned int	m_tileSizeX;
		unsigned int	m_tileSizeY;
		unsigned int	m_imageComponents;
		Filter*			m_pFilter;

		unsigned int	m_threadIndex;
	};

	class RenderThreadInitTask2 : public ThreadPoolTask
	{
	public:
		RenderThreadInitTask2(DistributionDiscrete* pLightDistribution, const LightsAndSamples* pLightSamples, unsigned int localisedSampleCount,
							  unsigned int threadIndex) :
			m_pLightDistribution(pLightDistribution), m_pLightsAndSamples(pLightSamples), m_localisedSampleCount(localisedSampleCount), m_threadIndex(threadIndex)
		{

		}

		DistributionDiscrete*	m_pLightDistribution;
		const LightsAndSamples* m_pLightsAndSamples;
		unsigned int			m_localisedSampleCount;

		unsigned int			m_threadIndex;
	};

	// type1 - Output Images, and RenderThreadContexts - for the moment, due to the way the code is structured
	// this per thread initialisation annoyingly needs to be done in two parts...
	bool init1(unsigned int tileSizeX, unsigned int tileSizeY, unsigned int imageComponents, Filter* pFilter)
	{
		m_type1 = true;
		for (unsigned int i = 0; i < m_numberOfThreads; i++)
		{
			RenderThreadInitTask1* pTask = new RenderThreadInitTask1(tileSizeX, tileSizeY, imageComponents, pFilter, i);
			addTaskNoLock(pTask);
		}

		startPoolAndWaitForCompletion();

		return m_results1.size() == m_numberOfThreads;
	}

	bool init2(DistributionDiscrete* pLightDistribution, const LightsAndSamples* pLightSamples, unsigned int localisedSampleCount)
	{
		m_type1 = false;

		for (unsigned int i = 0; i < m_numberOfThreads; i++)
		{
			RenderThreadInitTask2* pTask = new RenderThreadInitTask2(pLightDistribution, pLightSamples, localisedSampleCount, i);
			addTaskNoLock(pTask);
		}

		startPoolAndWaitForCompletion();

		return m_results2.size() == m_numberOfThreads;
	}

	const std::map<unsigned int, RenderThreadInitResult>& getResults1() const
	{
		return m_results1;
	}

	std::map<unsigned int, LightSampler*>& getResults2()
	{
		return m_results2;
	}

protected:
	virtual bool doTask(ThreadPoolTask* pTask, unsigned int threadID)
	{
		if (!pTask)
			return false;

		if (m_type1)
		{
			RenderThreadInitTask1* pThisTask = static_cast<RenderThreadInitTask1*>(pTask);

			// do the work of allocating (and importantly initialising) the per-thread memory within the particular
			// threads themselves, so that they get run on their target processor cores / sockets
			OutputImageTile* pNewImage = new OutputImageTile(pThisTask->m_tileSizeX, pThisTask->m_tileSizeY,
															 pThisTask->m_imageComponents, pThisTask->m_pFilter);

			RenderThreadContext* pNewRenderThreadContext = new RenderThreadContext(m_pRaytracer, m_pSceneInterface, pThisTask->m_threadIndex);

			RenderThreadInitResult result;
			result.pImageTile = pNewImage;
			result.pRenderThreadContext = pNewRenderThreadContext;

			m_lock.lock();

			m_results1[pThisTask->m_threadIndex] = result;

			m_lock.unlock();
		}
		else
		{
			RenderThreadInitTask2* pThisTask = static_cast<RenderThreadInitTask2*>(pTask);

			m_lock.lock();

			LightSampler* pNewLightSampler = NULL;

			if (pThisTask->m_localisedSampleCount == 0)
			{
				// we're not localised, so just do the constant one
				pNewLightSampler = new LightSamplerConstant(pThisTask->m_pLightDistribution, pThisTask->m_pLightsAndSamples);
			}
			else
			{
				pNewLightSampler = new LightSamplerLocalised(pThisTask->m_pLightDistribution, pThisTask->m_pLightsAndSamples, pThisTask->m_localisedSampleCount);
			}

			m_results2[pThisTask->m_threadIndex] = pNewLightSampler;

			m_lock.unlock();
		}

		return true;
	}

protected:

	Mutex			m_lock; // write lock

	bool			m_type1;

	std::map<unsigned int, RenderThreadInitResult>	m_results1;
	std::map<unsigned int, LightSampler*>			m_results2;

	const Raytracer*		m_pRaytracer;
	const SceneInterface*	m_pSceneInterface;
};

} // namespace Imagine

#endif // RENDER_THREAD_INITIALISER_H

