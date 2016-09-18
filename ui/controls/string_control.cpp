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

#include "string_control.h"

namespace Imagine
{

StringControl::StringControl(const std::string& name, std::string* pairedValue, std::string label) :
		Control(name, label)
{
	m_lineEdit = new QLineEdit();

	m_widget = m_lineEdit;

	m_pairedValue = pairedValue;

	m_lineEdit->setText(pairedValue->c_str());

	m_pConnectionProxy->registerEditingFinished(m_lineEdit);
}

StringControl::~StringControl()
{

}

bool StringControl::valueChanged()
{
	*m_pairedValue = m_lineEdit->text().toStdString();

	return true;
}

void StringControl::refreshFromValue()
{
	m_lineEdit->setText(m_pairedValue->c_str());
}

void StringControl::setValue(const std::string& value)
{
	m_lineEdit->setText(value.c_str());
}

std::string StringControl::getValue()
{
	return m_lineEdit->text().toStdString();
}

} // namespace Imagine
