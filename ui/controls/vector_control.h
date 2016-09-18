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

#ifndef VECTOR_CONTROL_H
#define VECTOR_CONTROL_H

#include "control.h"

#include "core/vector.h"

#include "widgets/double_spin_box_ex.h"

namespace Imagine
{

class VectorControl : public Control
{
public:
	VectorControl(const std::string& name, Vector* pairedValue, float min, float max, std::string label);
	virtual ~VectorControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

protected:
	DoubleSpinBoxEx*	m_Spins[3];

	Vector*				m_pairedValue;
};

} // namespace Imagine

#endif // VECTOR_CONTROL_H
