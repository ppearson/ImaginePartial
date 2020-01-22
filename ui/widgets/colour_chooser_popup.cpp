/*
 Imagine
 Copyright 2011-2018 Peter Pearson.

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

#include "colour_chooser_popup.h"

#include <QPainter>
#include <QPaintEvent>
#include <QToolTip>

#include "colour_button.h"

namespace Imagine
{

static const unsigned int kColourPopupRows = 9;
static const unsigned int kColourPopupColumns = 6;

static const int kColourPopupBoxSize = 12;
static const int kColourPopupMargin = 6;

ColourChooserPopup::ColourChooserPopup(ColourButton* owner) : QWidget(0, Qt::Popup),
	m_pPixmap(nullptr), m_pOwner(owner), m_selectedColourIndex(-1)
{
	setAttribute(Qt::WA_DeleteOnClose, false);
	hide();
	setMouseTracking(true);

	buildColours();
}

ColourChooserPopup::~ColourChooserPopup()
{
	if (m_pPixmap)
		delete m_pPixmap;
}

void ColourChooserPopup::buildColours()
{
	m_aColours.emplace_back(QColor(  0,   0,   0));
	m_aColours.emplace_back(QColor( 25,  25,  25));
	m_aColours.emplace_back(QColor( 50,  50,  50));
	m_aColours.emplace_back(QColor( 75,  75,  75));
	m_aColours.emplace_back(QColor(100, 100, 100));
	m_aColours.emplace_back(QColor(125, 125, 125));
	m_aColours.emplace_back(QColor(150, 150, 150));
	m_aColours.emplace_back(QColor(175, 175, 175));
	m_aColours.emplace_back(QColor(200, 200, 200));
	m_aColours.emplace_back(QColor(225, 225, 225));
	m_aColours.emplace_back(QColor(255, 255, 255));
	m_aColours.emplace_back(QColor(128, 128, 128));

	m_aColours.emplace_back(QColor(255,   0,   0));
	m_aColours.emplace_back(QColor(255,  64,  64));
	m_aColours.emplace_back(QColor(255, 128, 128));
	m_aColours.emplace_back(QColor(255, 196, 196));
	m_aColours.emplace_back(QColor(196,   0,   0));
	m_aColours.emplace_back(QColor(128,   0,   0));

	m_aColours.emplace_back(QColor(  0, 255,   0));
	m_aColours.emplace_back(QColor( 64, 255,  64));
	m_aColours.emplace_back(QColor(128, 255, 128));
	m_aColours.emplace_back(QColor(196, 255, 196));
	m_aColours.emplace_back(QColor(  0, 196,   0));
	m_aColours.emplace_back(QColor(  0, 128,   0));

	m_aColours.emplace_back(QColor(  0,   0, 255));
	m_aColours.emplace_back(QColor( 64,  64, 255));
	m_aColours.emplace_back(QColor(128, 128, 255));
	m_aColours.emplace_back(QColor(196, 196, 255));
	m_aColours.emplace_back(QColor(  0,   0, 196));
	m_aColours.emplace_back(QColor(  0,   0, 128));

	m_aColours.emplace_back(QColor(255, 255,   0));
	m_aColours.emplace_back(QColor(255, 255,  64));
	m_aColours.emplace_back(QColor(255, 255, 128));
	m_aColours.emplace_back(QColor(255, 255, 196));
	m_aColours.emplace_back(QColor(196, 196,   0));
	m_aColours.emplace_back(QColor(128, 128,   0));

	m_aColours.emplace_back(QColor(  0, 255, 255));
	m_aColours.emplace_back(QColor( 64, 255, 255));
	m_aColours.emplace_back(QColor(128, 255, 255));
	m_aColours.emplace_back(QColor(196, 255, 255));
	m_aColours.emplace_back(QColor(  0, 196, 196));
	m_aColours.emplace_back(QColor(  0, 128, 128));

	m_aColours.emplace_back(QColor(255,   0, 255));
	m_aColours.emplace_back(QColor(255,  96, 255));
	m_aColours.emplace_back(QColor(255, 128, 255));
	m_aColours.emplace_back(QColor(255, 196, 255));
	m_aColours.emplace_back(QColor(196,   0, 196));
	m_aColours.emplace_back(QColor(128,   0, 128));

	m_aColours.emplace_back(QColor(185, 148, 69)); // gold
	m_aColours.emplace_back(QColor(134, 83, 34));  // terracotta
	m_aColours.emplace_back(QColor(175, 161, 73));  // brass
}

void ColourChooserPopup::buildPixmap()
{

}

void ColourChooserPopup::doLayout()
{
	if (m_pPixmap)
	{
		delete m_pPixmap;
		m_pPixmap = nullptr;
	}

	setFocusPolicy(Qt::StrongFocus);

	m_aBoxes.clear();

	QSize size = getSize();

	m_pPixmap = new QPixmap(size.width(), size.height());

	resize(size.width(), size.height());

	QPainter painter(m_pPixmap);
	painter.fillRect(0, 0, size.width(), size.height(), palette().color(QPalette::Base));

	painter.setPen(Qt::black);

	int x;
	int y;

	unsigned int numColours = m_aColours.size();
	unsigned int count = 0;

	for (unsigned int i = 0; i < kColourPopupRows && count < numColours; i++)
	{
		y = 1 + kColourPopupMargin + (kColourPopupBoxSize + kColourPopupMargin) * i;
		for (unsigned int j = 0; j < kColourPopupColumns && count < numColours; j++)
		{
			x = 1 + kColourPopupMargin + (kColourPopupBoxSize + kColourPopupMargin) * j;
			
			QRect boxRect(x, y, kColourPopupBoxSize, kColourPopupBoxSize);

			if (count == m_selectedColourIndex)
			{
				QRect selectionRect(boxRect);
				selectionRect.adjust(-2, -2, 2, 2);

				painter.setPen(Qt::white);
				painter.drawRect(selectionRect);
				painter.setPen(Qt::black);
			}

			painter.setBrush(QBrush(m_aColours[count]));

			painter.drawRect(boxRect);

			m_aBoxes.emplace_back(boxRect);

			count++;
		}
	}

	painter.end();
}

QSize ColourChooserPopup::getSize()
{
	int width = 2 + kColourPopupMargin + (kColourPopupBoxSize + kColourPopupMargin) * kColourPopupColumns;
	int height = 2 + kColourPopupMargin + (kColourPopupBoxSize + kColourPopupMargin) * kColourPopupRows;

	return QSize(width, height);
}

void ColourChooserPopup::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (m_pPixmap)
		painter.drawPixmap(0, 0, *m_pPixmap);
	painter.setPen(Qt::black);
	painter.drawRect(geometry());

	painter.end();
}

void ColourChooserPopup::mousePressEvent(QMouseEvent* event)
{
	QToolTip::hideText();
	
	unsigned int newIndex = 0;
	bool found = false;
	std::vector<QRect>::iterator it = m_aBoxes.begin();

	QPoint localPoint = mapTo(this, event->pos());
	for (; it != m_aBoxes.end(); ++it)
	{
		QRect& rect = *it;

		if (rect.contains(localPoint))
		{
			found = true;
			m_selectedColourIndex = newIndex;
			break;
		}

		newIndex++;
	}

	if (!found)
	{
		hide();
		return;
	}

	if (!m_pOwner)
		return;

	m_pOwner->colourPicked(m_aColours[m_selectedColourIndex]);
	hide();
}

void ColourChooserPopup::mouseMoveEvent(QMouseEvent* event)
{
	// this is a somewhat nasty implementation, but the feature is useful, so...

	if (!(event->modifiers() & Qt::ShiftModifier))
		return;
	
	QPoint localPoint = mapFrom(this, event->pos());
	
	unsigned int newIndex = 0;
	unsigned int selectedIndex = -1u;
	bool found = false;
	std::vector<QRect>::iterator it = m_aBoxes.begin();
	
	for (; it != m_aBoxes.end(); ++it)
	{
		QRect& rect = *it;

		if (rect.contains(localPoint))
		{
			found = true;
			selectedIndex = newIndex;
			break;
		}

		newIndex++;
	}

	if (!found)
	{
		return;
	}
	
	QColor itemColour = m_aColours[selectedIndex];
	
	char szTemp[24];
	sprintf(szTemp, "(%0.2f, %0.2f, %0.2f)", itemColour.redF(), itemColour.greenF(), itemColour.blueF());
	
	QPoint globalPoint = mapToGlobal(event->pos());
	
	QToolTip::showText(globalPoint, szTemp);
}

} // namespace Imagine
