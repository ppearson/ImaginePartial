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

#ifndef SELECTION_RECT_RAYCAST_WORKER_H
#define SELECTION_RECT_RAYCAST_WORKER_H

#include "utils/threads/thread_pool.h"

#include "ui/selection_misc.h"

#include "accel/acceleration_structure.h"

class Camera;

class Object;

class SelectionRectRaycastTask : public Task
{
public:
	SelectionRectRaycastTask(SelectedObjects* pSelectedObjects, unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY);
	virtual ~SelectionRectRaycastTask() { }

//protected:

	SelectedObjects*		m_pSelectedObjects;

	unsigned int			m_startX;
	unsigned int			m_startY;
	unsigned int			m_endX;
	unsigned int			m_endY;
};

class SelectionRectRaycastWorker : public ThreadPool
{
public:
	SelectionRectRaycastWorker(Camera* pCamera, AccelerationStructure<Object, AccelerationOHPointer<Object> >* pAccel, bool selectThrough);
	virtual ~SelectionRectRaycastWorker();

	bool performRaycast(unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY, SelectedObjects& selectedObjects);

protected:
	virtual bool doTask(Task* pTask, unsigned int threadID);

	static Object* unconstObjectPointer(const Object* pObject);


protected:
	void doRaycasts(unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY, SelectedObjects& selectedObjects);

protected:
	Camera*							m_pCamera;
	AccelerationStructure<Object, AccelerationOHPointer<Object> >*	m_pAccel;
	bool						m_selectThrough;

	std::vector<SelectedObjects*>	m_aFinalSelectedObjects;
};

#endif // SELECTION_RECT_RAYCAST_WORKER_H
