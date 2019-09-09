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

#include "float_control.h"

#include <QHBoxLayout>
#include <math.h>

#include "parameter.h"

#include "ui/widgets/scrub_button.h"

namespace Imagine
{

FloatControl::FloatControl(const std::string& name, float* pairedValue, float min, float max, const std::string& label, unsigned int flags)
	: Control(name, label), m_minimum(min), m_maximum(max)
{
	m_doubleSpin = new DoubleSpinBoxEx();

	m_doubleSpin->setMinimumSize(100, 22);

	m_doubleSpin->setMinimum(min);
	m_doubleSpin->setMaximum(max);

	if (flags & eParameterFloatEditHighPrecision)
	{
		m_doubleSpin->setDecimals(6);
	}

	m_doubleSpin->setValue(*pairedValue);

	QWidget* pMainWidget = m_doubleSpin;

	if (flags & eParameterScrubButton || flags & eParameterScrubButtonFine)
	{
		m_delta = 0.1f;
		if (flags & eParameterScrubButtonFine)
		{
			if (flags & eParameterFloatEditHighPrecision)
			{
				m_delta = 0.00001f;
			}
			else
			{
				m_delta = 0.01f;
			}
		}

		QWidget* mainWidget = new QWidget();

		QHBoxLayout* layout = new QHBoxLayout(mainWidget);
		mainWidget->setLayout(layout);
		mainWidget->setMinimumSize(100, 26);
		layout->setSpacing(0);
		layout->setMargin(0);

		layout->addWidget(m_doubleSpin);

		layout->addSpacing(5);

		ScrubButton* pScrubButton = new ScrubButton();

		layout->addWidget(pScrubButton);

		pMainWidget = mainWidget;

		m_pConnectionProxy->registerDeltaChange(pScrubButton, 0);
	}

	m_widget = pMainWidget;

	m_pairedValue = pairedValue;

	m_lastValue = *pairedValue;

	m_pConnectionProxy->registerValueChangedDouble(m_doubleSpin);
}

FloatControl::~FloatControl()
{

}

bool FloatControl::valueChanged()
{
	// update paired float to string value

	*m_pairedValue = m_doubleSpin->value();

	if (*m_pairedValue != m_lastValue)
	{
		m_lastValue = *m_pairedValue;
		return true;
	}

	return false;
}

void FloatControl::refreshFromValue()
{
	m_doubleSpin->setValue(*m_pairedValue);
	m_lastValue = *m_pairedValue;
}

bool FloatControl::deltaChange(float delta, unsigned int index)
{
	bool changed = false;
	float currentValue = *m_pairedValue;
	delta *= m_delta;
	if (delta > 0.0f)
	{
		if (currentValue <= m_maximum - delta)
		{
			*m_pairedValue += delta;
			changed = true;
		}
		else if (currentValue < m_maximum)
		{
			*m_pairedValue = m_maximum;
			changed = true;
		}
	}
	else if (delta < 0.0f)
	{
		float aDelta = fabsf(delta);
		if (currentValue >= m_minimum + aDelta)
		{
			*m_pairedValue -= aDelta;
			changed = true;
		}
		else if (currentValue > m_minimum)
		{
			*m_pairedValue = m_minimum;
			changed = true;
		}
	}

	if (changed)
	{
		refreshFromValue();
	}

	return changed;
}

} // namespace Imagine
