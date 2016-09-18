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

#include "raytracer.h"

#include <cmath>
#include <ctime>
#include <limits>
#include <algorithm>
#include <assert.h>

#include "scene_interface.h"
#include "scene.h"

#include "objects/camera.h"
#include "lights/light.h"
#include "materials/material.h"
#include "bsdfs/baked_bsdf.h"

#include "raytracer/camera_ray_creators/camera_ray_creator.h"

#include "global_context.h"
#include "output_context.h"

#include "image/output_image.h"
#include "image/output_image_tile.h"

#include "utils/timer.h"
#include "utils/maths/rng.h"
#include "utils/params.h"
#include "utils/simd.h" // for flush to zero functions

#include "textures/texture.h"
#include "textures/mapping.h"

#include "raytracer/direct_illumination.h"
#include "raytracer/path_tracer_deep.h"

#include "raytracer/integrators/path.h"
#include "raytracer/integrators/path_aovs.h"
#include "raytracer/integrators/path_volume.h"

#include "raytracer/accumulators.h"

#include "raytracer/full_renderer.h"
#include "raytracer/adaptive_renderer.h"
#include "raytracer/preview_renderer.h"
#include "raytracer/aov_full_renderer.h"
#include "raytracer/path_tracer_distributed.h"
#include "raytracer/bi_directional_path_tracer.h"

#include "raytracer/debug_renderer.h"

#include "raytracer/renderer_background.h"

#include "raytracer/render_thread_initialiser.h"
#include "raytracer/render_thread_context.h"

#include "filters/filter_factory.h"

#include "remote/remote_state.h"

#include "tile_task_generator.h"

#include "render_statistics.h"

#include "utils/time_counter.h"
#include "utils/system.h"

