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

#include "colour_button.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QMouseEvent>

#include "utils/maths/maths.h"

#include "colour_chooser_popup.h"

namespace Imagine
{

static const int kDropArrowX = 30;

ColourButton::ColourButton(QWidget* parent) : QPushButton(parent), m_popupActive(false), m_popup(NULL)
{
	m_popup = new ColourChooserPopup(this);

	m_mouseOver[0] = false;
	m_mouseOver[1] = false;
	m_mouseOver[2] = false;

	updateInternalsForResize();

//	setMouseTracking(true);
	setFocusPolicy(Qt::NoFocus);

	setAttribute(Qt::WA_TranslucentBackground, true);

	setToolTip("Scrub individual colour components with mouse, or hold down Ctrl to gang all three.");
}

ColourButton::~ColourButton()
{
	if (m_popup)
	{
		delete m_popup;
		m_popup = NULL;
	}
}

void ColourButton::paintEvent(QPaintEvent* event)
{
	QPushButton::paintEvent(event);

	QPainter painter(this);

	painter.setBrush(Qt::black);

	int x = m_rectPopup.center().x();
	int y = m_rectPopup.center().y();

	QPolygon dropArrow;
	dropArrow << QPoint(x - 5, y - 4) << QPoint(x + 5, y - 4) << QPoint(x, y + 4);

//	painter.drawEllipse(m_rectPopup.center(), 5, 5);
	painter.drawPolygon(dropArrow);

	//
	// draw the colour and text values...
	painter.drawPixmap(m_rectColourArea.left(), m_rectColourArea.top(), m_backBuffer);
}

void ColourButton::mousePressEvent(QMouseEvent* event)
{
	m_changeColour = -1;

	// if in right hand section, show popup

	if (m_rectPopup.contains(event->pos()))
	{
		showPopup();
	}
	else
	{
		// record mouse down pos
		m_lastMousePos = event->pos();

		setCursor(Qt::BlankCursor);
		grabMouse();

		m_lastMouseScreenPos = QCursor::pos();

		if (m_rectRed.contains(m_lastMousePos))
		{
			m_changeColour = 0;
		}
		else if (m_rectGreen.contains(m_lastMousePos))
		{
			m_changeColour = 1;
		}
		else if (m_rectBlue.contains(m_lastMousePos))
		{
			m_changeColour = 2;
		}
	}
}

void ColourButton::mouseReleaseEvent(QMouseEvent* event)
{
	QCursor::setPos(m_lastMouseScreenPos);

	setCursor(Qt::ArrowCursor);

	releaseMouse();
}

void ColourButton::mouseDoubleClickEvent(QMouseEvent* event)
{
	emit clicked(); // emulate standard single-click
}

void ColourButton::focusOutEvent(QFocusEvent* event)
{
	if (m_popupActive && m_popup)
	{
		m_popup->hide();
		m_popupActive = false;
	}
}

void ColourButton::resizeEvent(QResizeEvent* event)
{
	updateInternalsForResize();
}

void ColourButton::updateInternalsForResize()
{
	QStyleOptionButton style_option;
	style_option.init(this);
	QRect rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &style_option, this);
	QRect popupArea(rect);

	int splitPos = rect.width() - kDropArrowX;
	popupArea.adjust(splitPos, 0, 0, 0);

	m_rectPopup = popupArea;

	QRect mainArea = style()->subElementRect(QStyle::SE_PushButtonContents, &style_option, this);
	QRect colourArea(mainArea);
	colourArea.adjust(5, 3, -kDropArrowX, -3);

	m_rectColourArea = colourArea;

	unsigned int valueSpacingWidth = colourArea.width() / 3;
	m_rectRed = colourArea;
	m_rectGreen = colourArea;
	m_rectBlue = colourArea;

	unsigned int division = m_rectColourArea.left() + valueSpacingWidth;

	m_rectRed.setRight(division);
	m_rectGreen.setLeft(division);

	division += valueSpacingWidth;

	m_rectGreen.setRight(division);
	m_rectBlue.setLeft(division);

	updateBackBuffer();
}

