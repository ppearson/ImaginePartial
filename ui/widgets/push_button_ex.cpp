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

#include "push_button_ex.h"

#include <QIcon>
#include <QString>
#include <QStyle>
#include <QStyleOptionButton>
#include <QMouseEvent>

namespace Imagine
{

PushButtonEx::PushButtonEx(QWidget* parent) : QPushButton(parent)
{
}

PushButtonEx::PushButtonEx(const QIcon& icon, const QString& text, QWidget* parent) : QPushButton(icon, text, parent)
{

}

void PushButtonEx::mousePressEvent(QMouseEvent* e)
{
	QStyleOptionButton style_option;
	style_option.init(this);
	QRect rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &style_option, this);
	QRect popupArea(rect);

	if (popupArea.contains(e->pos()))
	{
		emit clicked();
		return;
	}

	QAbstractButton::mousePressEvent(e);
}

} // namespace Imagine
