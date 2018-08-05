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

#include "control.h"

//#include "parameters_panel.h"
#include "parameters_panel_interface.h"

namespace Imagine
{

Control::Control(const std::string& name, const std::string& label) : m_widget(NULL), m_name(name), m_label(label),
	m_pParametersPanel(NULL), m_pConnectionProxy(NULL), m_enabled(true), m_canUndo(false)
{
	m_pConnectionProxy = new ControlConnectionProxy(this);
}

Control::~Control()
{
	if (m_pConnectionProxy)
		delete m_pConnectionProxy;
}

void Control::setParametersPanelInterface(ParametersPanelInterface* pParametersPanelInterface)
{
	m_pParametersPanel = pParametersPanelInterface;
	if (m_pParametersPanel)
	{
//		m_canUndo = pParametersPanel->doesParentSupportParameterUndos();
	}
}

bool Control::buttonClicked(unsigned int)
{
	return true;
}

void Control::menuSelected(int)
{

}

void Control::sendValueChanged()
{
	if (m_pParametersPanel)
		m_pParametersPanel->controlChanged(m_name);
}

QWidget* Control::getWidget()
{
	return m_widget;
}

const std::string& Control::getName() const
{
	return m_name;
}

const std::string& Control::getLabel() const
{
	return m_label;
}

const std::string& Control::getLabelOrName() const
{
	if (!m_label.empty())
		return m_label;

	return m_name;
}

bool Control::isEnabled() const
{
	return m_enabled;
}

} // namespace Imagine
