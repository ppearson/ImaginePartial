/*
 Imagine
 Copyright 2011-2016 Peter Pearson.

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

#ifndef INT_CONTROL_H
#define INT_CONTROL_H

#include "control.h"

#include "widgets/spin_box_ex.h"
#include <QSlider>
#include <QHBoxLayout>

namespace Imagine
{

class IntControl : public Control
{
public:
	IntControl(const std::string& name, int* pairedValue, int min, int max, const std::string& label,
				bool scrubHandle = false);
	virtual ~IntControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

	virtual bool deltaChange(float delta, unsigned int index);

protected:
	SpinBoxEx*			m_intSpin;

	int*				m_pairedValue;

	int					m_minimum;
	int					m_maximum;
};

} // namespace Imagine

#endif // INT_CONTROL_H
