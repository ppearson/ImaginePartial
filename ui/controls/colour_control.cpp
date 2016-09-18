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

namespace Imagine
{

ColourControl::ColourControl(const std::string& name, Colour3f* pairedValue, std::string label) : Control(name, label)
{
	m_pickButton = new ColourButton();

	m_widget = m_pickButton;

	m_pairedValue = pairedValue;

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
}


} // namespace Imagine
