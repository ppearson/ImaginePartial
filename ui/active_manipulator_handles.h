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

#ifndef ACTIVE_MANIPULATOR_HANDLES_H
#define ACTIVE_MANIPULATOR_HANDLES_H

#include <map>
#include <vector>

namespace Imagine
{

class ManipulatorHandle;

class ActiveManipulatorHandles
{
public:
	ActiveManipulatorHandles();

	void clear();

	void setManipulatorHandles(std::vector<ManipulatorHandle*>& manHandles);

	bool haveManipulatorHandle(unsigned int ID);
	ManipulatorHandle* getManipulatorHandle(unsigned int ID);

	void drawForDisplay();
	void drawForSelection();

protected:
	std::map<unsigned int, ManipulatorHandle*>	m_aManipulatorHandles;
};

} // namespace Imagine

#endif // ACTIVE_MANIPULATOR_HANDLES_H
