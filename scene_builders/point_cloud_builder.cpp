/*
 Imagine
 Copyright 2017-2019 Peter Pearson.

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

#include "point_cloud_builder.h"

#include <ctime>

#include "utils/maths/rng.h"

#include "scene.h"

#include "ui/view_context.h"
#include "selection_manager.h"

#include "objects/mesh.h"
#include "objects/primitives/sphere.h"
#include "objects/compound_object.h"
#include "objects/compound_static_spheres.h"
#include "objects/compound_instance.h"

#include "materials/material.h"

#include "geometry/editable_geometry_instance.h"
#include "geometry/triangle_geometry_instance.h"
#include "geometry/standard_geometry_instance.h"

#include "sampling/sampler_stratified.h"
#include "sampling/sampler_poisson_disc.h"
#include "sampling/sampler_common.h"
#include "sampling/geometry_sampler.h"

namespace Imagine
{

static const char* pointCloudBuilderTypeOptions[] = { "Sample from intersection sphere", "Point of existing shape", 0 };
static const char* pointCloudBuilderDistributionOptions[] = { "Poisson-disc", "Stratified", "Random", 0 };
static const char* pointCloudBuilderBoundShapeOptions[] = { "Sphere", "Upper hemisphere", "Lower hemisphere", 0 };
static const char* pointCloudBuilderSavePointCloudTypeOptions[] = { "Off", "ASCII Vec3f Col3b", 0 };

PointCloudBuilder::PointCloudBuilder() : SceneBuilder(), m_pointRadius(0.002f), m_type(0),
			m_distribution(1), m_boundShape(0),
			m_numberOfPoints(10000), m_sampleRadius(60.0f),
			m_savePointCloudType(0)
{

}

unsigned char PointCloudBuilder::getSceneBuilderTypeID()
{
	return 10;
}

std::string PointCloudBuilder::getSceneBuilderDescription()
{
	return "Point Cloud";
}

void PointCloudBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<float, float>("point_radius", "point radius", &m_pointRadius, eParameterFloat, 0.00001f, 2.0f, eParameterScrubButton));
	parameters.addParameter(new EnumParameter("type", "type", (unsigned char*)&m_type, pointCloudBuilderTypeOptions));
	
	parameters.addParameter(new EnumParameter("distribution", "distribution", (unsigned char*)&m_distribution, pointCloudBuilderDistributionOptions));
	parameters.addParameter(new EnumParameter("bounds_shape", "bounds shape", (unsigned char*)&m_boundShape, pointCloudBuilderBoundShapeOptions));
	
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("points_count", "Points count", &m_numberOfPoints,
																		   eParameterUInt, 1, 50000000, eParameterScrubButton));
	
	parameters.addParameter(new RangeParameter<float, float>("sample_radius", "sample radius", &m_sampleRadius, eParameterFloat, 5.0000f, 5000.0f, eParameterScrubButton));
	
	parameters.addParameter(new EnumParameter("save_pointcloud_type", "Save type", (unsigned char*)&m_savePointCloudType, pointCloudBuilderSavePointCloudTypeOptions));
	
	parameters.addParameter(new BasicParameter<std::string>("save_path", "Save path", &m_savePath, eParameterFile, eParameterFileParamGeneralSave));
}

bool PointCloudBuilder::controlChanged(const std::string& name, PostChangedActions& postChangedActions)
{
	if (name == "type")
	{
		if (m_type == 0)
		{
			postChangedActions.addShowItem("distribution");
			postChangedActions.addShowItem("bounds_shape");
			postChangedActions.addShowItem("points_count");
			postChangedActions.addShowItem("sample_radius");
		}
		else if (m_type == 1)
		{
			postChangedActions.addHideItem("distribution");
			postChangedActions.addHideItem("bounds_shape");
			postChangedActions.addHideItem("points_count");
			postChangedActions.addHideItem("sample_radius");
		}
	}
	
	return true;
}

void PointCloudBuilder::createScene(Scene& scene)
{
	std::vector<Point> aFinalItemPositions;
	
	bool applyTransform = false;
	float uniformScale = 1.0f;
	Vector position;
	
	if (m_type == 0)
	{
		// create points by intersecting against the scene...
		
		PreRenderRequirements rendRequirements(true, GeometryInstanceBuildRequirements(eAccelStructureStatusRendering), 1.0f);
		scene.doPreRenders(rendRequirements);
		
		uint32_t timeSeed = std::clock();
	
		RNG rng(timeSeed);
	
		Sample2DPacket samples;
	
		if (m_distribution == 0)
		{
			SamplerPoissonDisc sampleGenerator;
			sampleGenerator.generateSamples(samples, m_numberOfPoints, rng);
		}
		else if (m_distribution == 1)
		{
			SamplerStratified sampleGenerator;
			sampleGenerator.generateSamples(samples, m_numberOfPoints, rng);
		}
		else if (m_distribution == 2)
		{
			// random
	
			samples.samples.reserve(m_numberOfPoints);
	
			for (unsigned int i = 0; i < m_numberOfPoints; i++)
			{
				float xPos = rng.randomFloat(0.0f, 1.0f);
				float yPos = rng.randomFloat(0.0f, 1.0f);
	
				samples.samples.emplace_back(Sample2D(xPos, yPos));
			}
		}
		
		FILE* pPointCloudFile = nullptr;
		if (m_savePointCloudType == 1 && !m_savePath.empty())
		{
			pPointCloudFile = fopen(m_savePath.c_str(), "w");
		}
		
		std::vector<Sample2D>::const_iterator itSample = samples.samples.begin();
		for (; itSample != samples.samples.end(); ++itSample)
		{
			const Sample2D& sample = *itSample;
			
			Point startPosition;
			
			if (m_boundShape == 0)
			{
				startPosition = uniformSampleSphere(sample.x, sample.y).value;
			}
			else if (m_boundShape == 1)
			{
				startPosition = uniformSampleHemisphereN(sample.x, sample.y, Normal(0.0f, 1.0f, 0.0f));
			}
			else if (m_boundShape == 2)
			{
				startPosition = uniformSampleHemisphereN(sample.x, sample.y, Normal(0.0f, -1.0f, 0.0f));
			}
			startPosition *= m_sampleRadius;
			
			Normal direction = Normal() - Normal(startPosition);
			
			Ray occlusionRay(startPosition, direction, RAY_ALL);
		
			// BVH intersectors depend on signed inf values resulting from div-by-0 for axis-aligned rays...
			occlusionRay.calculateInverseDirection();
		
			HitResult hitResult;
			float t = 3000.0f;
		
			if (!scene.didHitObject(occlusionRay, t, hitResult))
				continue;
			
			const Point& finalPos = hitResult.hitPoint;
			
			if (pPointCloudFile)
			{
				const Object* pHitObject = hitResult.pObject;
				
				const Material* pMaterial = pHitObject->getMaterial();

				Colour3f ambientColour = pMaterial->ambientSample(hitResult);
				
				unsigned char red = (unsigned char)(ambientColour.r * 255.0f);
				unsigned char green = (unsigned char)(ambientColour.g * 255.0f);
				unsigned char blue = (unsigned char)(ambientColour.b * 255.0f);
				
				fprintf(pPointCloudFile, "%f %f %f %hhu %hhu %hhu\n", finalPos.x, finalPos.y, finalPos.z,
											red, green, blue);
			}
			
			aFinalItemPositions.emplace_back(finalPos);
		}
		
		if (pPointCloudFile)
		{
			fclose(pPointCloudFile);
		}
	}
	else
	{
		// get points from selection.
		
		// get the currently selected object
		const Object* pCurrentSelObject = SelectionManager::instance().getMainSelectedObject();
		if (!pCurrentSelObject)
			return;
		
		if (pCurrentSelObject->getObjectType() == eGeoMesh)
		{
			const GeometryInstance* pGeoInstance = pCurrentSelObject->getGeometryInstance();
			
			if (pGeoInstance)
			{			
				if (pGeoInstance->getTypeID() == 1)
				{
					const EditableGeometryInstance* pEditableGeoInstance = dynamic_cast<const EditableGeometryInstance*>(pGeoInstance);
					if (pEditableGeoInstance)
					{
						const std::deque<Point>& geoPoints = pEditableGeoInstance->getPoints();
						aFinalItemPositions.reserve(geoPoints.size());
						
						std::copy(geoPoints.begin(), geoPoints.end(), std::back_inserter(aFinalItemPositions));
					}
				}
				else if (pGeoInstance->getTypeID() == 3)
				{
					const StandardGeometryInstance* pStandardGeoInstance = dynamic_cast<const StandardGeometryInstance*>(pGeoInstance);
					if (pStandardGeoInstance)
					{
						const std::vector<Point>& geoPoints = pStandardGeoInstance->getPoints();
						aFinalItemPositions.reserve(geoPoints.size());
						
						std::copy(geoPoints.begin(), geoPoints.end(), std::back_inserter(aFinalItemPositions));
					}
				}
				
				// see if there's a transform. If so, extract it so we can apply it to the CompoundStaticSpheres,
				// so that it's in the same place as the original mesh.
				if (!pCurrentSelObject->transform().position().isNull() || pCurrentSelObject->transform().getUniformScale() != 1.0f)
				{
					applyTransform = true;
					position = pCurrentSelObject->transform().position().getVector();
					uniformScale = pCurrentSelObject->transform().getUniformScale();
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	
	CompoundStaticSpheres* pCSS = new CompoundStaticSpheres();

	pCSS->buildFromPositions(aFinalItemPositions, m_pointRadius);

	pCSS->setName("CompoundSpheresShape");
	
	if (applyTransform)
	{
		pCSS->transform().position().setFromVector(position);
		pCSS->transform().setUniformScale(uniformScale);
	}

	addObject(scene, pCSS);
}

} // namespace Imagine

namespace
{
	Imagine::SceneBuilder* createPointCloudBuilder()
	{
		return new Imagine::PointCloudBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(10, "Point Cloud", createPointCloudBuilder);
}
