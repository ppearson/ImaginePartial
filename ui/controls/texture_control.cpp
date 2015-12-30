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

#include "texture_control.h"

#include "textures/texture_parameters.h"

#include "ui/widgets/texture_widget.h"

TextureControl::TextureControl(const std::string& name, TextureParameters* pairedValue, std::string label, unsigned int flags) : Control(name, label),
	m_pTextureWidget(NULL), m_pairedValue(NULL)
{
	m_pTextureWidget = new TextureWidget(pairedValue, NULL, flags);

	m_pairedValue = pairedValue;

	m_widget = m_pTextureWidget;

	m_pConnectionProxy->registerTextureChanged(m_pTextureWidget);
}

TextureControl::~TextureControl()
{

}

bool TextureControl::valueChanged()
{
	return true;
}

void TextureControl::refreshFromValue()
{
	if (m_pTextureWidget)
	{
		if (m_pairedValue->getRequiredType() == Texture::eTypeColour || m_pairedValue->getRequiredType() == Texture::eTypeColourExact)
		{
			m_pTextureWidget->updateColourButtonFromPairedConstant();
		}
		else
		{
			m_pTextureWidget->updateFloatSliderFromPairedConstant();
		}
	}
}
