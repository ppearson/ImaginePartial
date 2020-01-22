/*
 Imagine
 Copyright 2012-2019 Peter Pearson.

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

#include "instance_shape_builder.h"

#include <algorithm>
#include <cstdio>

#include "scene.h"

#include "ui/view_context.h"
#include "selection_manager.h"

#include "objects/mesh.h"
#include "objects/primitives/sphere.h"
#include "objects/compound_object.h"
#include "objects/compound_static_spheres.h"
#include "objects/compound_instance.h"

#include "geometry/editable_geometry_instance.h"
#include "geometry/triangle_geometry_instance.h"
#include "geometry/standard_geometry_instance.h"

#include "core/matrix4.h"

#include "global_context.h"

#include "utils/system.h"
#include "utils/string_helpers.h"
#include "utils/timer.h"

namespace Imagine
{

static const char* instanceShapeBuilderIntersectionTestTypeOptions[] = { "Occlusion (fast)", "Raycast surface count (slow)", 0 };
static const char* instanceShapeBuilderTestDistributionTypeOptions[] = { "6 axis directions", "46 directions", 0 };
static const char* instanceShapeBuilderSavePositionsTypeOptions[] = { "Off", "ASCII Vec3f", "Binary (extent, num positions, positions)", 0 };

ObjectDetectorTask::ObjectDetectorTask(const ObjectDetectorSpecs& specs) : m_specs(specs)
{

}

ObjectDetectorWorker::ObjectDetectorWorker(InstanceShapeBuilder* pHost, unsigned int threads, std::vector<Point>& results)
	: ThreadPool(threads, false), m_pHost(pHost), m_aFinalPoints(results)
{

}

void ObjectDetectorWorker::addDetectTask(const ObjectDetectorTask::ObjectDetectorSpecs& specs)
{
	ObjectDetectorTask* pNewTask = new ObjectDetectorTask(specs);
	addTaskNoLock(pNewTask);
}

void ObjectDetectorWorker::detectObject()
{
	startPool(POOL_WAIT_FOR_COMPLETION);
}

void ObjectDetectorWorker::addResults(const std::vector<Point>& results)
{
	m_resultsLock.lock();

	std::copy(results.begin(), results.end(), std::back_inserter(m_aFinalPoints));

	m_resultsLock.unlock();
}

bool ObjectDetectorWorker::doTask(ThreadPoolTask* pTask, unsigned int threadID)
{
	ObjectDetectorTask* pOurTask = static_cast<ObjectDetectorTask*>(pTask);

	std::vector<Point> aFinalItemPositions;
	aFinalItemPositions.reserve(8192);

	const ObjectDetectorTask::ObjectDetectorSpecs& specs = pOurTask->m_specs;

	float startPosX = specs.startPos.x;
	float startPosY = specs.startPos.y;
	float startPosZ = specs.startPos.z;

	float xPos = startPosX;
	float yPos = startPosY;
	float zPos = startPosZ;

	float xObjSpacePos = specs.rawTestObjectBBMin.x;
	float yObjSpacePos = specs.rawTestObjectBBMin.y;
	float zObjSpacePos = specs.rawTestObjectBBMin.z;

	for (unsigned int xItems = 0; xItems < specs.numberX; xItems++)
	{
		yPos = startPosY;
		yObjSpacePos = specs.rawTestObjectBBMin.y;

		for (unsigned int yItems = 0; yItems < specs.numberY; yItems++)
		{
			zPos = startPosZ;
			zObjSpacePos = specs.rawTestObjectBBMin.z;
			
			for (unsigned int zItems = 0; zItems < specs.numberZ; zItems++)
			{
				// work out if we need to create an object in this position of this potential
				// instance in source geo's object space
				Point objSpacePos(xObjSpacePos, yObjSpacePos, zObjSpacePos);

				if (specs.pObjectDetector->isInObject(objSpacePos))
				{
					Point destinationPoint(xPos, yPos, zPos);
					aFinalItemPositions.emplace_back(destinationPoint);
				}

				zPos += specs.blockShapeExtent.z + specs.gap;
				zObjSpacePos += specs.sourceShapeStepExtent.z;
			}
			
			yPos += specs.blockShapeExtent.y + specs.gap;
			yObjSpacePos += specs.sourceShapeStepExtent.y;
		}

		xPos += specs.blockShapeExtent.x + specs.gap;
		xObjSpacePos += specs.sourceShapeStepExtent.x;
	}

	addResults(aFinalItemPositions);

	return true;
}

/////

const char* objectTypeOptions[] = { "Source Object", "Second selected object", "Sphere", 0 };

static Normal kSixAxisDirectionTests[6] = {
	Normal(0.0f, 1.0f, 0.0f),
	Normal(0.0f, -1.0f, 0.0f),
	Normal(1.0f, 0.0f, 0.0f),
	Normal(-1.0f, 0.0f, 0.0f),
	Normal(0.0f, 0.0f, 1.0f),
	Normal(0.0f, 0.0f, -1.0f)
};

static Normal kFortySixDirectionTests[46] = {
	Normal(0.0f, 1.0f, 0.0f),
	Normal(0.0f, -1.0f, 0.0f),
	Normal(1.0f, 0.0f, 0.0f),
	Normal(-1.0f, 0.0f, 0.0f),
	Normal(0.0f, 0.0f, 1.0f),
	Normal(0.0f, 0.0f, -1.0f),
	
	Normal(0.587785f, 0.809017f, 0.0f),
	Normal(0.475528f, 0.809017f, 0.345491f),
	Normal(0.181636f, 0.809017f, 0.559016f),
	Normal(-0.181635f, 0.809017f, 0.559017f),
	Normal(-0.475527f, 0.809017f, 0.345492f),
	Normal(-0.587785f, 0.809017f, 0.0f),
	Normal(-0.475529f, 0.809017f, -0.345490f),
	Normal(-0.181637f, 0.809017f, -0.559016f),
	Normal(0.181633f, 0.809017f, -0.559017f),
	Normal(0.475526f, 0.809017f, -0.345494f),
	
	Normal(0.951056f, 0.309018f, 0.0f),
	Normal(0.769421f, 0.309018f, 0.559016f),
	Normal(0.293893f, 0.309018f, 0.904508f),
	Normal(-0.293891f, 0.309018f, 0.904509f),
	Normal(-0.769419f, 0.309018f, 0.559018f),
	Normal(-0.951056f, 0.309018f, 0.0f),
	Normal(-0.769422f, 0.309018f, -0.559014f),
	Normal(-0.293896f, 0.309018f, -0.904507f),
	Normal(0.293889f, 0.309018f, -0.904509f),
	Normal(0.769418f, 0.309018f, -0.559021f),
	
	Normal(0.951057f, -0.309015f, 0.0f),
	Normal(0.769422f, -0.309015f, 0.559017f),
	Normal(0.293894f, -0.309015f, 0.904509f),
	Normal(-0.293891f, -0.309015f, 0.904509f),
	Normal(-0.769420f, -0.309015f, 0.559019f),
	Normal(-0.951057f, -0.309015f, 0.0f),
	Normal(-0.769423f, -0.309015f, -0.559015f),
	Normal(-0.293896f, -0.309015f, -0.904508f),
	Normal(0.293889f, -0.309015f, -0.904510f),
	Normal(0.769419f, -0.309015f, -0.559021f),
	
	Normal(0.587787f, -0.809016f, 0.0f),
	Normal(0.475530f, -0.809016f, 0.345492f),
	Normal(0.181637f, -0.809016f, 0.559018f),
	Normal(-0.181635f, -0.809016f, 0.559019f),
	Normal(-0.475529f, -0.809016f, 0.345493f),
	Normal(-0.587787f, -0.809016f, 0.0f),
	Normal(-0.475531f, -0.809016f, -0.345491f),
	Normal(-0.181638f, -0.809016f, -0.559018f),
	Normal(0.181634f, -0.809016f, -0.559019f),
	Normal(0.475528f, -0.809016f, -0.345495f)
};

InstanceShapeBuilder::InstanceShapeBuilder() : SceneBuilder(), m_scale(0.1f), m_objectType(eSourceObject), m_drawAsBBox(true),
	m_addToGroup(true), m_useBakedInstances(false), m_gap(0.1f), m_parallelBuild(true), m_intersectionEpsilon(0.001f),
	m_intersectionTestType(0), m_testDistributionType(0), m_savePositionsType(0)
{
}

InstanceShapeBuilder::~InstanceShapeBuilder()
{
}

unsigned char InstanceShapeBuilder::getSceneBuilderTypeID()
{
	return 5;
}

std::string InstanceShapeBuilder::getSceneBuilderDescription()
{
	return "Instance Shape";
}

void InstanceShapeBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<float, float>("scale", "Scale", &m_scale, eParameterFloat, 0.0001f, 1.0f, eParameterScrubButton));

	parameters.addParameter(new EnumParameter("object", "Object type", (unsigned char*)&m_objectType, objectTypeOptions));

	parameters.addParameter(new BasicParameter<bool>("set_to_bbox", "draw as bbox", &m_drawAsBBox, eParameterBool));
	parameters.addParameter(new BasicParameter<bool>("add_to_group", "add to group", &m_addToGroup, eParameterBool));

	parameters.addParameter(new BasicParameter<bool>("use_baked_instances", "Use baked instances", &m_useBakedInstances, eParameterBool));

	parameters.addParameter(new RangeParameter<float, float>("gap", "Gap", &m_gap, eParameterFloat, 0.00001f, 5.0f, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("parallel_build", "Build using threads", &m_parallelBuild, eParameterBool));
	
	parameters.addParameter(new RangeParameter<float, float>("intersection_epsilon", "Int. epsilon", &m_intersectionEpsilon, eParameterFloat, 0.000001f, 5.0f, eParameterScrubButton));
	
	parameters.addParameter(new EnumParameter("intersection_test_type", "Int. test type", (unsigned char*)&m_intersectionTestType, instanceShapeBuilderIntersectionTestTypeOptions));
	parameters.addParameter(new EnumParameter("test_distribution_type", "Test distribution type", (unsigned char*)&m_testDistributionType, instanceShapeBuilderTestDistributionTypeOptions));
	
	parameters.addParameter(new EnumParameter("save_positions_type", "Save positions type", (unsigned char*)&m_savePositionsType, instanceShapeBuilderSavePositionsTypeOptions));
	
	parameters.addParameter(new BasicParameter<std::string>("save_path", "Save path", &m_savePath, eParameterFile, eParameterFileParamGeneralSave));
}

void InstanceShapeBuilder::createScene(Scene& scene)
{
	// get the currently selected object
	Object* pCurrentSelObject = SelectionManager::instance().getMainSelectedObject();
	if (!pCurrentSelObject)
		return;

	if (m_objectType == eSecondSelectedObject && SelectionManager::instance().getSelection().getSelectionCount() == 1)
		return;

	Object* pSecondSelectedObject = SelectionManager::instance().getSelection().getSelectedObjectAtIndex(1);

	Object* pNewHolderObject = nullptr;

	unsigned int numberX = 0;
	unsigned int numberY = 0;
	unsigned int numberZ = 0;

	Object* pShapeTestObject = nullptr;
	pShapeTestObject = pCurrentSelObject;

	float sphereRadius = 1.0f;

	// build source geo accel structure
	pShapeTestObject->preRender(PreRenderRequirements(true, eAccelStructureStatusRendering, 0.0f));

	if (m_objectType == eSourceObject)
	{
		numberX = (unsigned int)1.0f / m_scale;
		numberY = (unsigned int)1.0f / m_scale;
		numberZ = (unsigned int)1.0f / m_scale;
		// factor in current scale...
		float currentScale = pCurrentSelObject->getUniformScale();
		currentScale *= m_scale;

		// if the type of object we want to make instances of is just a single mesh (not a compound object)
		if (pShapeTestObject->getObjectType() != eCollection)
		{
			GeometryInstanceGathered* pGeoInstance = pCurrentSelObject->getGeometryInstance();
			Vector scaleFactor(currentScale, currentScale, currentScale);
			GeometryInstanceGathered* pNewScaledGeoInstance = createScaledGeoInstanceCopy(pGeoInstance, scaleFactor);
			
			if (!pNewScaledGeoInstance)
			{
				return;
			}

			pNewHolderObject = new Mesh();
			pNewHolderObject->setGeometryInstance(pNewScaledGeoInstance);
		}
		else
		{
/*			
			// it's a compound object
			if (m_useBakedInstances)
			{
				CompoundObject* pSrcCO = dynamic_cast<CompoundObject*>(pCurrentSelObject);
				pNewHolderObject = new CompoundInstance(pSrcCO);

				pNewHolderObject->setUniformScale(currentScale);
			}
			else
			{
				return;
			}
*/
			return;
		}
	}
	else if (m_objectType == eSecondSelectedObject)
	{
		BoundaryBox blockShapebb = pSecondSelectedObject->getTransformedBoundaryBox();

		Vector extent = blockShapebb.getExtent();
		extent *= m_scale;

		Vector overallShapeExtent = pCurrentSelObject->getTransformedBoundaryBox().getExtent();

		numberX = (unsigned int)floorf(overallShapeExtent.x / extent.x);
		numberY = (unsigned int)floorf(overallShapeExtent.y / extent.y);
		numberZ = (unsigned int)floorf(overallShapeExtent.z / extent.z);

		float currentScale = m_scale * pSecondSelectedObject->getUniformScale();
		
		Vector blockShapeRotation = pSecondSelectedObject->transform().rotation().getVector();

		if (pSecondSelectedObject->getObjectType() != eCollection)
		{
			// if the type of object we want to make instances of is just a single mesh (not a compound object)
			GeometryInstanceGathered* pGeoInstance = pSecondSelectedObject->getGeometryInstance();

			// TODO: doesn't cope with rotated geometry...
			Vector scaleFactor(currentScale, currentScale, currentScale);
			GeometryInstanceGathered* pNewScaledGeoInstance = createScaledGeoInstanceCopy(pGeoInstance, scaleFactor);

			pNewHolderObject = new Mesh();
			pNewHolderObject->setGeometryInstance(pNewScaledGeoInstance);
		}
		else
		{
			// it's a compound object
			if (m_useBakedInstances)
			{
				CompoundObject* pSrcCO = dynamic_cast<CompoundObject*>(pSecondSelectedObject);
				pNewHolderObject = new CompoundInstance(pSrcCO);

				pNewHolderObject->setUniformScale(currentScale);
			}
			else
			{
				return;
			}
		}
		
		pNewHolderObject->transform().rotation().setFromVector(blockShapeRotation);
	}
	else if (m_objectType == eSphere)
	{
		BoundaryBox bb = pCurrentSelObject->getTransformedBoundaryBox();
		Vector extent = bb.getExtent();
		sphereRadius = extent[bb.minimumExtent()] / 2.0f;
		sphereRadius *= m_scale;

		float newDiameter = sphereRadius * 2.0f;

		numberX = (unsigned int)floorf(extent.x / newDiameter);
		numberY = (unsigned int)floorf(extent.y / newDiameter);
		numberZ = (unsigned int)floorf(extent.z / newDiameter);

		if (!m_addToGroup)
		{
			pNewHolderObject = new Sphere(sphereRadius, 14);
		}
	}

	if (m_objectType == eSphere && m_addToGroup)
	{
		// don't have holder object
	}
	else
	{
		if (!pNewHolderObject)
			return;

		if (pShapeTestObject->getObjectType() == eCollection)
		{
			pNewHolderObject->setDefaultMaterial();
		}
		else
		{
			pNewHolderObject->setMaterial(pShapeTestObject->getMaterial());
		}

		pNewHolderObject->constructGeometry();
		pNewHolderObject->recalculateBoundaryBoxFromGeometry();
		pNewHolderObject->updateBoundaryBox();
	}

	BoundaryBox blockShapebb;

	CompoundObject* pCO = nullptr;
	if (m_addToGroup && m_objectType != eSphere)
	{
		pCO = new CompoundObject();

		blockShapebb = pNewHolderObject->getTransformedBoundaryBox();
	}
	else
	{
		Point radPoint(sphereRadius, sphereRadius, sphereRadius);
		blockShapebb.includePoint(radPoint);
		blockShapebb.includePoint(-radPoint);
	}

	BoundaryBox rawSrcBBox = pShapeTestObject->getTransformedBoundaryBox(); // for intersecting against

	float gap = m_gap;// * m_scale;
	Vector blockShapeExtent = blockShapebb.getExtent();

	// work out the step increment for interior testing, based on how many of the block objects
	// we can fit in each dimension
	Vector sourceShapeStepExtent = rawSrcBBox.getExtent();
	sourceShapeStepExtent.x /= (float)numberX;
	sourceShapeStepExtent.y /= (float)numberY;
	sourceShapeStepExtent.z /= (float)numberZ;

	float startPosX = rawSrcBBox.getMinimum().x;
	float startPosY = rawSrcBBox.getMinimum().y;
	float startPosZ = rawSrcBBox.getMinimum().z;

	float gapOffsetX = (gap * float(numberX - 1)) / 2.0f;
	float gapOffsetY = (gap * float(numberY - 1)) / 2.0f;
	float gapOffsetZ = (gap * float(numberZ - 1)) / 2.0f;

	startPosX -= gapOffsetX;
	startPosY -= gapOffsetY;
	startPosZ -= gapOffsetZ;

	float xPos = startPosX;
	float yPos = startPosY;
	float zPos = startPosZ;

	float xObjSpacePos = rawSrcBBox.getMinimum().x;
	float yObjSpacePos = rawSrcBBox.getMinimum().y;
	float zObjSpacePos = rawSrcBBox.getMinimum().z;
	
	// Set up ObjectDetector
	ObjectDetector* pObjectDetector = nullptr;
	if (m_testDistributionType == 0)
	{
		// six axis tests
		if (m_intersectionTestType == 0)
		{
			pObjectDetector = new ObjectDetectorSixAxisOcclusion(pShapeTestObject, m_intersectionEpsilon);
		}
		else if (m_intersectionTestType == 1)
		{
			pObjectDetector = new ObjectDetectorSixAxisSurfaceCount(pShapeTestObject, m_intersectionEpsilon);
		}
	}
	else
	{
		// 46 direction tests
		if (m_intersectionTestType == 0)
		{
			pObjectDetector = new ObjectDetectorFortySixDirectionOcclusion(pShapeTestObject, m_intersectionEpsilon);
		}
		else if (m_intersectionTestType == 1)
		{
			pObjectDetector = new ObjectDetectorFortySixDirectionSurfaceCount(pShapeTestObject, m_intersectionEpsilon);
		}
	}

	std::vector<Point> aFinalItemPositions;

	unsigned int totalItems = numberX * numberY * numberZ;

	GlobalContext::instance().getLogger().info("Testing: %s positions...", formatNumberThousandsSeparator(totalItems).c_str());

	unsigned int numberOfThreadsAvailable = System::getNumberOfThreads();
	bool doInParallel = m_parallelBuild && numberOfThreadsAvailable > 1 && totalItems > 20000;

	if (doInParallel)
	{
		unsigned int numberOfTasksToCreate = numberOfThreadsAvailable * 2;
		// work out the longest axis so we can split that into different tasks
//		unsigned int largestAxis = (numberX > numberY ? (numberZ > numberX ? 2 : 0) : (numberY > numberZ ? 1 : 2));
		unsigned int largestAxis = 0; // just do X for the moment...
		unsigned int axisItems = 0;
		if (largestAxis == 0)
			axisItems = numberX;
		else
		{
			axisItems = (largestAxis == 1) ? numberY : numberZ;
		}
		
		if (numberOfTasksToCreate > axisItems)
		{
			numberOfTasksToCreate = axisItems;
		}

		unsigned int taskAxisSizes = axisItems / numberOfTasksToCreate;
		unsigned int remainder = axisItems % numberOfTasksToCreate;

		ObjectDetectorWorker detector(this, numberOfThreadsAvailable, aFinalItemPositions);

		// split into tasks...

		// just split across the X axis for now...

		ObjectDetectorTask::ObjectDetectorSpecs specs;

		specs.pObjectDetector = pObjectDetector;
		specs.gapOffset = Vector(gapOffsetX, gapOffsetY, gapOffsetZ);
		specs.gap = gap;
		specs.blockShapeExtent = blockShapeExtent;
		specs.startPos = Vector(startPosX, startPosY, startPosZ);
		specs.sourceShapeStepExtent = sourceShapeStepExtent;
		specs.numberY = numberY;
		specs.numberZ = numberZ;
		specs.rawTestObjectBBMin = rawSrcBBox.getMinimum();

		for (unsigned int i = 0; i < numberOfTasksToCreate; i++)
		{
			specs.numberX = (i == numberOfTasksToCreate - 1) ? taskAxisSizes + remainder : taskAxisSizes;

			detector.addDetectTask(specs);

			specs.rawTestObjectBBMin.x += sourceShapeStepExtent.x * (float)specs.numberX;
			specs.startPos.x += (float)specs.numberX * (blockShapeExtent.x + gap);
		}

		// run the threaded job manager to detect the positions - the results will be put into the aFinalItemPositions
		// vector
		detector.detectObject();
	}
	else
	{
		aFinalItemPositions.reserve(totalItems / 4);

		for (unsigned int xItems = 0; xItems < numberX; xItems++)
		{
			yPos = startPosY;
			yObjSpacePos = rawSrcBBox.getMinimum().y;

			for (unsigned int yItems = 0; yItems < numberY; yItems++)
			{
				zPos = startPosZ;
				zObjSpacePos = rawSrcBBox.getMinimum().z;
				
				for (unsigned int zItems = 0; zItems < numberZ; zItems++)
				{
					// work out if we need to create an object in this position of this potential
					// instance in source geo's object space
					Point objSpacePos(xObjSpacePos, yObjSpacePos, zObjSpacePos);

					if (pObjectDetector->isInObject(objSpacePos))
					{
						Point destinationPoint(xPos, yPos, zPos);
						aFinalItemPositions.emplace_back(destinationPoint);
					}

					zPos += blockShapeExtent.z + gap;
					zObjSpacePos += sourceShapeStepExtent.z;
				}

				yPos += blockShapeExtent.y + gap;
				yObjSpacePos += sourceShapeStepExtent.y;
			}

			xPos += blockShapeExtent.x + gap;
			xObjSpacePos += sourceShapeStepExtent.x;
		}
	}
	
	if (pObjectDetector)
	{
		delete pObjectDetector;
		pObjectDetector = nullptr;
	}

	unsigned int count = 0;
	char szName[32];

	GlobalContext::instance().getLogger().info("Creating Shape object from: %s positions.", formatNumberThousandsSeparator(aFinalItemPositions.size()).c_str());

	if (m_addToGroup && m_objectType == eSphere)
	{
		CompoundStaticSpheres* pCSS = new CompoundStaticSpheres();

		pCSS->buildFromPositions(aFinalItemPositions, sphereRadius);

		pCSS->setName("CompoundSpheresShape");

		addObject(scene, pCSS);
	}
	else
	{
		// now create and add the objects in the required positions...
		std::vector<Point>::iterator itItemPositions = aFinalItemPositions.begin();
		for (; itItemPositions != aFinalItemPositions.end(); ++itItemPositions)
		{
			const Point& point = *itItemPositions;

			Object* pNewObject = pNewHolderObject->clone();

			pNewObject->setPosition(Vector(point.x, point.y, point.z));

			sprintf(szName, "so_%d", count++);
			pNewObject->setName(szName, false);

			if (m_drawAsBBox)
				pNewObject->setDisplayType(eBoundaryBox);

			if (m_addToGroup)
			{
				pCO->addObject(pNewObject, false, false);
			}
			else
			{
				addObject(scene, pNewObject);
			}
		}

		if (m_addToGroup)
		{
			pCO->updateBoundaryBox();

			if (m_drawAsBBox)
				pCO->setDisplayType(eBoundaryBox);

			pCO->setName("InstanceShapeCO");

			pCO->setType(CompoundObject::eStatic);

			addObject(scene, pCO);
		}

		if (pNewHolderObject)
		{
			delete pNewHolderObject;
			pNewHolderObject = nullptr;
		}
	}
	
	if (m_savePositionsType != 0 && !m_savePath.empty())
	{
		// also save out positions to file...
		
		if (m_savePositionsType == 1)
		{
			GlobalContext::instance().getLogger().info("Saving positions to ASCII file: %s...", m_savePath.c_str());
			// ASCII
			FILE* pFile = fopen(m_savePath.c_str(), "w");
			if (pFile)
			{
				fprintf(pFile, "# source shape bounds extent: (%f, %f, %f)...\n", blockShapeExtent.x, blockShapeExtent.y, blockShapeExtent.z);
				fprintf(pFile, "# %zu vec3f positions...\n", aFinalItemPositions.size());
				std::vector<Point>::const_iterator itPos = aFinalItemPositions.begin();
				for (; itPos != aFinalItemPositions.end(); ++itPos)
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
			GlobalContext::instance().getLogger().info("Saving positions to binary file (3 floats src shape extent, uint32_t num positions, then vec3f positions): %s...", m_savePath.c_str());
			// binary
			FILE* pFile = fopen(m_savePath.c_str(), "wb");
			if (pFile)
			{
				fwrite(&blockShapeExtent.x, sizeof(float), 3, pFile);
				unsigned int numItems = aFinalItemPositions.size();
				fwrite(&numItems, sizeof(unsigned int), 1, pFile);
				
				std::vector<Point>::const_iterator itPos = aFinalItemPositions.begin();
				for (; itPos != aFinalItemPositions.end(); ++itPos)
				{
					const Point& point = *itPos;
					fwrite(&point.x, sizeof(float), 3, pFile);
				}
				fclose(pFile);
				GlobalContext::instance().getLogger().info("File saved successfully: %s...", m_savePath.c_str());
			}
		}
	}
}

