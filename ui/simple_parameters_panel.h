/*
 Imagine
 Copyright 2013-2014 Peter Pearson.

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

#ifndef SIMPLE_PARAMETERS_PANEL_H
#define SIMPLE_PARAMETERS_PANEL_H

#include <map>
#include <string>

#include "parameter.h"
#include "parameters_panel_interface.h"

class QWidget;
class QFormLayout;

namespace Imagine
{

class ParametersInterface;
class Control;

class TextureWidget;
class MaterialWidget;

class SimpleParametersPanel : ParametersPanelInterface
{
public:
	SimpleParametersPanel(ParametersInterface* pParent);
	virtual ~SimpleParametersPanel();

	void setTextureWidget(TextureWidget* pTextureWidget) { m_pTextureWidget = pTextureWidget; }
	void setMaterialWidget(MaterialWidget* pMaterialWidget) { m_pMaterialWidget = pMaterialWidget; }

	void addControl(Control* pControl);

	virtual void controlChanged(const std::string& name);

	void refreshControls();
	void refreshControl(const std::string& name);

	QWidget* getWidget() { return m_pWidget; }

protected:
	ParametersInterface*				m_pParent;
	QWidget*							m_pWidget;
	QFormLayout*						m_pLayout;

	// TODO: need some more generic way of doing this...
	TextureWidget*						m_pTextureWidget;
	MaterialWidget*						m_pMaterialWidget;

	std::map<std::string, Control*>		m_aControls;
};

} // namespace Imagine

#endif // SIMPLE_PARAMETERS_PANEL_H
