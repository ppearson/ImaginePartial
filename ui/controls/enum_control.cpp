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

#include "enum_control.h"

EnumControl::EnumControl(const std::string& name, unsigned char* pairedValue, const char** options, std::string label) : Control(name, label)
{
	m_comboBox = new QComboBox();

	m_comboBox->setMinimumSize(100, 22);

	m_widget = m_comboBox;

	m_pairedValue = pairedValue;

	if (options)
	{
		unsigned int i = 0;
		while (options[i])
		{
			m_comboBox->addItem(options[i++]);
		}
	}

	m_comboBox->setCurrentIndex(*m_pairedValue);

	m_pConnectionProxy->registerComboIndexChangedInt(m_comboBox);
}

EnumControl::~EnumControl()
{

}

bool EnumControl::valueChanged()
{
	*m_pairedValue = (unsigned char)m_comboBox->currentIndex();

	return true;
}

void EnumControl::refreshFromValue()
{
	m_comboBox->setCurrentIndex((int)*m_pairedValue);
}
