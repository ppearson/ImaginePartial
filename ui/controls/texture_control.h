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

#ifndef TEXTURE_CONTROL_H
#define TEXTURE_CONTROL_H

#include "control.h"

namespace Imagine
{

class TextureWidget;
class TextureParameters;

class TextureControl : public Control
{
public:
	TextureControl(const std::string& name, TextureParameters* pairedValue, std::string label, unsigned int flags);
	virtual ~TextureControl();

	virtual bool valueChanged();

	virtual void refreshFromValue();

protected:
	TextureWidget*		m_pTextureWidget;
	TextureParameters*	m_pairedValue;
};

} // namespace Imagine

#endif // TEXTURE_CONTROL_H
