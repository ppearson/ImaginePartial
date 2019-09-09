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

#include "parameters_panel.h"

#include <QProxyStyle>

#include <QLabel>
#include <QScrollArea>

#include "view_context.h"
#include "object.h" // not happy about this, but need the deferred-ness...

namespace Imagine
{

ParametersPanel::ParametersPanel(ParametersInterface* pParent, ParameterPanelType type) : m_pParent(pParent), m_type(type)
{
	m_form = new QWidget();
	m_form->setContentsMargins(0, 0, 0, 0);
//	m_form->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	m_layout = new QGridLayout(m_form);
	m_layout->setContentsMargins(2, 2, 2, 2);
	m_tabWidget = new TabWidgetEx(this, m_form);
	m_tabWidget->setContentsMargins(0, 0, 0, 0);
//	m_tabWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	m_layout->addWidget(m_tabWidget);

	// TODO: is this leaking? Deleting the previous one makes below line have
	//       no effect
	m_tabWidget->setStyle(new QProxyStyle());

	m_form->setMinimumWidth(310);
}

ParametersPanel::~ParametersPanel()
{
	// this also deletes all attached Widgets...
	if (m_form)
	{
		delete m_form;
		m_form = NULL;
	}

	std::map<std::string, ControlData>::iterator it = m_aControls.begin();
	std::map<std::string, ControlData>::iterator itEnd = m_aControls.end();
	for (; it != itEnd; ++it)
	{
		const ControlData& controlData = (*it).second;

		Control* pControl = controlData.control;

		if (pControl)
			delete pControl;
	}
}

void ParametersPanel::controlChanged(const std::string& name)
{
	bool changeHandled = false;

	PostChangedActions postChangedActions;

	if (m_pParent)
		changeHandled = m_pParent->controlChanged(name, postChangedActions);

	if (changeHandled)
	{
		// do these first to wait until re-rendering has finished...
		if (postChangedActions.shouldRefreshReRenderer())
		{
			ViewContext::instance().cancelReRender();
		}
		
		if (postChangedActions.shouldReconstructObjectGeometry())
		{
			// atrociously hacky...
			Object* pTestObject = dynamic_cast<Object*>(m_pParent);
			if (pTestObject)
			{
				pTestObject->constructGeometry();
			}
		}
		
		if (postChangedActions.hasRefreshItems())
		{
			const std::vector<std::string>& refreshItems = postChangedActions.getRefreshItems();
			std::vector<std::string>::const_iterator it = refreshItems.begin();
			for (; it != refreshItems.end(); ++it)
			{
				const std::string& controlName = *it;
				refreshControl(controlName);
			}
		}

		if (postChangedActions.hasHideItems())
		{
			const std::vector<std::string>& hideItems = postChangedActions.getHideItems();
			std::vector<std::string>::const_iterator it = hideItems.begin();
			for (; it != hideItems.end(); ++it)
			{
				const std::string& controlName = *it;
				hideControl(controlName);
			}
		}

		if (postChangedActions.hasShowItems())
		{
			const std::vector<std::string>& showItems = postChangedActions.getShowItems();
			std::vector<std::string>::const_iterator it = showItems.begin();
			for (; it != showItems.end(); ++it)
			{
				const std::string& controlName = *it;
				showControl(controlName);
			}
		}

		if (m_type == eObjectParameter)
		{
			ViewContext::instance().updateSelectedObjects();
		}
		else if (m_type == eMaterialParameter)
		{
			ViewContext::instance().updateMaterialEditor();
			//
			ViewContext::instance().updateSelectedObjects();
		}

		if (postChangedActions.shouldRefreshManipulators())
		{
			ViewContext::instance().refreshManipulatorsOfSelectedObject();
		}

		if (postChangedActions.shouldRedrawGLView())
		{
			ViewContext::instance().forceRedraw();
		}
		
		if (postChangedActions.shouldRefreshReRenderer())
		{
			ViewContext::instance().sceneChanged();
		}
	}
}

void ParametersPanel::addTab(const std::string& title)
{
	// need to cache the lastActiveTab number for the object (if there is one)
	// so that creating a new TabWidgetEx in the new ParametersPanel doesn't cause
	// the currentChanged(int) signal, which would reset the number to 0
	int cachedLastTab = m_pParent->getLastParametersPanelTab();

	QScrollArea* newTabPage = new QScrollArea();
	newTabPage->setWidgetResizable(true);

	QWidget* newTabPageContents = new QWidget(m_tabWidget);
	newTabPage->setWidget(newTabPageContents);

	QFormLayout* newTabLayout = new QFormLayout(newTabPageContents);
	newTabPageContents->setLayout(newTabLayout);

	// forces the QWidgets within the styled Tab pages to have default styles
	newTabPageContents->setStyleSheet("QWidget { font-size: 11px }");

	newTabPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	newTabLayout->setSizeConstraint(QLayout::SetMinimumSize);
	newTabLayout->setContentsMargins(2, 2, 2, 2);
	newTabLayout->setVerticalSpacing(2);

	newTabLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	newTabLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
	newTabLayout->setLabelAlignment(Qt::AlignLeft);

	m_aTabs.push_back(newTabPage);

	m_tabWidget->addTab(newTabPage, title.c_str());

	// reset the lastTabIndex on the object with our cached version
	m_pParent->setLastParametersPanelTab(cachedLastTab);
}

void ParametersPanel::addControl(Control* pControl, bool addLabel, unsigned int tab)
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

