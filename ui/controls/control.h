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

#ifndef CONTROL_H
#define CONTROL_H

#include <string>
#include <QWidget>

#include "control_connection_proxy.h"

namespace Imagine
{

class ParametersPanelInterface;

class Control
{
public:
	Control(const std::string& name, const std::string& label);
	virtual ~Control();

	void setParametersPanelInterface(ParametersPanelInterface* pParametersPanelInterface);

	// return value indicates that the value was actually modified
	virtual bool valueChanged() = 0;
	// return value indicates that the value was actually modified
	virtual bool sliderChanged(int value)
	{
		return valueChanged();
	}

	virtual bool buttonClicked(unsigned int index);
	virtual void menuSelected(int index);
	// return value indicates that the value was actually modified
	virtual bool deltaChange(float delta, unsigned int index)
	{
		return valueChanged();
	}

	void sendValueChanged();

	virtual void refreshFromValue() { }

	QWidget* getWidget();

	std::string getName() const;
	std::string getLabel() const;
	std::string getLabelOrName() const;
	bool isEnabled() const;

private:
	Control(const Control& rhs);

protected:
	QWidget*		m_widget;
	std::string		m_name;
	std::string		m_label;

	ParametersPanelInterface*	m_pParametersPanel;
	ControlConnectionProxy*		m_pConnectionProxy;

	bool			m_enabled;
	bool			m_canUndo;
};

} // namespace Imagine

#endif // CONTROL_H
