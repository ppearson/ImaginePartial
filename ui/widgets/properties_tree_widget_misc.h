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

#ifndef PROPERTIES_TREE_WIDGET_MISC_H
#define PROPERTIES_TREE_WIDGET_MISC_H

#include <QTreeView>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

#include <vector>
#include <set>

namespace Imagine
{

class PropertiesTreePropertiesDataStruct
{
public:

	class PropertyItem
	{
	public:
		enum PropertyType
		{
			eNone,
			eBool,
			eUInt,
			eFloat,
			eDouble,
			eEnum
		};

		PropertyItem();
		PropertyItem(const std::string& category, const std::string& name, PropertyType type, const void* pData, const char** pOptions = NULL);


//	protected:
		const void*		m_pData; // we don't own this, it points to a member variable in the Host
		PropertyType	m_type;
		std::string		m_name;

		// this is somewhat wasteful and duplicate, but makes things a little less complicated
		std::string		m_category;

		// for enum type
		const char**	m_pOptions; // we don't own this, and it probably points to static const data...
	};

	void addBool(const std::string& category, const std::string& name, const void* pData);
	void addUInt(const std::string& category, const std::string& name, const void* pData);
	void addFloat(const std::string& category, const std::string& name, const void* pData);
	void addEnum(const std::string& category, const std::string& name, const void* pData, const char** pOptions);


	const std::vector<PropertyItem>& getItems() const
	{
		return m_aItems;
	}

protected:
	void addProperty(const std::string& category, const std::string& name, PropertyItem::PropertyType type, const void* pData, const char** pOptions);

protected:

	std::vector<PropertyItem>	m_aItems;
};

class PropertiesTreeHost
{
public:
	PropertiesTreeHost()
	{

	}

	virtual void buildProperties(PropertiesTreePropertiesDataStruct& properties) = 0;

	virtual void propertyChanged()
	{

	}
};

class PropertiesTreeItem
{
public:
	PropertiesTreeItem(const QVariant& data, PropertiesTreeItem* parent = NULL, bool subItem = false);
	~PropertiesTreeItem();

	PropertiesTreeItem* child(int number);
	int childCount() const;

	QVariant data() const;

	PropertiesTreeItem* parent();

	int childNumber() const;

	bool isSubItem() const { return m_subItem; }

	void setData(const QVariant& value);

	void addChild(PropertiesTreeItem* pChild);

	void setFullData(const PropertiesTreePropertiesDataStruct::PropertyItem& data)
	{
		m_fullData = data;
	}

	const PropertiesTreePropertiesDataStruct::PropertyItem& getFullData() const
	{
		return m_fullData;
	}


protected:
	QList<PropertiesTreeItem*>	m_childItems;
	PropertiesTreeItem*			m_parentItem;
	QVariant					m_itemData;

	bool						m_subItem;

	PropertiesTreePropertiesDataStruct::PropertyItem				m_fullData;
};

class PropertiesTreeDataModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	PropertiesTreeDataModel(QObject *parent = 0);
	virtual ~PropertiesTreeDataModel();

	virtual QVariant data(const QModelIndex& index, int role) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation,
								int role = Qt::DisplayRole) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

//	bool hasChildren(const QModelIndex& parent) const;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& index) const;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	PropertiesTreeItem* getItem(const QModelIndex& index) const;

	void clear();

signals:

public slots:

protected:

	PropertiesTreeItem*	m_pRootItem;

};

class PropertiesTreeItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	PropertiesTreeItemDelegate(PropertiesTreeHost* pWidgetHost, QObject* parent = NULL);
	virtual ~PropertiesTreeItemDelegate();

//	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	virtual QString displayText(const QVariant& value, const QLocale& locale) const;


protected slots:
	void internalDataChanged();


protected:
	PropertiesTreeHost*		m_pWidgetHost; // host of the widget that owns us - this is just a copy, we don't own it
};

// custom QWidget subclasses in order to handle editing the values directly. This functionality is a bit
// crap, but using the data model and marshalling stuff through QVariant is pretty cumbersome, so doing it
// this way at least is easier and more direct...

class PropertiesCheckBox : public QCheckBox
{
	Q_OBJECT
public:
	PropertiesCheckBox(bool* pValue, QWidget* parent);

public slots:
	void valueChanged();

protected:
	bool*			m_pValue;
};

class PropertiesComboBox : public QComboBox
{
	Q_OBJECT
public:
	PropertiesComboBox(unsigned int* pValue, const char** pOptions, QWidget* parent);

public slots:
	void valueChanged();

protected:
	unsigned int*	m_pValue;
};

class PropertiesFloatSpinBox : public QDoubleSpinBox
{
	Q_OBJECT
public:
	PropertiesFloatSpinBox(float* pValue, QWidget* parent);

public slots:
	void valueChanged();

protected:
	float*			m_pValue;
};

class PropertiesIntSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	PropertiesIntSpinBox(unsigned int* pValue, QWidget* parent);

public slots:
	void valueChanged();

protected:
	unsigned int*	m_pValue;
};

} // namespace Imagine

#endif // PROPERTIES_TREE_WIDGET_MISC_H
