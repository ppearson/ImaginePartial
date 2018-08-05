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

#include "texture_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QColorDialog>

#include <cmath>

#include "colour_button.h"
#include "file_browse_widget.h"
#include "scrub_button.h"
#include "double_spin_box_ex.h"
#include "push_button_ex.h"
#include "float_slider_widget.h"

// for texture gen
#include "image/image_1f.h"
#include "image/image_1h.h"
#include "image/image_1b.h"
#include "image/image_colour3f.h"
#include "image/image_colour3h.h"
#include "image/image_colour3b.h"
#include "io/file_io_registry.h"
#include "io/image_reader.h"

#include "textures/image/image_texture_1f.h"
#include "textures/image/image_texture_1h.h"
#include "textures/image/image_texture_1b.h"
#include "textures/image/image_texture_3f.h"
#include "textures/image/image_texture_3h.h"
#include "textures/image/image_texture_3b.h"
#include "textures/texture_registry.h"
#include "textures/texture_parameters.h"
#include "textures/constant.h"
#include "textures/constant_float.h"

#include "parameter.h"
#include "simple_parameters_panel.h"
#include "simple_panel_builder.h"

#include "view_context.h"

#include "utils/file_helpers.h"

namespace Imagine
{

TextureWidget::TextureWidget(TextureParameters* pParams, QWidget* parent, unsigned int flags) : QWidget(parent),
	m_pTexture(NULL), m_scaleU(1.0f), m_scaleV(1.0f), m_performAutoFileBrowse(true), m_logScale(false), m_highPrecision(false)
{
	m_pPairedValue = pParams;
	if (m_pPairedValue)
	{
		m_outputType = m_pPairedValue->getRequiredType();
		m_pTexture = m_pPairedValue->getTexture();

		m_logScale = (flags & eParameterFloatSliderLogScale);
		m_highPrecision = (flags & eParameterFloatSliderHighPrecision);
	}

	initCommon();
}

TextureWidget::TextureWidget(Texture::RequiredType outputType, bool performAutoFileBrowse, QWidget* parent) : QWidget(parent),
	m_outputType(outputType), m_scaleU(1.0f), m_scaleV(1.0f), m_performAutoFileBrowse(performAutoFileBrowse), m_logScale(false), m_highPrecision(false)
{
	m_pPairedValue = NULL;
	m_pTexture = NULL;

	initCommon();
}

void TextureWidget::initCommon()
{
	m_pTopContentWidget = NULL;
	m_pTopContentWidgetLayout = NULL;
	m_pTopContentLastContent = NULL;

	m_pMainContentWidget = NULL;
	m_pMainContentWidgetLayout = NULL;
	m_pMainContentLastContent = NULL;
	m_pMainContentLastParametersPanel = NULL;

	m_pMenuNone = NULL;
	m_pMenuConstant = NULL;

	m_lastContentWasCustom = false;

	m_pConstantColour = NULL;
	m_cachedConstantColour = Colour3f(0.6f);

	m_pFloatSlider = NULL;

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* topLayoutWidget = new QWidget();

	QHBoxLayout* topLayout = new QHBoxLayout(topLayoutWidget);
	topLayout->setSpacing(0);
	topLayout->setMargin(0);
	topLayout->setContentsMargins(0, 0, 0, 0);

	mainLayout->addWidget(topLayoutWidget);
	mainLayout->addSpacing(0);


	m_pTopContentWidget = new QWidget();
	m_pTopContentWidgetLayout = new QVBoxLayout(m_pTopContentWidget);
	m_pTopContentWidgetLayout->setSpacing(0);
	m_pTopContentWidgetLayout->setMargin(0);
	m_pTopContentWidgetLayout->setContentsMargins(0, 0, 0, 0);

	//

	m_pMainContentWidget = new QWidget();
	m_pMainContentWidgetLayout = new QVBoxLayout(m_pMainContentWidget);
	m_pMainContentWidgetLayout->setContentsMargins(0, 0, 0, 0);

	m_pScaleContainerWidget = new QWidget();
	m_pScaleContainerLayout = new QHBoxLayout(m_pScaleContainerWidget);
	m_pScaleContainerLayout->setContentsMargins(0, 0, 0, 0);

	m_pTextScale1 = new DoubleSpinBoxEx();
	m_pTextScale1->setValue(m_scaleU);
	m_pTextScale1->setMinimum(0.00001);
	m_pTextScale1->setMaximum(1000);
	m_pTextScale2 = new DoubleSpinBoxEx();
	m_pTextScale2->setValue(m_scaleV);
	m_pTextScale2->setMinimum(0.00001);
	m_pTextScale2->setMaximum(1000);

	m_pScaleScrub1 = new ScrubButton();
	m_pScaleScrub1->setIndex(0);
	m_pScaleScrub2 = new ScrubButton();
	m_pScaleScrub2->setIndex(1);

	QLabel* pScaleLabel = new QLabel("Scale:");

	m_pScaleContainerLayout->addWidget(pScaleLabel);
	m_pScaleContainerLayout->addWidget(m_pTextScale1);
	m_pScaleContainerLayout->addWidget(m_pScaleScrub1);
	m_pScaleContainerLayout->addSpacing(10);
	m_pScaleContainerLayout->addWidget(m_pTextScale2);
	m_pScaleContainerLayout->addWidget(m_pScaleScrub2);

	m_pMainContentLastContent = NULL;

	mainLayout->addWidget(m_pMainContentWidget);

	topLayout->addWidget(m_pTopContentWidget);

	m_pTypeButton = new PushButtonEx();
	m_pTypeButton->setMaximumWidth(20);
	m_pTypeButton->setMinimumWidth(20);
	m_pTypeButton->setMaximumHeight(20);
	m_pTypeButton->setMinimumHeight(20);
	m_pTypeButton->setFocusPolicy(Qt::NoFocus);

	m_pTypeButton->setStyleSheet("QPushButton::menu-indicator{ image: url(blank_icon.png); }");

	topLayout->addSpacing(10);
	topLayout->addWidget(m_pTypeButton);

	QMenu* pNewMenu = new QMenu(m_pTypeButton);
	m_pTypeButton->setMenu(pNewMenu);

	m_pSignalMapper = new QSignalMapper(m_pTypeButton);

	///

	// add menu options and create any optional content controls

	if (m_pPairedValue && m_pPairedValue->isOptional())
	{
		m_pMenuNone = new QAction("None", m_pTypeButton);
		m_pMenuNone->setCheckable(true);
		connect(m_pMenuNone, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));
		m_pSignalMapper->setMapping(m_pMenuNone, 256);

		pNewMenu->addAction(m_pMenuNone);

		m_aTextureIDs.push_back((unsigned char)256); // For None type
	}