bool InstanceShapeBuilder::ObjectDetectorSixAxisOcclusion::isInObject(const Point& position) const
{
	// fire rays in 6 directions
	for (unsigned int testDir = 0; testDir < 6; testDir++)
	{
		Ray occlusionRay(position, kSixAxisDirectionTests[testDir], RAY_ALL);
		occlusionRay.tMin = m_intersectionEpsilon;

		// BVH intersectors depend on signed inf values resulting from div-by-0 for axis-aligned rays...
		occlusionRay.calculateInverseDirection();

		// if nothing has been hit, assume we're outside the shape...
		if (!m_pObject->doesOcclude(occlusionRay))
			return false;
	}

	return true;
}

bool InstanceShapeBuilder::ObjectDetectorSixAxisSurfaceCount::isInObject(const Point& position) const
{
	// fire rays in 6 directions
	for (unsigned int testDir = 0; testDir < 6; testDir++)
	{
		Ray occlusionRay(position, kSixAxisDirectionTests[testDir], RAY_ALL);
		occlusionRay.tMin = m_intersectionEpsilon;

		// BVH intersectors depend on signed inf values resulting from div-by-0 for axis-aligned rays...
		occlusionRay.calculateInverseDirection();

		unsigned int hitCount = 0;

		while (true)
		{
			HitResult hitResult;
			float t = 200.0f;

			if (!m_pObject->didHitObject(occlusionRay, t, hitResult))
				break;

			hitCount += 1;

			occlusionRay.startPosition = hitResult.hitPoint + (Vector)(occlusionRay.direction) * m_intersectionEpsilon * 0.1f;
		}
		
		bool isIn = (hitCount % 2) != 0;

		if (!isIn)
			return false;
	}

	return true;
}

