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

#include "vector_control.h"

#include <QHBoxLayout>

#include "widgets/double_spin_box_ex.h"

namespace Imagine
{

VectorControl::VectorControl(const std::string& name, Vector* pairedValue, float min, float max, const std::string& label) : Control(name, label)
{
	QWidget* mainWidget = new QWidget();

	QHBoxLayout* layout = new QHBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);

	for (unsigned int i = 0; i < 3; i++)
	{
		m_Spins[i] = new DoubleSpinBoxEx(mainWidget);

		m_Spins[i]->setMinimumSize(65, 22);
		m_Spins[i]->setMaximumHeight(22);

		m_Spins[i]->setMinimum(min);
		m_Spins[i]->setMaximum(max);

		m_Spins[i]->setValue((*pairedValue)[i]);

		layout->addWidget(m_Spins[i]);
	}

	m_widget = mainWidget;

	m_pairedValue = pairedValue;

	m_pConnectionProxy->registerValueChangedDouble(m_Spins[0]);
	m_pConnectionProxy->registerValueChangedDouble(m_Spins[1]);
	m_pConnectionProxy->registerValueChangedDouble(m_Spins[2]);
}

VectorControl::~VectorControl()
{

}

bool VectorControl::valueChanged()
{
	(*m_pairedValue)[0] = m_Spins[0]->value();
	(*m_pairedValue)[1] = m_Spins[1]->value();
	(*m_pairedValue)[2] = m_Spins[2]->value();

	return true;
}

void VectorControl::refreshFromValue()
{
	m_Spins[0]->setValue((*m_pairedValue)[0]);
	m_Spins[1]->setValue((*m_pairedValue)[1]);
	m_Spins[2]->setValue((*m_pairedValue)[2]);
}

} // namespace Imagine
