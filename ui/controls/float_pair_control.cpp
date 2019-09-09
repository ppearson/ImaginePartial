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

#include "float_pair_control.h"

#include <QHBoxLayout>
#include <math.h>

#include "parameter.h"
#include "widgets/double_spin_box_ex.h"
#include "ui/widgets/scrub_button.h"

namespace Imagine
{

FloatPairControl::FloatPairControl(const std::string& name, float* pairedValue1, float* pairedValue2, float min, float max, const std::string& label,
			 unsigned int flags) : Control(name, label), m_minimum1(min), m_maximum1(max), m_minimum2(min), m_maximum2(max)
{
	QWidget* mainWidget = new QWidget();
	mainWidget->setMinimumHeight(26);

	QHBoxLayout* layout = new QHBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);
//	layout->setContentsMargins(0, 0, 0, 0);

	m_doubleSpin1 = new DoubleSpinBoxEx();

	m_doubleSpin1->setMinimumSize(70, 22);

	m_doubleSpin1->setMinimum(min);
	m_doubleSpin1->setMaximum(max);

	m_doubleSpin1->setValue(*pairedValue1);

	m_doubleSpin2 = new DoubleSpinBoxEx();

//	m_doubleSpin1->setMi
	m_doubleSpin2->setMinimumSize(70, 22);

	m_doubleSpin2->setMinimum(min);
	m_doubleSpin2->setMaximum(max);

	m_doubleSpin2->setValue(*pairedValue2);

	layout->addWidget(m_doubleSpin1);

	if (flags & eParameterScrubButton || flags & eParameterScrubButtonFine)
	{
		m_delta = 0.1f;
		if (flags & eParameterScrubButtonFine)
			m_delta = 0.01f;

		ScrubButton* pScrubButton1 = new ScrubButton();
		ScrubButton* pScrubButton2 = new ScrubButton();

		layout->addSpacing(5);
		layout->addWidget(pScrubButton1);
		layout->addSpacerItem(new QSpacerItem(10, 0));
		layout->addWidget(m_doubleSpin2);
		layout->addSpacing(5);
		layout->addWidget(pScrubButton2);

		m_pConnectionProxy->registerDeltaChange(pScrubButton1, 0);
		m_pConnectionProxy->registerDeltaChange(pScrubButton2, 1);
	}
	else
	{
		layout->addSpacing(5);
		layout->addWidget(m_doubleSpin2);
	}

	m_widget = mainWidget;

	m_pairedValue1 = pairedValue1;
	m_pairedValue2 = pairedValue2;

	m_lastValue1 = *pairedValue1;
	m_lastValue2 = *pairedValue2;

	m_pConnectionProxy->registerValueChangedDouble(m_doubleSpin1);
	m_pConnectionProxy->registerValueChangedDouble(m_doubleSpin2);
}

FloatPairControl::FloatPairControl(const std::string& name, float* pairedValue1, float* pairedValue2, float min1, float max1,
				 float min2, float max2, const std::string& label, unsigned int flags) : Control(name, label), m_minimum1(min1), m_maximum1(max1),
				m_minimum2(min2), m_maximum2(max2)
{
	QWidget* mainWidget = new QWidget();
	mainWidget->setMinimumHeight(26);

	QHBoxLayout* layout = new QHBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(0);
	layout->setMargin(0);

	m_doubleSpin1 = new DoubleSpinBoxEx();

	m_doubleSpin1->setMinimumSize(70, 22);

	m_doubleSpin1->setMinimum(min1);
	m_doubleSpin1->setMaximum(max1);

	m_doubleSpin1->setValue(*pairedValue1);

	m_doubleSpin2 = new DoubleSpinBoxEx();

	m_doubleSpin2->setMinimumSize(70, 22);

	m_doubleSpin2->setMinimum(min2);
	m_doubleSpin2->setMaximum(max2);

	m_doubleSpin2->setValue(*pairedValue2);

	layout->addWidget(m_doubleSpin1);

	if (flags & eParameterScrubButton || flags & eParameterScrubButtonFine)
	{
		m_delta = 0.1f;
		if (flags & eParameterScrubButtonFine)
			m_delta = 0.01f;

		ScrubButton* pScrubButton1 = new ScrubButton();
		ScrubButton* pScrubButton2 = new ScrubButton();

		layout->addSpacing(5);
		layout->addWidget(pScrubButton1);
		layout->addSpacerItem(new QSpacerItem(10, 0));
		layout->addWidget(m_doubleSpin2);
		layout->addSpacing(5);
		layout->addWidget(pScrubButton2);

		m_pConnectionProxy->registerDeltaChange(pScrubButton1, 0);
		m_pConnectionProxy->registerDeltaChange(pScrubButton2, 1);
	}
	else
	{
		layout->addSpacing(5);
		layout->addWidget(m_doubleSpin2);
	}

	m_widget = mainWidget;

	m_pairedValue1 = pairedValue1;
	m_pairedValue2 = pairedValue2;

	m_lastValue1 = *pairedValue1;
	m_lastValue2 = *pairedValue2;

	m_pConnectionProxy->registerValueChangedDouble(m_doubleSpin1);
	m_pConnectionProxy->registerValueChangedDouble(m_doubleSpin2);
}

FloatPairControl::~FloatPairControl()
{

}

bool FloatPairControl::valueChanged()
{
	bool changed = false;

	*m_pairedValue1 = m_doubleSpin1->value();
	if (*m_pairedValue1 != m_lastValue1)
	{
		m_lastValue1 = *m_pairedValue1;
		changed = true;
	}

	*m_pairedValue2 = m_doubleSpin2->value();
	if (*m_pairedValue2 != m_lastValue2)
	{
		m_lastValue2 = *m_pairedValue2;
		changed = true;
	}

	return changed;
}

void FloatPairControl::refreshFromValue()
{
	m_doubleSpin1->setValue(*m_pairedValue1);
	m_doubleSpin2->setValue(*m_pairedValue2);

	m_lastValue1 = *m_pairedValue1;
	m_lastValue2 = *m_pairedValue2;
}

bool FloatPairControl::deltaChange(float delta, unsigned int index)
{
	bool changed = false;
	float* pPairedValue = index == 0 ? m_pairedValue1 : m_pairedValue2;

	float min = (index == 0) ? m_minimum1 : m_minimum2;
	float max = (index == 0) ? m_maximum1 : m_maximum2;

	float currentValue = *pPairedValue;
	delta *= m_delta;
	if (delta > 0.0f)
	{
		if (currentValue <= max - delta)
		{
			*pPairedValue += delta;
			changed = true;
		}
		else if (currentValue < max)
		{
			*pPairedValue = max;
			changed = true;
		}
	}
	else if (delta < 0.0f)
	{
		float aDelta = fabsf(delta);
		if (currentValue >= min + aDelta)
		{
			*pPairedValue -= aDelta;
			changed = true;
		}
		else if (currentValue > min)
		{
			*pPairedValue = min;
			changed = true;
		}
	}

	if (changed)
	{
		refreshFromValue();
	}

	return changed;
}

void FloatPairControl::setTooltips(const std::string& tooltip1, const std::string& tooltip2)
{
	m_doubleSpin1->setToolTip(tooltip1.c_str());
	m_doubleSpin2->setToolTip(tooltip2.c_str());
}

} // namespace Imagine
