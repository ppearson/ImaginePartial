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

#ifndef SCRUB_BUTTON_H
#define SCRUB_BUTTON_H

#include <QPushButton>

namespace Imagine
{

class ScrubButton : public QPushButton
{
	Q_OBJECT
public:
	explicit ScrubButton(QWidget* parent = 0);

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);

	void setIndex(int index) { m_index = index; }

signals:
	void deltaMove(float delta);
	void deltaMoveIndex(float delta, int index);

public slots:


protected:
	bool			m_mouseDown;
	int				m_lastMouseX;

	int				m_lastMouseScreenX;
	int				m_lastMouseScreenY;

	int				m_index;
};

} // namespace Imagine

#endif // SCRUB_BUTTON_H
