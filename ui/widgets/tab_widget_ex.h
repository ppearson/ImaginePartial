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

#ifndef TAB_WIDGET_EX_H
#define TAB_WIDGET_EX_H

#include <QTabWidget>

namespace Imagine
{

class ParametersPanel;

class TabWidgetEx : public QTabWidget
{
    Q_OBJECT
public:
	TabWidgetEx(ParametersPanel* pParametersPanel, QWidget *parent = 0);
	virtual ~TabWidgetEx();

public slots:
	void tabIndexChanged(int index);

protected:
	// this is used for the callback of tab's currentChanged
	ParametersPanel*	m_pParametersPanel;
};

} // namespace Imagine

#endif // TAB_WIDGET_EX_H
