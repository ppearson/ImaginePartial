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

#ifndef MATERIAL_WIDGET_H
#define MATERIAL_WIDGET_H

#include <QWidget>

#include <vector>

class QComboBox;
class QVBoxLayout;

namespace Imagine
{

class Material;

class SimpleParametersPanel;

class MaterialWidget : public QWidget
{
	Q_OBJECT
public:
	MaterialWidget(Material** pPairedMaterial, QWidget* parent = NULL);
	virtual ~MaterialWidget();

	void deleteCurrentMaterial();
	void setCurrentMaterial(Material* pMaterial);
	void showMaterial(Material* pMaterial);

	void materialHasChanged(bool refreshControls);

	void refreshMaterialControls();

signals:
	void changed();

public slots:
	void materialTypeChanged(int index);

protected:
	// We don't own these, but when changing type, we do delete the material and create
	// a new one, setting the pointer appropriately
	Material**				m_pActualMaterial; // points to the actual pointer in the Mix Material
	Material*				m_pMaterial; // points to the above

	SimpleParametersPanel*	m_pParamsPanel; // pointer to the last Params panel created


	QComboBox*				m_pMaterialType;
	QWidget*				m_pMaterialParamsContainer;
	QVBoxLayout*			m_pMaterialParamsLayout;
	QWidget*				m_pMaterialContentLastContent;

	std::vector<unsigned int>	m_aMaterialIDs;
};

} // namespace Imagine

#endif // MATERIAL_WIDGET_H