	if (m_outputType == Texture::eTypeColour || m_outputType == Texture::eTypeColourExact)
	{
		m_pConstantColour = new ColourButton();

		m_pMenuConstant = new QAction("Constant Colour", m_pTypeButton);
		m_pMenuConstant->setCheckable(true);
		connect(m_pMenuConstant, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));
		connect(m_pConstantColour, SIGNAL(clicked()), this, SLOT(constantColourButtonClicked()));
		m_pSignalMapper->setMapping(m_pMenuConstant, 0);

		pNewMenu->addAction(m_pMenuConstant);
	}
	else if (m_outputType == Texture::eTypeFloatExact)
	{
		// it's a single float value that will be used to drive something else

		m_pFloatSlider = new FloatSliderWidget(0.0f, 1.0f, true, m_logScale, m_highPrecision);

		m_pMenuConstant = new QAction("Constant Value", m_pTypeButton);
		m_pMenuConstant->setCheckable(true);
		connect(m_pMenuConstant, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));

		connect(m_pFloatSlider, SIGNAL(valueChanged()), this, SLOT(floatSliderChanged()));

		m_pSignalMapper->setMapping(m_pMenuConstant, 0);

		pNewMenu->addAction(m_pMenuConstant);
	}

	m_pImageFile = new FileBrowseWidget("");
	m_pImageFile->setDirectoriesOnly(false);

	m_pMenuImage = new QAction("Image", m_pTypeButton);
	m_pMenuImage->setCheckable(true);
	connect(m_pMenuImage, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));
	m_pSignalMapper->setMapping(m_pMenuImage, 1);

	pNewMenu->addAction(m_pMenuImage);

	pNewMenu->addSeparator();

	// manually add first two for constant and image
	if (m_outputType == Texture::eTypeColour)
	{
		m_aTextureIDs.push_back(0);
		m_aTextureIDs.push_back(1);
	}
	else
	{
		if (m_outputType == Texture::eTypeFloatExact)
		{
			m_aTextureIDs.push_back(0);
		}
		m_aTextureIDs.push_back(2); // float texture
	}

	if (m_outputType == Texture::eTypeColour)
	{
		m_offset = 2; // start at 2, after constant and Image
	}
	else
	{
		m_offset = 1; // we don't have a constant
		if (m_pPairedValue && m_pPairedValue->isOptional())
		{
			m_offset += 1;
		}

		if (m_outputType == Texture::eTypeFloatExact)
		{
			// fixed float - we do have a constant menu item..
			m_offset += 1;
		}
	}
	
	// sort them for display
	std::map<std::string, unsigned char> aSortedNames;

	TextureRegistry::TextureNames::iterator itTexture = TextureRegistry::instance().standardTextureNamesBegin();
	for (; itTexture != TextureRegistry::instance().standardTextureNamesEnd(); ++itTexture)
	{
		const std::string& textureName = (*itTexture).second;
		int textureTypeID = (int)(*itTexture).first;

		// hacky, but...
		if (textureName == "Constant")
			continue;
		
		aSortedNames[textureName] = (unsigned char)textureTypeID;
	}
	
	int menuIndex = m_offset;
	
	std::map<std::string, unsigned char>::const_iterator itTextName = aSortedNames.begin();
	for (; itTextName != aSortedNames.end(); ++itTextName)
	{
		const std::string& textureName = (*itTextName).first;
		int textureTypeID = (int)(*itTextName).second;
		
		m_aTextureIDs.push_back((unsigned char)textureTypeID);

		QAction* pNewTextureType = new QAction(textureName.c_str(), m_pTypeButton);
		m_pTypeButton->connect(pNewTextureType, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));
		m_pSignalMapper->setMapping(pNewTextureType, menuIndex);

		pNewTextureType->setCheckable(true);
		m_aMenuOther.push_back(pNewTextureType);

		// hack to set procedural texture menu checked state if it matched paired value
		if (m_pPairedValue)
		{
			if ((int)m_pPairedValue->getTextureTypeID() == textureTypeID)
			{
				pNewTextureType->setChecked(true);
			}
		}

		pNewMenu->addAction(pNewTextureType);

		menuIndex++;
	}

	// now add advanced ones
	pNewMenu->addSeparator();

	itTexture = TextureRegistry::instance().advancedTextureNamesBegin();
	for (; itTexture != TextureRegistry::instance().advancedTextureNamesEnd(); ++itTexture)
	{
		const std::string& textureName = (*itTexture).second;

		int textureTypeID = (int)(*itTexture).first;

		m_aTextureIDs.push_back((unsigned char)textureTypeID);

		QAction* pNewTextureType = new QAction(textureName.c_str(), m_pTypeButton);
		m_pTypeButton->connect(pNewTextureType, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));
		m_pSignalMapper->setMapping(pNewTextureType, menuIndex);

		pNewTextureType->setCheckable(true);
		m_aMenuOther.push_back(pNewTextureType);

		// hack to set procedural texture menu checked state if it matched paired value
		if (m_pPairedValue)
		{
			if ((int)m_pPairedValue->getTextureTypeID() == textureTypeID)
			{
				pNewTextureType->setChecked(true);
			}
		}

		pNewMenu->addAction(pNewTextureType);

		menuIndex++;
	}


	if (m_pPairedValue)
	{
		// if we're attached to a paired value, set the type dependent on that

		if (m_pPairedValue->isOptional() && !m_pPairedValue->isEnabled())
		{
			m_pMenuNone->setChecked(true);
			m_overallType = eTextureNone;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_none.png"));
		}
		else
		{
			int typeID = (int)m_pPairedValue->getTextureTypeID();

			if (typeID == 0)
			{
				m_overallType = eTextureConstant;
				m_pMenuConstant->setChecked(true);

				if (m_outputType == Texture::eTypeColour || m_outputType == Texture::eTypeColourExact)
				{
					// update cached colour first time around
					Constant* pConstant = static_cast<Constant*>(m_pPairedValue->getTexture());
					m_cachedConstantColour = pConstant->getColour();

					updateColourButtonFromPairedConstant();
				}
				else
				{
					// update cached float first time around
					ConstantFloat* pConstant = static_cast<ConstantFloat*>(m_pPairedValue->getTexture());
					m_cachedConstantFloat = pConstant->getFloat();

					updateFloatSliderFromPairedConstant();
				}
			}
			else if (typeID == 1 || typeID == 2) // colour or float texture images
			{
				m_overallType = eTextureImage;
				m_pMenuImage->setChecked(true);

				updateFileBrowserFromPairedImage();
				updateScalesFromPairedTexture();

				// float textures need to set this to 1 to emulate colour file type
				typeID = 1;
			}
			else
			{
				updateScalesFromPairedTexture();
			}

			showContent(typeID, false);
		}
	}
	else
	{
		// otherwise, do it based on output type...

		if (m_outputType == Texture::eTypeColour)
		{
			m_overallType = eTextureConstant;
			showContent(0, false);

			m_pMenuConstant->setChecked(true);
		}
		else
		{
			m_overallType = eTextureImage;

			m_pMenuImage->setChecked(true);

			showContent(1, false);
		}
	}

	////

	connect(m_pTypeButton, SIGNAL(clicked()), this, SLOT(typeButtonClicked()));
	connect(m_pSignalMapper, SIGNAL(mapped(int)), this, SLOT(textureTypeChanged(int)));

	if (m_outputType == Texture::eTypeColour)
	{
		connect(m_pConstantColour, SIGNAL(changed()), this, SLOT(colourPickerChanged()));
	}

	connect(m_pImageFile, SIGNAL(pathHasChanged()), this, SLOT(filePathChanged()));

	connect(m_pTextScale1, SIGNAL(editingFinished()), this, SLOT(uvScaleUChanged()));
	connect(m_pTextScale2, SIGNAL(editingFinished()), this, SLOT(uvScaleVChanged()));

	connect(m_pScaleScrub1, SIGNAL(deltaMoveIndex(float,int)), this, SLOT(textureScaleChanged(float,int)));
	connect(m_pScaleScrub2, SIGNAL(deltaMoveIndex(float,int)), this, SLOT(textureScaleChanged(float,int)));

	////
}

