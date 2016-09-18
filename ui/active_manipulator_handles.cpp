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

#include "active_manipulator_handles.h"

#include <QtOpenGL/QGLWidget>

#include "manipulator_handle.h"

namespace Imagine
{

ActiveManipulatorHandles::ActiveManipulatorHandles()
{
}

void ActiveManipulatorHandles::clear()
{
	std::map<unsigned int, ManipulatorHandle*>::iterator it = m_aManipulatorHandles.begin();
	for (; it != m_aManipulatorHandles.end(); ++it)
	{
		ManipulatorHandle* pManHandle = (*it).second;

		delete pManHandle;
	}

	m_aManipulatorHandles.clear();
}

void ActiveManipulatorHandles::setManipulatorHandles(std::vector<ManipulatorHandle*>& manHandles)
{
	clear();

	unsigned int pickIDIndex = 31;

	std::vector<ManipulatorHandle*>::iterator it = manHandles.begin();
	for (; it != manHandles.end(); ++it)
	{
		ManipulatorHandle* pManHandle = *it;
		m_aManipulatorHandles.insert(std::pair<unsigned int, ManipulatorHandle*>(pickIDIndex++, pManHandle));
	}
}

bool ActiveManipulatorHandles::haveManipulatorHandle(unsigned int ID)
{
	return m_aManipulatorHandles.count(ID) != 0;
}

ManipulatorHandle* ActiveManipulatorHandles::getManipulatorHandle(unsigned int ID)
{
	std::map<unsigned int, ManipulatorHandle*>::iterator itFind = m_aManipulatorHandles.find(ID);

	if (itFind == m_aManipulatorHandles.end())
		return NULL;

	ManipulatorHandle* pManHandle = (*itFind).second;
	return pManHandle;
}

void ActiveManipulatorHandles::drawForDisplay()
{
	std::map<unsigned int, ManipulatorHandle*>::iterator it = m_aManipulatorHandles.begin();
	for (; it != m_aManipulatorHandles.end(); ++it)
	{
		ManipulatorHandle* pManHandle = (*it).second;

		pManHandle->draw();
	}
}

void ActiveManipulatorHandles::drawForSelection()
{
	std::map<unsigned int, ManipulatorHandle*>::iterator it = m_aManipulatorHandles.begin();
	for (; it != m_aManipulatorHandles.end(); ++it)
	{
		unsigned int pickID = (*it).first;
		ManipulatorHandle* pManHandle = (*it).second;

		glPushName(pickID);

		pManHandle->draw();

		glPopName();
	}
}

} // namespace Imagine
