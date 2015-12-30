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

#ifndef MATERIAL_CONTROL_H
#define MATERIAL_CONTROL_H

#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>

#include <QSignalMapper>

#include "colour/colour3f.h"

#include "control.h"

class Object;
class SelectionManager;
class Material;

class MaterialControl : public Control
{
public:
	MaterialControl(const std::string& name, Object* parentObject, std::string label);
	void initCommon();
	~MaterialControl();

	virtual bool valueChanged();
	virtual bool buttonClicked(unsigned int index);
	virtual void menuSelected(int index);

	virtual void refreshFromValue();

	void refreshAvailableMaterials();
	void setMaterial(Material* pMaterial, bool freeExisting);
	Material* getMaterial();

	// bit of a hack
	void setSelectionManager();

protected:
	QWidget*			m_container;
	QHBoxLayout*		m_layout;
	QComboBox*			m_comboBox;
	QPushButton*		m_pEditMaterialButton;
	QPushButton*		m_pEditNewMaterialButton;

	QMenu*				m_pMenu;

	QSignalMapper*		m_signalMapper;

	Object*				m_pParentObject;
	SelectionManager*	m_pSelectionManager;

	bool				m_shiftPressed;
};

#endif // MATERIAL_CONTROL_H