TextureWidget::~TextureWidget()
{
	if (m_pMainContentLastParametersPanel)
	{
		delete m_pMainContentLastParametersPanel;
	}

	if (!m_pPairedValue && m_pTexture)
	{
		// if we're not paired, we created the texture, so destroy it
		delete m_pTexture;
		m_pTexture = NULL;
	}

	if (m_pScaleContainerWidget)
	{
		delete m_pScaleContainerWidget;
		m_pScaleContainerWidget = NULL;
	}

	if (m_pConstantColour)
	{
		delete m_pConstantColour;
		m_pConstantColour = NULL;
	}

	if (m_pImageFile)
	{
		delete m_pImageFile;
		m_pImageFile = NULL;
	}
}

void TextureWidget::showContent(int index, bool textureModified)
{
	// TODO: lots of duplication here...

	std::string textureName;

	// if we're a Colour output type, clear the top item
	if (m_outputType == Texture::eTypeColour)
	{
		clearTopLayout();

		if (index == 0)
		{
			updateColourButtonFromPairedConstant();

			m_pTopContentWidgetLayout->addWidget(m_pConstantColour);
			m_pTopContentLastContent = m_pConstantColour;
			m_pTopContentLastContent->show();

			m_pMainContentWidget->setVisible(false);
			if (m_pMainContentLastContent)
			{
				m_pMainContentWidgetLayout->removeWidget(m_pMainContentLastContent);
				m_pMainContentLastContent->hide();
				m_pMainContentLastContent = NULL;
			}

			if (m_pMainContentLastParametersPanel)
			{
				delete m_pMainContentLastParametersPanel;
				m_pMainContentLastParametersPanel = NULL;
			}

			m_lastContentWasCustom = false;

			m_overallType = eTextureConstant;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_constant.png"));
			m_pTypeButton->setToolTip("Constant");
		}
		else if (index == 1)
		{
			setupTopWidgetScale(true, NULL);
			updateScalesFromPairedTexture();

			if (m_pMainContentLastContent)
			{
				m_pMainContentWidgetLayout->removeWidget(m_pMainContentLastContent);
				m_pMainContentLastContent->hide();
			}

			if (m_pMainContentLastParametersPanel)
			{
				delete m_pMainContentLastParametersPanel;
				m_pMainContentLastParametersPanel = NULL;
			}

			m_pMainContentWidget->setVisible(true);
			m_pMainContentWidgetLayout->addWidget(m_pImageFile);
			m_pMainContentLastContent = m_pImageFile;
			m_pMainContentLastContent->show();
			m_lastContentWasCustom = false;

			m_overallType = eTextureImage;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_image.png"));
			m_pTypeButton->setToolTip("Image");

			if (m_performAutoFileBrowse && textureModified)
			{
				// if the current text is empty, browse automatically
				m_pImageFile->performBrowse(true);
			}
		}
		else if (index != 256)
		{
			if (m_pMainContentLastContent)
			{
				m_pMainContentWidgetLayout->removeWidget(m_pMainContentLastContent);
				m_pMainContentLastContent->hide();
			}

			if (m_pMainContentLastParametersPanel)
			{
				delete m_pMainContentLastParametersPanel;
				m_pMainContentLastParametersPanel = NULL;
			}

			// get actual textureID from menu ID
			unsigned int textureID = m_aTextureIDs[index];

			// paired values' textures are set higher up in textureTypeChanged()...
			if (!m_pPairedValue)
			{
				if (m_pTexture)
				{
					delete m_pTexture;
				}

				m_pTexture = TextureRegistry::instance().createTextureForTypeID((unsigned char)textureID);
			}

			if (m_pTexture)
			{
				textureName = TextureRegistry::instance().getTextureName((unsigned char)textureID);

				if (!m_pTexture->isScalable())
				{
					clearTopLayout();
				}
				else
				{
					setupTopWidgetScale(true, NULL);
					updateScalesFromPairedTexture();
				}

				Parameters textureParameters;
				m_pTexture->buildParameters(textureParameters, 0);

				SimpleParametersPanel* pParametersPanel = SimplePanelBuilder::buildParametersPanel(textureParameters, m_pTexture, eTextureParameter);

				if (pParametersPanel)
				{
					m_lastContentWasCustom = true;
					m_pMainContentLastParametersPanel = pParametersPanel;
					m_pMainContentLastContent = pParametersPanel->getWidget();
					m_pMainContentWidgetLayout->addWidget(m_pMainContentLastContent);

					pParametersPanel->setTextureWidget(this);

					m_pMainContentWidget->setVisible(true);
				}
				else
				{
					// there wasn't a parameters panel, so set the last content to NULL
					m_pMainContentLastContent = NULL;
				}
			}

			m_overallType = eTextureProcedural;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_procedural.png"));
			m_pTypeButton->setToolTip(textureName.c_str());
		}

		m_pMainContentWidgetLayout->update();
	}
	else
	{
		clearTopLayout();

		if (index == 0) // float slider
		{
			updateFloatSliderFromPairedConstant();

			m_pTopContentWidgetLayout->addWidget(m_pFloatSlider);
			m_pTopContentLastContent = m_pFloatSlider;
			m_pTopContentLastContent->show();

			m_pMainContentWidget->setVisible(false);
			if (m_pMainContentLastContent)
			{
				m_pMainContentWidgetLayout->removeWidget(m_pMainContentLastContent);
				m_pMainContentLastContent->hide();
				m_pMainContentLastContent = NULL;
			}

			if (m_pMainContentLastParametersPanel)
			{
				delete m_pMainContentLastParametersPanel;
				m_pMainContentLastParametersPanel = NULL;
			}

			m_lastContentWasCustom = false;

			m_overallType = eTextureConstant;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_constant.png"));
			m_pTypeButton->setToolTip("Constant");
		}
		else if (index == 1) // because there isn't a colour image type, this is actually the menu index instead of textureID
		{
			setupTopWidgetScale(true, NULL);

			if (m_pMainContentLastContent)
			{
				m_pMainContentWidgetLayout->removeWidget(m_pMainContentLastContent);
				m_pMainContentLastContent->hide();
			}

			if (m_pMainContentLastParametersPanel)
			{
				delete m_pMainContentLastParametersPanel;
				m_pMainContentLastParametersPanel = NULL;
			}

			m_pMainContentWidget->setVisible(true);
			m_pMainContentWidgetLayout->addWidget(m_pImageFile);
			m_pMainContentLastContent = m_pImageFile;
			m_pMainContentLastContent->show();

			m_lastContentWasCustom = false;

			m_overallType = eTextureImage;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_image.png"));
			m_pTypeButton->setToolTip("Image");

			if (m_performAutoFileBrowse)
			{
				// if the current text is empty, browse automatically
				m_pImageFile->performBrowse(true);
			}
		}
		else if (index != 256)
		{
			if (m_pMainContentLastContent)
			{
				m_pMainContentWidgetLayout->removeWidget(m_pMainContentLastContent);
				m_pMainContentLastContent->hide();
			}

			if (m_pMainContentLastParametersPanel)
			{
				delete m_pMainContentLastParametersPanel;
				m_pMainContentLastParametersPanel = NULL;
			}

			// get actual textureID from menu ID
			unsigned int textureID = m_aTextureIDs[index];

			// paired values' textures are set higher up in textureTypeChanged()...
			if (!m_pPairedValue)
			{
				// we're in control of the texture, so delete it
				if (m_pTexture)
				{
					delete m_pTexture;
				}

				m_pTexture = TextureRegistry::instance().createTextureForTypeID((unsigned char)textureID);
			}

			if (m_pTexture)
			{
				textureName = TextureRegistry::instance().getTextureName((unsigned char)textureID);

				if (!m_pTexture->isScalable())
				{
					clearTopLayout();
				}
				else
				{
					setupTopWidgetScale(true, NULL);
				}

				Parameters textureParameters;
				m_pTexture->buildParameters(textureParameters, 0);

				SimpleParametersPanel* pParametersPanel = SimplePanelBuilder::buildParametersPanel(textureParameters, m_pTexture, eTextureParameter);

				if (pParametersPanel)
				{
					m_pMainContentLastParametersPanel = pParametersPanel;
					m_pMainContentLastContent = pParametersPanel->getWidget();
					m_pMainContentWidgetLayout->addWidget(m_pMainContentLastContent);
					m_lastContentWasCustom = true;

					pParametersPanel->setTextureWidget(this);

					m_pMainContentWidget->setVisible(true);
				}
				else
				{
					// there wasn't a parameters panel, so set the last content to NULL
					m_pMainContentLastContent = NULL;
				}
			}

			m_overallType = eTextureProcedural;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_procedural.png"));
			m_pTypeButton->setToolTip(textureName.c_str());
		}
		else
		{
			// it's 256, so it's none

			clearTopLayout();

			if (m_pMainContentLastContent)
			{
				m_pMainContentWidgetLayout->removeWidget(m_pMainContentLastContent);
				m_pMainContentLastContent->hide();
				m_pMainContentLastContent = NULL;
			}

			if (m_pMainContentLastParametersPanel)
			{
				delete m_pMainContentLastParametersPanel;
				m_pMainContentLastParametersPanel = NULL;
			}

			m_overallType = eTextureNone;
			m_pTypeButton->setIcon(QIcon(":/imagine/images/texture_none.png"));
			m_pTypeButton->setToolTip("None");
		}

		m_pMainContentWidgetLayout->update();
	}
}

