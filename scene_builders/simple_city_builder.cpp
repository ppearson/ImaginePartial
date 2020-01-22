/*
 Imagine
 Copyright 2011-2012 Peter Pearson.

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

#include "simple_city_builder.h"

#include <ctime>
#include <cstdio>

#include "objects/primitives/plane.h"
#include "objects/primitives/cube.h"
#include "objects/compound_object.h"

#include "sampling/sampler_stratified.h"
#include "sampling/sampler_poisson_disc.h"
#include "sampling/sampler_common.h"

#include "utils/maths/rng.h"

namespace Imagine
{

const char* distributionOptions[] = { "Poisson-disc", "Stratified", "Random", 0 };

SimpleCityBuilder::SimpleCityBuilder() : SceneBuilder(), m_width(100), m_depth(100), m_numberOfBuildings(100), m_maxStories(1),
	m_distribution(0)
{
}

unsigned char SimpleCityBuilder::getSceneBuilderTypeID()
{
	return 2;
}

std::string SimpleCityBuilder::getSceneBuilderDescription()
{
	return "Simple City";
}

void SimpleCityBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("width", "width", &m_width, eParameterUInt, 1, 20000, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("depth", "depth", &m_depth, eParameterUInt, 1, 20000, eParameterScrubButton));

	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("num_buildings", "num buildings", &m_numberOfBuildings,
																		   eParameterUInt, 1, 200000, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("max_stories", "max stories", &m_maxStories, eParameterUInt,
																		   1, 5, eParameterScrubButton));

	parameters.addParameter(new EnumParameter("distribution", "distribution", (unsigned char*)&m_distribution, distributionOptions));
}

void SimpleCityBuilder::createScene(Scene& scene)
{
	Object* pGround = new Plane(float(m_depth), float(m_width));
	pGround->setName("Ground");

	addObject(scene, pGround);

	uint32_t timeSeed = std::clock();

	RNG rng(timeSeed);

	Sample2DPacket samples;

	if (m_distribution == 0)
	{
		SamplerPoissonDisc sampleGenerator;
		sampleGenerator.generateSamples(samples, m_numberOfBuildings, rng);
	}
	else if (m_distribution == 1)
	{
		SamplerStratified sampleGenerator;
		sampleGenerator.generateSamples(samples, m_numberOfBuildings, rng);
	}
	else
	{
		// random

		samples.samples.reserve(m_numberOfBuildings);

		for (unsigned int i = 0; i < m_numberOfBuildings; i++)
		{
			float xPos = rng.randomFloat(0.0f, 1.0f);
			float yPos = rng.randomFloat(0.0f, 1.0f);

			samples.samples.emplace_back(Sample2D(xPos, yPos));
		}
	}

	char szName[32];

	Object* pNewBuilding = nullptr;

	float xStart = -(float(m_width) / 2.0f);
	float yStart = -(float(m_depth) / 2.0f);

	for (unsigned int i = 0; i < m_numberOfBuildings; i++)
	{
		const Sample2D& samplePos = samples.get2DSample(i);

		float posX = xStart + float(m_width) * samplePos.x;
		float posZ = yStart + float(m_depth) * samplePos.y;

		unsigned int stories = rng.randomInt(m_maxStories);

		if (stories == 0 || stories == 1)
		{
			pNewBuilding = new Cube(2.0f);
			pNewBuilding->setPosition(Vector(posX, 2.0f, posZ));
		}
		else
		{
			CompoundObject* pCO = new CompoundObject();
			Object* pBase = new Cube(2.0f);
			pBase->setPosition(Vector(0.0f, 2.0f, 0.0f));

			Object* pTop = new Cube(1.4f);
			pTop->setPosition(Vector(0.0f, 4.0f + 1.4, 0.0f));

			pCO->addObject(pBase);
			pCO->addObject(pTop);

			pCO->setPosition(Vector(posX, 0.0f, posZ));

			pNewBuilding = dynamic_cast<Object*>(pCO);
		}

		sprintf(szName, "Building %d", i + 1);
		pNewBuilding->setName(szName);

		addObject(scene, pNewBuilding);
	}
}

} // namespace Imagine

namespace
{
	Imagine::SceneBuilder* createSimpleCityBuilder()
	{
		return new Imagine::SimpleCityBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(2, "Simple City", createSimpleCityBuilder);
}
