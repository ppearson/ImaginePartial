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

#include "material_control.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QApplication>

#include "ui/widgets/push_button_ex.h"

#include "view_context.h"

#include "ui/imagine_window.h"
#include "ui/selection_manager.h"

#include "materials/material_factory.h"

#include "materials/material.h"
#include "object.h"

namespace Imagine
{

MaterialControl::MaterialControl(const std::string& name, Object* parentObject, std::string label) : Control(name, label), m_pParentObject(parentObject),
	m_pSelectionManager(NULL), m_shiftPressed(false)
{
	initCommon();
}

void MaterialControl::initCommon()
{
	QWidget* mainWidget = new QWidget();

	QHBoxLayout* layout = new QHBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);

	mainWidget->setMinimumHeight(24);

	m_comboBox = new QComboBox();

	m_comboBox->setMinimumSize(70, 22);

	m_pEditMaterialButton = new PushButtonEx(QIcon(":/imagine/images/material_edit.png"), "", mainWidget);
	m_pEditMaterialButton->setFixedSize(20, 20);
	m_pEditMaterialButton->setToolTip("Edit current material");

	m_pEditNewMaterialButton = new PushButtonEx(QIcon(":/imagine/images/material_edit_new.png"), "", mainWidget);
	m_pEditNewMaterialButton->setFixedSize(20, 20);
	m_pEditNewMaterialButton->setToolTip("Clone material / Create new material (Hold shift for menu)");

	layout->addWidget(m_comboBox);
	layout->addSpacing(5);
	layout->addWidget(m_pEditMaterialButton);
	layout->addSpacing(10);
	layout->addWidget(m_pEditNewMaterialButton);

	m_widget = mainWidget;

	refreshAvailableMaterials();
	refreshFromValue();

	m_pMenu = new QMenu(m_pEditNewMaterialButton);
	m_signalMapper = new QSignalMapper(m_pEditNewMaterialButton);

	MaterialFactory::MaterialNames::iterator it = MaterialFactory::instance().materialNamesSimpleBegin();
	MaterialFactory::MaterialNames::iterator itEnd = MaterialFactory::instance().materialNamesSimpleEnd();

