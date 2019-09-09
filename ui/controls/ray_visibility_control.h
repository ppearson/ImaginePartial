/*
 Imagine
 Copyright 2013 Peter Pearson.

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

#ifndef RAY_VISIBILITY_CONTROL_H
#define RAY_VISIBILITY_CONTROL_H

#include "control.h"

#include "ui/widgets/ray_visibility_button.h"

namespace Imagine
{

class RayVisibilityControl : public Control
{
public:
	RayVisibilityControl(const std::string& name, unsigned char* pairedValue, const std::string& label);
	virtual ~RayVisibilityControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

protected:
	RayVisibilityButton*	m_pVisibilityButton;
	unsigned char*			m_pairedValue;
};

} // namespace Imagine

#endif // RAY_VISIBILITY_CONTROL_H
