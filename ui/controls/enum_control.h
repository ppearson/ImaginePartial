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

#ifndef ENUM_CONTROL_H
#define ENUM_CONTROL_H

#include "control.h"

#include <QComboBox>

namespace Imagine
{

class EnumControl : public Control
{
public:
	EnumControl(const std::string& name, unsigned char* pairedValue, const char** options, std::string label);
	virtual ~EnumControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

protected:
	QComboBox*			m_comboBox;
	unsigned char*		m_pairedValue;
};

} // namespace Imagine

#endif // ENUM_CONTROL_H
