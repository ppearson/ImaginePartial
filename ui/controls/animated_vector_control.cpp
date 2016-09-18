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

#include "animated_vector_control.h"

#include <QHBoxLayout>
#include <QAction>
#include <QMenu>

#include "ui/widgets/push_button_ex.h"

namespace Imagine
{

AnimatedVectorControl::AnimatedVectorControl(const std::string& name, AnimatedVector* pairedValue, float min, float max, std::string label) : Control(name, label)
{
	QWidget* mainWidget = new QWidget();

	QHBoxLayout* layout = new QHBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);

	mainWidget->setMinimumHeight(26);

	Vector value = pairedValue->getVector();
	bool isKey = pairedValue->isKeyed();

	for (unsigned int i = 0; i < 3; i++)
	{
		DoubleSpinBoxEx* pSpin = new DoubleSpinBoxEx(mainWidget);
		m_Spins[i] = pSpin;

		pSpin->setMinimumSize(65, 22);
		pSpin->setMaximumHeight(22);

		pSpin->setMinimum(min);
		pSpin->setMaximum(max);

		pSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		pSpin->setBGColour(isKey ? QColor(192, 192, 255) : Qt::white);

		pSpin->setValue((value)[i]);

		layout->addWidget(pSpin);
	}

	m_pAnimationButton = new PushButtonEx(QIcon(":/imagine/images/animation.png"), "", mainWidget);
	m_pAnimationButton->setMaximumWidth(20);
	m_pAnimationButton->setMinimumWidth(20);
	m_pAnimationButton->setMaximumHeight(20);
	m_pAnimationButton->setMinimumHeight(20);
	m_pAnimationButton->setToolTip("Set key");

	layout->addSpacing(5);
	layout->addWidget(m_pAnimationButton);

	m_widget = mainWidget;

	m_pairedValue = pairedValue;

	m_pConnectionProxy->registerValueChangedDouble(m_Spins[0]);
	m_pConnectionProxy->registerValueChangedDouble(m_Spins[1]);
	m_pConnectionProxy->registerValueChangedDouble(m_Spins[2]);

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

AnimatedVectorControl::~AnimatedVectorControl()
{

}

bool AnimatedVectorControl::valueChanged()
{
	Vector value(m_Spins[0]->value(), m_Spins[1]->value(), m_Spins[2]->value());

	m_pairedValue->setFromVector(value);

	return true;
}

bool AnimatedVectorControl::buttonClicked(unsigned int index)
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

void AnimatedVectorControl::menuSelected(int index)
{
	switch (index)
	{
		case 0: // set animated
		{
			m_pairedValue->setAllAnimated(true);
			break;
		}
		case 1: // delete animation
		{
			m_pairedValue->setAllAnimated(false);
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

void AnimatedVectorControl::refreshFromValue()
{
	bool isKey = m_pairedValue->isKeyed();

	Vector value = m_pairedValue->getVector();

	for (unsigned int i = 0; i < 3; i++)
	{
		m_Spins[i]->setBGColour(isKey ? QColor(192, 192, 255) : Qt::white);

		m_Spins[i]->setValue((value)[i]);
	}
}

void AnimatedVectorControl::setInterpolationTypeChecked(unsigned int index)
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

} // namespace Imagine
