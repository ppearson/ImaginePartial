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

#include "float_slider_control.h"

#include "ui/widgets/float_slider_widget.h"

namespace Imagine
{

FloatSliderControl::FloatSliderControl(const std::string& name, float* pairedValue, float min, float max, const std::string& label,
									   bool editControl, bool logScaleSlider, bool highPrecision) :
	Control(name, label), m_pSlider(nullptr)
{
	m_pSlider = new FloatSliderWidget(min, max, editControl, logScaleSlider, highPrecision);

	m_widget = m_pSlider;

	m_pairedValue = pairedValue;

	m_lastValue = *pairedValue;

	m_pSlider->setValue(m_lastValue);

	m_pConnectionProxy->registerSliderMovedInt(m_pSlider);
	m_pConnectionProxy->registerEditingFinished(m_pSlider);
}

FloatSliderControl::~FloatSliderControl()
{

}

bool FloatSliderControl::valueChanged()
{
	float value = m_pSlider->getValue();

	*m_pairedValue = value;

	if (*m_pairedValue != m_lastValue)
	{
		m_lastValue = *m_pairedValue;
		return true;
	}

	return false;
}

bool FloatSliderControl::sliderChanged(int value)
{
	float fValue = m_pSlider->getValue();

	*m_pairedValue = fValue;

	if (*m_pairedValue != m_lastValue)
	{
		m_lastValue = *m_pairedValue;
		return true;
	}

	// we didn't actually change anything...
	return false;
}

void FloatSliderControl::refreshFromValue()
{
	m_lastValue = *m_pairedValue;

	m_pSlider->setValue(m_lastValue);
}

} // namespace Imagine