void TextureWidget::clearTopLayout()
{
	// because we've got a spacer, we need to do it this way
	int items = m_pTopContentWidgetLayout->count();
	while (items > 0)
	{
		QLayoutItem* pItem = m_pTopContentWidgetLayout->itemAt(0);
		QWidget* pWidget = pItem->widget();
		if (pWidget)
		{
			pWidget->hide();
		}
		m_pTopContentWidgetLayout->removeItem(pItem);
		items = m_pTopContentWidgetLayout->count();
	}

	m_pTopContentWidgetLayout->update();
}

// TODO: if addWidget is false, don't bother removing then re-adding scale widget
void TextureWidget::setupTopWidgetScale(bool addWidget, Texture* pTexture)
{
	m_pTopContentWidgetLayout->addWidget(m_pScaleContainerWidget);
	m_pTopContentLastContent = m_pScaleContainerWidget;

	m_pTopContentWidgetLayout->update();

	m_pTopContentLastContent->show();
}

Texture* TextureWidget::generateFloatTexture() const
{
	Texture* pNewTexture = NULL;
	if (m_overallType == eTextureProcedural)
	{
		// because we're not paired, we control the texture
		if (!m_pPairedValue && m_pTexture)
		{
			pNewTexture = m_pTexture->clone();
			pNewTexture->updateTexture(true, false);
			return pNewTexture;
		}
	}

	std::string filePath = m_pImageFile->getPath();

	if (m_overallType == eTextureImage && filePath.empty())
	{
		return NULL;
	}

	// load in the texture...
	std::string extension = FileHelpers::getFileExtension(filePath);

	ImageReader* pImageReader = FileIORegistry::instance().createImageReaderForExtension(extension);
	if (pImageReader)
	{
		Image* pDisplacementMapImage = pImageReader->readGreyscaleImage(filePath, Image::IMAGE_FLAGS_EXACT);
		delete pImageReader;

		if (!pDisplacementMapImage)
			return NULL;

		if (pDisplacementMapImage->getImageType() == (Image::IMAGE_CHANNELS_1 | Image::IMAGE_FORMAT_FLOAT))
		{
			Image1f* pImage1f = static_cast<Image1f*>(pDisplacementMapImage);
			pNewTexture = new ImageTexture1f(pImage1f, pDisplacementMapImage->getWidth(), pDisplacementMapImage->getHeight(), 1.0f, 1.0f);
		}
		else if (pDisplacementMapImage->getImageType() == (Image::IMAGE_CHANNELS_1 | Image::IMAGE_FORMAT_HALF))
		{
			Image1h* pImage1h = static_cast<Image1h*>(pDisplacementMapImage);
			pNewTexture = new ImageTexture1h(pImage1h, pDisplacementMapImage->getWidth(), pDisplacementMapImage->getHeight(), 1.0f, 1.0f);
		}
		else if (pDisplacementMapImage->getImageType() == (Image::IMAGE_CHANNELS_1 | Image::IMAGE_FORMAT_BYTE))
		{
			Image1b* pImage1b = static_cast<Image1b*>(pDisplacementMapImage);
			pNewTexture = new ImageTexture1b(pImage1b, pDisplacementMapImage->getWidth(), pDisplacementMapImage->getHeight(), 1.0f, 1.0f);
		}
	}

	return pNewTexture;
}

