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

#include "properties_tree_widget_misc.h"

namespace Imagine
{

PropertiesTreePropertiesDataStruct::PropertyItem::PropertyItem() : m_pData(NULL), m_type(eNone), m_pOptions(NULL)
{

}

PropertiesTreePropertiesDataStruct::PropertyItem::PropertyItem(const std::string& category, const std::string& name, PropertyType type,
															   const void* pData, const char** pOptions) :
	m_pData(pData), m_type(type), m_name(name), m_category(category), m_pOptions(pOptions)
{

}

//

void PropertiesTreePropertiesDataStruct::addBool(const std::string& category, const std::string& name, const void* pData)
{
	addProperty(category, name, PropertyItem::eBool, pData, NULL);
}

void PropertiesTreePropertiesDataStruct::addUInt(const std::string& category, const std::string& name, const void* pData)
{
	addProperty(category, name, PropertyItem::eUInt, pData, NULL);
}

void PropertiesTreePropertiesDataStruct::addFloat(const std::string& category, const std::string& name, const void* pData)
{
	addProperty(category, name, PropertyItem::eFloat, pData, NULL);
}

void PropertiesTreePropertiesDataStruct::addEnum(const std::string& category, const std::string& name, const void* pData, const char** pOptions)
{
	addProperty(category, name, PropertyItem::eEnum, pData, pOptions);
}

void PropertiesTreePropertiesDataStruct::addProperty(const std::string& category, const std::string& name, PropertyItem::PropertyType type, const void* pData, const char** pOptions)
{
	m_aItems.push_back(PropertyItem(category, name, type, pData, pOptions));
}

//

PropertiesTreeItem::PropertiesTreeItem(const QVariant& data, PropertiesTreeItem* parent, bool subItem) :
	m_parentItem(parent), m_itemData(data), m_subItem(subItem)
{

}

PropertiesTreeItem::~PropertiesTreeItem()
{
	qDeleteAll(m_childItems);
}

PropertiesTreeItem* PropertiesTreeItem::child(int number)
{
	if (number >= m_childItems.count())
		return NULL;

	return m_childItems.value(number);
}

int PropertiesTreeItem::childCount() const
{
	return m_childItems.count();
}

QVariant PropertiesTreeItem::data() const
{
	return m_itemData;
}

PropertiesTreeItem* PropertiesTreeItem::parent()
{
	return m_parentItem;
}

int PropertiesTreeItem::childNumber() const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<PropertiesTreeItem*>(this));

	return 0;
}

void PropertiesTreeItem::setData(const QVariant& value)
{
	m_itemData = value;
}

void PropertiesTreeItem::addChild(PropertiesTreeItem* pChild)
{
	m_childItems.push_back(pChild);
}




////

PropertiesTreeDataModel::PropertiesTreeDataModel(QObject *parent) : QAbstractItemModel(parent),
	m_pRootItem(NULL)
{
	clear();
}

PropertiesTreeDataModel::~PropertiesTreeDataModel()
{
	delete m_pRootItem;
}

QVariant PropertiesTreeDataModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		PropertiesTreeItem* item = getItem(index);
		if (item)
		{
			if (!item->isSubItem())
			{
				if (index.column() == 0)
					return item->data();
				else
					return QVariant();
			}
			else
			{
				const PropertiesTreePropertiesDataStruct::PropertyItem& dataItem = item->getFullData();
				if (index.column() == 1)
				{
					if (dataItem.m_type == PropertiesTreePropertiesDataStruct::PropertyItem::eBool)
					{
						return QVariant();
					}
					else if (dataItem.m_type == PropertiesTreePropertiesDataStruct::PropertyItem::eFloat)
					{
						return QVariant();
					}
					else if (dataItem.m_type == PropertiesTreePropertiesDataStruct::PropertyItem::eEnum)
					{
						return QVariant();
					}
				}
				return item->data();
			}
		}
	}
	else if (role == Qt::DecorationRole)
	{
		PropertiesTreeItem* item = getItem(index);
		if (item)
		{

		}
	}

	return QVariant();
}