bool InstanceShapeBuilder::ObjectDetectorFortySixDirectionOcclusion::isInObject(const Point& position) const
{
	// fire rays in 6 directions
	for (unsigned int testDir = 0; testDir < 46; testDir++)
	{
		Ray occlusionRay(position, kFortySixDirectionTests[testDir], RAY_ALL);
		occlusionRay.tMin = m_intersectionEpsilon;

		// BVH intersectors depend on signed inf values resulting from div-by-0 for axis-aligned rays...
		occlusionRay.calculateInverseDirection();

		// if nothing has been hit, assume we're outside the shape...
		if (!m_pObject->doesOcclude(occlusionRay))
			return false;
	}

	return true;
}

bool InstanceShapeBuilder::ObjectDetectorFortySixDirectionSurfaceCount::isInObject(const Point& position) const
{
	// fire rays in 6 directions
	for (unsigned int testDir = 0; testDir < 46; testDir++)
	{
		Ray occlusionRay(position, kFortySixDirectionTests[testDir], RAY_ALL);
		occlusionRay.tMin = m_intersectionEpsilon;

		// BVH intersectors depend on signed inf values resulting from div-by-0 for axis-aligned rays...
		occlusionRay.calculateInverseDirection();

		unsigned int hitCount = 0;

		while (true)
		{
			HitResult hitResult;
			float t = 200.0f;

			if (!m_pObject->didHitObject(occlusionRay, t, hitResult))
				break;

			hitCount += 1;

			occlusionRay.startPosition = hitResult.hitPoint + (Vector)(occlusionRay.direction) * m_intersectionEpsilon * 0.1f;
		}
		
		bool isIn = (hitCount % 2) != 0;

		if (!isIn)
			return false;
	}

	return true;
}

