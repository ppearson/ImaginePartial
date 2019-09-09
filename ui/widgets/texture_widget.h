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

#ifndef TEXTURE_WIDGET_H
#define TEXTURE_WIDGET_H

#include <QWidget>
#include <QSignalMapper>

#include <string>
#include <vector>
#include <map>

#include "textures/texture.h"

class QComboBox;
class QMenu;
class QLineEdit;
class QPushButton;
class QAction;
class QVBoxLayout;
class QHBoxLayout;

namespace Imagine
{

class ColourButton;
class FileBrowseWidget;
class DoubleSpinBoxEx;
class ScrubButton;
class PushButtonEx;
class FloatSliderWidget;

class SimpleParametersPanel;

class ImageTexture1f;
class TextureParameters;

class TextureWidget : public QWidget
{
	Q_OBJECT

public:
	enum OverallTextureType
	{
		eTextureNone,
		eTextureConstant,
		eTextureImage,
		eTextureProcedural
	};

	// passing the TextureParameters* pointer to the widget is a bit naughty, as there should be a clean separation,
	// but as there's quite a bit of state to syncronise between the two, it's easier to do it this way than add loads of
	// bindings to ControlConnectionProxy to do the same
	TextureWidget(TextureParameters* pParams, QWidget* parent, unsigned int flags);

	// without a bound TextureParameters*, so it can be used stand-alone
	TextureWidget(Texture::RequiredType outputType, bool performAutoFileBrowse, QWidget* parent = NULL);

	void initCommon();

	virtual ~TextureWidget();

	void showContent(int index, bool textureModified);

	void clearTopLayout();
	void setupTopWidgetScale(bool addWidget, Texture* pTexture);

	Texture* generateFloatTexture() const;
	Texture* generateColourTexture() const;

	void clearMenuChecks();

	// helpers
	void updateColourButtonFromPairedConstant();
	void updateFloatSliderFromPairedConstant();
	void updateFileBrowserFromPairedImage();
	void updateScalesFromPairedTexture();

	void textureHasChanged();
	
protected:
	void addMenuItems(const std::map<std::string, unsigned char>& items, QMenu* newMenu,
					  unsigned int& nextMenuIndex);

public slots:
	// internal
	void textureTypeChanged(int index);
	void textureScaleChanged(float delta, int index);
	void colourPickerChanged();
	void floatSliderChanged();
	void filePathChanged();
	void uvScaleUChanged();
	void uvScaleVChanged();
	void typeButtonClicked();
	void constantColourButtonClicked(); // for double-click really

signals:
	// external
	void textureChanged();

protected:
	Texture::RequiredType 	m_outputType;
	OverallTextureType		m_overallType;
	std::vector<unsigned char>	m_aTextureIDs;
	unsigned int			m_offset; // offset after Constant and Image...

	TextureParameters*		m_pPairedValue;
	// we're only in control of this if m_pPairedValue is NULL, otherwise it's just a copy
	Texture*				m_pTexture;
	float					m_scaleU;
	float					m_scaleV;

	Colour3f				m_cachedConstantColour;
	float					m_cachedConstantFloat;

// Widgets...
	PushButtonEx*			m_pTypeButton;
	QSignalMapper*			m_pSignalMapper;

	QWidget*				m_pTopContentWidget;
	QVBoxLayout*			m_pTopContentWidgetLayout;
	QWidget*				m_pTopContentLastContent;

	// for main extended content
	QWidget*				m_pMainContentWidget;
	QVBoxLayout*			m_pMainContentWidgetLayout;
	QWidget*				m_pMainContentLastContent;
	SimpleParametersPanel*	m_pMainContentLastParametersPanel;
	bool					m_lastContentWasCustom; // indicates it wasn't a constant or Image, so we need to delete it

	ColourButton*			m_pConstantColour;
	FileBrowseWidget*		m_pImageFile;
	FloatSliderWidget*		m_pFloatSlider;

	QWidget*				m_pScaleContainerWidget;
	QHBoxLayout*			m_pScaleContainerLayout;

	DoubleSpinBoxEx*		m_pTextScale1;
	DoubleSpinBoxEx*		m_pTextScale2;
	ScrubButton*			m_pScaleScrub1;
	ScrubButton*			m_pScaleScrub2;

	// menus
	QAction*				m_pMenuNone;
	QAction*				m_pMenuConstant;
	QAction*				m_pMenuImage;
	std::vector<QAction*>	m_aMenuOther;

	bool					m_performAutoFileBrowse;
	bool					m_logScale;
	bool					m_highPrecision;
};

} // namespace Imagine

#endif // TEXTURE_WIDGET_H