QVariant PropertiesTreeDataModel::headerData(int section, Qt::Orientation orientation,
							int role) const
{
	return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags PropertiesTreeDataModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	if (index.column() == 0)
		return QAbstractItemModel::flags(index);

	return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

QModelIndex PropertiesTreeDataModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	PropertiesTreeItem* parentItem = getItem(parent);

	PropertiesTreeItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex PropertiesTreeDataModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	PropertiesTreeItem* childItem = getItem(index);
	PropertiesTreeItem* parentItem = childItem->parent();

	if (!parentItem || parentItem == m_pRootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

int PropertiesTreeDataModel::columnCount(const QModelIndex& parent) const
{
	// this has to always be the max and can't vary...
	return 2;
}

int PropertiesTreeDataModel::rowCount(const QModelIndex& parent) const
{
	PropertiesTreeItem* parentItem = getItem(parent);

	return parentItem->childCount();
}

PropertiesTreeItem* PropertiesTreeDataModel::getItem(const QModelIndex& index) const
{
	if (index.isValid())
	{
		PropertiesTreeItem* item = static_cast<PropertiesTreeItem*>(index.internalPointer());
		if (item)
			return item;
	}

	return m_pRootItem;
}

void PropertiesTreeDataModel::clear()
{
	beginResetModel();

	if (m_pRootItem)
		delete m_pRootItem;

	m_pRootItem = new PropertiesTreeItem(QString(""), NULL, false);

	endResetModel();
}

////

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(PropertiesTreeHost* pWidgetHost, QObject* parent) : QStyledItemDelegate(parent), m_pWidgetHost(pWidgetHost)
{

}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{

}

/*
QSize PropertiesTreeItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{

}
*/

QWidget* PropertiesTreeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	PropertiesTreeItem* item = static_cast<PropertiesTreeItem*>(index.internalPointer());

	if (!item)
	{
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	const PropertiesTreePropertiesDataStruct::PropertyItem& data = item->getFullData();

	if (data.m_type == PropertiesTreePropertiesDataStruct::PropertyItem::eBool)
	{
		PropertiesCheckBox* pNewWidget = new PropertiesCheckBox((bool*)data.m_pData, parent);

		return pNewWidget;
	}
	else if (data.m_type == PropertiesTreePropertiesDataStruct::PropertyItem::eUInt)
	{
		PropertiesIntSpinBox* pNewWidget = new PropertiesIntSpinBox((unsigned int*)data.m_pData, parent);

		return pNewWidget;
	}
	else if (data.m_type == PropertiesTreePropertiesDataStruct::PropertyItem::eFloat)
	{
		PropertiesFloatSpinBox* pNewWidget = new PropertiesFloatSpinBox((float*)data.m_pData, parent);

		return pNewWidget;
	}
	else if (data.m_type == PropertiesTreePropertiesDataStruct::PropertyItem::eEnum)
	{
		PropertiesComboBox* pNewWidget = new PropertiesComboBox((unsigned int*)data.m_pData, data.m_pOptions, parent);
		pNewWidget->setMaximumHeight(26);

		return pNewWidget;
	}

	QWidget* pEditorWidget = QStyledItemDelegate::createEditor(parent, option, index);

	return pEditorWidget;
}

void PropertiesTreeItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	// we purposefully don't implement this, so that the subclassed PropertiesWidget subclasses (below) implement
	// things themselves with regard to displaying values and setting them... This is pretty hacky, but...
}

void PropertiesTreeItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	QStyledItemDelegate::setModelData(editor, model, index);
}

void PropertiesTreeItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

QString PropertiesTreeItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
	return QStyledItemDelegate::displayText(value, locale);
}

void PropertiesTreeItemDelegate::internalDataChanged()
{
	if (m_pWidgetHost)
	{
		m_pWidgetHost->propertyChanged();
	}
}

//

PropertiesCheckBox::PropertiesCheckBox(bool* pValue, QWidget* parent) : QCheckBox(parent), m_pValue(pValue)
{
	setChecked(*m_pValue);

	QObject::connect(this, SIGNAL(toggled(bool)), this, SLOT(valueChanged()));
}

void PropertiesCheckBox::valueChanged()
{
	bool newValue = isChecked();

	*m_pValue = newValue;
}

//

PropertiesComboBox::PropertiesComboBox(unsigned int* pValue, const char** pOptions, QWidget* parent) : QComboBox(parent), m_pValue(pValue)
{
	unsigned int i = 0;
	while (pOptions[i])
	{
		addItem(pOptions[i++]);
	}

	unsigned int currentIndex = *pValue;
	setCurrentIndex(currentIndex);

	QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged()));
}

void PropertiesComboBox::valueChanged()
{
	unsigned int newValue = (unsigned int)currentIndex();

	*m_pValue = newValue;
}

//

PropertiesFloatSpinBox::PropertiesFloatSpinBox(float* pValue, QWidget* parent) : QDoubleSpinBox(parent), m_pValue(pValue)
{
	setMinimum(0.0f);
	setMaximum(100.0f);

	float actualValue = *m_pValue;
	setValue(actualValue);

	QObject::connect(this, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
}

void PropertiesFloatSpinBox::valueChanged()
{
	float newValue = (float)value();

	*m_pValue = newValue;
}

//

PropertiesIntSpinBox::PropertiesIntSpinBox(unsigned int* pValue, QWidget* parent) : QSpinBox(parent), m_pValue(pValue)
{
	setMinimum(0);
	setMaximum(64000);

	unsigned int actualValue = *m_pValue;
	setValue(actualValue);

	QObject::connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
}

void PropertiesIntSpinBox::valueChanged()
{
	unsigned int newValue = (unsigned int)value();

	*m_pValue = newValue;
}

} // namespace Imagine