Texture* TextureWidget::generateColourTexture() const
{
	Texture* pNewTexture = NULL;
	if (m_overallType == eTextureProcedural)
	{
		// because we're not paired, we control the texture
		if (!m_pPairedValue && m_pTexture)
		{
			pNewTexture = m_pTexture->clone();
			pNewTexture->updateTexture(true, false);
			return pNewTexture;
		}
	}

	std::string filePath = m_pImageFile->getPath();

	if (m_overallType == eTextureImage && filePath.empty())
	{
		return NULL;
	}

	// load in the texture...
	std::string extension = FileHelpers::getFileExtension(filePath);

	ImageReader* pImageReader = FileIORegistry::instance().createImageReaderForExtension(extension);
	if (pImageReader)
	{
		Image* pDisplacementMapImage = pImageReader->readColourImage(filePath, Image::IMAGE_FORMAT_NATIVE);
		delete pImageReader;

		if (!pDisplacementMapImage)
			return NULL;

		if (pDisplacementMapImage->getImageType() == (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_FLOAT))
		{
			ImageColour3f* pImageColour3f = static_cast<ImageColour3f*>(pDisplacementMapImage);
			pNewTexture = new ImageTexture3f(pImageColour3f, pDisplacementMapImage->getWidth(), pDisplacementMapImage->getHeight(), 1.0f, 1.0f);
		}
		else if (pDisplacementMapImage->getImageType() == (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_HALF))
		{
			ImageColour3h* pImageColour3h = static_cast<ImageColour3h*>(pDisplacementMapImage);
			pNewTexture = new ImageTexture3h(pImageColour3h, pDisplacementMapImage->getWidth(), pDisplacementMapImage->getHeight(), 1.0f, 1.0f);
		}
		else if (pDisplacementMapImage->getImageType() == (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_BYTE))
		{
			ImageColour3b* pImageColour3b = static_cast<ImageColour3b*>(pDisplacementMapImage);
			pNewTexture = new ImageTexture3b(pImageColour3b, pDisplacementMapImage->getWidth(), pDisplacementMapImage->getHeight(), 1.0f, 1.0f);
		}
	}

	return pNewTexture;
}