	// make sure there's a tab to add it to
	if (tab >= m_aTabs.size())
		return;

	pControl->setParametersPanelInterface(this);

	QScrollArea* pTabPage = (QScrollArea*)m_aTabs[tab];
	QWidget* pScrollArea = pTabPage->widget();
	QFormLayout* pTabLayout = (QFormLayout*)pScrollArea->layout();

	if (addLabel)
	{
		std::string label = pControl->getLabelOrName();
		pTabLayout->addRow(label.c_str(), pControl->getWidget());
	}
	else
	{
		pTabLayout->addRow(pControl->getWidget());
	}

	m_aControls[pControl->getName()] = ControlData(pControl, pTabLayout);
}

bool ParametersPanel::removeControl(const std::string& controlName)
{

	return true;
}

void ParametersPanel::addDescriptorLine(const std::string& desc, unsigned int tab)
{
	// make sure there's a tab to add it to
	if (tab >= m_aTabs.size())
		return;

	QScrollArea* pTabPage = (QScrollArea*)m_aTabs[tab];
	QWidget* pScrollArea = pTabPage->widget();
	QFormLayout* pTabLayout = (QFormLayout*)pScrollArea->layout();

	QLabel* pDescriptorLabel = new QLabel(desc.c_str());

	pTabLayout->addRow(pDescriptorLabel);
}

void ParametersPanel::refreshControls()
{
	std::map<std::string, ControlData>::iterator it = m_aControls.begin();
	std::map<std::string, ControlData>::iterator itEnd = m_aControls.end();
	for (; it != itEnd; ++it)
	{
		const ControlData& controlData = (*it).second;

		Control* pControl = controlData.control;

		pControl->refreshFromValue();
	}
}

void ParametersPanel::refreshControl(const std::string& name)
{
	std::map<std::string, ControlData>::iterator itFind = m_aControls.find(name);

	if (itFind == m_aControls.end())
		return;

	Control* pControl = (*itFind).second.control;

	pControl->refreshFromValue();
}

void ParametersPanel::hideControl(const std::string& name)
{
	std::map<std::string, ControlData>::iterator itFind = m_aControls.find(name);

	if (itFind == m_aControls.end())
		return;

	const ControlData& controlData = (*itFind).second;

	Control* pControl = controlData.control;

	QWidget* pWidget = pControl->getWidget();
	pWidget->setVisible(false);

	if (controlData.layout)
	{
		controlData.layout->labelForField(pWidget)->setVisible(false);
	}
}

void ParametersPanel::showControl(const std::string& name)
{
	std::map<std::string, ControlData>::iterator itFind = m_aControls.find(name);

	if (itFind == m_aControls.end())
		return;

	const ControlData& controlData = (*itFind).second;

	Control* pControl = controlData.control;

	QWidget* pWidget = pControl->getWidget();
	pWidget->setVisible(true);

	if (controlData.layout)
	{
		controlData.layout->labelForField(pWidget)->setVisible(true);
	}
}

void ParametersPanel::showTab(int tabIndex)
{
	if (tabIndex != -1)
		m_tabWidget->setCurrentIndex(tabIndex);
}

void ParametersPanel::tabIndexChanged(int tabIndex)
{
	m_pParent->setLastParametersPanelTab(tabIndex);
}

bool ParametersPanel::doesParentSupportParameterUndos() const
{
	return m_pParent->hasParameterUndo();
}

} // namespace Imagine
