/*
 Imagine
 Copyright 2016 Peter Pearson.

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

#ifndef SHADER_NODE_VIEW_WIDGET_H
#define SHADER_NODE_VIEW_WIDGET_H

#include <QGraphicsView>
#include <QWidget>
#include <QPointF>

#include <vector>
#include <map>
#include <string>

#include "core/hash.h"

class QSignalMapper;
class QAction;

namespace Imagine
{

class ShaderOp;

class ShaderNodeUI;
class ShaderConnectionUI;

class ShaderNodesCollection;

class ParametersInterface;

class ShaderNodeViewWidgetHost
{
public:
	// can be hierarchical, e.g. "Texture/Ramp"
	virtual void getListOfNodeNames(std::vector<std::string>& aNodeNames) const = 0;
	virtual void getAllowedCreationCategories(std::vector<std::string>& aCategories) const = 0;

	virtual ShaderOp* createNewShaderOp(const std::string& categoryName, const std::string& nodeName) = 0;

	virtual void showParametersPanelForOp(ParametersInterface*& paramInterface) = 0;
};

class ShaderNodeViewWidget : public QGraphicsView
{
Q_OBJECT
public:
	ShaderNodeViewWidget(ShaderNodeViewWidgetHost* pHost, QWidget* pParent);
	virtual ~ShaderNodeViewWidget();

	struct NodeCreationMenuItem
	{
		NodeCreationMenuItem(const std::string& cat, const std::string& nm, QAction* pAct) :
			name(nm), category(cat), pAction(pAct)
		{
		}

		std::string name;
		std::string category;
		QAction*	pAction;
	};

	struct NodeCreationMenuCategory
	{
		NodeCreationMenuCategory()
		{
		}

		std::vector<NodeCreationMenuItem> nodeItems;

		unsigned int	index; //
	};

	void setNodesCollection(ShaderNodesCollection* pNodesCollection) { m_pNodesCollection = pNodesCollection; }

	void updateCreationMenu();

	void createUIItemsFromNodesCollection();

	void wheelEvent(QWheelEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

	//


protected:

	enum MouseMode
	{
		eMouseNone,
		eMousePan,
		eMouseDrag,
		eMouseConnect,
		eMouseSelect
	};

	enum SelectionType
	{
		eSelectionNone,
		eSelectionNode,
		eSelectionEdgeOutput,
		eSelectionEdgeInput
	};

	enum SelectedConnectionDragType
	{
		eSelConDragNone,
		eSelConDragExistingInput,
		eSelConDragExistingOutput,
		eSelConDragNewInput,
		eSelConDragNewOutput
	};

	void showRightClickMenu(QMouseEvent* event);

	QPointF getEventPos(QMouseEvent* event) const;

	void createNewNodeMenu(unsigned int categoryIndex, int menuIndex);

public slots:
	// hacky, but it's difficult to use a proxy mapper
	void menuNewNode0(int index);
	void menuNewNode1(int index);
	void menuNewNode2(int index);

protected:
	MouseMode			m_mouseMode;

	QPointF				m_lastMousePos;
	QPointF				m_lastMouseScenePos;

	float				m_scale;

	SelectionType		m_selectionType;
	QGraphicsItem*		m_pSelectedItem;

	ShaderNodesCollection*		m_pNodesCollection;
	ShaderNodeViewWidgetHost*	m_pHost;

	// UI-only representation
	std::vector<ShaderNodeUI*>			m_aNodes;
	std::vector<ShaderConnectionUI*>	m_aConnections;

	// temporary connection item, which is used for interaction
	// TODO: this is very fragile - need to think about a better way of doing this...
	ShaderConnectionUI*			m_pInteractionConnectionItem;
	// describes what the current / last state of the above is...
	SelectedConnectionDragType	m_selConnectionDragType;

	// for creation menu stuff
	// this is really hacky, but there's no easy way of doing this currently
	// only support up to three category menus currently
	std::map<std::string, NodeCreationMenuCategory*>	m_aMenuCategories;
	std::vector<NodeCreationMenuCategory*>	m_aMenuCategoriesIndexed; // copy of pointers to the above for indexing into by the slot

	// TODO: once we can support C++11 and Qt5, we can do this in a nicer way with lambdas
	QSignalMapper*	m_creationMenuSignalMapper[3];
};

} // namespace Imagine

#endif // SHADER_NODE_VIEW_WIDGET_H