void TextureWidget::clearMenuChecks()
{
	if (m_pMenuNone)
		m_pMenuNone->setChecked(false);

	if (m_pMenuConstant)
		m_pMenuConstant->setChecked(false);

	m_pMenuImage->setChecked(false);

	std::vector<QAction*>::iterator itMenu = m_aMenuOther.begin();
	for (; itMenu != m_aMenuOther.end(); ++itMenu)
	{
		QAction* pAction = *itMenu;

		pAction->setChecked(false);
	}
}

// helpers

void TextureWidget::updateColourButtonFromPairedConstant()
{
	if (!m_pPairedValue || m_pPairedValue->getTextureTypeID() != 0)
		return;

	Constant* pConstant = static_cast<Constant*>(m_pPairedValue->getTexture());

	if (pConstant)
	{
		Colour3f colour = pConstant->getColour();
		m_pConstantColour->setColourf(colour.r, colour.g, colour.b);
	}
}

void TextureWidget::updateFloatSliderFromPairedConstant()
{
	if (!m_pPairedValue || m_pPairedValue->getTextureTypeID() != 0)
		return;

	ConstantFloat* pConstant = static_cast<ConstantFloat*>(m_pPairedValue->getTexture());

	if (pConstant)
	{
		float value = pConstant->getFloat();
		m_pFloatSlider->setValue(value);
	}
}

