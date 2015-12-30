/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#include "scrub_button.h"

#include <QIcon>
#include <QString>
#include <QStyle>
#include <QStyleOptionButton>
#include <QMouseEvent>
#include <QCursor>

ScrubButton::ScrubButton(QWidget* parent) : QPushButton(QIcon(":/imagine/images/scrub_button.png"), "", parent),
	m_mouseDown(false), m_lastMouseX(0), m_lastMouseScreenX(0), m_lastMouseScreenY(0), m_index(-1)
{
	setMaximumWidth(20);
	setMinimumWidth(20);
	setMaximumHeight(20);
	setMinimumHeight(20);

	setFocusPolicy(Qt::NoFocus);
}

void ScrubButton::mousePressEvent(QMouseEvent* e)
{
	QStyleOptionButton style_option;
	style_option.init(this);
	QRect rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &style_option, this);
	QRect popupArea(rect);

	if (popupArea.contains(e->pos()))
	{
		m_mouseDown = true;
		m_lastMouseX = e->pos().x();

		setCursor(Qt::BlankCursor);

		grabMouse();

		m_lastMouseScreenX = QCursor::pos().x();
		m_lastMouseScreenY = QCursor::pos().y();
		return;
	}
}

void ScrubButton::mouseReleaseEvent(QMouseEvent* e)
{
	if (!m_mouseDown)
		return;

	QCursor::setPos(m_lastMouseScreenX, m_lastMouseScreenY);

	setCursor(Qt::ArrowCursor);

	m_mouseDown = false;

	releaseMouse();
}

void ScrubButton::mouseMoveEvent(QMouseEvent* e)
{
	if (!m_mouseDown)
		return;

//	e->accept();

	int newMousePosX = e->pos().x();
	if (newMousePosX != m_lastMouseX)
	{
		int delta = newMousePosX - m_lastMouseX;
		float fDelta = (float)delta;

		if (e->modifiers() & Qt::SHIFT)
			fDelta *= 0.25f;

		// reset the cursor so it doesn't actually move (much)
		QCursor::setPos(m_lastMouseScreenX, m_lastMouseScreenY);
//		m_lastMouseX = newMousePosX;

		if (m_index == -1)
		{
			emit deltaMove(fDelta);
		}
		else
		{
			emit deltaMoveIndex(fDelta, m_index);
		}
	}
}
