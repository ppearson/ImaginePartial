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

#include "double_spin_box_ex.h"

#include <QLineEdit>

DoubleSpinBoxEx::DoubleSpinBoxEx(int precision) : QDoubleSpinBox(NULL), m_decimalPlacesToShow(0)
{
	setDecimals(precision);
	setButtonSymbols(QAbstractSpinBox::NoButtons);

	//
	QLineEdit* pLE = lineEdit();
	connect(pLE, SIGNAL(returnPressed()), this, SLOT(editReturnPressed()));
}

DoubleSpinBoxEx::DoubleSpinBoxEx(QWidget* parent, int precision) : QDoubleSpinBox(parent), m_decimalPlacesToShow(0)
{
	setDecimals(precision);
	setButtonSymbols(QAbstractSpinBox::NoButtons);

	//
	QLineEdit* pLE = lineEdit();
	connect(pLE, SIGNAL(returnPressed()), this, SLOT(editReturnPressed()));
}

void DoubleSpinBoxEx::stepBy(int steps)
{
	QDoubleSpinBox::stepBy(steps);

	// call editingFinished()
	editingFinished();
}

void DoubleSpinBoxEx::wheelEvent(QWheelEvent *event)
{
	QDoubleSpinBox::wheelEvent(event);

	editingFinished();
}

QString DoubleSpinBoxEx::textFromValue(double val) const
{
	QString returnString = QString("%1").arg(val, 0, 'f', decimals());

	if (m_decimalPlacesToShow < 0)
		return returnString;

	return removeTrailingZeros(returnString);
}

QValidator::State DoubleSpinBoxEx::validate(QString& input, int& pos) const
{
//	return QValidator::Acceptable;
	return QDoubleSpinBox::validate(input, pos);
}

QString DoubleSpinBoxEx::removeTrailingZeros(const QString& string) const
{
	QString decSep = locale().decimalPoint();

	int decPos = string.lastIndexOf(decSep);
	if (decPos == -1)
		return string;

	if (m_decimalPlacesToShow < 0)
		return string;

	QString newString = string;

	int numberOfTrailingZerosToRemove = (newString.length() - decPos) - 1;
	numberOfTrailingZerosToRemove -= m_decimalPlacesToShow;

	int count = 0;
	for (int i = string.length() - 1; i >= decPos; i--)
	{
		if (count++ == numberOfTrailingZerosToRemove)
			break;

#ifndef IMAGINE_QT_5
		char cTest = newString.at(i).toAscii();
#else
		char cTest = newString.at(i).toLatin1();
#endif

		if (cTest == '0' || cTest == decSep.at(0))
			newString = newString.left(newString.length() - 1);
		else
			break;
	}

	// hack - something's not quite right above, meaning we often get a trailing '.', so just remove that
	// manually...
	if (newString.right(1) == ".")
	{
		newString = newString.left(newString.length() - 1);
	}

	return newString;
}

void DoubleSpinBoxEx::setBGColour(QColor colour)
{
	QLineEdit* pLE = lineEdit();
	QPalette palette(pLE->palette());
	palette.setColor(QPalette::Base, colour);
	pLE->setPalette(palette);
	update();
}

void DoubleSpinBoxEx::resetDecimalPlacesToShow()
{
	m_decimalPlacesToShow = 0;
}

void DoubleSpinBoxEx::setDecimalPlacesToShow(int places)
{
	// only overwrite old value if new one's bigger. To have less decimal places shown,
	// resetDecimalPlacesToShow() needs to be called, and the User needs to press Enter
	if (places > m_decimalPlacesToShow)
		m_decimalPlacesToShow = places;
}

void DoubleSpinBoxEx::keyPressEvent(QKeyEvent* event)
{
	if (event->key() != Qt::Key_Down && event->key() != Qt::Key_Up)
	{
/*
		if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
		{
			QString textValue = cleanText();

			double currentValue = value();

			// see if we're an expression
			if (textValue.indexOf("=") != -1)
			{
				std::string fullExpression = textValue.toStdString();

				size_t valueStart = fullExpression.find_first_not_of("=*");
				if (valueStart == std::string::npos)
				{
					QDoubleSpinBox::keyPressEvent(event);
					return;
				}

				std::string assignmentOp = fullExpression.substr(0, valueStart);

				std::string expression = fullExpression.substr(valueStart);

				if (assignmentOp == "*=")
				{
					float mul = atof(expression.c_str());

					double newValue = currentValue * mul;

					setValue(newValue);
				}

				editingFinished();
				return;
			}
		}
*/
		QDoubleSpinBox::keyPressEvent(event);
		return;
	}

	if (event->modifiers() & Qt::CTRL)
	{
		QLineEdit* pLE = lineEdit();
		// see if we've got a selection first...
		int textPositionToChange = -1;
		int selection = pLE->selectionStart();
		if (selection == -1)
		{
			// we don't have a selection, so get the caret position...
			int caretPos = pLE->cursorPosition();
			if (caretPos != -1)
			{
				textPositionToChange = caretPos;
			}
		}
		else
		{
			textPositionToChange = selection;
		}

		QString textValue = cleanText();

		int decimalPos = textValue.indexOf(".");

		bool positive = true;
		if (event->key() == Qt::Key_Down)
			positive = false;

		if (textPositionToChange < textValue.size() && textPositionToChange >= 0 && textPositionToChange != decimalPos)
		{
			stepPositionValue(textValue, textPositionToChange, positive);
		}
		else
		{
			if (textPositionToChange == textValue.size())
			{
				QString localTextValue = textValue;

				// add decimal on the end
				if (decimalPos == -1)
				{
					localTextValue += ".";
				}

				if (textValue == "0" && !positive) // special-case 0 and negative
				{
					localTextValue = "-0.1";
				}
				else
				{
					localTextValue += "1";
				}

				int posTemp = 0;
				if (validate(localTextValue, posTemp) == QValidator::Acceptable)
				{
					double dValue = valueFromText(localTextValue);
					setValue(dValue);

					pLE->setCursorPosition(localTextValue.size() - 1);

					editingFinished();
				}
			}
		}
	}
	else
	{
		QDoubleSpinBox::keyPressEvent(event);
	}
}

