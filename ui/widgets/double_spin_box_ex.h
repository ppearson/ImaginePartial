/*
 Imagine
 Copyright 2011-2013 Peter Pearson.

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

#ifndef DOUBLESPINBOXEX_H
#define DOUBLESPINBOXEX_H

#include <QDoubleSpinBox>
#include <QKeyEvent>

namespace Imagine
{

class DoubleSpinBoxEx : public QDoubleSpinBox
{
	Q_OBJECT
public:
	DoubleSpinBoxEx(int precision = 4);
	DoubleSpinBoxEx(QWidget* parent, int precision = 4);

	virtual void stepBy(int steps);
	virtual void wheelEvent(QWheelEvent *event);

	virtual QString textFromValue(double val) const;

	virtual QValidator::State validate(QString& input, int& pos) const;

	QString removeTrailingZeros(const QString& string) const;

	void setBGColour(QColor colour);

	void resetDecimalPlacesToShow();
	void setDecimalPlacesToShow(int places);

protected:
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

	void stepPositionValue(QString& textValue, int position, bool positiveChange);

signals:

public slots:
	void editReturnPressed();

protected:
	int		m_decimalPlacesToShow; // used to indicate whether we should knock off trailing zeros

};

} // namespace Imagine

#endif // DOUBLESPINBOXEX_H