void TextureWidget::updateFileBrowserFromPairedImage()
{
	if (!m_pPairedValue || (m_pPairedValue->getTextureTypeID() != 1 && m_pPairedValue->getTextureTypeID() != 2))
		return;

	if (m_outputType == Texture::eTypeColour)
	{
		ImageTextureBase* pImageTexture = static_cast<ImageTextureBase*>(m_pPairedValue->getTexture());
		if (pImageTexture)
		{
			std::string path = pImageTexture->getImagePath();
			m_pImageFile->setPath(path);
		}
	}
	else
	{
		ImageTextureBase* pImageTexture = static_cast<ImageTextureBase*>(m_pPairedValue->getTexture());
		if (pImageTexture)
		{
			std::string path = pImageTexture->getImagePath();
			m_pImageFile->setPath(path);
		}
	}
}

void TextureWidget::updateScalesFromPairedTexture()
{
	if (!m_pPairedValue || m_pPairedValue->getTextureTypeID() == 0)
		return;

	if (!m_pTexture)
		return;

	m_scaleU = m_pTexture->getScaleU();
	m_scaleV = m_pTexture->getScaleV();

	m_pTextScale1->setValue(m_scaleU);
	m_pTextScale2->setValue(m_scaleV);
}

void TextureWidget::textureHasChanged()
{
	emit textureChanged();
}

//

void TextureWidget::textureTypeChanged(int index)
{
	if (m_pPairedValue)
	{
		unsigned int finalTextureID = index;
		
		ViewContext::instance().cancelReRender();

		if (m_pPairedValue->isOptional() && index == 256)
		{
			m_pPairedValue->setEnabled(false); // this deletes the texture
		}
		else
		{
			if (m_pPairedValue->isOptional())
			{
				m_pPairedValue->setEnabled(true);
			}

			if (finalTextureID == 0)
			{
				m_pPairedValue->changeTextureInstanceToType(finalTextureID);
				// set colour to cached colour
				Constant* pConstant = static_cast<Constant*>(m_pPairedValue->getTexture());
				pConstant->setColour(m_cachedConstantColour);
			}
			else if (finalTextureID == 1)
			{
				if (m_pPairedValue->getRequiredType() == Texture::eTypeColour)
				{
					m_pPairedValue->setToColourImageTexture(m_pImageFile->getPath());
				}
				else
				{
					m_pPairedValue->setToFloatImageTexture(m_pImageFile->getPath());
				}
			}
			else
			{
				// get actual textureID from menu ID
				unsigned char textureID = m_aTextureIDs[index];
				m_pPairedValue->changeTextureInstanceToType(textureID);
			}

			m_pTexture = m_pPairedValue->getTexture();
		}
	}

	clearMenuChecks();

	if (index == 256)
	{
		m_pMenuNone->setChecked(true);
	}
	else if (index == 0)
	{
		m_pMenuConstant->setChecked(true);
	}
	else if (index == 1)
	{
		m_pMenuImage->setChecked(true);
	}
	else
	{
		// otherwise, decrement by offset of 2 to get into array
		unsigned int otherIndex = index - m_offset;

		QAction* pAction = m_aMenuOther[otherIndex];

		pAction->setChecked(true);
	}

	showContent(index, true);

	emit textureChanged();
}