void DoubleSpinBoxEx::keyReleaseEvent(QKeyEvent* event)
{
	// if it was Enter, we want to trim trailing zeros again, as it was probably entered by the user...
	if (event->key() == Qt::Key_Return)
	{
		resetDecimalPlacesToShow();
		setDecimalPlacesToShow(0);
	}

	QDoubleSpinBox::keyReleaseEvent(event);
}

void DoubleSpinBoxEx::stepPositionValue(QString& textValue, int position, bool positiveChange)
{
	// get number at position
	QChar num = textValue.at(position);

	if (!num.isDigit())
		return;

	bool originalIsPositive = textValue.at(0) != '-';

	QString localTextValue = textValue;

//	fprintf(stderr, "String: %s\n", textValue.toStdString().c_str());

	// TODO: this stuff seems rediculously overcomplicated...

	bool positionAtLimit = false;
	int finalPositionToChange = position;

	if (num == '9')
	{
		if (positiveChange)
		{
			// can we shift the digit we're changing to the left?
			if (position > 1)
			{
				int itemsToReset = 0;
				while (finalPositionToChange > 1)
				{
					finalPositionToChange -= 1;
					itemsToReset += 1;

					if (textValue.at(finalPositionToChange) == '.' && finalPositionToChange > 1)
					{
						// shift it over one more
						finalPositionToChange -= 1;
						itemsToReset += 1;
					}

					if (textValue.at(finalPositionToChange) != '9')
						break;
				}

				// if finalPosition is decimal point, shift it over by one...
				if (localTextValue.at(finalPositionToChange) == '.' && finalPositionToChange > 0)
				{
					finalPositionToChange -= 1;
					itemsToReset += 1;
				}

				// reset current pos to 0
				for (int i = finalPositionToChange + 1; i < finalPositionToChange + 1 + itemsToReset; i++)
				{
					// if it's not a . replace it
					if (localTextValue.at(i) != '.')
						localTextValue.replace(i, 1, '0');
				}
			}
			else
			{
				positionAtLimit = true;
			}
		}
	}
	else if (num == '0')
	{
		if (!positiveChange)
		{
			// can we shift the digit we're changing to the left?
			if (position > 1)
			{
				int itemsToReset = 0;
				while (finalPositionToChange > 1)
				{
					finalPositionToChange -= 1;
					itemsToReset += 1;

					if (textValue.at(finalPositionToChange) == '.' && finalPositionToChange > 1)
					{
						// shift it over one more
						finalPositionToChange -= 1;
						itemsToReset += 1;
					}

					if (textValue.at(finalPositionToChange) != '0')
						break;
				}

				// if finalPosition is decimal point, shift it over by one...
				if (localTextValue.at(finalPositionToChange) == '.' && finalPositionToChange > 0)
				{
					finalPositionToChange -= 1;
					itemsToReset += 1;
				}

				// reset current pos to 9
				int endPos = std::min(finalPositionToChange + 1 + itemsToReset, localTextValue.size());
				for (int i = finalPositionToChange + 1; i < endPos; i++)
				{
					// if it's not a . replace it
					if (localTextValue.at(i) != '.')
						localTextValue.replace(i, 1, '9');
				}
			}
			else
			{
				positionAtLimit = true;
			}
		}
	}

	QChar changeValue = localTextValue.at(finalPositionToChange);

	if (!positionAtLimit)
	{
#ifndef IMAGINE_QT_5
		char cValue = changeValue.toAscii();
#else
		char cValue = changeValue.toLatin1();
#endif

//		cValue += (positiveChange) ? 1 : -1;
		cValue += ((positiveChange && originalIsPositive) || (!positiveChange && !originalIsPositive)) ? 1 : -1;
		localTextValue.replace(finalPositionToChange, 1, cValue);

		// check it's valid first
		int posTemp = 0;
		if (validate(localTextValue, posTemp) == QValidator::Acceptable)
		{
			// set this here, so that setValue() can use it...
//			m_wasSetByDigitStepping = (cValue == '0');

			int decimalPos = localTextValue.indexOf(".");
			if (decimalPos != -1)
			{
				setDecimalPlacesToShow(position - decimalPos);
			}
			else
			{
				resetDecimalPlacesToShow();
				setDecimalPlacesToShow(0);
			}

			double dValue = valueFromText(localTextValue);
			setValue(dValue);

			editingFinished();
		}
	}

	// if we've shifted the diget, update caret
/*	if (finalPositionToChange != position)
	{
		lineEdit()->setCursorPosition(finalPositionToChange);
	}
*/

}

void DoubleSpinBoxEx::editReturnPressed()
{
	// reset flag turning off trailing zero trimming
	resetDecimalPlacesToShow();
}
