/*
 Imagine
 Copyright 2011-2017 Peter Pearson.

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

#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <vector>
#include <string>

#include "utils/threads/thread_pool.h"

#include "scene_common.h"

#include "colour/colour3f.h"
#include "colour/colour4f.h"

#include "raytracer_common.h"

#include "remote/render_client_job_manager.h"

namespace Imagine
{

class Object;
class SceneInterface;
class Camera;
class CameraRayCreator;
class Renderer;
class TileTaskGenerator;
class Params;
class OutputImage;
class OutputImageTile;
class Filter;
class Texture;
class RendererBackground;
class SampleGeneratorFactory;

class ImageTextureCache;

class RemoteState;

class RenderThreadContext;

class DebugPathCollection;

class RenderTask : public ThreadPoolTask
{
public:
	RenderTask(unsigned int startX, unsigned int startY, unsigned int width, unsigned int height, TileState state, unsigned int taskIndex);
	virtual ~RenderTask();

	unsigned int getStartX() { return m_startX; }
	unsigned int getStartY() { return m_startY; }

	unsigned int getWidth() { return m_width; }
	unsigned int getHeight() { return m_height; }

	TileState getState() const { return m_state; }
	void incrementState() { m_state = (TileState)((int)m_state + 1); }

	bool extraChannelsDone() const { return m_extraChannelsDone; }
	void setExtraChannelsDone(bool extraDone) { m_extraChannelsDone = extraDone; }

	unsigned int getIterations() const { return m_iterationCount; }
	void incrementIterations() { m_iterationCount++; }

	unsigned int getTaskIndex() const { return m_taskIndex; }

	bool shouldDiscard() const { return m_discard; }
	void setDiscard(bool discard) { m_discard = discard; }



protected:
	TileState		m_state;
	unsigned int	m_startX;
	unsigned int	m_startY;

	unsigned int	m_width;
	unsigned int	m_height;

	bool			m_extraChannelsDone;

	unsigned int	m_iterationCount;

	unsigned int	m_taskIndex;

	bool			m_discard;
};

class Raytracer : public ThreadPool
{
public:
	Raytracer(SceneInterface& scene, OutputImage* outputImage, Params& settings, bool preview, unsigned int threads);
	Raytracer(SceneInterface& scene, unsigned int threads, bool progressive);
	virtual ~Raytracer();

	friend class Renderer;
	template <typename Accumulator>
	friend class DirectIllumination;
	friend class PathTracer;
	friend class PathTracerDeep;

	// this is for re-using an existing Raytracer
	void initialise(OutputImage* outputImage, const Params& settings, bool isReRender = false);
	void initRendererAndIntegrator(const Params& settings);

	void setExtraChannels(unsigned int extraChannels) { m_extraChannels = extraChannels; }
	unsigned int getExtraChannels() const { return m_extraChannels; }

	void setHost(RaytracerHost* host) { m_pHost = host; }

	void setRemoteRenderSettings(const RemoteState& remoteRenderState);

	void configureBackground();
	void setAmbientColour(Colour3f colour) { m_ambientColour = colour; }

	void setPreview(bool preview) { m_preview = preview; }

	void setStatisticsOutputPath(const std::string& statsOutputPath) { m_statsOutputPath = statsOutputPath; }

	// render the scene as an entire image
	void renderScene(float time, const Params* pParams, bool waitForCompletion, bool isRestart = false);

	// render just a single tile - currently used for scanline rendering in Nuke
	void renderTile(OutputImageTile& outputTile, unsigned int x, unsigned int y, unsigned int r, unsigned int t, bool deep);

	void resetForReRender();

	virtual void createTileJobs();

	void taskDone();

	void processExtraChannels(RenderTask* pTask, unsigned int threadID) const;

	HitResult processRayExtra(RenderThreadContext& rtc, ShadingContext& shadingContext, Ray& viewRay, float& t) const;

	void updateCameraRayCreator();

	size_t getRendererMemoryUsage() const;
	float getRayEpsilon() const { return m_rayEpsilon; }

	void setupRenderThreadContextLightSampling();

	bool isProgressive() const { return m_progressive; }

	void setDebugPathCollection(DebugPathCollection* pDPC) { m_pDebugPathCollection = pDPC;}
	DebugPathCollection* getDebugPathCollection() const { return m_pDebugPathCollection; }

protected:
	virtual bool doTask(ThreadPoolTask* pTask, unsigned int threadID);

protected:
	SceneInterface&			m_scene;
	OutputImage*			m_pOutputImage;

	bool					m_useRemoteClients;
	RenderClientJobManager	m_clientJobManager;

	Mutex					m_imageLock;

	Renderer*				m_pRenderer;
	Filter*					m_pFilter;
	unsigned int			m_tileApronSize;

	SampleGeneratorFactory*	m_pSampleGeneratorFactory;

	bool					m_progressive;

	unsigned int			m_extraChannels;

	StatisticsType			m_statsType;
	StatisticsOutputType	m_statsOutputType;
	std::string				m_statsOutputPath;

	// these are used for each thread to write into its own tile, which is then copied to
	// the target image when the tile is complete.
	std::vector<OutputImageTile*>		m_aThreadTempImages;
	std::vector<RenderThreadContext*>	m_aRenderThreadContexts;

	bool					m_preview; // if this is true, timing won't be printed to stderr

	uint32_t				m_timeSeed;

	// we don't own this...
	Camera*					m_pRenderCamera;

	// we own this
	CameraRayCreator*		m_pCameraRayCreator;

	RaytracerHost*			m_pHost;

	ImageTextureCache*		m_pGlobalImageCache;

	RenderBackgroundType	m_backgroundType;
	const RendererBackground*	m_pBackground;
	Colour3f				m_ambientColour;

	unsigned int			m_width;
	unsigned int			m_height;

	unsigned int			m_renderWindowX;
	unsigned int			m_renderWindowY;
	unsigned int			m_renderWindowWidth;
	unsigned int			m_renderWindowHeight;

	//! ray bounce limits
	unsigned int			m_rayBouncesOverall;
	unsigned int			m_rayBouncesDiffuse;
	unsigned int			m_rayBouncesGlossy;
	unsigned int			m_rayBouncesReflection;
	unsigned int			m_rayBouncesRefraction;

	LightSamplingType		m_lightSampling;
	bool					m_sampleLights;
	unsigned int			m_lightSamples;

	bool					m_motionBlur;
	bool					m_depthOfField;

	float					m_rayEpsilon;
	float					m_shadowRayEpsilon;

	unsigned int			m_tileSize;
	unsigned int			m_tileOrder;

	DebugPathCollection*	m_pDebugPathCollection;
};

} // namespace Imagine

#endif // RAYTRACER_H
