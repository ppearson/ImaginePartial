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

#include "colour_control.h"

#include <QColorDialog>
#include <QHBoxLayout>

#include "widgets/double_spin_box_ex.h"

namespace Imagine
{

ColourControl::ColourControl(const std::string& name, Colour3f* pairedValue, const std::string& label,
							 bool editControls, bool colourValues) : Control(name, label),
							m_editControls(editControls)
{
	m_pairedValue = pairedValue;

	m_pickButton = new ColourButton();

	if (!m_editControls)
	{
		m_widget = m_pickButton;

		for (unsigned int i = 0; i < 3; i++)
		{
			m_Spins[i] = NULL;
		}
	}
	else
	{
		m_pickButton->setMinimumWidth(250);
		
		QWidget* mainWidget = new QWidget();

		QHBoxLayout* layout = new QHBoxLayout(mainWidget);
		mainWidget->setLayout(layout);
		layout->setSpacing(1);
		layout->setMargin(0);

		float minVal = 0.0f;
		float maxVal = 1.0f;

		layout->addWidget(m_pickButton);

		for (unsigned int i = 0; i < 3; i++)
		{
			m_Spins[i] = new DoubleSpinBoxEx(mainWidget);

			m_Spins[i]->setMinimumSize(25, 22);
			m_Spins[i]->setMaximumHeight(22);

			m_Spins[i]->setMinimum(minVal);
			m_Spins[i]->setMaximum(maxVal);

			m_Spins[i]->setValue((*pairedValue)[i]);

			layout->addWidget(m_Spins[i]);
		}

		m_pConnectionProxy->registerValueChangedDoubleAlternative(m_Spins[0]);
		m_pConnectionProxy->registerValueChangedDoubleAlternative(m_Spins[1]);
		m_pConnectionProxy->registerValueChangedDoubleAlternative(m_Spins[2]);

		m_widget = mainWidget;
	}

	refreshFromValue();

	m_pConnectionProxy->registerButtonClicked(m_pickButton);
	m_pConnectionProxy->registerItemChanged(m_pickButton);
}

ColourControl::~ColourControl()
{

}

bool ColourControl::valueChanged()
{
	QColor colour = m_pickButton->getColour();

	float red = colour.redF();
	float green = colour.greenF();
	float blue = colour.blueF();

	*m_pairedValue = Colour3f(red, green, blue);

	if (m_editControls)
	{
		m_Spins[0]->setValue((*m_pairedValue)[0]);
		m_Spins[1]->setValue((*m_pairedValue)[1]);
		m_Spins[2]->setValue((*m_pairedValue)[2]);
	}

	return true;
}

bool ColourControl::valueChangedAlternative()
{
	if (!m_editControls)
		return false;
	
	(*m_pairedValue)[0] = m_Spins[0]->value();
	(*m_pairedValue)[1] = m_Spins[1]->value();
	(*m_pairedValue)[2] = m_Spins[2]->value();

	float red = m_pairedValue->r;
	float green = m_pairedValue->g;
	float blue = m_pairedValue->b;

	QColor colour;
	colour.setRgbF(red, green, blue);

	m_pickButton->setColour(colour);

	return true;
}

bool ColourControl::buttonClicked(unsigned int index)
{
	float red = m_pairedValue->r;
	float green = m_pairedValue->g;
	float blue = m_pairedValue->b;

	QColor colour;
	colour.setRgbF(red, green, blue);

	colour = QColorDialog::getColor(colour);

	// if colour is invalid, the user cancelled the colour dialog
	if (!colour.isValid())
		return false;

	m_pickButton->setColour(colour);
	m_pickButton->update();

	red = colour.redF();
	green = colour.greenF();
	blue = colour.blueF();

	*m_pairedValue = Colour3f(red, green, blue);

	if (m_editControls)
	{
		m_Spins[0]->setValue((*m_pairedValue)[0]);
		m_Spins[1]->setValue((*m_pairedValue)[1]);
		m_Spins[2]->setValue((*m_pairedValue)[2]);
	}

	return true;
}

void ColourControl::refreshFromValue()
{
	float red = m_pairedValue->r;
	float green = m_pairedValue->g;
	float blue = m_pairedValue->b;

	QColor colour;
	colour.setRgbF(red, green, blue);

	m_pickButton->setColour(colour);

	if (m_editControls)
	{
		m_Spins[0]->setValue((*m_pairedValue)[0]);
		m_Spins[1]->setValue((*m_pairedValue)[1]);
		m_Spins[2]->setValue((*m_pairedValue)[2]);
	}
}


} // namespace Imagine
