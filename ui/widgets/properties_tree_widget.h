/*
 Imagine
 Copyright 2017 Peter Pearson.

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

#ifndef PROPERTIES_TREE_WIDGET_H
#define PROPERTIES_TREE_WIDGET_H

#include <QWidget>

class QItemSelection;

namespace Imagine
{

class TreeViewEx;
class PropertiesTreeDataModel;
class PropertiesTreeItemDelegate;
class PropertiesTreeHost;

class PropertiesTreeWidget : public QWidget
{
	Q_OBJECT
public:
	PropertiesTreeWidget(QWidget* parent, PropertiesTreeHost* pHost);
	virtual ~PropertiesTreeWidget();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	
	void buildItems();
	
	void refreshItems();
	
signals:

public slots:
	void selectionChanged(const QItemSelection& before, const QItemSelection& after);

protected:
	TreeViewEx*					m_pTreeView;
	PropertiesTreeDataModel*	m_pModel;
	PropertiesTreeItemDelegate* m_pItemDelgate;
	
	PropertiesTreeHost*			m_pPropHost;
};

} // namespace Imagine

#endif // PROPERTIES_TREE_WIDGET_H
