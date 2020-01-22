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

#include "room_builder.h"

#include "scene.h"
#include "objects/primitives/plane.h"

#include "materials/standard_material.h"

namespace Imagine
{

const char* materialOptions[] = { "Default", "Floor Only", "Floor and Walls", 0 };

RoomBuilder::RoomBuilder() : SceneBuilder(), m_width(20), m_depth(20), m_height(15), m_front(false), m_ceiling(false),
	m_planeDivisions(1), m_materials(eFloorOnly)
{
}

RoomBuilder::~RoomBuilder()
{
}

unsigned char RoomBuilder::getSceneBuilderTypeID()
{
	return 1;
}

std::string RoomBuilder::getSceneBuilderDescription()
{
	return "Room";
}

void RoomBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("width", "width", &m_width, eParameterUInt, 1, 400, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("depth", "depth", &m_depth, eParameterUInt, 1, 400, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("height", "height", &m_height, eParameterUInt, 1, 400, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("front", "front", &m_front, eParameterBool));
	parameters.addParameter(new BasicParameter<bool>("ceiling", "ceiling", &m_ceiling, eParameterBool));

	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("plane_divisions", "plane divisions", &m_planeDivisions, eParameterUInt,
																		   1, 256, eParameterScrubButton));

	parameters.addParameter(new EnumParameter("materials", "materials", (unsigned char*)&m_materials, materialOptions));
}

void RoomBuilder::createScene(Scene& scene)
{
	Object* pFloor = new Plane(float(m_depth), float(m_width), m_planeDivisions);
	pFloor->setName("Floor");

	Object* pBackWall = new Plane(float(m_height), float(m_width), m_planeDivisions); // these are flipped because of the rotation
	pBackWall->setName("Back wall");
	pBackWall->setRotation(Vector(0.0f, 0.0f, -90.0f));
	pBackWall->setPosition(Vector(-(float)m_depth / 2.0f, float(m_height) / 2.0f, 0.0f));

	Object* pLeftWall = new Plane(float(m_depth), float(m_height), m_planeDivisions);
	pLeftWall->setName("Left wall");
	pLeftWall->setRotation(Vector(90.0f, 0.0f, 0.0f));
	pLeftWall->setPosition(Vector(0.0f, float(m_height) / 2.0f, -(float)m_width / 2.0f));

	Object* pRightWall = new Plane(float(m_depth), float(m_height), m_planeDivisions);
	pRightWall->setName("Right wall");
	pRightWall->setRotation(Vector(-90.0f, 0.0f, 0.0f));
	pRightWall->setPosition(Vector(0.0f, float(m_height) / 2.0f, (float)m_width / 2.0f));

	if (m_materials != eDefaultMaterial)
	{
		Material* pFloorMaterial = new StandardMaterial();
		pFloorMaterial->setName("Floor");

		scene.getMaterialManager().addMaterial(pFloorMaterial);

		pFloor->setMaterial(pFloorMaterial);
	}

	addObject(scene, pFloor);
	addObject(scene, pBackWall);
	addObject(scene, pLeftWall);
	addObject(scene, pRightWall);

	Material* pFloorMaterial = nullptr;

	if (m_materials == eFloorAndWalls)
	{
		pFloorMaterial = new StandardMaterial();
		pFloorMaterial->setName("Wall");

		scene.getMaterialManager().addMaterial(pFloorMaterial);

		pBackWall->setMaterial(pFloorMaterial);
		pLeftWall->setMaterial(pFloorMaterial);
		pRightWall->setMaterial(pFloorMaterial);
	}

	if (m_front)
	{
		Object* pFrontWall = new Plane(float(m_height), float(m_width), m_planeDivisions); // these are flipped because of the rotation
		pFrontWall->setName("Front wall");
		pFrontWall->setRotation(Vector(0.0f, 0.0f, 90.0f));
		pFrontWall->setPosition(Vector((float)m_depth / 2.0f, float(m_height) / 2.0f, 0.0f));

		if (pFloorMaterial)
		{
			pFrontWall->setMaterial(pFloorMaterial);
		}

		addObject(scene, pFrontWall);
	}

	if (m_ceiling)
	{
		Object* pCeiling = new Plane(float(m_depth), float(m_width), m_planeDivisions);
		pCeiling->setName("Ceiling");
		pCeiling->setRotation(Vector(-180.0f, 0.0f, 0.0f));
		pCeiling->setPosition(Vector(0.0f, float(m_height), 0.0f));

		addObject(scene, pCeiling);
	}
}

} // namespace Imagine

namespace
{
	Imagine::SceneBuilder* createRoomSceneBuilder()
	{
		return new Imagine::RoomBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(1, "Room", createRoomSceneBuilder);
}
