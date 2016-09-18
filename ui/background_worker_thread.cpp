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

#include "background_worker_thread.h"

#include <vector>

#include "ui/view_context.h"
#include "scene.h"
#include "geometry/geometry_manager.h"
#include "geometry/geometry_instance_pre_render_workers.h"

#include "global_context.h"

#include "utils/system.h"

#include "lights/light.h"

namespace Imagine
{

BackgroundWorkerTask::BackgroundWorkerTask()
{

}

BackgroundWorkerUpdateGeometryTask::BackgroundWorkerUpdateGeometryTask()
{

}

void BackgroundWorkerUpdateGeometryTask::doTask()
{
	Scene* pScene = ViewContext::instance().getScene();

	GeometryManager& geomManager = pScene->getGeometryManager();

	// do all GeometryInstances in a thread pool to do them in parallel
	unsigned int threads = GlobalContext::instance().getWorkerThreads();
	GeometryInstanceBuildAccelWorker threadPool(threads);
	std::set<GeometryInstance*> aGeoInstances;
	geomManager.getGeometryInstances(aGeoInstances, true, eAccelStructureStatusRendering);

	threadPool.doBuildAccel(aGeoInstances, eAccelStructureStatusRendering);
}

BackgroundWorkerUpdateAllGeometryInstancesForPickingTask::BackgroundWorkerUpdateAllGeometryInstancesForPickingTask()
{

}

void BackgroundWorkerUpdateAllGeometryInstancesForPickingTask::doTask()
{
	Scene* pScene = ViewContext::instance().getScene();

	GeometryManager& geomManager = pScene->getGeometryManager();

	// do all GeometryInstances in a thread pool to do them in parallel
	unsigned int threads = GlobalContext::instance().getWorkerThreads();
	GeometryInstanceBuildAccelWorker threadPool(threads);
	std::set<GeometryInstance*> aGeoInstances;
	geomManager.getGeometryInstances(aGeoInstances, true, eAccelStructureStatusPicking);

	// add the lights as well
	std::vector<Light*>::iterator itLight = pScene->lightItBegin();
	std::vector<Light*>::iterator itLightEnd = pScene->lightItEnd();
	for (; itLight != itLightEnd; ++itLight)
	{
		Light* pLight = *itLight;
		GeometryInstanceGathered* pGI = pLight->getGeometryInstance();
		if (pGI)
		{
			if (!pGI->isUpToDate(eAccelStructureStatusPicking))
				aGeoInstances.insert(pGI);
		}
	}

	threadPool.doBuildAccel(aGeoInstances, eAccelStructureStatusPicking);
}

BackgroundWorkerThread::BackgroundWorkerThread(QObject *parent) : QThread(parent), m_pTask(NULL)
{
}

BackgroundWorkerThread::~BackgroundWorkerThread()
{
	wait();
}

void BackgroundWorkerThread::performTask(BackgroundWorkerTask* pTask)
{
	// if we already have a task, bail out...
	if (m_pTask)
		return;

	m_pTask = pTask;
	start();
}

void BackgroundWorkerThread::run()
{
	if (m_pTask)
	{
		m_pTask->doTask();
		delete m_pTask;
		m_pTask = NULL;
	}
}

} // namespace Imagine
