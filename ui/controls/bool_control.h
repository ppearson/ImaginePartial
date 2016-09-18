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

#ifndef BOOL_CONTROL_H
#define BOOL_CONTROL_H

#include "control.h"

#include <QCheckBox>

namespace Imagine
{

class BoolControl : public Control
{
public:
	BoolControl(const std::string& name, bool* pairedValue, std::string label);
	virtual ~BoolControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

protected:
	QCheckBox*			m_checkBox;
	bool*				m_pairedValue;
};

} // namespace Imagine

#endif // BOOL_CONTROL_H