void TextureWidget::textureScaleChanged(float delta, int index)
{
	if (!m_pTexture)
		return;

	delta *= 0.01f;

	float currentValue = (index == 0) ? m_scaleU : m_scaleV;

	float max = 1000.0f;
	float min = 0.0001f;

	bool changed = false;

	if (delta > 0.0f)
	{
		if (currentValue <= max - delta)
		{
			currentValue += delta;
			changed = true;
		}
		else if (currentValue < max)
		{
			currentValue = max;
			changed = true;
		}
	}
	else if (delta < 0.0f)
	{
		float aDelta = fabsf(delta);
		if (currentValue >= min + aDelta)
		{
			currentValue -= aDelta;
			changed = true;
		}
		else if (currentValue > min)
		{
			currentValue = min;
			changed = true;
		}
	}

	if (changed)
	{
		// update widget to match value
		if (index == 0) // U
		{
			m_scaleU = currentValue;
			m_pTextScale1->setValue(m_scaleU);
		}
		else // V
		{
			m_scaleV = currentValue;
			m_pTextScale2->setValue(m_scaleV);
		}

		m_pTexture->setScaleValues(m_scaleU, m_scaleV);

		emit textureChanged();
	}
}

void TextureWidget::colourPickerChanged()
{
	// this assumes we only get this slot called if we're a colour item so the colour button is valid...

	Colour3f newColour;
	m_pConstantColour->getColourF(newColour.r, newColour.g, newColour.b);

	// assume we've got a constant colour texture

	Constant* pConstant = static_cast<Constant*>(m_pTexture);

	if (pConstant)
	{
		pConstant->setColour(newColour);
	}

	emit textureChanged();
}

void TextureWidget::floatSliderChanged()
{
	// this assumes we only get this slot called if we're a float with EXACT item so the float slider is valid...

	float value = m_pFloatSlider->getValue();

	// assume we've got a constant colour texture

	ConstantFloat* pConstant = static_cast<ConstantFloat*>(m_pTexture);

	if (pConstant)
	{
		pConstant->setFloat(value);
	}

	emit textureChanged();
}

void TextureWidget::filePathChanged()
{
	std::string newPath = m_pImageFile->getPath();

	if (!FileHelpers::isTokenFile(newPath) && !FileHelpers::doesFileExist(newPath))
		return;

	if (m_pPairedValue)
	{
		if (m_pPairedValue->getRequiredType() == Texture::eTypeColour)
		{
			m_pPairedValue->setToColourImageTexture(newPath);
		}
		else
		{
			m_pPairedValue->setToFloatImageTexture(newPath);
		}

		// update our pointer to the new internal texture
		m_pTexture = m_pPairedValue->getTexture();
	}

	emit textureChanged();
}

void TextureWidget::uvScaleUChanged()
{
	float newValue = m_pTextScale1->value();

	if (newValue != m_scaleU)
	{
		m_scaleU = newValue;

		if (m_pTexture)
		{
			m_pTexture->setScaleValues(m_scaleU, m_scaleV);
		}

		emit textureChanged();
	}
}

void TextureWidget::uvScaleVChanged()
{
	float newValue = m_pTextScale2->value();

	if (newValue != m_scaleV)
	{
		m_scaleV = newValue;

		if (m_pTexture)
		{
			m_pTexture->setScaleValues(m_scaleU, m_scaleV);
		}

		emit textureChanged();
	}
}

void TextureWidget::typeButtonClicked()
{
	QMenu* pMenu = m_pTypeButton->menu();
	QSize menuSize = pMenu->sizeHint();

	QSize buttonSize = m_pTypeButton->geometry().size();

	pMenu->move(m_pTypeButton->mapToGlobal(QPoint(buttonSize.width() - menuSize.width(), buttonSize.height())));
	pMenu->show();
}

void TextureWidget::constantColourButtonClicked()
{
	Colour3f previousColour;
	m_pConstantColour->getColourF(previousColour.r, previousColour.g, previousColour.b);

	QColor colour;
	colour.setRgbF(previousColour.r, previousColour.g, previousColour.b);

	colour = QColorDialog::getColor(colour);

	// if colour is invalid, the user cancelled the colour dialog
	if (!colour.isValid())
		return;

	float red = colour.redF();
	float green = colour.greenF();
	float blue = colour.blueF();

	// assume we've got a constant colour texture
	Constant* pConstant = static_cast<Constant*>(m_pTexture);

	if (pConstant)
	{
		pConstant->setColour(Colour3f(red, green, blue));
	}

	m_pConstantColour->setColourf(red, green, blue);

	emit textureChanged();
}

} // namespace Imagine
