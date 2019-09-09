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

#include "animated_float_control.h"

#include <QHBoxLayout>
#include <QAction>
#include <QMenu>

#include <math.h>

#include "parameter.h"

#include "ui/widgets/push_button_ex.h"
#include "ui/widgets/scrub_button.h"

namespace Imagine
{

AnimatedFloatControl::AnimatedFloatControl(const std::string& name, AnimationCurve* pairedValue, float min, float max, const std::string& label,
										   unsigned int flags) : Control(name, label), m_minimum(min), m_maximum(max)
{
	QWidget* mainWidget = new QWidget();

	mainWidget->setMinimumHeight(26);

	QHBoxLayout* layout = new QHBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);

	float value = pairedValue->getValue();
	bool isKey = pairedValue->isKey();

	m_pSpin = new DoubleSpinBoxEx(mainWidget);

	m_pSpin->setMinimumSize(65, 22);
	m_pSpin->setMaximumHeight(22);

	m_pSpin->setMinimum(min);
	m_pSpin->setMaximum(max);

	m_pSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_pSpin->setBGColour(isKey ? QColor(192, 192, 255) : m_pSpin->palette().color(QPalette::Base));

	m_pSpin->setValue(value);

	layout->addWidget(m_pSpin);
	layout->addSpacing(5);
	
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
		
		ScrubButton* pScrubButton = new ScrubButton();

		layout->addWidget(pScrubButton);

		layout->addSpacing(15);

