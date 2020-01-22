/*
 Imagine
 Copyright 2019 Peter Pearson.

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

#include "menger_sponge_builder.h"

#include "objects/mesh.h"
#include "objects/compound_object.h"
#include "objects/primitives/cube.h"

#include "geometry/standard_geometry_instance.h"

#include "materials/standard_material.h"

#include "global_context.h"

#include "utils/string_helpers.h"

#include "utils/timer.h"

#include "scene.h"

namespace Imagine
{

static const char* mengerSpongeBuilderSavePositionsTypeOptions[] = { "Off", "ASCII Vec3f", "Binary (extent, num positions, positions)", 0 };

MengerSpongeBuilder::MengerSpongeBuilder() :
	m_iterations(4),
    m_overallWidth(10.0f),
	m_makeGroup(true),
    m_gap(0.0f),
	m_savePositionsType(0)
{

}

unsigned char MengerSpongeBuilder::getSceneBuilderTypeID()
{
	return 11;
}

std::string MengerSpongeBuilder::getSceneBuilderDescription()
{
	return "Menger Sponge";
}

void MengerSpongeBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("iterations", "iterations", &m_iterations, eParameterUInt, 1, 400, eParameterScrubButton));

	parameters.addParameter(new RangeParameter<float, float>("overallWidth", "overall width", &m_overallWidth, eParameterFloat, 0.1f, 1000.0f, eParameterScrubButtonFine));

	parameters.addParameter(new BasicParameter<bool>("make_group", "make group", &m_makeGroup, eParameterBool));

	parameters.addParameter(new RangeParameter<float, float>("gap", "gap", &m_gap, eParameterFloat, 0.0f, 1.0f, eParameterScrubButtonFine | eParameterFloatSliderHighPrecision));
	
	parameters.addParameter(new EnumParameter("save_positions_type", "Save positions type", (unsigned char*)&m_savePositionsType, mengerSpongeBuilderSavePositionsTypeOptions));
	
	parameters.addParameter(new BasicParameter<std::string>("save_path", "Save path", &m_savePath, eParameterFile, eParameterFileParamGeneralSave));
}

void MengerSpongeBuilder::createScene(Scene& scene)
{
	if (m_iterations == 1)
	{
		// just to be correct...

		Cube* pNewCube = new Cube(m_overallWidth * 0.5f);
		pNewCube->setName("MengerSponge");

		addObject(scene, pNewCube);
	}
	else
	{
		BoundaryBox overallBBox;
		float extent = m_overallWidth * 0.5f;
		overallBBox.includePoint(Point(-extent, -extent, -extent));
		overallBBox.includePoint(Point(extent, extent, extent));

		std::vector<Object*> cubes;
		
		unsigned int numFinalCubes = 20;
		
		if (m_iterations > 2)
		{
			for (unsigned int i = 0; i < m_iterations - 2; i++)
			{
				numFinalCubes *= 20;
			}
			cubes.reserve(numFinalCubes);
			
			GlobalContext::instance().getLogger().info("Creating Menger Sponge geo out of: %s cubes...", formatNumberThousandsSeparator(numFinalCubes).c_str());
		}
		
		std::vector<Point>* pSavePositions = nullptr;
		std::vector<Point> savePositions;
		
		if (m_savePositionsType != 0 && !m_savePath.empty())
		{
			// reserve
			savePositions.reserve(numFinalCubes);
			pSavePositions = &savePositions;
		}
		
		// pre-calculate the sub-cube subdivision sizes
		std::vector<float> subCubeSizes(m_iterations);
		float nextWidth = m_overallWidth;
		for (unsigned int i = 0; i < m_iterations; i++)
		{
			nextWidth /= 3.0f;
			subCubeSizes[i] = nextWidth;
		}
		
		{
//			Timer t1("Generating cube positions");
			generateSubCubes(cubes, overallBBox, m_iterations, 0, subCubeSizes, pSavePositions);
		}
		
		if (m_savePositionsType != 0 && !m_savePath.empty())
		{
			// also save out positions to file...
			
			float cubeExtentVal = subCubeSizes[m_iterations - 1];
			Vector cubeExtent(cubeExtentVal, cubeExtentVal, cubeExtentVal);
			
			if (m_savePositionsType == 1)
			{
				GlobalContext::instance().getLogger().info("Saving positions to ASCII file: %s...", m_savePath.c_str());
				// ASCII
				FILE* pFile = fopen(m_savePath.c_str(), "w");
				if (pFile)
				{
					fprintf(pFile, "# cube bounds extent: (%f, %f, %f)...\n", cubeExtent.x, cubeExtent.y, cubeExtent.z);
					fprintf(pFile, "# %zu vec3f positions...\n", savePositions.size());
					std::vector<Point>::const_iterator itPos = savePositions.begin();
					for (; itPos != savePositions.end(); ++itPos)
					{
						const Point& point = *itPos;
						fprintf(pFile, "%f, %f, %f\n", point.x, point.y, point.z);
					}
					fclose(pFile);
					GlobalContext::instance().getLogger().info("File saved successfully: %s...", m_savePath.c_str());
				}			
			}
			else if (m_savePositionsType == 2)
			{
				GlobalContext::instance().getLogger().info("Saving positions to binary file (3 floats cube shape extent, uint32_t num positions, then vec3f positions): %s...", m_savePath.c_str());
				// binary
				FILE* pFile = fopen(m_savePath.c_str(), "wb");
				if (pFile)
				{
					fwrite(&cubeExtent.x, sizeof(float), 3, pFile);
					unsigned int numItems = savePositions.size();
					fwrite(&numItems, sizeof(unsigned int), 1, pFile);
					
					std::vector<Point>::const_iterator itPos = savePositions.begin();
					for (; itPos != savePositions.end(); ++itPos)
					{
						const Point& point = *itPos;
						fwrite(&point.x, sizeof(float), 3, pFile);
					}
					fclose(pFile);
					GlobalContext::instance().getLogger().info("File saved successfully: %s...", m_savePath.c_str());
				}
			}
		}
		
		if (m_makeGroup)
		{
			CompoundObject* pCO = new CompoundObject();

			pCO->reserveSubObjectCount(cubes.size());
			
			for (Object* object : cubes)
			{
				pCO->addObject(object, false, false);
			}
			
//			pCO->updateBoundaryBox();

			pCO->setDisplayType(eBoundaryBox);

			pCO->setName("Menger Sponge");

			pCO->setType(CompoundObject::eStatic);

			addObject(scene, pCO);
		}
		else
		{
			// just add them directly to the scene...

			for (Object* object : cubes)
			{
				addObject(scene, object);
			}
		}
	}
}

void MengerSpongeBuilder::generateSubCubes(std::vector<Object*>& cubes, const BoundaryBox& overallBBox,
										   int levelsRemaining, int levelCount, std::vector<float>& subCubeSizes, std::vector<Point>* savePositions)
{
	// TODO: take gap into account...
	// TODO: double precision?
//	float subCubeSizeExtent = overallBBox.getExtent().x / 3.0f;
	float subCubeSizeExtent = subCubeSizes[levelCount];

	// work out start and end axis positions (divisions) within the overallBBox...
	float startPositionsX[3];
	float startPositionsY[3];
	float startPositionsZ[3];

	for (unsigned int i = 0; i < 3; i++)
	{
		float temp = (float)i * subCubeSizeExtent;
		startPositionsX[i] = overallBBox.getMinimum().x + temp;
		startPositionsY[i] = overallBBox.getMinimum().y + temp;
		startPositionsZ[i] = overallBBox.getMinimum().z + temp;
	}

	if (levelsRemaining <= 2)
	{
		// make actual cubes

		float subCubeSize = subCubeSizeExtent / 2.0f;

		for (unsigned int x = 0; x < 3; x++)
		{
			float posX = startPositionsX[x] + subCubeSize;

			for (unsigned int y = 0; y < 3; y++)
			{
				float posY = startPositionsY[y] + subCubeSize;

				for (unsigned int z = 0; z < 3; z++)
				{
					float posZ = startPositionsZ[z] + subCubeSize;

					if ((x != 1 && y != 1) || (x != 1 && z != 1) || (y != 1 && z != 1))
					{
						Cube* pNewSubCube = new Cube(subCubeSize);
						pNewSubCube->transform().position().setFromVector(Vector(posX, posY, posZ));
						
						if (savePositions)
						{
							savePositions->emplace_back(Point(posX, posY, posZ));
						}

						cubes.emplace_back(pNewSubCube);
					}
				}
			}
		}
	}
	else
	{
		// recurse futher...

		for (unsigned int x = 0; x < 3; x++)
		{
			float extentXStart = startPositionsX[x];
			float extentXEnd = extentXStart + subCubeSizeExtent;

			for (unsigned int y = 0; y < 3; y++)
			{
				float extentYStart = startPositionsY[y];
				float extentYEnd = extentYStart + subCubeSizeExtent;

				for (unsigned int z = 0; z < 3; z++)
				{
					float extentZStart = startPositionsZ[z];
					float extentZEnd = extentZStart + subCubeSizeExtent;

					if ((x != 1 && y != 1) || (x != 1 && z != 1) || (y != 1 && z != 1))
					{
						BoundaryBox subBBox;
						subBBox.includePoint(Point(extentXStart, extentYStart, extentZStart));
						subBBox.includePoint(Point(extentXEnd, extentYEnd, extentZEnd));

						generateSubCubes(cubes, subBBox, levelsRemaining - 1, levelCount + 1, subCubeSizes, savePositions);
					}
				}
			}
		}
	}
}


} // namespace Imagine

namespace
{
    Imagine::SceneBuilder* createMengerSpongeBuilder()
	{
		return new Imagine::MengerSpongeBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(11, "Menger Sponge", createMengerSpongeBuilder);
}
