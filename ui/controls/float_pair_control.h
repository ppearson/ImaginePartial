/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#ifndef FLOAT_PAIR_CONTROL_H
#define FLOAT_PAIR_CONTROL_H

#include "control.h"

namespace Imagine
{

class DoubleSpinBoxEx;

// TODO: templatise this with FloatControl<2>

class FloatPairControl : public Control
{
public:
	// two float spin controls with the same range - i.e. UV params
	FloatPairControl(const std::string& name, float* pairedValue1, float* pairedValue2, float min, float max, const std::string& label,
				 unsigned int flags = 0);

	// two float spin controls with different ranges
	FloatPairControl(const std::string& name, float* pairedValue1, float* pairedValue2, float min1, float max1,
					 float min2, float max2, const std::string& label, unsigned int flags = 0);

	virtual ~FloatPairControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

	virtual bool deltaChange(float delta, unsigned int index);

	void setTooltips(const std::string& tooltip1, const std::string& tooltip2);

protected:
	DoubleSpinBoxEx*	m_doubleSpin1;
	DoubleSpinBoxEx*	m_doubleSpin2;

	float*				m_pairedValue1;
	float*				m_pairedValue2;

	float				m_lastValue1;
	float				m_lastValue2;

	float				m_minimum1;
	float				m_maximum1;

	float				m_minimum2;
	float				m_maximum2;

	float				m_delta;
};

} // namespace Imagine

#endif // FLOAT_PAIR_CONTROL_H
