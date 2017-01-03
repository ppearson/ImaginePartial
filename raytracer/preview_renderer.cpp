/*
 Imagine
 Copyright 2012-2016 Peter Pearson.

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

#include "preview_renderer.h"

#include <algorithm>

#include "raytracer.h"
#include "scene.h"

#include "raytracer/camera_ray_creators/camera_ray_creator.h"

#include "sampling/sample_generator_stratified.h"

#include "raytracer/integrators/path.h"
#include "raytracer/integrators/path_volume.h"

#include "raytracer/accumulators.h"

#include "image/output_image.h"
#include "image/output_image_tile.h"

#include "utils/maths/rng.h"
#include "utils/params.h"
#include "utils/time_counter.h"

namespace Imagine
{

//! preview progressive renderer

template <typename Integrator, typename Accumulator, typename TimeCounter>
bool PreviewRenderer<Integrator, Accumulator, TimeCounter>::processTask(RenderTask* pRTask, unsigned int threadID)
{
	OutputImageTile* pOurImage = this->getThreadTempImage(threadID);

	pOurImage->resetSamples();

	unsigned int startX = pRTask->getStartX();
	unsigned int startY = pRTask->getStartY();

	unsigned int tileWidth = pRTask->getWidth();
	unsigned int tileHeight = pRTask->getHeight();

	uint32_t rngSeed = this->getRndSeed(startX, startY, tileWidth, tileHeight, threadID);
	rngSeed += pRTask->getIterations();
	RNG rng(rngSeed);

	RenderThreadContext* pRenderThreadCtx = this->getRenderThreadContext(threadID);
	ShadingContext shadingContext(pRenderThreadCtx);

	SampleGeneratorStratified sampleGenerator(rng);
	SampleGenerator::SampleGeneratorRequirements sgReq(1, this->m_numLights, this->getBounceLimitOverall());
	sgReq.flags = SampleGenerator::GENERATE_DIRECTION_SAMPLES | SampleGenerator::GENERATE_SURFACE_SAMPLES;
	if (this->hasDepthOfField())
	{
		sgReq.flags |= SampleGenerator::GENERATE_LENS_SAMPLES;
	}
	if (this->hasMotionBlur())
	{
		sgReq.flags |= SampleGenerator::GENERATE_TIME_SAMPLES;
	}
	sampleGenerator.configureSampler(sgReq, this->m_pLights, this->getLightSampleCount());

	Colour4f colour;

	unsigned int iteration = pRTask->getIterations();
	unsigned int giIterations = pRTask->getIterations() + 1;

	const CameraRayCreator* pCamRayCreator = this->getCameraRayCreator();

	// if we're the first iteration, do a draft basic run
	if (iteration == 0 && this->m_iterations > 1)
	{
		IntegratorState draftIntegratorState(*this, this->getScene(), rng);
		float totalAlphaForTile = 0.0f;

		for (unsigned int y = 0; y < tileHeight; y++)
		{
			float fPixelYPos = (float)y + startY + 0.5f;

			for (unsigned int x = 0; x < tileWidth; x++)
			{
				if (!pRTask->isActive())
				{
					pRTask->setDiscard(true);
					return true;
				}

				colour = Colour4f();

				float fPixelXPos = (float)x + startX + 0.5f;

				Ray viewRay = pCamRayCreator->createBasicCameraRay(fPixelXPos, fPixelYPos);

				if (viewRay.type == RAY_UNDEFINED)
					continue;

				SampleBundle samples(fPixelXPos, fPixelYPos);
				sampleGenerator.generateSampleBundle(samples);

				colour += this->m_integrator.processRay(*pRenderThreadCtx, shadingContext, viewRay, draftIntegratorState, samples, 0);

				pOurImage->colourAt(x, y) = colour;
				pOurImage->setSamplesAt(x, y, 1.0f);

				totalAlphaForTile += colour.a;
			}
		}

		// copy our finished tile into the output image
		this->getOutputImage()->addColourTile(startX - this->getRenderWindowX(), startY - this->getRenderWindowY(),
												  tileWidth, tileHeight, 0, 0, *pOurImage);

		if ((!this->hasDepthOfField() && !this->hasMotionBlur()) && totalAlphaForTile == 0.0f)
		{
			return true; // didn't hit anything, so conservatively, we might as well not re-add this, as there's probably no objects
						 // to hit next time, and it'll save us sending out and intersecting rays for this tile for the next iterations...
		}
		else if (giIterations < this->m_iterations && pRTask->isActive())
		{
			pRTask->incrementIterations();
			return false; // we want the thread pool to re-use it
		}
		else
		{
			return true; // we've finished with it...
		}
	}

	// do extra channels that can't be anti-aliased or averaged
	if (!pRTask->extraChannelsDone())
	{
		this->m_raytracer.processExtraChannels(pRTask, threadID);
		pRTask->setExtraChannelsDone(true);
	}

	sgReq.samplesPerPixel = this->m_globalIlluminationSamplesPerIteration;
	sampleGenerator.configureSampler(sgReq, this->m_pLights, this->getLightSampleCount());

	float fSamples = (float)this->m_globalIlluminationSamplesPerIteration;
	IntegratorState integratorState(*this, this->getScene(), rng);

	for (unsigned int y = 0; y < tileHeight; y++)
	{
		float fPixelYPos = (float)y + startY;

		for (unsigned int x = 0; x < tileWidth; x++)
		{
			if (!pRTask->isActive())
			{
				pRTask->setDiscard(true);
				return true;
			}

			colour = Colour4f();

			float fPixelXPos = (float)x + startX;

			SampleBundle samples(fPixelXPos, fPixelYPos);
			sampleGenerator.generateSampleBundle(samples);

			for (unsigned int sample = 0; sample < this->m_globalIlluminationSamplesPerIteration; sample++)
			{
				const Sample2D& samplePos = this->m_aCameraSamples[iteration - 1].get2DSample(sample);

				float pixelXPos = fPixelXPos + samplePos.x;
				float pixelYPos = fPixelYPos + samplePos.y;

				Ray viewRay = pCamRayCreator->createCameraRay(pixelXPos, pixelYPos, samples, sample);

				if (viewRay.type == RAY_UNDEFINED)
					continue;

				colour += this->m_integrator.processRay(*pRenderThreadCtx, shadingContext, viewRay, integratorState, samples, sample);
			}

			pOurImage->colourAt(x, y) = colour;
			pOurImage->setSamplesAt(x, y, fSamples);
		}
	}

	// copy our finished tile into the output image
	this->getOutputImage()->addColourTile(startX - this->getRenderWindowX(), startY - this->getRenderWindowY(),
											  tileWidth, tileHeight, 0, 0, *pOurImage);

	if (giIterations < this->m_iterations)
	{
		pRTask->incrementIterations();

		return false; // we want the thread pool to re-use it
	}

	return true; // we've finished with the task
}

template bool PreviewRenderer<Path, Colour4fStandard, TimerCounter>::processTask(RenderTask* pRTask, unsigned int threadID);
template bool PreviewRenderer<PathVolume, Colour4fStandard, TimerCounter>::processTask(RenderTask* pRTask, unsigned int threadID);

template bool PreviewRenderer<Path, Colour4fStandard, TimerCounterNull>::processTask(RenderTask* pRTask, unsigned int threadID);
template bool PreviewRenderer<PathVolume, Colour4fStandard, TimerCounterNull>::processTask(RenderTask* pRTask, unsigned int threadID);

} // namespace Imagine