GeometryInstanceGathered* InstanceShapeBuilder::createScaledGeoInstanceCopy(GeometryInstanceGathered* pGeoInstance, const Vector& scale)
{
	unsigned int geoIDType = pGeoInstance->getTypeID();

	// make sure we support the GeometryInstanceGathered type for scaling
	if (geoIDType != 1 && geoIDType != 2 && geoIDType != 3)
		return nullptr;

	GeometryInstance* pNewGeoInstance = pGeoInstance->clone();

	Matrix4 mScale;
	mScale.setScale(scale.x, scale.y, scale.z);

	if (pNewGeoInstance->getTypeID() == 1) // Editable Geometry instance
	{
		EditableGeometryInstance* pEditableGeoInstance = static_cast<EditableGeometryInstance*>(pNewGeoInstance);

		std::deque<Point>::iterator it = pEditableGeoInstance->getPoints().begin();
		std::deque<Point>::iterator itEnd = pEditableGeoInstance->getPoints().end();

		for (; it != itEnd; ++it)
		{
			Point& point = *it;

			point = mScale.transform(point);
		}
	}
	else if (pNewGeoInstance->getTypeID() == 2) // triangle geometry instance
	{
		TriangleGeometryInstance* pTriangleGeoInstance = static_cast<TriangleGeometryInstance*>(pNewGeoInstance);

		std::vector<Point>::iterator it = pTriangleGeoInstance->getPoints().begin();
		std::vector<Point>::iterator itEnd = pTriangleGeoInstance->getPoints().end();

		for (; it != itEnd; ++it)
		{
			Point& point = *it;

			point = mScale.transform(point);
		}
	}
	else if (pNewGeoInstance->getTypeID() == 3) // standard geometry instance
	{
		StandardGeometryInstance* pStandardGeoInstance = static_cast<StandardGeometryInstance*>(pNewGeoInstance);

		std::vector<Point>::iterator it = pStandardGeoInstance->getPoints().begin();
		std::vector<Point>::iterator itEnd = pStandardGeoInstance->getPoints().end();

		for (; it != itEnd; ++it)
		{
			Point& point = *it;

			point = mScale.transform(point);
		}
	}

	// set this to false so standard primitives are able to be serialised...
	pNewGeoInstance->setGeneratedGeometry(false);

	return static_cast<GeometryInstanceGathered*>(pNewGeoInstance);
}

} // namespace Imagine

namespace
{
	Imagine::SceneBuilder* createInstanceShapeBuilderSceneBuilder()
	{
		return new Imagine::InstanceShapeBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(5, "Instance Shape", createInstanceShapeBuilderSceneBuilder);
}