void ColourButton::mouseMoveEvent(QMouseEvent* event)
{
	m_mouseOver[0] = false;
	m_mouseOver[1] = false;
	m_mouseOver[2] = false;

	if (!m_rectColourArea.contains(event->pos()))
	{
		return;
	}

	bool changedByScrub = false;

	float deltaChange = 0.0f;
	// work out delta mouse move is mouse button is down and the mouse was over a particular colour
	if ((event->buttons() & Qt::LeftButton) && m_changeColour >= 0)
	{
		static const float increment = 1.0f / 255.0f;
		QPoint newMousePos = event->pos();

		int mouseXChange = (newMousePos - m_lastMousePos).x();
		deltaChange = (float)mouseXChange * increment;

		if (event->modifiers() & Qt::SHIFT)
			deltaChange *= 0.1f;

		QCursor::setPos(m_lastMouseScreenPos);
//		m_lastMousePos = newMousePos;

		// if we've got a valid diff, change the appropriate value
		if (mouseXChange != 0)
		{
			if (event->modifiers() & Qt::CTRL)
			{
				float newColour = clamp(m_colour.redF() + deltaChange);
				m_colour.setRedF(newColour);
				newColour = clamp(m_colour.greenF() + deltaChange);
				m_colour.setGreenF(newColour);
				newColour = clamp(m_colour.blueF() + deltaChange);
				m_colour.setBlueF(newColour);
			}
			else
			{
				if (m_changeColour == 0) // R
				{
					float newColour = clamp(m_colour.redF() + deltaChange);
					m_colour.setRedF(newColour);
				}
				else if (m_changeColour == 1)
				{
					float newColour = clamp(m_colour.greenF() + deltaChange);
					m_colour.setGreenF(newColour);
				}
				else if (m_changeColour == 2)
				{
					float newColour = clamp(m_colour.blueF() + deltaChange);
					m_colour.setBlueF(newColour);
				}
			}
			changedByScrub = true;
		}
	}

	if (changedByScrub)
	{
//		event->ignore();
		updateInternalColourValues();

		changed();
	}
}

void ColourButton::setColour(int red, int green, int blue)
{
	m_colour.setRgb(red, green, blue);
	updateInternalColourValues();
}

void ColourButton::setColourf(float red, float green, float blue)
{
	m_colour.setRgbF(red, green, blue);
	updateInternalColourValues();
}

void ColourButton::setColour(QColor& colour)
{
	m_colour = colour;
	updateInternalColourValues();
}

QColor ColourButton::getColour()
{
	return m_colour;
}

void ColourButton::getColourF(float& r, float& g, float& b)
{
	r = m_colour.redF();
	g = m_colour.greenF();
	b = m_colour.blueF();
}

void ColourButton::colourPicked(QColor& colour)
{
	m_colour = colour;

	updateInternalColourValues();

	changed();
}

void ColourButton::showPopup()
{
	m_popupActive = true;

	QSize popupSize = m_popup->getSize();

	m_popup->move(mapToGlobal(QPoint(geometry().width() - popupSize.width(), geometry().height())));
	m_popup->doLayout();
	m_popup->show();
	m_popup->setVisible(true); // for some reason, Linux needs this...
}

void ColourButton::updateInternalColourValues()
{
	m_strRed = QString("%1").arg(m_colour.redF(), 0, 'g', 2);
	m_strGreen = QString("%1").arg(m_colour.greenF(), 0, 'g', 2);
	m_strBlue = QString("%1").arg(m_colour.blueF(), 0, 'g', 2);

	updateBackBuffer();
}

void ColourButton::updateBackBuffer()
{
	QRect localColourRect(m_rectColourArea);
	localColourRect.adjust(-m_rectColourArea.left(), -m_rectColourArea.top(), -m_rectColourArea.left(), -m_rectColourArea.top() - 1);

	QRect pixmapSize(localColourRect);
	pixmapSize.adjust(0, 0, 1, 1); // for some reason, we need to padd this to get the full size...
	m_backBuffer = QPixmap(pixmapSize.size());
//	m_backBuffer.fill(palette().color(QPalette::Base));
	m_backBuffer.fill(Qt::transparent);

	QPainter painter(&m_backBuffer);
	painter.initFrom(this);

	painter.setPen(Qt::black);
	painter.setBrush(m_colour);
	painter.drawRect(localColourRect);

	// draw values...

	QFont newFont = font();
	newFont.setPointSize(9);
	setFont(newFont);

	QFontMetrics metrics(font());

	unsigned int redWidth = metrics.width(m_strRed);
	unsigned int greenWidth = metrics.width(m_strGreen);
	unsigned int blueWidth = metrics.width(m_strBlue);

	unsigned int offsetX = m_rectColourArea.left();

	// QPainter origin is top left...
	unsigned int textX = localColourRect.left();
	unsigned int textY = localColourRect.top() + metrics.height() - 1;
	if (localColourRect.height() < metrics.height())
	{
		// on some Linux envs with KDE, metrics is actually a weird value which doesn't make sense, so bodge it for the moment
		textY -= 4;
	}

	// set text colour appropriate for background colour
	float luminance = m_colour.lightnessF();
	if (luminance < 0.45f || (m_colour.blueF() > 0.8f && m_colour.redF() < 0.5f))
	{
		painter.setPen(Qt::white);
	}
	else
	{
		painter.setPen(Qt::black);
	}


	textX = m_rectRed.center().x() - (redWidth / 2) - offsetX;
	painter.drawText(textX, textY, m_strRed);

	textX = m_rectGreen.center().x() - (greenWidth / 2) - offsetX;
	painter.drawText(textX, textY, m_strGreen);

	textX = m_rectBlue.center().x() - (blueWidth / 2) - offsetX;
	painter.drawText(textX, textY, m_strBlue);

	update();
}

} // namespace Imagine