	for (; it != itEnd; ++it)
	{
		const std::string& materialName = (*it).second;
		int materialIndex = (int)(*it).first;
		QAction* pNewMaterial = new QAction(materialName.c_str(), m_pEditNewMaterialButton);
		m_pEditNewMaterialButton->connect(pNewMaterial, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
		m_signalMapper->setMapping(pNewMaterial, materialIndex);

		m_pMenu->addAction(pNewMaterial);
	}

	m_pMenu->addSeparator();

	it = MaterialFactory::instance().materialNamesAdvancedBegin();
	itEnd = MaterialFactory::instance().materialNamesAdvancedEnd();

	for (; it != itEnd; ++it)
	{
		const std::string materialName = (*it).second;
		int materialIndex = (int)(*it).first;
		QAction* pNewMaterial = new QAction(materialName.c_str(), m_pEditNewMaterialButton);
		m_pEditNewMaterialButton->connect(pNewMaterial, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
		m_signalMapper->setMapping(pNewMaterial, materialIndex);

		m_pMenu->addAction(pNewMaterial);
	}

	m_pConnectionProxy->registerComboIndexChangedInt(m_comboBox);
	m_pConnectionProxy->registerButtonClicked(m_pEditMaterialButton, 0);
	m_pConnectionProxy->registerButtonClicked(m_pEditNewMaterialButton, 1);
	m_pConnectionProxy->registerMenuSelected(m_signalMapper);
}

MaterialControl::~MaterialControl()
{

}

bool MaterialControl::valueChanged()
{
	MaterialManager& mm = ViewContext::instance().getScene()->getMaterialManager();

	std::string materialName = m_comboBox->currentText().toStdString();

	Material* pMaterial = mm.getMaterialFromName(materialName);
	setMaterial(pMaterial, true);

	return true;
}

bool MaterialControl::buttonClicked(unsigned int index)
{
	std::string materialName = m_comboBox->currentText().toStdString();
	MaterialManager& mm = ViewContext::instance().getScene()->getMaterialManager();
	Material* pMaterial = mm.getMaterialFromName(materialName);

	ImagineWindow* pMainWindow = ViewContext::instance().getMainWindow();

	if (index == 0)
	{
		refreshAvailableMaterials();
		refreshFromValue();
		pMainWindow->showMaterialEditor(pMaterial);

		return false;
	}
	else if (index == 1)
	{
		// see if shift was held down...
		if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
		{
			QSize menuSize = m_pMenu->sizeHint();
			m_pMenu->move(m_pEditNewMaterialButton->mapToGlobal(QPoint(m_pEditNewMaterialButton->geometry().width() - menuSize.width(),
																	   m_pEditNewMaterialButton->geometry().height())));
			m_pMenu->show();
		}
		else
		{
			bool selectedNewName = false;
			QString newMatName = "";

			while (!selectedNewName)
			{
				bool ok;
				newMatName = QInputDialog::getText(m_widget, "Material Name", "New Material Name", QLineEdit::Normal, newMatName, &ok);
				if (ok)
				{
					if (mm.doesMaterialExist(newMatName.toStdString()))
					{
						QMessageBox mBox;
						mBox.setWindowTitle("Material Error");
						mBox.setText("Material with that name already exists.");

						mBox.exec();
					}
					else
					{
						Material* pNewMaterial = pMaterial->clone();
						pNewMaterial->setName(newMatName.toStdString());
						mm.addMaterial(pNewMaterial, true);
						setMaterial(pNewMaterial, true);
						refreshAvailableMaterials();
						refreshFromValue();
						pMainWindow->showMaterialEditor(pNewMaterial);

						selectedNewName = true;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return true;
}

void MaterialControl::menuSelected(int index)
{
	MaterialManager& mm = ViewContext::instance().getScene()->getMaterialManager();
	ImagineWindow* pMainWindow = ViewContext::instance().getMainWindow();

	bool selectedNewName = false;
	QString newMatName = "";

	while (!selectedNewName)
	{
		bool ok;
		newMatName = QInputDialog::getText(m_widget, "Material Name", "New Material Name", QLineEdit::Normal, newMatName, &ok);
		if (ok)
		{
			if (mm.doesMaterialExist(newMatName.toStdString()))
			{
				QMessageBox mBox;
				mBox.setWindowTitle("Material Error");
				mBox.setText("Material with that name already exists.");

				mBox.exec();
			}
			else
			{
				Material* pNewMaterial = MaterialFactory::instance().createMaterialForTypeID(index);
				pNewMaterial->setName(newMatName.toStdString());
				mm.addMaterial(pNewMaterial, true);
				setMaterial(pNewMaterial, true);
				refreshAvailableMaterials();
				refreshFromValue();
				pMainWindow->showMaterialEditor(pNewMaterial);

				selectedNewName = true;
			}
		}
		else
		{
			break;
		}
	}
}

void MaterialControl::refreshFromValue()
{
	refreshAvailableMaterials();
	Material* pMaterial = getMaterial();

	m_comboBox->blockSignals(true);

	if (pMaterial)
	{
		std::string name = pMaterial->getName();

		// TODO: this isn't great...
		int index = m_comboBox->findText(name.c_str());
		if (index >= 0)
			m_comboBox->setCurrentIndex(index);
	}

	m_comboBox->blockSignals(false);
}

void MaterialControl::refreshAvailableMaterials()
{
	MaterialManager& mm = ViewContext::instance().getScene()->getMaterialManager();

	std::vector<std::string> aNames;
	mm.getMaterialNames(aNames);

	m_comboBox->blockSignals(true);

	m_comboBox->clear();

	std::vector<std::string>::iterator it = aNames.begin();
	for (; it != aNames.end(); ++it)
	{
		const std::string& name = *it;
		m_comboBox->addItem(name.c_str());
	}

	m_comboBox->blockSignals(false);
}

void MaterialControl::setMaterial(Material* pMaterial, bool freeExisting)
{
	if (m_pParentObject)
		m_pParentObject->setMaterial(pMaterial, freeExisting);
	else if (m_pSelectionManager)
		m_pSelectionManager->setMaterial(pMaterial, freeExisting);
}

Material* MaterialControl::getMaterial()
{
	if (m_pParentObject)
		return m_pParentObject->getMaterial();
	else if (m_pSelectionManager)
		return m_pSelectionManager->getMaterial();

	return NULL;
}

void MaterialControl::setSelectionManager()
{
	m_pParentObject = NULL;
	m_pSelectionManager = &SelectionManager::instance();
}

} // namespace Imagine
