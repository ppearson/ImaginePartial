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

#ifndef MATERIAL_EDIT_CONTROL_H
#define MATERIAL_EDIT_CONTROL_H

#include "control.h"

class Material;

class MaterialWidget;

class MaterialEditControl : public Control
{
public:
	MaterialEditControl(const std::string& name, Material** pairedMaterial, std::string label = "");
	virtual ~MaterialEditControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();


protected:
	MaterialWidget*	m_pActualWidget;
};

#endif // MATERIAL_EDIT_CONTROL_H
