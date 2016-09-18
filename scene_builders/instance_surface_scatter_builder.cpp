/*
 Imagine
 Copyright 2013-2014 Peter Pearson.

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

#include "instance_surface_scatter_builder.h"

#include <ctime>

#include "scene.h"

#include "core/quaternion.h"

#include "sampling/sampler_stratified.h"
#include "sampling/sampler_poisson_disc.h"
#include "sampling/sampler_common.h"

#include "objects/compound_object.h"
#include "objects/compound_instance.h"

#include "selection_manager.h"

#include "materials/standard_material.h"

#include "utils/maths/rng.h"

namespace Imagine
{

const char* sscatterDistributionOptions[] = { "Poisson-disc", "Stratified", "Random", "Grid", 0 };

InstanceSurfaceScatterBuilder::InstanceSurfaceScatterBuilder() : m_lastObjectSwitchover(0), m_alternatingMaterials(false), m_numMaterials(3)
{
	m_width = 100.0f;
	m_depth = 100.0f;

	m_raycastStartHeight = 40.0f;

	m_targetInstanceCount = 100;
	m_exactNumber = false;

	m_minimumCutoff = -5.0f;

	m_uniformScaleVariation = 0.0f;

	m_distribution = 0;

	m_surfaceYOffset = 0.0f;

	m_alignToSurface = true;

	m_randomYRotation = false;
	m_drawAsBBox = false;
	m_useBakedInstances = false;
	m_addToGroup = false;

	m_randomlyUseMultipleSelection = false;
}

unsigned char InstanceSurfaceScatterBuilder::getSceneBuilderTypeID()
{
	return 6;
}

std::string InstanceSurfaceScatterBuilder::getSceneBuilderDescription()
{
	return "Instance Surface Scatter";
}

void InstanceSurfaceScatterBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<float, float>("width", "Width", &m_width, eParameterFloat, 0.01f, 30000.0f, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<float, float>("depth", "Depth", &m_depth, eParameterFloat, 0.01f, 30000.0f, eParameterScrubButton));

	parameters.addParameter(new RangeParameter<float, float>("start_height", "Start Height", &m_raycastStartHeight, eParameterFloat, -500.0f, 1000.0f,
															 eParameterScrubButton));

	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("target_instance_count", "Instance count", &m_targetInstanceCount,
																		   eParameterUInt, 1, 50000000, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("exact_number", "Exact number", &m_exactNumber, eParameterBool));

	parameters.addParameter(new EnumParameter("distribution", "distribution", (unsigned char*)&m_distribution, sscatterDistributionOptions));

	parameters.addParameter(new RangeParameter<float, float>("surface_y_offset", "Pos Y Offset", &m_surfaceYOffset, eParameterFloat, -50.0f, 50.0f,
															 eParameterScrubButton));

	parameters.addParameter(new RangeParameter<float, float>("min_cuttoff", "Minimum Cutoff", &m_minimumCutoff, eParameterFloat, -500.0f, 500.0f,
															 eParameterScrubButton));

	parameters.addParameter(new RangeParameter<float, float>("uscale_variation", "Scale variation", &m_uniformScaleVariation, eParameterFloat,
															 0.0f, 5.0f, eParameterScrubButton | eParameterScrubButtonFine));

	parameters.addParameter(new BasicParameter<bool>("align_to_surface", "Align to surface", &m_alignToSurface, eParameterBool));
	parameters.addParameter(new BasicParameter<bool>("random_y_rotation", "Random Y Rotation", &m_randomYRotation, eParameterBool));
	parameters.addParameter(new BasicParameter<bool>("set_to_bbox", "draw as bbox", &m_drawAsBBox, eParameterBool));
	parameters.addParameter(new BasicParameter<bool>("add_to_group", "add to group", &m_addToGroup, eParameterBool));

	parameters.addParameter(new BasicParameter<bool>("use_baked_instances", "Use baked instances", &m_useBakedInstances, eParameterBool));

	parameters.addParameter(new BasicParameter<bool>("randomly_use_multiple_selection", "Randomly pick item",
													 &m_randomlyUseMultipleSelection, eParameterBool));

	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("last_object_switchover", "Last obj. switch", &m_lastObjectSwitchover,
																		   eParameterUInt, 0, 5000000, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("alternating_materials", "alt. materials", &m_alternatingMaterials, eParameterBool));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("num_materials", "number of materials", &m_numMaterials,
																		   eParameterUInt, 1, 20, eParameterScrubButton));

}

void InstanceSurfaceScatterBuilder::createScene(Scene& scene)
{
	TestPosInfo posInfo;

	posInfo.pCurrentSelectedObject = SelectionManager::instance().getMainSelectedObject();
	if (!posInfo.pCurrentSelectedObject)
		return;

	posInfo.numSelectedItems = SelectionManager::instance().getSelection().getSelectionCount();

	posInfo.randomlyPickObject = m_randomlyUseMultipleSelection && (posInfo.numSelectedItems > 1);

	posInfo.doSwitchover = posInfo.randomlyPickObject && m_lastObjectSwitchover > 0;

	posInfo.scaleVariation = m_uniformScaleVariation > 0.0f;

	PreRenderRequirements rendRequirements(true, GeometryInstanceBuildRequirements(eAccelStructureStatusRendering), 1.0f);
	scene.doPreRenders(rendRequirements);

	// work out centre offset from base of bbox
	posInfo.shapeBBox = posInfo.pCurrentSelectedObject->getTransformedBoundaryBox();
	posInfo.heightOffset = posInfo.shapeBBox.getExtent().y * 0.5f;

	if (m_alternatingMaterials && !m_useBakedInstances)
	{
		char szName[16];
		for (unsigned int i = 0; i < m_numMaterials; i++)
		{
			memset(szName, 0, 16);
			sprintf(szName, "IMaterial_%d", i);
			Material* pNewMaterial = new StandardMaterial();
			pNewMaterial->setName(szName);

			scene.getMaterialManager().addMaterial(pNewMaterial);

			posInfo.aMaterials.push_back(pNewMaterial);
		}
	}

	//

	if (m_addToGroup)
	{
		posInfo.pNewCO = new CompoundObject();
	}

	posInfo.shouldMakeBakedInstances = m_useBakedInstances;

	if (posInfo.shouldMakeBakedInstances && posInfo.randomlyPickObject)
	{
		posInfo.aMultipleSrcCO.reserve(posInfo.numSelectedItems);
		for (unsigned int i = 0; i < posInfo.numSelectedItems; i++)
		{
			CompoundObject* pLocalCO = dynamic_cast<CompoundObject*>(SelectionManager::instance().getSelection().getSelectedObjectAtIndex(i));
			if (!pLocalCO)
			{
				posInfo.shouldMakeBakedInstances = false;
				break;
			}

			posInfo.aMultipleSrcCO.push_back(pLocalCO);
		}
	}
	else
	{
		// check that source selected object is actually a Compound Object...
		if (posInfo.shouldMakeBakedInstances && posInfo.pCurrentSelectedObject->getObjectType() == eCollection)
		{
			posInfo.pSrcCOForBakedInstances = dynamic_cast<CompoundObject*>(posInfo.pCurrentSelectedObject);
			if (!posInfo.pSrcCOForBakedInstances)
			{
				posInfo.shouldMakeBakedInstances = false;
			}
		}
		else
		{
			posInfo.shouldMakeBakedInstances = false;
		}
	}

	//

	RNG rng;

	// if we're going for an exact count - i.e. we continue generating samples until we've actually added the required
	// number of instances, it only really makes sense to do this with poisson-disc (maybe random as well in future?)
	// distribution, so use a different method for that

	bool generateExactNumberOfInstances = m_exactNumber && (m_distribution == 0);

	if (generateExactNumberOfInstances)
	{

	}
	else
	{
		// otherwise, do everything up-front

		// generate points
		std::vector<Point> candidateStartPoints;
		generateCandidateStartPoints(candidateStartPoints);

		std::vector<Point>::const_iterator itPoint = candidateStartPoints.begin();
		for (; itPoint != candidateStartPoints.end(); ++itPoint)
		{
			const Point& position = *itPoint;

			if (testPositionPickAndAddObject(scene, position, rng, posInfo))
			{
				posInfo.createdCount ++;
				posInfo.switchCount ++;
			}

			posInfo.testedCount ++;
		}
	}

	if (m_addToGroup)
	{
		posInfo.pNewCO->updateBoundaryBox();

		posInfo.pNewCO->setName("SurfaceScatterCO");

		if (m_drawAsBBox)
			posInfo.pNewCO->setDisplayType(eBoundaryBox);

		posInfo.pNewCO->setType(CompoundObject::eContainer);

		addObject(scene, posInfo.pNewCO);
	}
}

void InstanceSurfaceScatterBuilder::generateCandidateStartPoints(std::vector<Point>& points) const
{
	uint32_t timeSeed = std::clock();

	RNG rng(timeSeed);

	Sample2DPacket samples;

	if (m_distribution == 0)
	{
		SamplerPoissonDisc sampleGenerator;
		sampleGenerator.generateSamples(samples, m_targetInstanceCount, rng);
	}
	else if (m_distribution == 1)
	{
		SamplerStratified sampleGenerator;
		sampleGenerator.generateSamples(samples, m_targetInstanceCount, rng);
	}
	else if (m_distribution == 2)
	{
		// random

		samples.samples.reserve(m_targetInstanceCount);

		for (unsigned int i = 0; i < m_targetInstanceCount; i++)
		{
			float xPos = rng.randomFloat(0.0f, 1.0f);
			float yPos = rng.randomFloat(0.0f, 1.0f);

			samples.samples.push_back(Sample2D(xPos, yPos));
		}
	}

	// now transform 0.0-1.0 sample pos to X,Z scene world space
	std::vector<Sample2D>::const_iterator itSample = samples.samples.begin();
	for (; itSample != samples.samples.end(); ++itSample)
	{
		Sample2D newSample = *itSample;

		newSample.x -= 0.5f;
		newSample.y -= 0.5f;

		newSample.x *= m_width;
		newSample.y *= m_depth;

		points.push_back(Point(newSample.x, m_raycastStartHeight, newSample.y));
	}
}

bool InstanceSurfaceScatterBuilder::testPositionPickAndAddObject(Scene& scene, const Point& position, RNG& rng, TestPosInfo& posInfo)
{
	Ray occlusionRay(position, Normal(0.0f, -1.0f, 0.0f), RAY_ALL);

	// BVH intersectors depend on signed inf values resulting from div-by-0 for axis-aligned rays...
	occlusionRay.calculateInverseDirection();

	HitResult hitResult;
	float t = 1000.0f;

	if (scene.didHitObject(occlusionRay, t, hitResult))
	{
		if (hitResult.hitPoint.y < m_minimumCutoff)
			return false;

		Object* pSrcObject = posInfo.pCurrentSelectedObject;

		Point newPoint = position;
		newPoint.y -= t;

		unsigned int randomIndex = 0;
		if (posInfo.randomlyPickObject)
		{
			// TODO: Given that we're picking a random index AND a random location, we don't get the best
			//       distribution here...

			if (posInfo.doSwitchover)
			{
				if (posInfo.switchCount < m_lastObjectSwitchover)
				{
					randomIndex = rng.randomInt(posInfo.numSelectedItems - 1);
					pSrcObject = SelectionManager::instance().getSelection().getSelectedObjectAtIndex(randomIndex);
				}
				else
				{
					// pick the last object
					randomIndex = posInfo.numSelectedItems - 1;
					pSrcObject = SelectionManager::instance().getSelection().getSelectedObjectAtIndex(randomIndex);
				}
			}
			else
			{
				// just randomly pick...
				randomIndex = rng.randomInt(posInfo.numSelectedItems);
				pSrcObject = SelectionManager::instance().getSelection().getSelectedObjectAtIndex(randomIndex);
			}

			// need to update the height of the object
			BoundaryBox shapeBBox = pSrcObject->getTransformedBoundaryBox();
			posInfo.heightOffset = shapeBBox.getExtent().y * 0.5f;
		}

		float scaleVariation = 1.0f;

		if (posInfo.scaleVariation)
		{
			// scale variation can be larger or smaller than current object, split around the middle
			float halfVariation = m_uniformScaleVariation * 0.5f;
			scaleVariation = 1.0f - halfVariation + (m_uniformScaleVariation * rng.randomFloat());
		}

		float localHeightOffset = posInfo.heightOffset;

		if (posInfo.scaleVariation)
		{
			localHeightOffset *= scaleVariation;
		}

		newPoint.y += localHeightOffset; // centre the object's bbox on the surface that was hit

		// TODO: an option to ratio this based on the surface flatness amount, so that objects on a flat surface
		//       don't have as much as an offset, whereas objects on a steep incline (trees for example) have more of an offset
		newPoint.y += m_surfaceYOffset;

		Object* pNewObject = NULL;

		if (!posInfo.shouldMakeBakedInstances)
		{
			pNewObject = pSrcObject->clone();

			if (m_alternatingMaterials)
			{
				Material* pMat = posInfo.aMaterials[posInfo.altMaterialIndex];

				pNewObject->setMaterial(pMat);

				posInfo.altMaterialIndex++;

				if (posInfo.altMaterialIndex >= m_numMaterials)
					posInfo.altMaterialIndex = 0;
			}
		}
		else
		{
			if (posInfo.randomlyPickObject)
			{
				pNewObject = new CompoundInstance(posInfo.aMultipleSrcCO[randomIndex]);
			}
			else
			{
				pNewObject = new CompoundInstance(posInfo.pSrcCOForBakedInstances);
			}
		}

		pNewObject->setPosition(Vector(newPoint.x, newPoint.y, newPoint.z));

		if (posInfo.scaleVariation)
		{
			float currentUScale = pNewObject->transform().getUniformScale();
			pNewObject->transform().setUniformScale(currentUScale * scaleVariation);
		}

		if (m_alignToSurface)
		{
			// work out what the rotation should be to make it roughly flat on the surface - this won't be physically-accurate, but it's
			// a good starting point
			Quaternion rotateQ = Quaternion::fromDirections(Normal(0.0f, 1.0f, 0.0f), hitResult.geometryNormal);
			Matrix4 rotateM = rotateQ.toMatrix();
			Vector rotation = rotateM.getRotationYXZ();

			if (m_randomYRotation)
			{
				rotation.y = rng.randomFloat() * 360.0f;
			}

			pNewObject->setRotation(rotation);
		}
		else
		{
			if (m_randomYRotation)
			{
				pNewObject->setRotation(Vector(0.0f, rng.randomFloat() * 360.0f, 0.0f));
			}
		}

		if (m_drawAsBBox)
			pNewObject->setDisplayType(eBoundaryBox);

		sprintf(posInfo.szName, "ScatteredOb_%d", posInfo.createdCount); // calling function needs to increment this
		pNewObject->setName(posInfo.szName, false);

		if (m_addToGroup)
		{
			posInfo.pNewCO->addObject(pNewObject, false);
		}
		else
		{
			addObject(scene, pNewObject);
		}
	}

	return true;
}

} // namespace Imagine

namespace
{
	Imagine::SceneBuilder* createInstanceSurfaceScatterBuilder()
	{
		return new Imagine::InstanceSurfaceScatterBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(6, "Instance Surface Scatter", createInstanceSurfaceScatterBuilder);
}
