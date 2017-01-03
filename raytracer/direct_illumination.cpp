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

#include "direct_illumination.h"

#include "raytracer.h"
#include "scene.h"

#include "materials/material.h"

#include "bsdfs/baked_bsdf.h"

#include "raytracer/camera_ray_creators/camera_ray_creator.h"
#include "raytracer/render_thread_context.h"
#include "raytracer/accumulators.h"

#include "lights/light.h"

#include "image/output_image.h"
#include "image/output_image_tile.h"

#include "sampling/sample_generator_stratified.h"
#include "sampling/sampler_stratified.h"

#include "utils/maths/rng.h"
#include "utils/params.h"

namespace Imagine
{

#define USE_SAMPLED_AO 1

// old original raytracer, which is effectively direct lighting + ambient occlusion

template <typename Accumulator>
DirectIllumination<Accumulator>::DirectIllumination(Raytracer& rt, const Params& settings) : Renderer(rt, settings),
	m_rtAmbientOcclusion(&getScene(), &rt),
	m_ambientOnly(false), m_totalTasks(0), m_tasksDone(0), m_roughSampleBundle(0.0f, 0.0f)
{
	uint32_t rngSeed = getTimeSeed();

	// for preview, make things deterministic, otherwise we get flickering as we scrub the sliders in Material Editor
	// due to camera samples being random..
	rngSeed = 42;

	RNG rng(rngSeed);

	m_antiAliasing = settings.getUInt("antiAliasing");
	m_samplesPerPixel = m_antiAliasing * m_antiAliasing;
	m_antiAliasingSamples = (float)m_samplesPerPixel;

	// configure antialiasing class if necessary
	if (m_antiAliasing > 1)
	{
		SamplerStratified cameraSampleGenerator;
		cameraSampleGenerator.generateSamples(m_cameraSamples, m_samplesPerPixel, rng);
	}

	m_invSamplesPerIt = 1.0f / m_samplesPerPixel;

	m_diffuseReflection = settings.getBool("diffuseReflection");
	if (m_diffuseReflection)
	{
		m_diffuseReflectionSamples = settings.getUInt("diffuseReflectionSamples");
		m_invDiffReflectionSamples = 1.0f / (float)m_diffuseReflectionSamples;
		m_invTotalDiffReflectionSamples = 1.0f / (float)(m_diffuseReflectionSamples * m_diffuseReflectionSamples);
		m_cachedSampleIncrements.resize(m_diffuseReflectionSamples);
		for (unsigned int i = 0; i < m_diffuseReflectionSamples; i++)
		{
			m_cachedSampleIncrements[i] = float(i) * m_invDiffReflectionSamples;
		}
	}

	m_ambientOcclusion = settings.getBool("ambientOcclusion");
	m_ambientOcclusionSamples = 0;
	if (m_ambientOcclusion)
	{
		m_ambientOcclusionSamples = settings.getUInt("ambientOcclusionSamples");
		m_rtAmbientOcclusion.setSampleCount(m_ambientOcclusionSamples);
		m_rtAmbientOcclusion.setDistanceAttenuation(settings.getFloat("ambientOcclusionDistanceAttenuation"));
	}
}

template DirectIllumination<Colour4fStandardNoSamples>::DirectIllumination(Raytracer& rt, const Params& settings);
template DirectIllumination<Colour4fFilteredNoSamples>::DirectIllumination(Raytracer& rt, const Params& settings);

template <typename Accumulator>
void DirectIllumination<Accumulator>::initialise()
{
	// if no lights, assume we want ambient occlusion, so skip direct illumination states...
	if (m_numLights == 0)
	{
		m_ambientOnly = true;
	}

	unsigned int tasks = getNumTasks();

	m_totalTasks = tasks * 2;

	if (m_antiAliasing > 1)
		m_totalTasks += tasks;

	if (m_numLights == 0 || !m_raytracer.isProgressive())
	{
		m_totalTasks = tasks;
	}
	else
	{
		// the initial draft centre rays don't *really* need to be counted here, but seeing as they are tasks
		// they need to be for the percentage calculation to work...
//		m_tasksDone += -tasks;
	}
}

template <typename Accumulator>
bool DirectIllumination<Accumulator>::processTask(RenderTask* pRTask, unsigned int threadID)
{
	if (m_raytracer.isProgressive())
	{
		return doProgressiveTask(pRTask, threadID);
	}
	else
	{
		return doFullTask(pRTask, threadID);
	}
}

template <typename Accumulator>
bool DirectIllumination<Accumulator>::doProgressiveTask(RenderTask* pTask, unsigned int threadID)
{
	OutputImageTile* pOurImage = m_raytracer.m_aThreadTempImages[threadID];
	pOurImage->resetSamples();

	unsigned int startX = pTask->getStartX();
	unsigned int startY = pTask->getStartY();

	unsigned int tileWidth = pTask->getWidth();
	unsigned int tileHeight = pTask->getHeight();

	uint32_t rngSeed = getTimeSeed() - (threadID * tileWidth) - threadID - startX - startY;
	rngSeed += pTask->getState();
	rngSeed += pTask->getIterations();
	RNG rng(rngSeed);

	RenderThreadContext* pRenderThreadCtx = getRenderThreadContext(threadID);
	ShadingContext shadingContext(pRenderThreadCtx);

	TileState currentState = pTask->getState();

	SampleGeneratorStratified sampleGenerator(rng);
	unsigned int samplesPP = currentState <= eTSInitial ? 1 : m_samplesPerPixel;
	SampleGenerator::SampleGeneratorRequirements sgReq(samplesPP, m_numLights, getBounceLimitOverall());
	sgReq.flags = this->hasDepthOfField() ? SampleGenerator::GENERATE_LENS_SAMPLES : 0;
#if USE_SAMPLED_AO
	if (m_ambientOcclusion)
	{
		sgReq.flags |= SampleGenerator::GENERATE_AO_SAMPLES;
		sgReq.aoSamples = m_ambientOcclusionSamples * m_ambientOcclusionSamples;
	}
#endif
	sampleGenerator.configureSampler(sgReq, m_pLights, this->getLightSampleCount());

	const CameraRayCreator* pCamRayCreator = this->getCameraRayCreator();

	if (currentState == eTSBlank) // currently blank
	{
		unsigned int centreX = tileWidth / 2;
		unsigned int centreY = tileHeight / 2;

		unsigned int pixelXPos = centreX + startX;
		unsigned int pixelYPos = m_raytracer.m_height - (centreY + startY); // flip the image upside-down

		Ray viewRay = pCamRayCreator->createBasicCameraRay((float)(pixelXPos), (float)(pixelYPos));

		SampleBundle samples(0.0f, 0.0f);
		sampleGenerator.generateSampleBundle(samples);

		PathState pathState(getBounceLimitOverall());

		Colour4f colour = processRayRecurse(*pRenderThreadCtx, shadingContext, viewRay, RENDER_NORMAL, pathState, rng, samples, 0);

		m_raytracer.m_pOutputImage->setTileColour(startX, startY, tileWidth, tileHeight, colour);
		m_raytracer.m_pOutputImage->setTileSamples(startX, startY, tileWidth, tileHeight, 1.0f);

		pTask->incrementState();

		return false; // we want to re-use the task
	}
	else if (currentState == eTSInitial) // initial centre ray has been fired
	{
		for (unsigned int y = 0; y < tileHeight; y++)
		{
			float pixelYPos = (float)(y + startY) + 0.5f;

			for (unsigned int x = 0; x < tileWidth; x++)
			{
				float pixelXPos = (float)(x + startX) + 0.5f;

				SampleBundle samples(pixelXPos, pixelYPos);
				sampleGenerator.generateSampleBundle(samples);

				Ray viewRay = pCamRayCreator->createBasicCameraRay((float)(pixelXPos), (float)(pixelYPos));

				PathState pathState(getBounceLimitOverall());

				Colour4f colour = processRayRecurse(*pRenderThreadCtx, shadingContext, viewRay, RENDER_NORMAL, pathState, rng, samples, 0);

				pOurImage->colourAt(x, y) = colour;
			}
		}

		m_raytracer.processExtraChannels(pTask, threadID);

		m_raytracer.m_pOutputImage->setTileSamples(startX, startY, tileWidth, tileHeight, 1.0f);
		m_raytracer.m_pOutputImage->copyColourTile(startX - m_raytracer.m_renderWindowX, startY - m_raytracer.m_renderWindowY,
												   tileWidth, tileHeight, 0, 0, *pOurImage);

		pTask->incrementState();

		return false; // re-use the task again
	}
	else if (currentState == eTSDraft) // un-anti-aliased tile has been rendered
	{
		// do anti-aliasing if we've been asked to
		if (m_antiAliasing > 1)
		{
			for (unsigned int y = 0; y < tileHeight; y++)
			{
				float pixelYPos = (float)(y + startY);

				for (unsigned int x = 0; x < tileWidth; x++)
				{
					float pixelXPos = (float)(x + startX);

					SampleBundle samples(pixelXPos, pixelYPos);
					sampleGenerator.generateSampleBundle(samples);

					Colour4f colour;
					for (unsigned int sample = 0; sample < m_samplesPerPixel; sample++)
					{
						const Sample2D& samplePos = m_cameraSamples.get2DSample(sample);

						float fPixelXPos = pixelXPos + samplePos.x;
						float fPixelYPos = pixelYPos + samplePos.y;

						Ray viewRay = pCamRayCreator->createBasicCameraRay(fPixelXPos, fPixelYPos);

						PathState pathState(getBounceLimitOverall());

						colour += processRayRecurse(*pRenderThreadCtx, shadingContext, viewRay, RENDER_NORMAL, pathState, rng, samples, sample);
					}

					pOurImage->colourAt(x, y) = colour;
					pOurImage->setSamplesAt(x, y, m_antiAliasingSamples);
				}
			}

			// copy our finished tile into the output image
			m_raytracer.m_pOutputImage->addColourTile(startX, startY, tileWidth, tileHeight, 0, 0, *pOurImage);
		}

		pTask->incrementState();

		return false; // re-use the task again
	}
	else
	{
		// if we're not doing ambient occlusion, abort out early
		if (!m_ambientOcclusion)
			return true;

		for (unsigned int y = 0; y < tileHeight; y++)
		{
			float pixelYPos = (float)(y + startY);

			for (unsigned int x = 0; x < tileWidth; x++)
			{
				float pixelXPos = (float)(x + startX);

				Colour4f colour;

				SampleBundle samples(pixelXPos, pixelYPos);
				sampleGenerator.generateSampleBundle(samples);

				if (m_antiAliasing == 1) // no anti-aliasing
				{
					Ray viewRay = pCamRayCreator->createBasicCameraRay(pixelXPos, pixelYPos);

					PathState pathState(getBounceLimitOverall());
					colour = processRayRecurse(*pRenderThreadCtx, shadingContext, viewRay, RENDER_AMBIENT_OCCLUSION, pathState, rng, samples, 0);
				}
				else
				{
					for (unsigned int sample = 0; sample < m_samplesPerPixel; sample++)
					{
						const Sample2D& samplePos = m_cameraSamples.get2DSample(sample);

						float fPixelXPos = pixelXPos + samplePos.x;
						float fPixelYPos = pixelYPos + samplePos.y;

						Ray viewRay = pCamRayCreator->createCameraRay(fPixelXPos, fPixelYPos, samples, sample);

						if (viewRay.type == RAY_UNDEFINED)
							continue;

						PathState pathState(getBounceLimitOverall());

						colour += processRayRecurse(*pRenderThreadCtx, shadingContext, viewRay, RENDER_AMBIENT_OCCLUSION, pathState, rng, samples, sample);
					}

					colour *= m_invSamplesPerIt;
				}

				pOurImage->colourAt(x, y) = colour;
				// this isn't quite right, but it ensures that added colour from the AO pass doesn't get averaged away
				// in the shadow areas of the direct lighting colour data already in the output image
				pOurImage->setSamplesAt(x, y, m_ambientOnly ? 1.0f : 0.0f);
			}
		}

		// copy our finished tile into the output image
		m_raytracer.m_pOutputImage->addColourTile(startX, startY, tileWidth, tileHeight, 0, 0, *pOurImage);
	}

	// we're finished with the task, so return true
	return true;
}

#define USE_ACCUMULATOR 0

template <typename Accumulator>
bool DirectIllumination<Accumulator>::doFullTask(RenderTask* pTask, unsigned int threadID)
{
	OutputImageTile* pOurImage = m_raytracer.m_aThreadTempImages[threadID];

	unsigned int startX = pTask->getStartX();
	unsigned int startY = pTask->getStartY();

	unsigned int tileWidth = pTask->getWidth();
	unsigned int tileHeight = pTask->getHeight();

	uint32_t rngSeed = m_raytracer.m_timeSeed - (threadID * tileWidth) - startX - startY;
	RNG rng(rngSeed);

	RenderThreadContext* pRenderThreadCtx = getRenderThreadContext(threadID);
	ShadingContext shadingContext(pRenderThreadCtx);

	// generate the samples we need
	SampleGeneratorStratified sampleGenerator(rng);
	SampleGenerator::SampleGeneratorRequirements sgReq(m_samplesPerPixel, m_numLights, getBounceLimitOverall());
	sgReq.flags = this->hasDepthOfField() ? SampleGenerator::GENERATE_LENS_SAMPLES : 0;
#if USE_SAMPLED_AO
	if (m_ambientOcclusion)
	{
		sgReq.flags |= SampleGenerator::GENERATE_AO_SAMPLES;
		sgReq.aoSamples = m_ambientOcclusionSamples * m_ambientOcclusionSamples;
	}
#endif
	sampleGenerator.configureSampler(sgReq, m_pLights, this->getLightSampleCount());

	Colour4f colour;

	const CameraRayCreator* pCamRayCreator = this->getCameraRayCreator();

	if (m_samplesPerPixel == 1)
	{
		for (unsigned int y = 0; y < tileHeight; y++)
		{
			float pixelYPos = (float)(y + startY);

			for (unsigned int x = 0; x < tileWidth; x++)
			{
				float pixelXPos = (float)(x + startX);

				colour = Colour4f();

				SampleBundle samples(pixelXPos, pixelYPos);
				sampleGenerator.generateSampleBundle(samples);

				float fPixelXPos = pixelXPos + 0.5f;
				float fPixelYPos = pixelYPos + 0.5f;

				Ray viewRay = pCamRayCreator->createBasicCameraRay(fPixelXPos, fPixelYPos);

				PathState pathState(getBounceLimitOverall());

				colour += processRayRecurse(*pRenderThreadCtx, shadingContext, viewRay, RENDER_ALL, pathState, rng, samples, 0);

				pOurImage->colourAt(x, y) = colour;
			}
		}
	}
	else
	{
		for (unsigned int y = 0; y < tileHeight; y++)
		{
			float pixelYPos = (float)(y + startY);

			for (unsigned int x = 0; x < tileWidth; x++)
			{
				if (!pTask->isActive())
					return true;

				float pixelXPos = (float)(x + startX);

				colour = Colour4f();

				SampleBundle samples(pixelXPos, pixelYPos);
				sampleGenerator.generateSampleBundle(samples);

				for (unsigned int sample = 0; sample < m_samplesPerPixel; sample++)
				{
					const Sample2D& samplePos = m_cameraSamples.get2DSample(sample);

					float fPixelXPos = pixelXPos + samplePos.x;
					float fPixelYPos = pixelYPos + samplePos.y;

					Ray viewRay = pCamRayCreator->createCameraRay(fPixelXPos, fPixelYPos, samples, sample);

					PathState pathState(getBounceLimitOverall());

					Colour4f localColour = processRayRecurse(*pRenderThreadCtx, shadingContext, viewRay, RENDER_ALL, pathState, rng, samples, sample);
#if USE_ACCUMULATOR
					m_accumulator.accumulateSample(localColour, pOurImage, x, y, samplePos);
					m_accumulator.addColour(localColour, colour);
#else
					colour += localColour;
#endif
				}

#if USE_ACCUMULATOR
				m_accumulator.setTilePixelColour(colour, m_invSamplesPerIt, 1.0f, pOurImage, x, y);
#else
				colour *= m_invSamplesPerIt;

				pOurImage->colourAt(x, y) = colour;
#endif
			}
		}
	}

	m_raytracer.processExtraChannels(pTask, threadID);
#if USE_ACCUMULATOR
	m_accumulator.addTileToOutputImage(pOurImage, getOutputImage(), startX - getRenderWindowX(), startY - getRenderWindowY(),
									   tileWidth, tileHeight);
#else
	// copy our finished tile into the output image
	m_raytracer.m_pOutputImage->copyColourTile(startX - m_raytracer.m_renderWindowX, startY - m_raytracer.m_renderWindowY,
											   tileWidth, tileHeight, 0, 0, *pOurImage);
#endif

	return true;
}

template <typename Accumulator>
Colour4f DirectIllumination<Accumulator>::processRayRecurse(RenderThreadContext& rtc, ShadingContext& shadingContext, const Ray& ray,
															unsigned int flags, PathState& pathState, RNG& rng, SampleBundle& samples, unsigned int sampleIndex)
{
	Colour4f colour;

	HitResult hitResult;
	hitResult.setShadingContext(shadingContext);

	float t = ray.tMax;

	Ray localRay(ray);
	localRay.tMin = std::max(getRayEpsilon(), localRay.tMin);

	const Object* pHitObject;

	bool didHit = m_raytracer.m_scene.didHitObject(localRay, t, hitResult);

	// if we didn't hit anything, skip
	if (!didHit)
	{
		return colour;
	}

	// get the actual hitObject from the hitResult so that if a compound object with sub-objects
	// was hit, we get the correct material
	pHitObject = hitResult.pObject;

	const Material* pMaterial = pHitObject->getMaterial();

	BakedBSDF availableBSDF;

	// shade the material to get the shader normal at the surface
	pMaterial->shade(localRay, hitResult, &availableBSDF);

	Point lastRayIntersection = hitResult.hitPoint;

	if (flags & RENDER_AMBIENT_OCCLUSION && m_ambientOcclusion)
	{
		Colour3f ambMatColour = pMaterial->ambientSample(hitResult);
		ambMatColour *= 0.9f;

		ambMatColour *= m_raytracer.m_ambientColour;
#if USE_SAMPLED_AO
		float occlusion = m_rtAmbientOcclusion.getOcclusionAtPointExistingSamples(hitResult, samples, sampleIndex);
#else
		float occlusion = m_rtAmbientOcclusion.getOcclusionAtPoint(hitResult, rng);
#endif
		ambMatColour *= (1.0f - occlusion);

		colour += ambMatColour;
	}
/*	else
	{
		// ambient lighting without any occlusion testing
		Colour3f ambMatColour = pMaterial->ambientSample(hitResult);
		ambMatColour *= 0.7f;

		ambMatColour *= m_raytracer.m_ambientColour;

		colour += ambMatColour;
	}
*/

	unsigned int bsdfEvalFlags = getBSDFEvalFlags();

	if (flags & RENDER_NORMAL && m_numLights)
	{
		colour += Renderer::calculateFinalLighting(rtc, shadingContext, hitResult, localRay, samples, sampleIndex, pathState.bounceLevel, bsdfEvalFlags);
	}

	float reflection = pMaterial->getReflection();
	if (reflection > 0.0001f && pathState.bounceLevel < pathState.bounceLimit)
	{
		float reflect = Normal::dot(ray.direction, hitResult.shaderNormal) * 2.0f;

		Normal reflectedDirection = hitResult.shaderNormal;
		reflectedDirection *= reflect;
		Normal newDirection = ray.direction - reflectedDirection;

		float diffReflection = 1.0f - pMaterial->getGloss();
		if (!m_diffuseReflection || diffReflection < 0.02f)
		{
			// perfect specular reflection
			Ray reflectedRay(lastRayIntersection, newDirection, RAY_REFLECTION);
			reflectedRay.tMin = getRayEpsilon() * hitResult.intersectionError;

			pathState.bounceLevel += 1;

			Colour4f reflectedColour = processRayRecurse(rtc, shadingContext, reflectedRay, flags, pathState, rng, samples, sampleIndex);
			reflectedColour *= reflection;
			colour += reflectedColour;
		}
		else
		{
			// diffuse reflection
			Colour4f reflectedColour;

			float deltaMin = 0.5f - diffReflection;
			float deltaMax = 0.5f + diffReflection;
			float deltaDiff = deltaMax - deltaMin;

			float jitterDelta = deltaDiff * m_invDiffReflectionSamples;

			for (unsigned int xInd = 0; xInd < m_diffuseReflectionSamples; xInd++)
			{
				float xFraction = m_cachedSampleIncrements[xInd] + deltaMin;
				for (unsigned int yInd = 0; yInd < m_diffuseReflectionSamples; yInd++)
				{
					float yFraction = m_cachedSampleIncrements[yInd] + deltaMin;

					float xSample = xFraction + rng.randomFloat(0.0f, jitterDelta);
					float ySample = yFraction + rng.randomFloat(0.0f, jitterDelta);

//					Normal sampleDirection = uniformSampleHemisphereN(xSample, ySample, newDirection);
					Normal rn1 = Normal(newDirection.z, newDirection.y, -newDirection.x);
					Normal rn2 = Normal::cross(newDirection, rn1);

					Normal sd1 = newDirection + rn1 * xSample * diffReflection;
					Normal sd2 = rn2 * ySample * diffReflection;

					Normal sampleDirection = sd1 + sd2;
					sampleDirection.normalise();

					Point sampleStartPos = lastRayIntersection;
//					sampleStartPos += sampleDirection * 0.1f;

					Ray reflectedRay(sampleStartPos, sampleDirection, RAY_GLOSSY);
					reflectedRay.tMin = getRayEpsilon() * hitResult.intersectionError;

					pathState.bounceLevel += 1;
					// TODO: seem to be getting recursion we shouldn't be here, even after offsetting the startpos slightly....
					Colour4f reflectedColour1 = processRayRecurse(rtc, shadingContext, reflectedRay, flags, pathState, rng, samples, sampleIndex);
					reflectedColour += reflectedColour1;
				}
			}

			reflectedColour *= m_invTotalDiffReflectionSamples;
			reflectedColour *= reflection;
			colour += reflectedColour;
		}
	}

	float transparency = pMaterial->getTransparency();
	if (transparency > 0.01f && pathState.bounceLevel < pathState.bounceLimit)
	{
		float newRefractionIndex = pMaterial->getRefractionIndex();

		float n = pathState.lastRefractionIndex / newRefractionIndex;

		Ray refractedRay(lastRayIntersection, ray.direction, RAY_REFRACTION);
		refractedRay.tMin = getRayEpsilon() * hitResult.intersectionError;
		if (n != 1.0f)
		{
			float c1 = Normal::dot(ray.direction, hitResult.shaderNormal);
			float c2 = sqrtf(1.0f - (n * n) * (1.0f - (c1 * c1)));

			Normal refractedDirection = ray.direction * n;
			Normal nee = hitResult.shaderNormal;
			nee *= (n * c1 - c2);
			refractedDirection += nee;

			refractedRay.direction = refractedDirection;
		}

		pathState.bounceLevel += 1;
		pathState.lastRefractionIndex = newRefractionIndex;

		Colour4f refractedColour = processRayRecurse(rtc, shadingContext, refractedRay, flags, pathState, rng, samples, sampleIndex);
		refractedColour *= transparency;
		colour += refractedColour;
	}

	// TODO: could probably combine this with above, but for legacy's sake, let's leave it as is currently...

	// see if it should be transmissive (transparent) - because this is direct lighting, we're assuming only a few rays per pixel
	// so we can't use stochastic transparency, so need to use continuation rays
	bool continueThroughObject = false;
	float alphaValue = 1.0f;
	if (pHitObject->hasFlag(OBJECT_FLAG_HAS_ALPHA_TEXTURE))
	{
		// we should only reach here if alpha was non-0.0f - areas with 0.0f would not have registered
		// as hit events
		const Texture* pAlphaTexture = pMaterial->getAlphaTexture();
		if (pAlphaTexture)
		{
			continueThroughObject = true;
			alphaValue = pAlphaTexture->getFloatBlend(hitResult, 0);
		}
	}

	if (continueThroughObject)
	{
		Ray continuationRay(hitResult.hitPoint, ray.direction, RAY_REFRACTION);

		Colour4f continuedColour = processRayRecurse(rtc, shadingContext, continuationRay, flags, pathState, rng, samples, sampleIndex);

		// assume disassociated alpha for the moment
		continuedColour *= (1.0f - alphaValue);
		colour += continuedColour;

		colour.a += alphaValue;
	}
	else
	{
		// this needs to go last
		colour.a = 1.0f;
	}

	return colour;
}

template <typename Accumulator>
float DirectIllumination<Accumulator>::calculateProgress() const
{
	m_tasksDone++;

	float percentDone = 0.0f;

	if (!m_raytracer.isProgressive())
	{
		unsigned int activeThreads = m_raytracer.m_controller.threadsActive();
		float tasksRemaining = (float)(m_raytracer.m_aTasks.size() + activeThreads);

		percentDone = ((m_raytracer.m_fOriginalNumberOfTasks - tasksRemaining) * m_raytracer.m_invOriginalNumTasks) * 100.0f;
	}
	else
	{
		percentDone = ((float)m_tasksDone / (float)m_totalTasks) * 100.0f;
	}

	return percentDone;
}

} // namespace Imagine
