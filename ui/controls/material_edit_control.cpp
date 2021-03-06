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

#include "material_edit_control.h"

#include "ui/widgets/material_widget.h"

namespace Imagine
{

MaterialEditControl::MaterialEditControl(const std::string& name, Material** pairedMaterial, const std::string& label) :
	Control(name, label), m_pActualWidget(nullptr)
{
	m_pActualWidget = new MaterialWidget(pairedMaterial, nullptr);

	m_widget = m_pActualWidget;

	m_pConnectionProxy->registerItemChanged(m_widget);
}

MaterialEditControl::~MaterialEditControl()
{

}

bool MaterialEditControl::valueChanged()
{
	return true;
}

void MaterialEditControl::refreshFromValue()
{
	m_pActualWidget->refreshMaterialControls();
}

} // namespace Imagine
