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

#ifndef SPIN_BOX_EX_H
#define SPIN_BOX_EX_H

#include <QSpinBox>

class SpinBoxEx : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBoxEx(QWidget *parent = 0);

	virtual void stepBy(int steps);
	virtual void wheelEvent(QWheelEvent *event);

signals:

public slots:

};

#endif // SPIN_BOX_EX_H
