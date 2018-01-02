/*
 Imagine
 Copyright 2015-2016 Peter Pearson.

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

#include "plank_structure_builder.h"

#include "objects/mesh.h"
#include "objects/compound_object.h"

#include "geometry/standard_geometry_instance.h"

#include "materials/standard_material.h"

#include "scene.h"

namespace Imagine
{

static uint32_t kPlankPolyIndices[26] = { 0, 1, 2, 3, 7, 6, 5, 4, 3, 2, 6, 7, 2, 1, 5, 6, 1, 0, 4, 5, 0, 3, 7, 4 };
static uint32_t kPlankPolyOffsets[6] = { 4, 8, 12, 16, 20, 24 };

const char* structureTypeOptions[] = { "Simple Tower 1", "Square Building", 0 };

PlankStructureBuilder::PlankStructureBuilder() : m_width(2), m_depth(2), m_structureType(eSimpleTower1),
	m_layers(32), m_makeGroup(true), m_variance(0.0f), m_gap(0.0f)
{

}

unsigned char PlankStructureBuilder::getSceneBuilderTypeID()
{
	return 9;
}

std::string PlankStructureBuilder::getSceneBuilderDescription()
{
	return "Plank Structure";
}

void PlankStructureBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("width", "width", &m_width, eParameterUInt, 1, 400, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("depth", "depth", &m_depth, eParameterUInt, 1, 400, eParameterScrubButton));

	parameters.addParameter(new EnumParameter("type", "Type", (unsigned char*)&m_structureType, structureTypeOptions));

	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("layers", "layers", &m_layers, eParameterUInt, 1, 4096, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("make_group", "make group", &m_makeGroup, eParameterBool));

	parameters.addParameter(new RangeParameter<float, float>("variance", "variance", &m_variance, eParameterFloat, 0.0f, 1.0f, eParameterScrubButtonFine));
	parameters.addParameter(new RangeParameter<float, float>("gap", "gap", &m_gap, eParameterFloat, 0.0f, 1.0f, eParameterScrubButtonFine | eParameterFloatSliderHighPrecision));
}

void PlankStructureBuilder::createScene(Scene& scene)
{
	{
		// create a source plank
		StandardGeometryInstance* pNewGeoInstance = new StandardGeometryInstance();

		std::vector<Point>& aPoints = pNewGeoInstance->getPoints();

		m_plankLengthScale = 1.0f;
		m_plankHeightScale = 0.05f;
		m_plankWidthScale = 0.2f;

		float xVal = m_plankLengthScale * 0.5f;
		float yVal = m_plankHeightScale * 0.5f;
		float zVal = m_plankWidthScale * 0.5f;

		aPoints.push_back(Point(-xVal, yVal, zVal));
		aPoints.push_back(Point(xVal, yVal, zVal));
		aPoints.push_back(Point(xVal, yVal, -zVal));
		aPoints.push_back(Point(-xVal, yVal, -zVal));

		aPoints.push_back(Point(-xVal, -yVal, zVal));
		aPoints.push_back(Point(xVal, -yVal, zVal));
		aPoints.push_back(Point(xVal, -yVal, -zVal));
		aPoints.push_back(Point(-xVal, -yVal, -zVal));

		std::vector<uint32_t>& aPolyIndices = pNewGeoInstance->getPolygonIndices();
		std::vector<uint32_t>& aPolyOffsets = pNewGeoInstance->getPolygonOffsets();

		std::copy(kPlankPolyIndices, kPlankPolyIndices + 24, std::back_inserter(aPolyIndices));
		std::copy(kPlankPolyOffsets, kPlankPolyOffsets + 6, std::back_inserter(aPolyOffsets));

		unsigned int geoBuildFlags = GeometryInstance::GEO_BUILD_TESSELATE | GeometryInstance::GEO_BUILD_CALC_VERT_NORMALS |
										GeometryInstance::GEO_BUILD_CALC_BBOX;

		pNewGeoInstance->setGeoBuildFlags(geoBuildFlags);

		pNewGeoInstance->buildRawGeometryData(geoBuildFlags);

		m_aSourcePlanks.push_back(pNewGeoInstance);
	}

	std::vector<Object*> aPlanks;

	if (m_structureType == eSimpleTower1)
	{
		createSimpleTower1(aPlanks, scene);
	}
	else if (m_structureType == eSquareBuilding1)
	{
		createSquareBuilding(aPlanks, scene);
	}

	// add them to the scene

	std::vector<Object*>::iterator itPlank = aPlanks.begin();

	if (!m_makeGroup)
	{
		for (; itPlank != aPlanks.end(); ++itPlank)
		{
			Object* pPlank = *itPlank;

			scene.addObject(pPlank, false, false, true);
		}
	}
	else
	{
		CompoundObject* pNewCO = new CompoundObject();
		pNewCO->setName("Structure1");

		for (; itPlank != aPlanks.end(); ++itPlank)
		{
			Object* pPlank = *itPlank;

			pNewCO->addObject(pPlank, false);
		}

		pNewCO->setType(CompoundObject::eStatic);
		pNewCO->updateBoundaryBox();

		scene.addObject(pNewCO, false, false, true);
	}
}

void PlankStructureBuilder::createNewPlank(std::vector<Object*>& aPlanks, const Vector& pos, const Vector& rot) const
{
	StandardGeometryInstance* pGeo = m_aSourcePlanks[0];

	Mesh* pNewPlank = NULL;

	pNewPlank = new Mesh();
	pNewPlank->setName("1", false);

	pNewPlank->setGeometryInstance(pGeo);
	pNewPlank->setDefaultMaterial();

	pNewPlank->setPosition(pos);
	pNewPlank->setRotation(rot);

	aPlanks.push_back(pNewPlank);
}

void PlankStructureBuilder::createNewPlank(std::vector<Object*>& aPlanks, const Vector& pos) const
{
	createNewPlank(aPlanks, pos, Vector(0.0f, 0.0f, 0.0f));
}

void PlankStructureBuilder::createSimpleTower1(std::vector<Object*>& aPlanks, Scene& scene)
{
	float widthOffset = m_plankLengthScale * 0.5f - (m_plankWidthScale * 0.5f);
	float heightStart = m_plankHeightScale * 0.5f;

	// create flat base of two items face down...
	createNewPlank(aPlanks, Vector(0.0, heightStart, -widthOffset));
	createNewPlank(aPlanks, Vector(0.0, heightStart, widthOffset));

	heightStart += m_plankHeightScale;

	float offsetVariation = 1.0f;
	float offsetAmount = 1.0f / (float)(m_layers / 2);
	bool varyDecreasing = true;

	for (unsigned int i = 0; i < m_layers; i++)
	{
		// now two flat lateral planks
		createNewPlank(aPlanks, Vector(-widthOffset, heightStart, 0.0f), Vector(0.0f, 90.0f, 0.0f));
		createNewPlank(aPlanks, Vector(widthOffset, heightStart, 0.0f), Vector(0.0f, 90.0f, 0.0f));

		heightStart += m_plankHeightScale * 0.5f;

		heightStart += m_plankWidthScale * 0.5f;

		float zPos = widthOffset;

		if (m_variance > 0.0f)
		{
			zPos = widthOffset * offsetVariation;

			// adjust the varyAmount for the next level
			if (offsetVariation == 1.0f)
			{
				varyDecreasing = true;
			}
			else if (offsetVariation <= (1.0f - m_variance))
			{
				varyDecreasing = false;
			}

			if (varyDecreasing)
			{
				offsetVariation -= offsetAmount;
			}
			else
			{
				offsetVariation += offsetAmount;
			}
		}

		heightStart += m_gap;

		// now two tilted planks to create height
		createNewPlank(aPlanks, Vector(0.0, heightStart, -zPos), Vector(90.0f, 0.0f, 0.0f));
		createNewPlank(aPlanks, Vector(0.0, heightStart, zPos), Vector(90.0f, 0.0f, 0.0f));

		heightStart += m_gap;

		heightStart += m_plankWidthScale * 0.5f;
		heightStart += m_plankHeightScale * 0.5f;
	}

	// two final lateral planks
	// now two flat lateral planks
	createNewPlank(aPlanks, Vector(-widthOffset, heightStart, 0.0f), Vector(0.0f, 90.0f, 0.0f));
	createNewPlank(aPlanks, Vector(widthOffset, heightStart, 0.0f), Vector(0.0f, 90.0f, 0.0f));
}

void PlankStructureBuilder::createSquareBuilding(std::vector<Object*>& aPlanks, Scene& scene)
{
	unsigned int numRowsX = m_width * 2 + 1;
	unsigned int numRowsZ = m_depth * 2 + 1;

	float gap = m_plankLengthScale * 0.5f;
	float heightStart = m_plankHeightScale * 0.5f;

	float startX = 0.0f;
	float startZ = 0.0f;

	for (unsigned int layer = 0; layer < m_layers; layer++)
	{
		// flat ones...
		float posX = startX;
		for (unsigned int x = 0; x < m_width; x++)
		{
			float posZ = startZ;
			for (unsigned int z = 0; z < numRowsZ; z++)
			{
				createNewPlank(aPlanks, Vector(posX, heightStart, posZ));

				posZ += gap;
			}

			posX += m_plankLengthScale;
		}

		heightStart += m_gap;

		heightStart += m_plankHeightScale * 0.5f;

		// offset for half width of next tilted planks
		heightStart += m_plankWidthScale * 0.5f;

		// now tilted ones for hight
		posX = startX - gap + (m_plankHeightScale * 0.5f);

		Vector rotation(-90.0f, 90.0f, 0.0f);
		for (unsigned int x = 0; x < m_width + 1; x++)
		{
			float posZ = startZ + gap - (m_plankHeightScale * 0.5f);
			for (unsigned int z = 0; z < m_depth; z++)
			{
				createNewPlank(aPlanks, Vector(posX, heightStart, posZ), rotation);

				posZ += m_plankLengthScale + (m_plankHeightScale);
			}

			posX += (m_plankLengthScale - (m_plankHeightScale * 0.5f));
		}

		heightStart += m_gap;

		heightStart += m_plankWidthScale * 0.5f + (m_plankHeightScale * 0.5f);
	}

	// create final flat items
	float posX = startX;
	for (unsigned int x = 0; x < m_width; x++)
	{
		float posZ = startZ;
		for (unsigned int z = 0; z < numRowsZ; z++)
		{
			createNewPlank(aPlanks, Vector(posX, heightStart, posZ));

			posZ += gap;
		}

		posX += m_plankLengthScale;
	}
}


} // namespace Imagine

namespace
{
	Imagine::SceneBuilder* createPlankStructureBuilder()
	{
		return new Imagine::PlankStructureBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(9, "Plank Structure", createPlankStructureBuilder);
}
