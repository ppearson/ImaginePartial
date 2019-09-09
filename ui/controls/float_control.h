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

#ifndef FLOAT_CONTROL_H
#define FLOAT_CONTROL_H

#include "control.h"

#include "widgets/double_spin_box_ex.h"

namespace Imagine
{

class FloatControl : public Control
{
public:
	FloatControl(const std::string& name, float* pairedValue, float min, float max, const std::string& label,
				 unsigned int flags = 0);
	virtual ~FloatControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

	virtual bool deltaChange(float delta, unsigned int index);

protected:
	DoubleSpinBoxEx*	m_doubleSpin;
	float*				m_pairedValue;

	float				m_delta;
	float				m_minimum;
	float				m_maximum;

	float				m_lastValue;
};

} // namespace Imagine

#endif // FLOAT_CONTROL_H
