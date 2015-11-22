/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#include "instance_array_builder.h"

#include <stdio.h>

#include "scene.h"

#include "ui/view_context.h"
#include "selection_manager.h"

#include "objects/compound_object.h"
#include "objects/compound_instance.h"

#include "materials/standard_material.h"

#include "geometry/baked/baked_geometry_gathered.h"

InstanceArrayBuilder::InstanceArrayBuilder() : SceneBuilder(), m_width(10), m_depth(10), m_height(1),
	m_gapX(2.0f), m_gapY(2.0f), m_gapZ(2.0f), m_alternatingMaterials(false), m_numMaterials(3), m_drawAsBBox(false),
	m_addToGroup(false), m_useBakedInstances(false)
{
}

InstanceArrayBuilder::~InstanceArrayBuilder()
{
}

unsigned char InstanceArrayBuilder::getSceneBuilderTypeID()
{
	return 3;
}

std::string InstanceArrayBuilder::getSceneBuilderDescription()
{
	return "Instance Array";
}

void InstanceArrayBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("width", "width", &m_width, eParameterUInt, 1, 1000, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("depth", "depth", &m_depth, eParameterUInt, 1, 1000, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("height", "height", &m_height, eParameterUInt, 1, 1000, eParameterScrubButton));

	parameters.addParameter(new RangeParameter<float, float>("gapX", "X Gap", &m_gapX, eParameterFloat, 0.0f, 1000.0f, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<float, float>("gapY", "Y Gap", &m_gapY, eParameterFloat, 0.0f, 1000.0f, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<float, float>("gapZ", "Z Gap", &m_gapZ, eParameterFloat, 0.0f, 1000.0f, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("alternating_materials", "alt. materials", &m_alternatingMaterials, eParameterBool));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("num_materials", "number of materials", &m_numMaterials,
																		   eParameterUInt, 1, 20, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("set_to_bbox", "draw as bbox", &m_drawAsBBox, eParameterBool));
	parameters.addParameter(new BasicParameter<bool>("add_to_group", "add to group", &m_addToGroup, eParameterBool));

	parameters.addParameter(new BasicParameter<bool>("use_baked_instances", "Use baked instances", &m_useBakedInstances, eParameterBool));
}

void InstanceArrayBuilder::createScene(Scene& scene)
{
	// get the currently selected object
	Object* pCurrentSelObject = SelectionManager::instance().getMainSelectedObject();
	if (!pCurrentSelObject)
		return;

	BoundaryBox bb = pCurrentSelObject->getTransformedBoundaryBox();

	Vector bbExtent = bb.getExtent();

	float totalXExtent = (bbExtent.x * float(m_width)) + (m_gapX * float(m_width - 1));
	float totalYExtent = (bbExtent.y * float(m_height)) + (m_gapY * float(m_height - 1));
	float totalZExtent = (bbExtent.z * float(m_depth)) + (m_gapZ * float(m_depth - 1));

	float startPosX = -(totalXExtent * 0.5f) + (bbExtent.x * 0.5f);
	float startPosY = -(totalYExtent * 0.5f) + (bbExtent.y * 0.5f) + pCurrentSelObject->getPosition().y;
	float startPosZ = -(totalZExtent * 0.5f) + (bbExtent.z * 0.5f);

	float xPos = startPosX;
	float yPos = startPosY;
	float zPos = startPosZ;

	std::vector<Material*> aMaterials;
	if (m_alternatingMaterials)
	{
		char szName[16];
		for (unsigned int i = 0; i < m_numMaterials; i++)
		{
			memset(szName, 0, 16);
			sprintf(szName, "IMaterial_%d", i);
			Material* pNewMaterial = new StandardMaterial();
			pNewMaterial->setName(szName);

			scene.getMaterialManager().addMaterial(pNewMaterial);

			aMaterials.push_back(pNewMaterial);
		}
	}

	CompoundObject* pCO = NULL;
	if (m_addToGroup)
	{
		pCO = new CompoundObject();
	}

	bool shouldMakeBakedInstances = m_useBakedInstances;

	CompoundObject* pSrcCOForBakedInstances = NULL;

	// check that source selected object is actually a Compound Object...
	if (shouldMakeBakedInstances && pCurrentSelObject->getObjectType() == eCollection)
	{
		pSrcCOForBakedInstances = dynamic_cast<CompoundObject*>(pCurrentSelObject);
		if (!pSrcCOForBakedInstances)
		{
			shouldMakeBakedInstances = false;
		}
	}

	unsigned int count = 0;
	char szName[32];

	unsigned int altMaterialIndex = 0;

	for (unsigned int xItems = 0; xItems < m_width; xItems++)
	{
		yPos = startPosY;

		for (unsigned int yItems = 0; yItems < m_height; yItems++)
		{
			zPos = startPosZ;

			for (unsigned int zItems = 0; zItems < m_depth; zItems++)
			{
				Object* pNewObject = NULL;

				if (!shouldMakeBakedInstances)
				{
					pNewObject = pCurrentSelObject->clone();
				}
				else
				{
					pNewObject = new CompoundInstance(pSrcCOForBakedInstances);
				}

				pNewObject->setPosition(Vector(xPos, yPos, zPos));

				if (m_drawAsBBox)
					pNewObject->setDisplayType(eBoundaryBox);

				if (m_alternatingMaterials)
				{
					Material* pMat = aMaterials[altMaterialIndex];

					pNewObject->setMaterial(pMat);

					altMaterialIndex++;

					if (altMaterialIndex >= m_numMaterials)
						altMaterialIndex = 0;
				}

				sprintf(szName, "InstArrayObj_%d", count++);
				pNewObject->setName(szName);

				zPos += bbExtent.z + m_gapZ;

				if (m_addToGroup)
				{
					pCO->addObject(pNewObject);
				}
				else
				{
					addObject(scene, pNewObject);
				}
			}

			yPos += bbExtent.y + m_gapY;
		}

		xPos += bbExtent.x + m_gapX;
	}

	if (m_addToGroup)
	{
		pCO->updateBoundaryBox();

		pCO->setName("InstanceArrayCO");

		if (m_drawAsBBox)
			pCO->setDisplayType(eBoundaryBox);

		pCO->setType(CompoundObject::eStatic);

		addObject(scene, pCO);
	}

//	scene.deleteObject(pCurrentSelObject);
}

namespace
{
	SceneBuilder* createInstanceArrayBuilderSceneBuilder()
	{
		return new InstanceArrayBuilder();
	}

	const bool registered = SceneBuilderFactory::instance().registerSceneBuilder(3, "Instance Array", createInstanceArrayBuilderSceneBuilder);
}

