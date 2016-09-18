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

#include "ray_visibility_control.h"

namespace Imagine
{

RayVisibilityControl::RayVisibilityControl(const std::string& name, unsigned char* pairedValue, std::string label)
	: Control(name, label), m_pVisibilityButton(NULL)
{
	m_pVisibilityButton = new RayVisibilityButton();

	m_pairedValue = pairedValue;

	m_pVisibilityButton->setStateFromBitmask(*m_pairedValue);

	m_widget = m_pVisibilityButton;

	m_pConnectionProxy->registerRayVisButtonSelChanged(m_pVisibilityButton);
}

RayVisibilityControl::~RayVisibilityControl()
{

}

bool RayVisibilityControl::valueChanged()
{
	*m_pairedValue = m_pVisibilityButton->getStateBitmask();

	return true;
}

void RayVisibilityControl::refreshFromValue()
{
	m_pVisibilityButton->setStateFromBitmask(*m_pairedValue);
}

} // namespace Imagine
