/*
 Imagine
 Copyright 2014 Peter Pearson.

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

#include "material_widget.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QObject>

#include "materials/material.h"

#include "materials/material_factory.h"

#include "ui/simple_parameters_panel.h"
#include "ui/simple_panel_builder.h"

MaterialWidget::MaterialWidget(Material** pPairedMaterial, QWidget* parent) : QWidget(parent), m_pParamsPanel(NULL)
{
	if (pPairedMaterial)
	{
		m_pActualMaterial = pPairedMaterial;
		m_pMaterial = *m_pActualMaterial;
	}
	else
	{
		m_pMaterial = NULL;
		m_pActualMaterial = NULL;
	}

	QVBoxLayout* layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->setMargin(0);

	m_pMaterialType = new QComboBox();

	m_pMaterialParamsContainer = new QWidget();
	m_pMaterialParamsLayout = new QVBoxLayout(m_pMaterialParamsContainer);
	m_pMaterialParamsLayout->setContentsMargins(0, 0, 0, 0);

	m_pMaterialContentLastContent = NULL;

	layout->addWidget(m_pMaterialType);
	layout->addWidget(m_pMaterialParamsContainer);

	MaterialFactory::MaterialNames::iterator it = MaterialFactory::instance().materialNamesSimpleBegin();
	MaterialFactory::MaterialNames::iterator itEnd = MaterialFactory::instance().materialNamesSimpleEnd();

	for (; it != itEnd; ++it)
	{
		const std::string materialName = (*it).second;
		unsigned int materialID = (unsigned int)(*it).first;

		m_pMaterialType->addItem(materialName.c_str());

		m_aMaterialIDs.push_back(materialID);
	}

	QObject::connect(m_pMaterialType, SIGNAL(currentIndexChanged(int)), this, SLOT(materialTypeChanged(int)));

	unsigned int currentlySelectedIndex = 0;

	if (m_pMaterial)
	{
		unsigned int typeID = m_pMaterial->getMaterialTypeID();
		std::vector<unsigned int>::iterator itFind = std::find(m_aMaterialIDs.begin(), m_aMaterialIDs.end(), typeID);

		if (itFind != m_aMaterialIDs.end())
		{
			currentlySelectedIndex = itFind - m_aMaterialIDs.begin();
		}
		showMaterial(m_pMaterial);
	}

	m_pMaterialType->setCurrentIndex(currentlySelectedIndex);
}

MaterialWidget::~MaterialWidget()
{
	if (m_pMaterialType)
	{
		delete m_pMaterialType;
		m_pMaterialType = NULL;
	}

	if (m_pParamsPanel)
	{
		if (m_pMaterialContentLastContent)
		{
			m_pMaterialParamsLayout->removeWidget(m_pMaterialContentLastContent);
		}

		delete m_pParamsPanel;
		m_pParamsPanel = NULL;
	}
	else
	{
		if (m_pMaterialContentLastContent)
		{
			m_pMaterialParamsLayout->removeWidget(m_pMaterialContentLastContent);
			delete m_pMaterialContentLastContent;
			m_pMaterialContentLastContent = NULL;
		}
	}

	if (m_pMaterialParamsContainer)
	{
		delete m_pMaterialParamsContainer;
		m_pMaterialParamsContainer = NULL;
	}
}

// this should only be called if materialTypeChanged() is going to be
// called afterwards
void MaterialWidget::deleteCurrentMaterial()
{
	// delete any UI for it first...
	if (m_pMaterialContentLastContent)
	{
		m_pMaterialParamsLayout->removeWidget(m_pMaterialContentLastContent);

		if (m_pParamsPanel)
		{
			delete m_pParamsPanel;
			m_pParamsPanel = NULL;

			m_pMaterialContentLastContent = NULL;
		}
		else
		{
			// not sure how this situation can happen, but...
			delete m_pMaterialContentLastContent;
			m_pMaterialContentLastContent = NULL;
		}
	}

	if (m_pMaterial)
	{
		delete m_pMaterial;
	}
}

void MaterialWidget::setCurrentMaterial(Material* pMaterial)
{
	m_pMaterial = pMaterial;
	*m_pActualMaterial = pMaterial;
}

void MaterialWidget::showMaterial(Material* pMaterial)
{
	Parameters parameters;
	pMaterial->buildParameters(parameters, ParametersInterface::BUILD_PARAMS_EMBEDDED);

	SimpleParametersPanel* pParametersPanel = SimplePanelBuilder::buildParametersPanel(parameters, pMaterial, eMaterialParameter);
	if (!pParametersPanel)
		return;

	m_pParamsPanel = pParametersPanel;

	pParametersPanel->setMaterialWidget(this);

	QWidget* pMatWidget = pParametersPanel->getWidget();

	m_pMaterialParamsLayout->addWidget(pMatWidget);
	m_pMaterialContentLastContent = pMatWidget;
}

void MaterialWidget::materialHasChanged(bool refreshControls)
{
	if (refreshControls)
	{
		refreshMaterialControls();
	}

	emit changed();
}

void MaterialWidget::refreshMaterialControls()
{
	if (m_pParamsPanel)
	{
		m_pParamsPanel->refreshControls();
	}
}

void MaterialWidget::materialTypeChanged(int index)
{
	unsigned int materialID = m_aMaterialIDs[index];

	Material* pNewMaterial = MaterialFactory::instance().createMaterialForTypeID(materialID);

	if (!pNewMaterial)
	{
		// something went wrong, so bail out...
		return;
	}

	deleteCurrentMaterial();
	setCurrentMaterial(pNewMaterial);
	showMaterial(pNewMaterial);

	emit changed();
}
