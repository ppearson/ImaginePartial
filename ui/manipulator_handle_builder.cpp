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

#include "manipulator_handle_builder.h"

#include <vector>

#include "manipulator.h"
#include "manipulator_handle.h"

namespace Imagine
{

ManipulatorHandleBuilder::ManipulatorHandleBuilder()
{
}

bool ManipulatorHandleBuilder::buildManipulatorHandles(Manipulators& manipulators, std::vector<ManipulatorHandle*>& manHandles)
{
	std::vector<Manipulator*>::iterator it = manipulators.m_aManipulators.begin();
	for (; it != manipulators.m_aManipulators.end(); ++it)
	{
		Manipulator* pManipulator = *it;

		ManipulatorType type = pManipulator->getType();
		Object* pObject = pManipulator->getObject();
		std::string name = pManipulator->getName();

		ManipulatorHandle* pNewHandle = nullptr;

		if (type == ePosition3D)
		{
			Position3DManipulator* pTypePoint = static_cast<Position3DManipulator*>(pManipulator);
			pNewHandle = new Position3DManipulatorHandle(pObject, name, pTypePoint->getPairedValue());
		}

		if (pNewHandle)
		{
			manHandles.emplace_back(pNewHandle);
		}
	}

	return !manHandles.empty();
}

} // namespace Imagine
