/*
 Imagine
 Copyright 2011-2014 Peter Pearson.

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

#include "float_slider_widget.h"

#include <QHBoxLayout>
#include <QSlider>

#include <cmath>

#include "double_spin_box_ex.h"

namespace Imagine
{

FloatSliderWidget::FloatSliderWidget(float min, float max, bool editControl, bool logScaleSlider, bool highPrecision,
									 QWidget* parent) : QWidget(parent), m_pDoubleSpin(nullptr), m_pSlider(nullptr),
									m_logScale(logScaleSlider)
{
	m_pSlider = new QSlider(Qt::Horizontal);
	m_pSlider->setMinimumWidth(130);

	m_pSlider->setTickPosition(QSlider::TicksBelow);

	QHBoxLayout* layout = new QHBoxLayout(this);
	setLayout(layout);
	layout->setMargin(0);

	if (editControl)
	{
		layout->setSpacing(1);

		int precision = 4;

		if (highPrecision)
			precision = 6;

		m_pDoubleSpin = new DoubleSpinBoxEx(precision);

		m_pDoubleSpin->setMinimumSize(70, 22);

		m_pDoubleSpin->setMinimum(min);
		m_pDoubleSpin->setMaximum(max);

		layout->addWidget(m_pDoubleSpin);

		layout->addSpacing(5);
		layout->addWidget(m_pSlider);

		QObject::connect(m_pDoubleSpin, SIGNAL(editingFinished()), this, SLOT(spinChanged()));
	}
	else
	{
		layout->addWidget(m_pSlider);
	}

	if (!m_logScale)
	{
		m_converter = 100.0;
	}
	else
	{
		m_converter = 1000.0;
	}

	m_min = min * m_converter;
	m_max = max * m_converter;

	m_pSlider->setTickInterval((m_max - m_min) / 10);

	m_pSlider->setRange(m_min, m_max);

	m_pSlider->setSingleStep(1);

	if (m_logScale)
	{
		m_offset = 0.0f;
		if (min == 0.0f)
		{
			m_offset = 1.0f;
			min += m_offset;
			max += m_offset;
		}
		m_fMin = min;
		m_fMax = max;
		m_logMax = logf(max);
		m_logMin = logf(min);
		m_scale = (m_logMax - m_logMin) / (m_fMax - m_fMin);
	}

	QObject::connect(m_pSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int)));
	QObject::connect(m_pSlider, SIGNAL(actionTriggered(int)), this, SLOT(sliderActionTriggered(int)));
}

FloatSliderWidget::~FloatSliderWidget()
{
	if (m_pDoubleSpin)
	{
		delete m_pDoubleSpin;
		m_pDoubleSpin = nullptr;
	}

	if (m_pSlider)
	{
		delete m_pSlider;
		m_pSlider = nullptr;
	}
}

void FloatSliderWidget::setValue(float value)
{
	int localValue = 0; // local value of slider

	if (!m_logScale)
	{
		localValue = value * m_converter;
	}
	else
	{
		float expValue = getLinearValue(value);
		localValue = expValue * m_converter;
	}

	// TODO: block signals?

	m_pSlider->blockSignals(true);

	m_pSlider->setSliderPosition(localValue);
	m_pSlider->setValue(localValue);

	m_pSlider->blockSignals(false);

	if (m_pDoubleSpin)
	{
		m_pDoubleSpin->setValue(value);
	}
}

float FloatSliderWidget::getValue() const
{
	float floatValue;

	if (!m_pDoubleSpin)
	{
		// get it from the slider
		int sliderPosition = m_pSlider->value();

		floatValue = (float)(sliderPosition) / m_converter;

		if (m_logScale)
		{
			floatValue = getExpValue(floatValue);
		}
	}
	else
	{
		floatValue = m_pDoubleSpin->value();
	}

	return floatValue;
}

void FloatSliderWidget::spinChanged()
{
	float fValue = m_pDoubleSpin->value();

	int sliderValue = 0;
	if (!m_logScale)
	{
		sliderValue = fValue * m_converter;
	}
	else
	{
		float temp = getLinearValue(fValue);
		sliderValue = temp * m_converter;
	}

	m_pSlider->blockSignals(true);

	m_pSlider->setSliderPosition(sliderValue);
	m_pSlider->setValue(sliderValue);

	m_pSlider->blockSignals(false);

	emit editingFinished();
	emit valueChanged();
}

void FloatSliderWidget::sliderChanged(int position)
{
	if (position < m_min)
		position = m_min;

	if (position > m_max)
		position = m_max;

	// need to set the slider pos here so calls to getValue() work
	m_pSlider->blockSignals(true);
	m_pSlider->setValue(position);
	m_pSlider->blockSignals(false);

	float floatValue = 0.0f;

	if (!m_logScale)
	{
		floatValue = (float)(position) / m_converter;
	}
	else
	{
		floatValue = (float)(position) / m_converter;

		floatValue = getExpValue(floatValue);
	}

	if (m_pDoubleSpin)
	{
		m_pDoubleSpin->blockSignals(true);
		m_pDoubleSpin->setValue(floatValue);
		m_pDoubleSpin->blockSignals(false);
	}

	emit sliderMoved(position);
	emit valueChanged();
}

void FloatSliderWidget::sliderActionTriggered(int actionTriggered)
{
	// annoyingly, on OS X, clicking the mouse outside the handle only triggers SliderMove,
	// which is also triggered by scrubbing, which makes things much more complicated to isolate
	// this event, so for the moment, don't bother catering for OS X...
	// This has the unfortunate side-effect of not allowing us to process mouse-wheel events, which
	// are classed as SliderMove, but don't trigger sliderMoved() which seems a bit inconsistent...

	if (actionTriggered != QAbstractSlider::SliderSingleStepAdd &&
		actionTriggered != QAbstractSlider::SliderSingleStepSub &&
		actionTriggered != QAbstractSlider::SliderPageStepAdd &&
		actionTriggered != QAbstractSlider::SliderPageStepSub
//	    && actionTriggered != QAbstractSlider::SliderMove
		)
	{
		return;
	}

	int position = m_pSlider->value();

	float floatValue = 0.0f;

	if (!m_logScale)
	{
		floatValue = (float)(position) / m_converter;
	}
	else
	{
		floatValue = (float)(position) / m_converter;

		floatValue = getExpValue(floatValue);
	}

	if (m_pDoubleSpin)
	{
		m_pDoubleSpin->blockSignals(true);
		m_pDoubleSpin->setValue(floatValue);
		m_pDoubleSpin->blockSignals(false);
	}

	emit valueChanged();
}

float FloatSliderWidget::getLinearValue(float x) const
{
	float value = 0;

	x += m_offset;
	value = (((logf(x) - m_logMin) / m_scale) + m_fMin);

	value -= m_offset;

	return value;
}

float FloatSliderWidget::getExpValue(float x) const
{
	float value = 0.0f;

	x += m_offset;
	value = expf(m_logMin + (m_scale * (x - m_fMin)));

	value -= m_offset;

	return value;
}

} // namespace Imagine
