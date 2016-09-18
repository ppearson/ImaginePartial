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

#include "tree_view_ex.h"

#include <QMouseEvent>

namespace Imagine
{

TreeViewEx::TreeViewEx(QWidget *parent) : QTreeView(parent)
{
}

TreeViewEx::~TreeViewEx()
{
}

// this allows deselection of the current selection by clicking on a blank area
void TreeViewEx::mousePressEvent(QMouseEvent* event)
{
	QModelIndex item = indexAt(event->pos());

	if (!item.isValid())
	{
		selectionModel()->clear();
	}

	QTreeView::mousePressEvent(event);
}

} // namespace Imagine
