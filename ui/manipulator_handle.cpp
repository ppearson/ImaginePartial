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

#include "manipulator_handle.h"

#include <QtOpenGL/QGLWidget>

#include "object.h"

#include "core/vector.h"
#include "core/point.h"

#include "ui/opengl_ex.h"

void Position3DManipulatorHandle::draw()
{
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_ALWAYS);

	glColor3f(1.0f, 0.0f, 0.0f);

	glBegin(GL_QUADS);

	Point point = m_pairedValue;

	OpenGLEx::drawSquareOnFlatPlane(point, 0.10f);

	glEnd();

	glEnable(GL_LIGHTING);
	glDepthFunc(GL_LESS);
}


void Position3DManipulatorHandle::applyDelta(Vector& delta)
{
	m_pairedValue += delta;

	PostChangedActions actions;

	m_pObject->controlChanged(m_name, actions);
	m_pObject->refreshControl(m_name);
}

Point Position3DManipulatorHandle::getCentreOfPosition()
{
	return (Point)m_pairedValue;
}
