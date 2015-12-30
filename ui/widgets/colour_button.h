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

#ifndef COLOUR_BUTTON_H
#define COLOUR_BUTTON_H

#include <QPushButton>

class ColourChooserPopup;

class ColourButton : public QPushButton
{
	Q_OBJECT
public:
	ColourButton(QWidget* parent = 0);
	~ColourButton();

	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void focusOutEvent(QFocusEvent* event);

	virtual void resizeEvent(QResizeEvent* event);
	void updateInternalsForResize();
	virtual void mouseMoveEvent(QMouseEvent* event);

	void setColour(int red, int green, int blue);
	void setColourf(float red, float green, float blue);

	void setColour(QColor& colour);

	QColor getColour();
	void getColourF(float& r, float& g, float& b);

	void colourPicked(QColor& colour);

	void showPopup();

	void updateInternalColourValues();
	void updateBackBuffer();

signals:
	void changed();
public slots:

protected:
	QColor				m_colour;

	int					m_width;
	int					m_splitPos;

	QPoint				m_lastMousePos; // in button-space
	QPoint				m_lastMouseScreenPos;
	int					m_changeColour;

	bool				m_popupActive;
	ColourChooserPopup*	m_popup;

	//// stuff for drawing the values

	QString				m_strRed;
	QString				m_strGreen;
	QString				m_strBlue;

	QRect				m_rectPopup;
	QRect				m_rectColourArea;
	QRect				m_rectRed;
	QRect				m_rectGreen;
	QRect				m_rectBlue;

	bool				m_mouseOver[3];

	QPixmap				m_backBuffer;
};

#endif // COLOUR_BUTTON_H