		m_pConnectionProxy->registerDeltaChange(pScrubButton, 0);
	}

	m_pAnimationButton = new PushButtonEx(QIcon(":/imagine/images/animation.png"), "", mainWidget);
	m_pAnimationButton->setMaximumWidth(20);
	m_pAnimationButton->setMinimumWidth(20);
	m_pAnimationButton->setMaximumHeight(20);
	m_pAnimationButton->setMinimumHeight(20);
	m_pAnimationButton->setToolTip("Set key");

	layout->addWidget(m_pAnimationButton);

	m_widget = mainWidget;

	m_pairedValue = pairedValue;

	m_pConnectionProxy->registerValueChangedDouble(m_pSpin);

	m_pConnectionProxy->registerButtonClicked(m_pAnimationButton);

	// set up menu actions
	m_actSetAnimated = new QAction("Set Animated", m_pAnimationButton);
	m_actDeleteAnimation = new QAction("Delete Animation", m_pAnimationButton);
	m_actSetKey = new QAction("Set Key", m_pAnimationButton);
	m_actDeleteKey = new QAction("Delete Key", m_pAnimationButton);
	m_actLinearInterpolation = new QAction("Linear Interpolation", m_pAnimationButton);
	m_actQuadraticInterpolation = new QAction("Quadratic Interpolation", m_pAnimationButton);
	m_actCubicInterpolation = new QAction("Cubic Interpolation", m_pAnimationButton);

	m_actLinearInterpolation->setCheckable(true);
	m_actQuadraticInterpolation->setCheckable(true);
	m_actCubicInterpolation->setCheckable(true);

	setInterpolationTypeChecked(m_pairedValue->getInterpolationType());

	m_signalMapper = new QSignalMapper(m_pAnimationButton);

	m_pAnimationButton->connect(m_actSetAnimated, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
	m_signalMapper->setMapping(m_actSetAnimated, 0);

	m_pAnimationButton->connect(m_actDeleteAnimation, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
	m_signalMapper->setMapping(m_actDeleteAnimation, 1);

	m_pAnimationButton->connect(m_actSetKey, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
	m_signalMapper->setMapping(m_actSetKey, 2);

	m_pAnimationButton->connect(m_actDeleteKey, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
	m_signalMapper->setMapping(m_actDeleteKey, 3);

	m_pAnimationButton->connect(m_actLinearInterpolation, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
	m_signalMapper->setMapping(m_actLinearInterpolation, 4);

	m_pAnimationButton->connect(m_actQuadraticInterpolation, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
	m_signalMapper->setMapping(m_actQuadraticInterpolation, 5);

	m_pAnimationButton->connect(m_actCubicInterpolation, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
	m_signalMapper->setMapping(m_actCubicInterpolation, 6);

	m_pConnectionProxy->registerMenuSelected(m_signalMapper);
}

AnimatedFloatControl::~AnimatedFloatControl()
{

}

bool AnimatedFloatControl::valueChanged()
{
	float value = m_pSpin->value();

	m_pairedValue->setValue(value);

	return true;
}

bool AnimatedFloatControl::buttonClicked(unsigned int index)
{
	bool isAnimated = m_pairedValue->isAnimated();

	QMenu* m = new QMenu(m_pAnimationButton);
	if (!isAnimated)
	{
		m->addAction(m_actSetAnimated);
	}
	else
	{
		m->addAction(m_actDeleteAnimation);
		m->addAction(m_actSetKey);
		m->addAction(m_actDeleteKey);
		m->addSeparator();

		m->addAction(m_actLinearInterpolation);
		m->addAction(m_actQuadraticInterpolation);
		m->addAction(m_actCubicInterpolation);
	}

	QSize menuSize = m->sizeHint();

	QSize buttonSize = m_pAnimationButton->geometry().size();

	m->move(m_pAnimationButton->mapToGlobal(QPoint(buttonSize.width() - menuSize.width(), buttonSize.height())));
	m->show();

	return false;
}

void AnimatedFloatControl::menuSelected(int index)
{
	switch (index)
	{
		case 0: // set animated
		{
			m_pairedValue->setAnimated(true);
			break;
		}
		case 1: // delete animation
		{
			m_pairedValue->setAnimated(false);
			break;
		}
		case 2: // set key
		{
			m_pairedValue->setKey();
			break;
		}
		case 3: // delete key
		{
			m_pairedValue->deleteKey();
			break;
		}
		case 4: // linear interpolation
		{
			m_pairedValue->setInterpolationType(eLinearInterpolation);
			setInterpolationTypeChecked(0);
			break;
		}
		case 5: // quadratic interpolation
		{
			m_pairedValue->setInterpolationType(eQuadraticInterpolation);
			setInterpolationTypeChecked(1);
			break;
		}
		case 6: // cubic interpolation
		{
			m_pairedValue->setInterpolationType(eCubicInterpolation);
			setInterpolationTypeChecked(2);
			break;
		}
	}

	refreshFromValue();
}

void AnimatedFloatControl::refreshFromValue()
{
	bool isKey = m_pairedValue->isKey();

	float value = m_pairedValue->getValue();

	m_pSpin->setBGColour(isKey ? QColor(192, 192, 255) : m_pSpin->palette().color(QPalette::Base));

	m_pSpin->setValue(value);
}

void AnimatedFloatControl::setInterpolationTypeChecked(unsigned int index)
{
	m_actLinearInterpolation->setChecked(false);
	m_actQuadraticInterpolation->setChecked(false);
	m_actCubicInterpolation->setChecked(false);

	switch (index)
	{
		default:
		case 0:
			m_actLinearInterpolation->setChecked(true);
			break;
		case 1:
			m_actQuadraticInterpolation->setChecked(true);
			break;
		case 2:
			m_actCubicInterpolation->setChecked(true);
			break;
	}
}

bool AnimatedFloatControl::deltaChange(float delta, unsigned int index)
{
	bool changed = false;
	float currentValue = m_pairedValue->getValue();
	delta *= m_delta;
	if (delta > 0.0f)
	{
		if (currentValue <= m_maximum - delta)
		{
			float newValue = currentValue + delta;
			m_pairedValue->setValue(newValue);
			changed = true;
		}
		else if (currentValue < m_maximum)
		{
			m_pairedValue->setValue(m_maximum);
			changed = true;
		}
	}
	else if (delta < 0.0f)
	{
		float aDelta = fabsf(delta);
		if (currentValue >= m_minimum + aDelta)
		{
			float newValue = currentValue - aDelta;
			m_pairedValue->setValue(newValue);
			changed = true;
		}
		else if (currentValue > m_minimum)
		{
			m_pairedValue->setValue(m_minimum);
			changed = true;
		}
	}

	refreshFromValue();

	return changed;
}

} // namespace Imagine
