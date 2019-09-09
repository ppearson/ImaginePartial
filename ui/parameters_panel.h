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

#ifndef PARAMETERS_PANEL_H
#define PARAMETERS_PANEL_H

#include <QWidget>
#include <QFormLayout>
#include <QGridLayout>

#include <map>
#include <string>

#include "widgets/tab_widget_ex.h"
#include "controls/control.h"

#include "parameter.h"
#include "parameters_panel_interface.h"

namespace Imagine
{

class ParametersPanel : ParametersPanelInterface
{
public:
	ParametersPanel(ParametersInterface* pParent, ParameterPanelType type);
	virtual ~ParametersPanel();

	struct ControlData
	{
		ControlData() : control(NULL), layout(NULL)
		{

		}

		ControlData(Control* pControl, QFormLayout* pLayout) : control(pControl), layout(pLayout)
		{

		}

		Control*		control;
		QFormLayout*	layout;
	};

	void addTab(const std::string& title);

	void addControl(Control* pControl, bool addLabel, unsigned int tab);
	bool removeControl(const std::string& controlName);

	void addDescriptorLine(const std::string& desc, unsigned int tab = 0);

	virtual void controlChanged(const std::string& name);

	void refreshControls();
	void refreshControl(const std::string& name);
	void hideControl(const std::string& name);
	void showControl(const std::string& name);

	// these are used to try and remember the active tab page on a per class basis
	void tabIndexChanged(int tabIndex);
	void showTab(int tabIndex);

	ParameterPanelType getType() const { return m_type; }

	QWidget* getWidget() { return m_form; }

	bool doesParentSupportParameterUndos() const;

protected:
	ParametersInterface*				m_pParent;
	QWidget*							m_form;
	QGridLayout*						m_layout;
	TabWidgetEx*						m_tabWidget;

	std::vector<QWidget*>				m_aTabs;
	std::map<std::string, ControlData>	m_aControls;

	unsigned int						m_ownerID;

	ParameterPanelType					m_type;
};

} // namespace Imagine

#endif // PARAMETERS_PANEL_H
