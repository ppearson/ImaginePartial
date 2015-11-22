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

#ifndef INSTANCE_SHAPE_BUILDER_H
#define INSTANCE_SHAPE_BUILDER_H

#include "scene_builder.h"

#include "core/point.h"

#include "utils/threads/thread_pool.h"

class GeometryInstanceGathered;
class Vector;
class Object;

class InstanceShapeBuilder;

class ObjectDetectorTask : public Task
{
public:
	struct ObjectDetectorSpecs
	{
		Object*			pTestObject;
		Vector			startPos;
		Point			rawTestObjectBBMin;
		Vector			blockShapeExtent;
		Vector			sourceShapeStepExtent;
		Vector			gapOffset;
		float			gap;
		unsigned int	numberX;
		unsigned int	numberY;
		unsigned int	numberZ;
	};

	ObjectDetectorTask(const ObjectDetectorSpecs& specs);
	virtual ~ObjectDetectorTask() { }

//protected:
	ObjectDetectorSpecs m_specs;
};

class ObjectDetectorWorker : public ThreadPool
{
public:
	ObjectDetectorWorker(InstanceShapeBuilder* pHost, unsigned int threads, std::vector<Point>& results);
	virtual ~ObjectDetectorWorker() { }

	void addDetectTask(const ObjectDetectorTask::ObjectDetectorSpecs& specs);
	void detectObject();

	void addResults(const std::vector<Point>& results);

protected:
	virtual bool doTask(Task* pTask, unsigned int threadID);

protected:
	InstanceShapeBuilder*	m_pHost;

	Mutex					m_resultsLock;
	std::vector<Point>&		m_aFinalPoints;
};

class InstanceShapeBuilder : public SceneBuilder
{
public:
	InstanceShapeBuilder();
	virtual ~InstanceShapeBuilder();

	enum ObjectType
	{
		eSourceObject,
		eSecondSelectedObject,
		eSphere
	};

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);

	virtual void createScene(Scene& scene);

	static bool isInObject(const Point& position, Object* pObject);

	GeometryInstanceGathered* createScaledGeoInstanceCopy(GeometryInstanceGathered* pGeoInstance, const Vector& scale);


protected:
	float			m_scale;
	ObjectType		m_objectType;
	bool			m_drawAsBBox;
	bool			m_addToGroup;
	float			m_gap;
	bool			m_parallelBuild;
};

#endif // INSTANCE_SHAPE_BUILDER_H
