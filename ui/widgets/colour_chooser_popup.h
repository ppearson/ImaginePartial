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

#ifndef COLOUR_CHOOSER_POPUP_H
#define COLOUR_CHOOSER_POPUP_H

#include <QWidget>
#include <QPixmap>
#include <QColor>

#include <QList>
#include <QRect>

namespace Imagine
{

class ColourButton;

class ColourChooserPopup : public QWidget
{
    Q_OBJECT
public:
	ColourChooserPopup(ColourButton* owner);
	virtual ~ColourChooserPopup();

	void buildColours();
	void buildPixmap();

	void doLayout();

	QSize getSize();

signals:
	void closed();

public slots:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

protected:
	QPixmap*	m_pPixmap;
	ColourButton*	m_pOwner;

	unsigned int	m_selectedColourIndex;
	std::vector<QColor>	m_aColours;
	std::vector<QRect>	m_aBoxes;
};

} // namespace Imagine

#endif // COLOUR_CHOOSER_POPUP_H