namespace Imagine
{

RenderTask::RenderTask(unsigned int startX, unsigned int startY, unsigned int width, unsigned int height, TileState state, unsigned int taskIndex)
	: m_state(state), m_startX(startX), m_startY(startY), m_width(width), m_height(height), m_extraChannelsDone(false), m_iterationCount(0),
	  m_taskIndex(taskIndex)
{
}

RenderTask::~RenderTask()
{
}

static const unsigned int kTileSize = 32;

Raytracer::Raytracer(SceneInterface& scene, OutputImage* outputImage, Params& settings, bool preview, unsigned int threads)
	: ThreadPool(threads, false), m_scene(scene), m_pOutputImage(outputImage), m_useRemoteClients(false), m_pRenderer(NULL), m_pFilter(NULL),
	  m_tileApronSize(0), m_progressive(settings.getBool("progressive")), m_extraChannels(0), m_statsType(eStatisticsNone),
	  m_statsOutputType(eStatsOutputConsole), m_preview(preview), m_pRenderCamera(NULL), m_pCameraRayCreator(NULL), m_pHost(NULL),
	  m_pGlobalImageCache(NULL), m_backgroundType(eBackgroundNone),
	  m_pBackground(NULL), m_lightSampling(eLSFullAllLights), m_sampleLights(false), m_lightSamples(0), m_motionBlur(false), m_depthOfField(false),
	  m_pDebugPathCollection(NULL)
{
	initialise(outputImage, settings);

	if (GlobalContext::instance().getRenderThreadsLowPriority())
		m_lowPriorityThreads = true;
}

// this is just used for preview renders, so we can make certain assumptions...
Raytracer::Raytracer(SceneInterface& scene, unsigned int threads, bool progressive) : ThreadPool(threads), m_scene(scene),
	m_pOutputImage(NULL), m_useRemoteClients(false), m_pRenderer(NULL), m_pFilter(NULL), m_tileApronSize(0), m_progressive(progressive),
	m_preview(true), m_pRenderCamera(NULL), m_pCameraRayCreator(NULL), m_pHost(NULL), m_pGlobalImageCache(NULL),
	m_backgroundType(eBackgroundNone), m_pBackground(NULL),
	m_lightSampling(eLSFullAllLights), m_sampleLights(false), m_lightSamples(0), m_motionBlur(false), m_pDebugPathCollection(NULL)
{
	// assumes that initialise() is going to be called later on
	m_tileOrder = 0;

	if (GlobalContext::instance().getRenderThreadsLowPriority())
		m_lowPriorityThreads = true;
}

Raytracer::~Raytracer()
{
	std::vector<OutputImageTile*>::iterator itTempImage = m_aThreadTempImages.begin();
	for (; itTempImage != m_aThreadTempImages.end(); ++itTempImage)
	{
		delete *itTempImage;
	}

	std::vector<RenderThreadContext*>::iterator itRenderThreadContext = m_aRenderThreadContexts.begin();
	for (; itRenderThreadContext != m_aRenderThreadContexts.end(); ++itRenderThreadContext)
	{
		delete *itRenderThreadContext;
	}

	if (m_pRenderer)
	{
		delete m_pRenderer;
		m_pRenderer = NULL;
	}

	if (m_pFilter)
	{
		delete m_pFilter;
		m_pFilter = NULL;
	}

	if (m_pBackground)
	{
		delete m_pBackground;
		m_pBackground = NULL;
	}

	if (m_pCameraRayCreator)
	{
		delete m_pCameraRayCreator;
		m_pCameraRayCreator = NULL;
	}

	if (m_pGlobalImageCache)
	{
		delete m_pGlobalImageCache;
		m_pGlobalImageCache = NULL;
	}
}

void Raytracer::initialise(OutputImage* outputImage, const Params& settings)
{
	m_pOutputImage = outputImage;

	m_timeSeed = std::clock();

	m_width = settings.getUInt("width");
	m_height = settings.getUInt("height");

	if (!settings.getBool("renderCrop"))
	{
		// we're rendering the whole image
		m_renderWindowX = 0;
		m_renderWindowY = 0;
		m_renderWindowWidth = m_width;
		m_renderWindowHeight = m_height;
	}
	else
	{
		m_renderWindowX = settings.getUInt("cropX");
		m_renderWindowY = settings.getUInt("cropY");
		m_renderWindowWidth = settings.getUInt("cropWidth");
		m_renderWindowHeight = settings.getUInt("cropHeight");
	}

	m_rayBouncesOverall = settings.getUInt("rbOverall", 8);
	m_rayBouncesDiffuse = settings.getUInt("rbDiffuse", 3);
	m_rayBouncesGlossy = settings.getUInt("rbGlossy", 3);
	m_rayBouncesReflection = settings.getUInt("rbReflection", 5);
	m_rayBouncesRefraction = settings.getUInt("rbRefraction", 5);

	m_rayEpsilon = m_scene.getRayEpsilon();
	m_shadowRayEpsilon = m_scene.getShadowRayEpsilon();
	m_rayEpsilon = settings.getFloat("rayEpsilon", m_rayEpsilon);
	m_shadowRayEpsilon = settings.getFloat("shadowRayEpsilon", m_shadowRayEpsilon);

	m_tileSize = settings.getUInt("tile_size", kTileSize);
	m_tileOrder = settings.getUInt("tile_order", 3);

	unsigned int imageFlags = COMPONENT_RGBA;
	if (m_progressive || settings.getUInt("integrator") > 0)
		imageFlags = imageFlags | COMPONENT_SAMPLES;

	if (settings.getBool("output_z"))
	{
		m_extraChannels |= COMPONENT_DEPTH_NORMALISED;
		imageFlags |= COMPONENT_DEPTH_NORMALISED;
	}
	else if (settings.getBool("output_realz"))
	{
		m_extraChannels |= COMPONENT_DEPTH_REAL;
		imageFlags |= COMPONENT_DEPTH_REAL;
	}

	if (settings.getBool("output_normals"))
	{
		m_extraChannels |= COMPONENT_NORMAL;
		imageFlags |= COMPONENT_NORMAL;
	}

	if (settings.getBool("output_wpp"))
	{
		m_extraChannels |= COMPONENT_WPP;
		imageFlags |= COMPONENT_WPP;
	}

	if (settings.getBool("output_shadows"))
	{
		m_extraChannels |= COMPONENT_SHADOWS;
		imageFlags |= COMPONENT_SHADOWS;
	}

	if (settings.getBool("deep"))
	{
		imageFlags |= COMPONENT_DEEP;
	}

	m_statsType = (StatisticsType)settings.getUInt("statsType", 0);
	m_statsOutputType = (StatisticsOutputType)settings.getUInt("statsOutputType", 0);

	if (m_pGlobalImageCache)
	{
		delete m_pGlobalImageCache;
		m_pGlobalImageCache = NULL;
	}

	// Create the global Image Texture Cache...

	// TODO: work out what Image Texture Cache to use

	// for material preview only at the moment...
	bool useTextureCacheSet = settings.getBool("useTextureCaching", false);

	GlobalContext::TextureCachingType textureCacheType = GlobalContext::instance().getTextureCachingType();
	if (textureCacheType != GlobalContext::eTextureCachingNone || useTextureCacheSet)
	{
		bool textureFileHandleCaching = settings.getBool("useTextureFileHandleCaching", true);

		size_t memLimit = GlobalContext::instance().getTextureCacheMemoryLimit();
		// ImageTextureCache needs memory size specified as KB
		memLimit *= 1024;
		size_t fileHandleLimit = GlobalContext::instance().getTextureCacheFileHandleLimit();

		m_pGlobalImageCache = new ImageTextureCache(memLimit, textureFileHandleCaching, fileHandleLimit, m_numberOfThreads);

		if (m_pGlobalImageCache)
		{
			unsigned int textureTileDataFixType = settings.getUInt("textureTileDataFix", 0);

			m_pGlobalImageCache->setFixTileDataType((ImageTextureCache::FixTileValuesType)textureTileDataFixType);

			float textureGlobalMipmapBias = settings.getFloat("textureGlobalMipmapBias", 0.0f);
			if (textureGlobalMipmapBias != 0.0f)
			{
				m_pGlobalImageCache->setGlobalMipmapLevelBias((int)textureGlobalMipmapBias);
			}

			bool deleteTileItems = settings.getBool("textureDeleteTileItems", false);
			m_pGlobalImageCache->setDeleteTileItems(deleteTileItems);
		}
	}

	// initialise Filter
	if (m_pFilter)
	{
		delete m_pFilter;
		m_pFilter = NULL;
	}

	unsigned int filterType = settings.getUInt("filter_type", 0);
	m_pFilter = FilterFactory::createFilter(filterType, 0.5f);

	m_pFilter->initialise(true); // clamp negative filter lobes for the moment

	m_tileApronSize = m_pFilter->getApronSize();

	// for the moment, delete any existing per-thread stuff - for re-rendering, this has overheads,
	// but it lets us cope with global state changes (different number of threads) easily
	if (!m_aThreadTempImages.empty())
	{
		std::vector<OutputImageTile*>::iterator it = m_aThreadTempImages.begin();
		for (; it != m_aThreadTempImages.end(); ++it)
		{
			delete *it;
		}
		m_aThreadTempImages.clear();
	}

	if (!m_aRenderThreadContexts.empty())
	{
		std::vector<RenderThreadContext*>::iterator itRenderThreadContext = m_aRenderThreadContexts.begin();
		for (; itRenderThreadContext != m_aRenderThreadContexts.end(); ++itRenderThreadContext)
		{
			delete *itRenderThreadContext;
		}
		m_aRenderThreadContexts.clear();
	}

	// allocate temporary Images so each render thread has its own tile image
	// it can write into

	bool haveInitialisedPerThreadData = false;

	// if we have more than one processor socket, initialise things differently
	System::CPUInfo cpuInfo = System::getCPUInfo();
	bool initOnThreads = cpuInfo.numSockets > 1 && m_numberOfThreads > 1;

	if (initOnThreads)
	{
		RenderThreadInitHelper threadInitHelper(m_numberOfThreads, true, this, &m_scene);

		if (threadInitHelper.init1(m_tileSize, m_tileSize, imageFlags, m_pFilter))
		{
			haveInitialisedPerThreadData = true;

			// just need to hook everything up...
			const std::map<unsigned int, RenderThreadInitHelper::RenderThreadInitResult>& threadResults = threadInitHelper.getResults1();

			// if init1() returned true, then we have the correct number of results for the number of threads we want...
			// so we can just do a push_back on the vectors straight from the map, as it's ordered based on the thread index

			std::map<unsigned int, RenderThreadInitHelper::RenderThreadInitResult>::const_iterator itResult = threadResults.begin();
			for (; itResult != threadResults.end(); ++itResult)
			{
				const RenderThreadInitHelper::RenderThreadInitResult& result = itResult->second;

				m_aThreadTempImages.push_back(result.pImageTile);

				// create approprate time counter for thread, based off stats type
				ThreadTimeCounter* pNewTimeCounter = NULL;
				if (m_statsType != eStatisticsFull)
				{
					pNewTimeCounter = new ThreadTimeCounterNull();
				}
				else
				{
					pNewTimeCounter = new ThreadTimeCounterReal();
				}

				result.pRenderThreadContext->setTimeCounter(pNewTimeCounter);

				result.pRenderThreadContext->setMainImageTextureCache(m_pGlobalImageCache);

				if (m_pGlobalImageCache)
				{
					m_pGlobalImageCache->addMicrocache(result.pRenderThreadContext->getTextureMicrocache());
				}

				m_aRenderThreadContexts.push_back(result.pRenderThreadContext);
			}
		}
	}

	if (!haveInitialisedPerThreadData)
	{
		// do it ourselves in the main thread

		for (unsigned int i = 0; i < m_numberOfThreads; i++)
		{
			OutputImageTile* pNewImage = new OutputImageTile(m_tileSize, m_tileSize, imageFlags, m_pFilter);

			if (pNewImage)
				m_aThreadTempImages.push_back(pNewImage);

			RenderThreadContext* pNewRenderThreadContext = new RenderThreadContext(this, &m_scene, i);

			if (pNewRenderThreadContext)
			{
				// TODO: this won't always be the value to set (e.g. per thread)...
				pNewRenderThreadContext->setMainImageTextureCache(m_pGlobalImageCache);

				// create approprate time counter for thread, based off stats type
				ThreadTimeCounter* pNewTimeCounter = NULL;
				if (m_statsType != eStatisticsFull)
				{
					pNewTimeCounter = new ThreadTimeCounterNull();
				}
				else
				{
					pNewTimeCounter = new ThreadTimeCounterReal();
				}

				pNewRenderThreadContext->setTimeCounter(pNewTimeCounter);

				if (m_pGlobalImageCache)
				{
					m_pGlobalImageCache->addMicrocache(pNewRenderThreadContext->getTextureMicrocache());
				}

				m_aRenderThreadContexts.push_back(pNewRenderThreadContext);
			}
		}
	}

	bool isReRenderPreview = settings.getBool("re_render");

	// if we're re-render preview, set picking to true, so we use the OpenGL View camera...
	m_scene.configureSceneForRender(isReRenderPreview);

	unsigned int lightSamplingType = settings.getUInt("lightSamplingType", 0);
	if (lightSamplingType == 0)
	{
		m_lightSampling = eLSFullAllLights;
		m_sampleLights = false;
		m_lightSamples = 0;
	}
	else
	{
		m_lightSampling = (LightSamplingType)((unsigned int)eLSFullAllLights + lightSamplingType);
		m_sampleLights = true;
		m_lightSamples = settings.getUInt("lightSamples", 1);
	}

	m_pRenderCamera = m_scene.getRenderCamera();

	m_motionBlur = settings.getBool("motionBlur");
	m_depthOfField = settings.getBool("depthOfField");

	updateCameraRayCreator();

	// configure background
	configureBackground();

	initRendererAndIntegrator(settings);
}

void Raytracer::initRendererAndIntegrator(const Params& settings)
{
	if (m_pRenderer)
		delete m_pRenderer;

	bool debugRender = settings.getBool("debugRender", false);
	if (debugRender)
	{
		m_pRenderer = new DebugRenderer(*this, settings, m_numberOfThreads);
		return;
	}

	unsigned int integratorType = settings.getUInt("integrator", 0);

	// TODO: this is pretty horrendous, especially the templated-render/integrator stuff...

	bool transparentShadows = m_scene.haveLightsWithTransparentShadows();

	if (integratorType == 0)
	{
		if (settings.getUInt("filter_type", 0) == 0) // if box reconstruction filter, we can avoid doing filtering altogether
		{
			m_pRenderer = new DirectIllumination<Colour4fStandardNoSamples>(*this, settings);
		}
		else
		{
			m_pRenderer = new DirectIllumination<Colour4fFilteredNoSamples>(*this, settings);
		}
	}
	// special case non-path ones for the moment
	else if (integratorType == 2)
	{
		m_pRenderer = new PathTracerDistributed(*this, settings, m_numberOfThreads, transparentShadows);
	}
	else if (integratorType == 3)
	{
		m_pRenderer = new BiDirectionalPathTracer(*this, settings, m_numberOfThreads);
	}
	else
	{
		if (settings.getBool("preview"))
		{
			if (!transparentShadows)
			{
				if (settings.getBool("volumetric", false))
				{
					m_pRenderer = new PreviewRenderer<PathVolume, Colour4fStandard, TimerCounterNull>(*this, settings, Colour4fStandard(), m_numberOfThreads);
				}
				else
				{
					m_pRenderer = new PreviewRenderer<Path, Colour4fStandard, TimerCounterNull>(*this, settings, Colour4fStandard(), m_numberOfThreads);
				}
			}
			else
			{
				// always do PathVolume, as it supports transparent shadows
				m_pRenderer = new PreviewRenderer<PathVolume, Colour4fStandard, TimerCounterNull>(*this, settings, Colour4fStandard(), m_numberOfThreads);
			}
			return;
		}

		if (settings.getBool("deep"))
		{
			m_pRenderer = new PathTracerDeep(*this, settings, m_numberOfThreads);
			return;
		}

		bool requireSpecialAOVs = settings.getBool("output_shadows");
		requireSpecialAOVs |= settings.getBool("output_diffuse_direct");
		requireSpecialAOVs |= settings.getBool("output_diffuse_indirect");
		requireSpecialAOVs |= settings.getBool("output_specular_direct");

		if (requireSpecialAOVs)
		{
			m_pRenderer = new AOVFullRenderer<PathAOVs>(*this, settings, m_numberOfThreads);

			// for the moment, make sure we're using full light sampling for PathAOV integrator
			// TODO: this needs to be changed
			m_lightSampling = eLSFullAllLights;
			m_sampleLights = false;
			m_lightSamples = 0;

			return;
		}

		if (settings.getBool("adaptive"))
		{
			if (settings.getUInt("filter_type", 0) == 0) // if box reconstruction filter, we can avoid doing filtering altogether
			{
				if (m_statsType != eStatisticsFull)
				{
					if (transparentShadows || settings.getBool("volumetric", false))
					{
						m_pRenderer = new AdaptiveRenderer<PathVolume, Colour4fStandard, TimerCounterNull>(*this, settings, m_numberOfThreads);
					}
					else
					{
						m_pRenderer = new AdaptiveRenderer<Path, Colour4fStandard, TimerCounterNull>(*this, settings, m_numberOfThreads);
					}
				}
				else
				{
					if (transparentShadows || settings.getBool("volumetric", false))
					{
						m_pRenderer = new AdaptiveRenderer<PathVolume, Colour4fStandard, TimerCounter>(*this, settings, m_numberOfThreads);
					}
					else
					{
						m_pRenderer = new AdaptiveRenderer<Path, Colour4fStandard, TimerCounter>(*this, settings, m_numberOfThreads);
					}
				}
			}
			else
			{
				if (m_statsType != eStatisticsFull)
				{
					if (transparentShadows || settings.getBool("volumetric", false))
					{
						m_pRenderer = new AdaptiveRenderer<PathVolume, Colour4fFiltered, TimerCounterNull>(*this, settings, m_numberOfThreads);
					}
					else
					{
						m_pRenderer = new AdaptiveRenderer<Path, Colour4fFiltered, TimerCounterNull>(*this, settings, m_numberOfThreads);
					}
				}
				else
				{
					if (transparentShadows || settings.getBool("volumetric", false))
					{
						m_pRenderer = new AdaptiveRenderer<PathVolume, Colour4fFiltered, TimerCounter>(*this, settings, m_numberOfThreads);
					}
					else
					{
						m_pRenderer = new AdaptiveRenderer<Path, Colour4fFiltered, TimerCounter>(*this, settings, m_numberOfThreads);
					}
				}
			}
			return;
		}

		if (m_statsType != eStatisticsFull)
		{
			if (settings.getUInt("filter_type", 0) == 0) // if box reconstruction filter, we can avoid doing filtering altogether
			{
				if (transparentShadows || settings.getBool("volumetric", false))
				{
					m_pRenderer = new FullRenderer<PathVolume, Colour4fStandard, TimerCounterNull>(*this, settings, Colour4fStandard(), m_numberOfThreads);
				}
				else
				{
					m_pRenderer = new FullRenderer<Path, Colour4fStandard, TimerCounterNull>(*this, settings, Colour4fStandard(), m_numberOfThreads);
				}
			}
			else
			{
				if (transparentShadows || settings.getBool("volumetric", false))
				{
					m_pRenderer = new FullRenderer<PathVolume, Colour4fFiltered, TimerCounterNull>(*this, settings, Colour4fFiltered(), m_numberOfThreads);
				}
				else
				{
					m_pRenderer = new FullRenderer<Path, Colour4fFiltered, TimerCounterNull>(*this, settings, Colour4fFiltered(), m_numberOfThreads);
				}
			}
		}
		else
		{
			// we want full stats with timings, so...

			if (settings.getUInt("filter_type", 0) == 0) // if box reconstruction filter, we can avoid doing filtering altogether
			{
				if (transparentShadows || settings.getBool("volumetric", false))
				{
					m_pRenderer = new FullRenderer<PathVolume, Colour4fStandard, TimerCounter>(*this, settings, Colour4fStandard(), m_numberOfThreads);
				}
				else
				{
					m_pRenderer = new FullRenderer<Path, Colour4fStandard, TimerCounter>(*this, settings, Colour4fStandard(), m_numberOfThreads);
				}
			}
			else
			{
				if (transparentShadows || settings.getBool("volumetric", false))
				{
					m_pRenderer = new FullRenderer<PathVolume, Colour4fFiltered, TimerCounter>(*this, settings, Colour4fFiltered(), m_numberOfThreads);
				}
				else
				{
					m_pRenderer = new FullRenderer<Path, Colour4fFiltered, TimerCounter>(*this, settings, Colour4fFiltered(), m_numberOfThreads);
				}
			}
		}
	}
}

void Raytracer::setRemoteRenderSettings(const RemoteState& remoteRenderState)
{
	if (remoteRenderState.shouldUseRemoteClients())
	{
		RenderClientItem itemToUse;
		bool haveValidRemoteClientToUse = remoteRenderState.getBestAvailableRenderClient(itemToUse);
		if (haveValidRemoteClientToUse)
		{
			m_clientJobManager.setClient(itemToUse);
			m_useRemoteClients = true;

			return;
		}

		fprintf(stderr, "Error: Can't find any remote render clients to use...\n");
	}

	// otherwise, don't use it
	m_useRemoteClients = false;
}

void Raytracer::configureBackground()
{
	if (m_pBackground)
	{
		delete m_pBackground;
		m_pBackground = NULL;
	}

	m_backgroundType = m_scene.getBackgroundType();
	if (m_backgroundType == eBackgroundTexture)
	{
		const Texture* pTexture = m_scene.getBackgroundTexture().getTexture();

		if (!pTexture)
		{
			m_backgroundType = eBackgroundNone;
			return;
		}

		// otherwise, create an appropriate background class to use
		if (pTexture->isConstant())
		{
			m_pBackground = new RendererBackgroundConstant(pTexture);
		}
		else
		{
			// assume it's an image, work out if the image needs filtering (it's a lot bigger than the render size), and
			// create the appropriate type, then pass in the render size

			// TODO: need better heuristic than this, but this prevents blurring with just double sized textures which don't normally show aliasing
			bool needFiltering = (pTexture->getWidth() >= (m_renderWindowWidth * 3)) || (pTexture->getHeight() >= (m_renderWindowHeight * 3));
			if (needFiltering)
			{
				m_pBackground = new RendererBackgroundImageFiltered(pTexture, m_renderWindowWidth, m_renderWindowHeight);
			}
			else
			{
				m_pBackground = new RendererBackgroundImage(pTexture, m_renderWindowWidth, m_renderWindowHeight);
			}
		}
	}
}

void Raytracer::renderScene(float time, const Params* pParams)
{
	// create the tile jobs first, so that we can send stuff to remote clients to render
	createTileJobs();

	if (m_useRemoteClients)
	{
		// kick off remote render on remote clients...
		const Scene& localScene = dynamic_cast<Scene&>(m_scene);

		m_clientJobManager.setOutputImage(m_pOutputImage);

		m_clientJobManager.startRemoteRender(localScene, pParams);
	}

	updateCameraRayCreator();

	{
		Timer time1("Scene pre-renders", !m_preview);

		m_pRenderCamera->init(m_width, m_height, time);

		bool fastAccelBuild = false;
		unsigned int parallelBuildLevels = 2;

		bool reRender = false;

		if (pParams)
		{
			reRender = pParams->getBool("integrated_rerender", false);
			fastAccelBuild = pParams->getBool("accelFastBuild", false);
			parallelBuildLevels = pParams->getUInt("accelParallelLevels", 2);
		}

		// update the objects to do transform caching, acceleration structure building, etc
		if (!m_motionBlur)
		{
			PreRenderRequirements requirements(true, eAccelStructureStatusRendering, time);
			requirements.fastBuild = fastAccelBuild;
			requirements.parallelLevels = parallelBuildLevels;
			requirements.reRender = reRender;
			m_scene.doPreRenders(requirements);
		}
		else
		{
			float shutterOpen = time;
			float shutterClose = time;
			m_pCameraRayCreator->getShutterTimes(shutterOpen, shutterClose);
			PreRenderRequirements requirements(true, eAccelStructureStatusRendering, shutterOpen, shutterClose);
			requirements.fastBuild = fastAccelBuild;
			requirements.parallelLevels = parallelBuildLevels;
			requirements.reRender = reRender;
			m_scene.doPreRenders(requirements);
		}
	}

	m_timeSeed = std::clock();

	// this is needed after the tasks have been added... is it??
	// sets up lights and lightsamples
	m_pRenderer->init(time);

	// if we're doing light sampling, set up per render thread context config
	if (m_lightSampling != eLSFullAllLights)
	{
		setupRenderThreadContextLightSampling();
	}

	// flush to zero and no denormals
	_mm_setcsr(_mm_getcsr() | (1<<15) | (1<<6));

	{
		RenderStatistics totalStatistics;
		if (m_statsType != eStatisticsNone)
		{
			totalStatistics.recordInitialPreRenderStatistics();
		}

		Timer time1("Actual rendering", !m_preview);

		// explicitly turn on affinity setting for threads...
		m_setAffinity = true;

		startPoolAndWaitForCompletion();

		if (m_pHost)
		{
			// call finished on the host, so it can finalise stuff
			m_pHost->finished();
		}

		if (m_statsType != eStatisticsNone)
		{
			std::string statsPath = m_statsOutputPath;
			if (m_statsOutputType == eStatsOutputFile && statsPath.empty())
			{
				m_statsOutputType = eStatsOutputConsole;
			}
			// build up total RenderStatistics from all render threads's totals
			std::vector<RenderThreadContext*>::const_iterator itRenderThreadContext = m_aRenderThreadContexts.begin();
			for (; itRenderThreadContext != m_aRenderThreadContexts.end(); ++itRenderThreadContext)
			{
				const RenderThreadContext* pRTT = *itRenderThreadContext;
				const RenderStatistics& rs = pRTT->getRenderStatistics();

				totalStatistics.mergeStatistics(rs);
			}

			if (m_pGlobalImageCache)
			{
				m_pGlobalImageCache->bakeSummaryStatistics(totalStatistics.getImageTextureCacheStats());
			}

			totalStatistics.populateOverallStatistics();

			if (m_statsOutputType == eStatsOutputConsole)
			{
				totalStatistics.printStatistics();
			}
			else
			{
				int frameNumber = OutputContext::instance().getFrame();
				totalStatistics.writeStatistics(statsPath, frameNumber);
			}
		}

		// if remote render, need to wait for results server
		// TODO: this should probably be above...
		if (m_useRemoteClients)
		{
			m_clientJobManager.wait();
		}
	}
}

void Raytracer::createTileJobs()
{
	unsigned int width = m_renderWindowWidth;
	unsigned int height = m_renderWindowHeight;

	unsigned int tilesX = width / m_tileSize;
	if (width % m_tileSize > 0)
		tilesX ++;

	unsigned int tilesY = height / m_tileSize;
	if (height % m_tileSize > 0)
		tilesY ++;

	TileState initalState = eTSBlank;
	// if no lights, assume we want ambient occlusion, so skip direct illumination states...
	if (m_scene.getRenderLightCount() == 0)
	{
		initalState = eTSAA;
	}

	std::vector<TileCoord> aTiles;
	TileTaskGeneratorFactory::generateTilePositions(tilesX, tilesY, aTiles, m_tileOrder);

	if (!m_useRemoteClients)
	{
		unsigned int taskIndex = 0;
		// everything is rendered locally
		std::vector<TileCoord>::iterator it = aTiles.begin();
		for (; it != aTiles.end(); ++it)
		{
			const TileCoord& tc = *it;

			unsigned int xPos = (tc.x * m_tileSize) + m_renderWindowX;
			unsigned int yPos = (tc.y * m_tileSize) + m_renderWindowY;

			unsigned int tileWidth = std::min(m_renderWindowX + width - xPos, m_tileSize);
			unsigned int tileHeight = std::min(m_renderWindowY + height - yPos, m_tileSize);

			RenderTask* pNewTask = new RenderTask(xPos, yPos, tileWidth, tileHeight, initalState, taskIndex++);
			addTaskNoLock(pNewTask);
		}
	}
	else
	{
		// we're rendering some tiles remotely, so work out the ratio we're going to send over...
		unsigned int remoteThreadsAvailable = m_clientJobManager.getTotalRenderClientThreads();
		unsigned int localThreadsAvailable = m_numberOfThreads;

		unsigned int totalThreads = remoteThreadsAvailable + localThreadsAvailable;
		float ratioRemoteJobs = (float)totalThreads / (float)remoteThreadsAvailable;
		unsigned int totalTasks = aTiles.size();
		unsigned int numTasksToSend = (float)totalTasks / ratioRemoteJobs;

		unsigned int pickStride = totalTasks / numTasksToSend;
		std::vector<TileCoord> aRemoteTiles;

		bool inversePick = false;

		// TODO:: fix this properly
		if (pickStride == 1)
		{
			// we're sending a larger ratio of tasks remotely than we're doing locally...
			pickStride = totalTasks / (totalTasks - numTasksToSend);

			inversePick = true;
		}

		unsigned int index = 0;
		std::vector<TileCoord>::iterator it = aTiles.begin();
		for (; it != aTiles.end(); ++it)
		{
			const TileCoord& tc = *it;

			if (!inversePick)
			{
				if (index++ % pickStride == 0)
				{
					// add to tiles we're going to send to the remote client to render...
					aRemoteTiles.push_back(tc);
					// continue to the next task...
					continue;
				}
			}
			else
			{
				if (index++ % pickStride != 0)
				{
					// add to tiles we're going to send to the remote client to render...
					aRemoteTiles.push_back(tc);
					// continue to the next task...
					continue;
				}
			}

			unsigned int xPos = (tc.x * m_tileSize) + m_renderWindowX;
			unsigned int yPos = (tc.y * m_tileSize) + m_renderWindowY;

			unsigned int tileWidth = std::min(m_renderWindowX + width - xPos, m_tileSize);
			unsigned int tileHeight = std::min(m_renderWindowY + height - yPos, m_tileSize);

			RenderTask* pNewTask = new RenderTask(xPos, yPos, tileWidth, tileHeight, initalState, index);
			addTaskNoLock(pNewTask);
		}

		m_clientJobManager.addTasks(aRemoteTiles);
	}
}

void Raytracer::taskDone()
{
	// update progress if we've got a host
	if (m_pHost)
	{
		float percentDone = m_pRenderer->calculateProgress();

		m_pHost->progressChanged(percentDone);
	}
}

// do extra channels that shouldn't be anti-aliased...
void Raytracer::processExtraChannels(RenderTask* pTask, unsigned int threadID) const
{
	OutputImageTile* pOurImage = m_aThreadTempImages[threadID];

	if (m_extraChannels)
	{
		unsigned int startX = pTask->getStartX();
		unsigned int startY = pTask->getStartY();

		unsigned int tileWidth = pTask->getWidth();
		unsigned int tileHeight = pTask->getHeight();

		for (unsigned int y = 0; y < tileHeight; y++)
		{
			unsigned int pixelYPos = y + startY;
			float fPixelYPos = (float)pixelYPos + 0.5f;

			for (unsigned int x = 0; x < tileWidth; x++)
			{
				unsigned int pixelXPos = x + startX;
				float fPixelXPos = (float)pixelXPos + 0.5f;

				Ray viewRay = m_pCameraRayCreator->createBasicCameraRay(fPixelXPos, fPixelYPos);

				if (viewRay.type == RAY_UNDEFINED)
					continue;

				float depth;
				HitResult hitResult = processRayExtra(viewRay, depth);

				if (m_extraChannels & COMPONENT_DEPTH)
					pOurImage->setDepthAt(x, y, depth);

				if (m_extraChannels & COMPONENT_NORMAL)
				{
					Colour3f normal(hitResult.shaderNormal.x, hitResult.shaderNormal.y, hitResult.shaderNormal.z);
					pOurImage->setNormalAt(x, y, normal);
				}

				if (m_extraChannels & COMPONENT_WPP)
				{
					Colour3f wpp(hitResult.hitPoint.x, hitResult.hitPoint.y, hitResult.hitPoint.z);
					pOurImage->setWPPAt(x, y, wpp);
				}
			}
		}

		// copy our finished tile into the output image
		m_pOutputImage->copyExtraTile(startX, startY, tileWidth, tileHeight, 0, 0, *pOurImage);
	}
}

HitResult Raytracer::processRayExtra(Ray& viewRay, float& t) const
{
	HitResult hitResult;

	t = viewRay.tMax;

	viewRay.calculateInverseDirection();

	const Object* pHitObject = NULL;

	bool didHit = m_scene.didHitObject(viewRay, t, hitResult);

	if (didHit)
	{
		// get the actual hitObject from the hitResult so that if a compound object with sub-objects
		// was hit, we get the correct material
		pHitObject = hitResult.pObject;

		const Material* pMaterial = pHitObject->getMaterial();
		assert(pMaterial);

		BakedBSDF availableBSDF;

		// shade the material to get the shader normal at the surface
		pMaterial->shade(viewRay, hitResult, &availableBSDF);
	}
	else
	{
		t = 0.0f;
	}

	return hitResult;
}

void Raytracer::updateCameraRayCreator()
{
	// initialise the CameraRayCreator from the RenderCamera, replacing any existing one
	if (m_pCameraRayCreator)
	{
		delete m_pCameraRayCreator;
		m_pCameraRayCreator = NULL;
	}

	// use the overall image to initialise the camera projection plane
	m_pCameraRayCreator = m_pRenderCamera->createCameraRayCreator(m_width, m_height, OutputContext::instance().getFrame(),
																  m_depthOfField, m_motionBlur);

	if (GlobalContext::instance().getTextureCachingType() != GlobalContext::eTextureCachingNone)
	{
		m_pCameraRayCreator->setCreateDifferentials(true);
	}
}

size_t Raytracer::getRendererMemoryUsage() const
{
	size_t memSize = 0;
	memSize += sizeof(*this);

	std::vector<OutputImageTile*>::const_iterator it = m_aThreadTempImages.begin();
	for (; it != m_aThreadTempImages.end(); ++it)
	{
		memSize += (*it)->getMemorySize();
	}

	if (m_pOutputImage)
	{
		memSize += m_pOutputImage->getMemorySize();
	}

	return memSize;
}

void Raytracer::setupRenderThreadContextLightSampling()
{
	DistributionDiscrete* pLightDistribution = m_pRenderer->getLightDistribution();
	const LightsAndSamples* pLightsAndSamples = m_pRenderer->getLightsAndSamplesStruct();
	unsigned int numLights = m_pRenderer->getNumLights();

	// create a light sampler class per render thread, which is owned (and needs to be deleted)
	// by the RenderThreadContext

	unsigned int localisedSampleCount = std::min(numLights, 64u);

	System::CPUInfo cpuInfo = System::getCPUInfo();
	bool initOnThreads = cpuInfo.numSockets > 1;
	bool haveInitialisedPerThreadData = false;

	if (initOnThreads)
	{
		RenderThreadInitHelper threadInitHelper(m_numberOfThreads, true, this, &m_scene);

		unsigned int taskLocalisedSampleCount = localisedSampleCount;
		// hacky - make this 0 if type != eLSSampleLightsRadianceLocalised
		if (m_lightSampling != eLSSampleLightsRadianceLocalised)
		{
			taskLocalisedSampleCount = 0;
		}

		if (threadInitHelper.init2(pLightDistribution, pLightsAndSamples, taskLocalisedSampleCount))
		{
			// just need to hook everything up...
			std::map<unsigned int, LightSampler*>& threadResults = threadInitHelper.getResults2();

			// if init2() returned true, then we have the correct number of results for the number of threads we want...
			// so we can just index into the render thread context vector, as it's ordered based on the thread index

			std::map<unsigned int, LightSampler*>::iterator itResult = threadResults.begin();
			for (; itResult != threadResults.end(); ++itResult)
			{
				const unsigned int& threadIndex = itResult->first;
				LightSampler* pLightSampler = itResult->second;

				RenderThreadContext* pRTC = m_aRenderThreadContexts[threadIndex];

				pRTC->setLightSampler(pLightSampler);
			}

			haveInitialisedPerThreadData = true;
		}
		else
		{
			fprintf(stderr, "Threaded init of LightSampler failed.\n");
		}
	}

	if (!haveInitialisedPerThreadData)
	{
		std::vector<RenderThreadContext*>::iterator itRenderThreadContext = m_aRenderThreadContexts.begin();
		for (; itRenderThreadContext != m_aRenderThreadContexts.end(); ++itRenderThreadContext)
		{
			RenderThreadContext* pRTC = *itRenderThreadContext;

			LightSampler* pNewLightSampler = NULL;

			if (m_lightSampling != eLSSampleLightsRadianceLocalised)
			{
				pNewLightSampler = new LightSamplerConstant(pLightDistribution, pLightsAndSamples);
			}
			else
			{
				pNewLightSampler = new LightSamplerLocalised(pLightDistribution, pLightsAndSamples, localisedSampleCount);
			}

			pRTC->setLightSampler(pNewLightSampler);
		}
	}
}

bool Raytracer::doTask(Task* pTask, unsigned int threadID)
{
	if (!pTask)
		return false;

	RenderTask* pThisTask = static_cast<RenderTask*>(pTask);

	bool ret = m_pRenderer->processTask(pThisTask, threadID);
#ifndef IMAGINE_EMBEDDED_MODE
	if (m_pHost && !m_wasCancelled)
#else
	if (m_pHost)
#endif
	{
		RaytracerHost::TileInfo tileInfo;

		tileInfo.x = pThisTask->getStartX();
		tileInfo.y = pThisTask->getStartY();

		tileInfo.width = pThisTask->getWidth();
		tileInfo.height = pThisTask->getHeight();

		tileInfo.tileApronSize = m_tileApronSize;

		m_pHost->tileDone(tileInfo, threadID);
	}

	return ret;
}

} // namespace Imagine
