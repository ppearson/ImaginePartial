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

#ifndef ANIMATED_VECTOR_CONTROL_H
#define ANIMATED_VECTOR_CONTROL_H

#include "control.h"

#include <QPushButton>
#include <QSignalMapper>

#include "core/animated_vector.h"

#include "widgets/double_spin_box_ex.h"

namespace Imagine
{

class AnimatedVectorControl : public Control
{
public:
	AnimatedVectorControl(const std::string& name, AnimatedVector* pairedValue, float min, float max, std::string label);
	virtual ~AnimatedVectorControl();

	virtual bool valueChanged();

	virtual bool buttonClicked(unsigned int index);
	virtual void menuSelected(int index);

	virtual void refreshFromValue();

	void setInterpolationTypeChecked(unsigned int index);

protected:
	DoubleSpinBoxEx*	m_Spins[3];
	QPushButton*		m_pAnimationButton;

	AnimatedVector*		m_pairedValue;

	// actions

	QAction*		m_actSetAnimated;
	QAction*		m_actDeleteAnimation;
	QAction*		m_actSetKey;
	QAction*		m_actDeleteKey;
	QAction*		m_actLinearInterpolation;
	QAction*		m_actQuadraticInterpolation;
	QAction*		m_actCubicInterpolation;

	QSignalMapper*	m_signalMapper;
};

} // namespace Imagine

#endif // ANIMATED_VECTOR_CONTROL_H
