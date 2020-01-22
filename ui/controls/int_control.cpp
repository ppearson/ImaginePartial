/*
 Imagine
 Copyright 2011-2016 Peter Pearson.

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

#include "int_control.h"

#include <QHBoxLayout>
#include <cmath>

#include "ui/widgets/scrub_button.h"

namespace Imagine
{

IntControl::IntControl(const std::string& name, int* pairedValue, int min, int max, const std::string& label,
						 bool scrubHandle) :
	Control(name, label), m_minimum(min), m_maximum(max)
{
	m_intSpin = new SpinBoxEx();

	m_intSpin->setMinimumSize(100, 22);

	m_intSpin->setMinimum(min);
	m_intSpin->setMaximum(max);

	m_intSpin->setValue(*pairedValue);

	QWidget* pMainWidget = m_intSpin;

	if (scrubHandle)
	{
		QWidget* mainWidget = new QWidget();
		mainWidget->setMinimumSize(100, 26);
		mainWidget->setContentsMargins(0, 0, 0, 0);

		QHBoxLayout* layout = new QHBoxLayout(mainWidget);
		mainWidget->setLayout(layout);
		layout->setSpacing(0);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setMargin(0);

		layout->addWidget(m_intSpin);

		layout->addSpacing(5);

		ScrubButton* pScrubButton = new ScrubButton();

		layout->addWidget(pScrubButton);

		pMainWidget = mainWidget;

		m_pConnectionProxy->registerDeltaChange(pScrubButton, 0);
	}

	m_widget = pMainWidget;

	m_pairedValue = pairedValue;

	m_pConnectionProxy->registerValueChangedDouble(m_intSpin);
}

IntControl::~IntControl()
{
}

bool IntControl::valueChanged()
{
	int newValue = m_intSpin->value();

	if (*m_pairedValue != newValue)
	{
		*m_pairedValue = m_intSpin->value();
		return true;
	}

	return false;
}

void IntControl::refreshFromValue()
{
	m_intSpin->setValue(*m_pairedValue);
}

bool IntControl::deltaChange(float delta, unsigned int index)
{
	bool changed = false;
	int iDelta = (int)delta;
	// need to check the bounds.
	int currentValue = *m_pairedValue;
	if (iDelta > 0)
	{
		if (currentValue <= m_maximum - iDelta)
		{
			*m_pairedValue += iDelta;
			changed = true;
		}

	}
	else if (iDelta < 0)
	{
		int aDelta = abs(iDelta);
		if (currentValue >= m_minimum + aDelta)
		{
			*m_pairedValue -= aDelta;
			changed = true;
		}
	}

	refreshFromValue();

	return changed;
}

} // namespace Imagine
