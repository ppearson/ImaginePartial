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

#include "properties_tree_widget.h"

#include <QHBoxLayout>

#include <map>
#include <set>

#include "ui/widgets/tree_view_ex.h"
#include "ui/widgets/properties_tree_widget_misc.h"

namespace Imagine
{

PropertiesTreeWidget::PropertiesTreeWidget(QWidget* parent, PropertiesTreeHost* pHost) :
    QWidget(parent),
    m_pTreeView(NULL), m_pModel(NULL), m_pItemDelgate(NULL), m_pPropHost(pHost)
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);

	setLayout(layout);

	m_pTreeView = new TreeViewEx(this);
	m_pTreeView->setHeaderHidden(true);
//	m_pTreeView->setFrameStyle(QFrame::NoFrame);
	m_pTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_pTreeView->setAttribute(Qt::WA_MacShowFocusRect, 0); // hide the OS X blue focus rect
	
	m_pModel = new PropertiesTreeDataModel(this);
	
	m_pTreeView->setModel(m_pModel);
	
	m_pItemDelgate = new PropertiesTreeItemDelegate(pHost, this);
	
	m_pTreeView->setItemDelegate(m_pItemDelgate);

	layout->addWidget(m_pTreeView);
}

PropertiesTreeWidget::~PropertiesTreeWidget()
{
	
}

QSize PropertiesTreeWidget::minimumSizeHint() const
{
	return QSize(10, 10);
}

QSize PropertiesTreeWidget::sizeHint() const
{
	return QSize(10, 10);
}

void PropertiesTreeWidget::buildItems()
{
	if (!m_pPropHost)
		return;
	
	PropertiesTreeItem* pRoot = m_pModel->getItem(QModelIndex());
	if (!pRoot)
		return;
	
	PropertiesTreePropertiesDataStruct propertiesData;
	m_pPropHost->buildProperties(propertiesData);
	
	// create first-level items for the categories, building up a map of them
	std::map<std::string, PropertiesTreeItem*>	aFirstLevelItems;
	const std::vector<PropertiesTreePropertiesDataStruct::PropertyItem>& items = propertiesData.getItems();
	
	std::vector<PropertiesTreePropertiesDataStruct::PropertyItem>::const_iterator itItem = items.begin();
	for (; itItem != items.end(); ++itItem)
	{
		const PropertiesTreePropertiesDataStruct::PropertyItem& item = *itItem;
		
		// see if we've got the category in the map already...
		std::map<std::string, PropertiesTreeItem*>::iterator itFind = aFirstLevelItems.find(item.m_category);
		
		PropertiesTreeItem* pCategoryItem = NULL;
		if (itFind != aFirstLevelItems.end())
		{
			pCategoryItem = (*itFind).second;
		}
		else
		{
			// create it
			PropertiesTreeItem* pNewCategoryItem = new PropertiesTreeItem(QString(item.m_category.c_str()), pRoot, false);
			aFirstLevelItems[item.m_category] = pNewCategoryItem;
			
			pCategoryItem = pNewCategoryItem;
			
			pRoot->addChild(pCategoryItem);
		}
		
		// now add the actual item
		
		PropertiesTreeItem* pNewItem = new PropertiesTreeItem(QString(item.m_name.c_str()), pCategoryItem, true);
		
		pNewItem->setFullData(item);
		pCategoryItem->addChild(pNewItem);
	}
	
	refreshItems();
}

void PropertiesTreeWidget::refreshItems()
{
	// hack to set items to always be in edit mode, so we don't have to mess about with overriding paint() methods
	// in delegates to draw custom controls at all times (and not just when editing) (although checkboxes are catered for already to do this
	// with Qt::CheckStateRole, it's not possible to get things like spin controls and combo boxes doing this), which seems rediculously
	// over-complicated for what we want...
	
	// iterate over first level items (so categories) of column 0 to get the children - it's a bit round-about this way of
	// doing things, but...
	for (int i = 0; i < m_pModel->rowCount(); i++)
	{
		QModelIndex categoryIndex = m_pModel->index(i, 0);
		
		// now iterate over the rows of this item for column 1, which will be the effective children
		for (int j = 0; j < m_pModel->rowCount(categoryIndex); j++)
		{
			QModelIndex itemIndex = m_pModel->index(j, 1, categoryIndex);
			
			m_pTreeView->openPersistentEditor(itemIndex);
		}
	}
	
	// this has to be done after we've added items...
	m_pTreeView->setColumnWidth(0, 230);
	
	m_pTreeView->expandAll();
}

void PropertiesTreeWidget::selectionChanged(const QItemSelection& newIndex, const QItemSelection& oldIndex)
{
	QList<QModelIndex> indexes = newIndex.indexes();
	// just select the first for now

	if (indexes.isEmpty())
		return;

	QModelIndex& index = indexes[0];
	if (!index.isValid())
		return;

}

} // namespace Imagine
