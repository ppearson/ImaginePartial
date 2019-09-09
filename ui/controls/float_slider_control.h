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

#ifndef FLOAT_SLIDER_CONTROL_H
#define FLOAT_SLIDER_CONTROL_H

#include "control.h"

namespace Imagine
{

class FloatSliderWidget;

class FloatSliderControl : public Control
{
public:
	FloatSliderControl(const std::string& name, float* pairedValue, float min, float max, const std::string& label,
					   bool editControl, bool logScaleSlider, bool highPrecision);
	virtual ~FloatSliderControl();

	virtual bool valueChanged();
	virtual bool sliderChanged(int value);

	virtual void refreshFromValue();

protected:
	FloatSliderWidget*	m_pSlider;
	float*				m_pairedValue;

	float				m_lastValue;
};

} // namespace Imagine

#endif // FLOAT_SLIDER_CONTROL_H
