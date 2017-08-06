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

#include "tab_widget_ex.h"

#include <QStyle>

#include "parameters_panel.h"

namespace Imagine
{

static const std::string sStyle = "QTabWidget::pane { /* The tab widget frame */"
"     /*border-top: 2px solid #C2C7CB;*/"
" }"
" QTabBar::tab {"
"     background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                                 stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
"                                 stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
"     border: 1px solid #C4C4C3;"
"     border-top-left-radius: 4px;"
"     border-top-right-radius: 4px;"
"     min-width: 8ex;"
"     padding: 2px;"
" }"
""
" QTabBar::tab:selected, QTabBar::tab:hover {"
"     background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                                 stop: 0 #fafafa, stop: 0.4 #f4f4f4,"
"                                 stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);"
" }"
""
" QTabBar::tab:selected {"
"     border-color: #9B9B9B;"
"     border-bottom-color: #C2C7CB; /* same as pane color */"
" }"
""
" QTabBar::tab:!selected {"
"     margin-top: 2px; /* make non-selected tabs look smaller */"
" }"
""
" /* make use of negative margins for overlapping tabs */"
" QTabBar::tab:selected {"
"     /* expand/overlap to the left and right by 4px */"
"     margin-left: -4px;"
"     margin-right: -4px;"
" }"
""
" QTabBar::tab:first:selected {"
"     margin-left: 0; /* the first selected tab has nothing to overlap with on the left */"
" }"
""
" QTabBar::tab:last:selected {"
"     margin-right: 0; /* the last selected tab has nothing to overlap with on the right */"
" }"
""
" QTabBar::tab:only-one {"
"     margin: 0; /* if there is only one tab, we don't want overlapping margins */"
" }";

TabWidgetEx::TabWidgetEx(ParametersPanel* pParametersPanel, QWidget *parent) : QTabWidget(parent), m_pParametersPanel(pParametersPanel)
{
//	setStyleSheet(sStyle.c_str());
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChanged(int)));
}

TabWidgetEx::~TabWidgetEx()
{
	QStyle* pStyle = style();
	if (pStyle)
	{
		delete pStyle;
	}
}

void TabWidgetEx::tabIndexChanged(int index)
{
	if (m_pParametersPanel)
		m_pParametersPanel->tabIndexChanged(index);
}

} // namespace Imagine
