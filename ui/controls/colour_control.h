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

#ifndef COLOUR_CONTROL_H
#define COLOUR_CONTROL_H

#include "colour/colour3f.h"

#include "control.h"

#include <QHBoxLayout>

#include "widgets/colour_button.h"

class ColourControl : public Control
{
public:
	ColourControl(const std::string& name, Colour3f* pairedValue, std::string label);
	virtual ~ColourControl();

	virtual bool valueChanged();
	virtual bool buttonClicked(unsigned int index);

	virtual void refreshFromValue();

protected:
	ColourButton*		m_pickButton;
	Colour3f*			m_pairedValue;
};

#endif // COLOUR_CONTROL_H
