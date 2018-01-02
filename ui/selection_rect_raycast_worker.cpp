/*
 Imagine
 Copyright 2013 Peter Pearson.

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

#include "selection_rect_raycast_worker.h"

#include "object.h"
#include "accel/kdtree_volume.h"

#include "objects/camera.h"

namespace Imagine
{


SelectionRectRaycastTask::SelectionRectRaycastTask(SelectedObjects* pSelectedObjects, unsigned int startX, unsigned int startY,
												   unsigned int endX, unsigned int endY) :
			m_pSelectedObjects(pSelectedObjects), m_startX(startX), m_startY(startY), m_endX(endX), m_endY(endY)
{
}

///////

SelectionRectRaycastWorker::SelectionRectRaycastWorker(Camera* pCamera, AccelerationStructure<Object, AccelerationOHPointer<Object> >* pAccel,
													   bool selectThrough)
		: ThreadPool(4, false), m_pCamera(pCamera), m_pAccel(pAccel), m_selectThrough(selectThrough)
{
}

SelectionRectRaycastWorker::~SelectionRectRaycastWorker()
{
	std::vector<SelectedObjects*>::iterator it = m_aFinalSelectedObjects.begin();
	for (; it != m_aFinalSelectedObjects.end(); ++it)
	{
		delete *it;
	}
}

bool SelectionRectRaycastWorker::performRaycast(unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY, SelectedObjects& selectedObjects)
{
	if ((endX - startX) <= 128 && (endY - startY) <= 128)
	{
		// no need for multiple threads....

		doRaycasts(startX, startY, endX, endY, selectedObjects);

		return true;
	}

	unsigned int outputObjectsToCreate = 4 - 1;
	// we'll set the pointer for the first task to be the output object:
	// This will reduce the need to merge unnecessarily

	for (unsigned int i = 0; i < outputObjectsToCreate; i++)
	{
		m_aFinalSelectedObjects.push_back(new SelectedObjects());
	}

	unsigned int width = endX - startX;
	unsigned int height = endY - startY;

	unsigned int halfWidth = width / 2;
	unsigned int halfHeight = height / 2;

	SelectionRectRaycastTask* pTask1 = new SelectionRectRaycastTask(&selectedObjects, startX, startY, startX + halfWidth, startY + halfHeight);
	addTaskNoLock(pTask1);

	SelectionRectRaycastTask* pTask2 = new SelectionRectRaycastTask(m_aFinalSelectedObjects[0], startX + halfWidth + 1, startY, endX, startY + halfHeight);
	addTaskNoLock(pTask2);

	SelectionRectRaycastTask* pTask3 = new SelectionRectRaycastTask(m_aFinalSelectedObjects[1], startX, startY + halfHeight + 1, startX + halfWidth, endY);
	addTaskNoLock(pTask3);

	SelectionRectRaycastTask* pTask4 = new SelectionRectRaycastTask(m_aFinalSelectedObjects[2], startX + halfWidth + 1, startY + halfHeight + 1,
												endX, endY);
	addTaskNoLock(pTask4);

	startPool(POOL_WAIT_FOR_COMPLETION);

	// merge the results

	for (unsigned int i = 0; i < outputObjectsToCreate; i++)
	{
		selectedObjects.merge(m_aFinalSelectedObjects[i]);
	}

	return true;
}

bool SelectionRectRaycastWorker::doTask(ThreadPoolTask* pTask, unsigned int threadID)
{
	SelectionRectRaycastTask* pThisTask = static_cast<SelectionRectRaycastTask*>(pTask);

	SelectedObjects& selectedObjects = *pThisTask->m_pSelectedObjects;

	doRaycasts(pThisTask->m_startX, pThisTask->m_startY, pThisTask->m_endX, pThisTask->m_endY, selectedObjects);

	return true;
}

Object* SelectionRectRaycastWorker::unconstObjectPointer(const Object* pObject)
{
	return const_cast<Object*>(pObject);
}

void SelectionRectRaycastWorker::doRaycasts(unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY, SelectedObjects& selectedObjects)
{
	if (!m_selectThrough)
	{
		// just raycast up to first object...
		for (unsigned int y = startY; y <= endY; y++)
		{
			float fY = (float)y;
			for (unsigned int x = startX; x <= endX; x++)
			{
				Ray selectionRay = m_pCamera->createRay((float)x, fY);
				selectionRay.calculateInverseDirection();

				float t = 50000.0f;

				SelectionHitResult selResult;
				if (m_pAccel->getHitObjectLazy(selectionRay, t, selResult, 0))
				{
					//
					selectedObjects.accumulateSelection(unconstObjectPointer(selResult.m_pObjects[0]), unconstObjectPointer(selResult.m_pObjects[1]));
				}
			}
		}
	}
	else
	{
		// trace through objects...
		for (unsigned int y = startY; y <= endY; y++)
		{
			float fY = (float)y;
			for (unsigned int x = startX; x <= endX; x++)
			{
				Ray selectionRay = m_pCamera->createRay((float)x, fY);
				selectionRay.calculateInverseDirection();

				while (true)
				{
					SelectionHitResult selResult;

					float t = 10000.0f;

					if (!m_pAccel->getHitObjectLazy(selectionRay, t, selResult, 0))
						break;

					//
					selectedObjects.accumulateSelection(unconstObjectPointer(selResult.m_pObjects[0]), unconstObjectPointer(selResult.m_pObjects[1]));

					// advance the ray position

					selectionRay.startPosition = (selectionRay.pointAt(t));
				}
			}
		}
	}
}

} // namespace Imagine
