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

#include "bool_control.h"

namespace Imagine
{

BoolControl::BoolControl(const std::string& name, bool* pairedValue, std::string label) : Control(name, label)
{
	m_checkBox = new QCheckBox();

	m_checkBox->setMinimumHeight(26);

	m_widget = m_checkBox;

	m_pairedValue = pairedValue;

	m_checkBox->setChecked(*m_pairedValue);

	m_pConnectionProxy->registerCheckboxToggled(m_checkBox);
}

BoolControl::~BoolControl()
{

}

bool BoolControl::valueChanged()
{
	*m_pairedValue = m_checkBox->isChecked();
	return true;
}

void BoolControl::refreshFromValue()
{
	m_checkBox->setChecked(*m_pairedValue);
}

} // namespace Imagine
