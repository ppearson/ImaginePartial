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

#ifndef STRING_CONTROL_H
#define STRING_CONTROL_H

#include "control.h"

#include <QLineEdit>

namespace Imagine
{

class StringControl : public Control
{
public:
	StringControl(const std::string& name, std::string* pairedValue, const std::string& label = "");
	virtual ~StringControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

	void setValue(const std::string& value);
	std::string getValue();

protected:
	QLineEdit*		m_lineEdit;
	std::string*	m_pairedValue;
};

} // namespace Imagine

#endif // STRING_CONTROL_H
