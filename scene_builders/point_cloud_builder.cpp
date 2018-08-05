#include "point_cloud_builder.h"

#include <ctime>

#include "utils/maths/rng.h"

#include "scene.h"

#include "objects/compound_static_spheres.h"

#include "sampling/sampler_stratified.h"
#include "sampling/sampler_poisson_disc.h"
#include "sampling/sampler_common.h"
#include "sampling/geometry_sampler.h"

namespace Imagine
{

const char* pointCloudBuilderDistributionOptions[] = { "Poisson-disc", "Stratified", "Random", 0 };
const char* pointCloudBuilderBoundShapeOptions[] = { "Sphere", "Upper hemisphere", "Lower hemisphere", 0 };

PointCloudBuilder::PointCloudBuilder() : SceneBuilder(), m_distribution(1), m_boundShape(0),
						m_numberOfPoints(10000), m_radius(60.0f)
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
	parameters.addParameter(new EnumParameter("distribution", "distribution", (unsigned char*)&m_distribution, pointCloudBuilderDistributionOptions));
	parameters.addParameter(new EnumParameter("bounds_shape", "bounds shape", (unsigned char*)&m_boundShape, pointCloudBuilderBoundShapeOptions));
	
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("points_count", "Points count", &m_numberOfPoints,
																		   eParameterUInt, 1, 50000000, eParameterScrubButton));
	
	parameters.addParameter(new RangeParameter<float, float>("radius", "radius", &m_radius, eParameterFloat, 5.0000f, 5000.0f, eParameterScrubButton));
}

void PointCloudBuilder::createScene(Scene& scene)
{
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

			samples.samples.push_back(Sample2D(xPos, yPos));
		}
	}
	
	std::vector<Point> aFinalItemPositions;
	
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
		startPosition *= m_radius;
		
		Normal direction = Normal() - Normal(startPosition);
		
		Ray occlusionRay(startPosition, direction, RAY_ALL);
	
		// BVH intersectors depend on signed inf values resulting from div-by-0 for axis-aligned rays...
		occlusionRay.calculateInverseDirection();
	
		HitResult hitResult;
		float t = 3000.0f;
	
		if (!scene.didHitObject(occlusionRay, t, hitResult))
			continue;
		
		Point finalPos = hitResult.hitPoint;
		
		aFinalItemPositions.push_back(finalPos);
	}
	
	CompoundStaticSpheres* pCSS = new CompoundStaticSpheres();

	pCSS->buildFromPositions(aFinalItemPositions, 0.001f);

	pCSS->setName("CompoundSpheresShape");

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
