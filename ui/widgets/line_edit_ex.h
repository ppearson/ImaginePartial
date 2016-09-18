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

#ifndef LINE_EDIT_EX_H
#define LINE_EDIT_EX_H

#include <QLineEdit>

namespace Imagine
{

class LineEditEx : public QLineEdit
{
	Q_OBJECT
public:
	LineEditEx(QWidget *parent = 0);

	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dropEvent(QDropEvent* event);

};

} // namespace Imagine

#endif // LINE_EDIT_EX_H
