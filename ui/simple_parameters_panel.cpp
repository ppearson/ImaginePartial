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

#include "simple_parameters_panel.h"

#include <QLabel>
#include <QFormLayout>

#include "ui/widgets/texture_widget.h"
#include "ui/widgets/material_widget.h"

#include "ui/controls/control.h"

#include "view_context.h"

namespace Imagine
{

SimpleParametersPanel::SimpleParametersPanel(ParametersInterface* pParent) : m_pParent(pParent),
	m_pTextureWidget(NULL), m_pMaterialWidget(NULL)
{
	m_pWidget = new QWidget();
	m_pWidget->setContentsMargins(0, 0, 0, 0);

	m_pWidget->setStyleSheet("QWidget { font-size: 11px }");

	m_pLayout = new QFormLayout(m_pWidget);
	m_pLayout->setSizeConstraint(QLayout::SetMinimumSize);
	m_pLayout->setContentsMargins(2, 2, 2, 2);
	m_pLayout->setVerticalSpacing(4);

	m_pLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	m_pLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_pLayout->setLabelAlignment(Qt::AlignLeft);
}

SimpleParametersPanel::~SimpleParametersPanel()
{
	if (m_pWidget)
	{
		delete m_pWidget;
		m_pWidget = NULL;
	}

	std::map<std::string, Control*>::iterator it = m_aControls.begin();
	std::map<std::string, Control*>::iterator itEnd = m_aControls.end();
	for (; it != itEnd; ++it)
	{
		Control* pControl = (*it).second;

		if (pControl)
			delete pControl;
	}
}

void SimpleParametersPanel::addControl(Control* pControl)
{
	if (!pControl)
		return;

	// we can't add controls without a Widget
	if (pControl->getWidget() == NULL)
		return;

	// don't allow controls without a name
	if (pControl->getName().empty())
		return;

	// only add if doesn't exist already
	if (m_aControls.count(pControl->getName()) > 0)
		return;

	pControl->setParametersPanelInterface(this);

	std::string label = pControl->getLabelOrName();
	m_pLayout->addRow(label.c_str(), pControl->getWidget());

	m_aControls[pControl->getName()] = pControl;
}

void SimpleParametersPanel::controlChanged(const std::string& name)
{
	PostChangedActions postChangedActions;

	bool changeHandled = false;
	if (m_pParent)
		changeHandled = m_pParent->controlChanged(name, postChangedActions);

	if (changeHandled)
	{
		// notify Material that we've changed via the Texture Widget
		if (m_pTextureWidget)
		{
			m_pTextureWidget->textureHasChanged();
		}
		else if (m_pMaterialWidget)
		{
			bool refreshControls = postChangedActions.hasRefreshItems();
			m_pMaterialWidget->materialHasChanged(refreshControls);
		}
	}

	// TODO: is this needed?
	//
	ViewContext::instance().updateSelectedObjects();
}

void SimpleParametersPanel::refreshControls()
{
	std::map<std::string, Control*>::iterator it = m_aControls.begin();
	std::map<std::string, Control*>::iterator itEnd = m_aControls.end();
	for (; it != itEnd; ++it)
	{
		Control* pControl = (*it).second;

		pControl->refreshFromValue();
	}
}

void SimpleParametersPanel::refreshControl(const std::string& name)
{
	std::map<std::string, Control*>::iterator itFind = m_aControls.find(name);

	if (itFind == m_aControls.end())
		return;

	Control* pControl = (*itFind).second;

	pControl->refreshFromValue();
}

} // namespace Imagine
