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

#include "shader_node_view_widget.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPointF>
#include <QGLWidget>

#include <QWheelEvent>
#include <QMenu>
#include <QSignalMapper>

#include <cmath>
#include <set>

#include "shading/shader_node.h"
#include "shading/shader_nodes_collection.h"

#include "shader_node_view_items.h"

#include "shading/shader_op.h"

#include "utils/string_helpers.h"

#include "parameter.h"

namespace Imagine
{

// TODO: this is a bit overly-complex, but by design the GUI stuff is completely segmented from the backing data structures
//       so that the GUI part isn't always needed and operates as an additional layer on top of the data structures.
//       But it will be possible to reduce the amount of code here eventually.

ShaderNodeViewWidget::ShaderNodeViewWidget(ShaderNodeViewWidgetHost* pHost, QWidget* pParent)
	: QGraphicsView(pParent),
	  m_mouseMode(eMouseNone), m_scale(0.8f), m_selectionType(eSelectionNone), m_pSelectedItem(nullptr),
	  m_pNodesCollection(nullptr), m_pHost(pHost), m_pInteractionConnectionItem(nullptr), m_selConnectionDragType(eSelConDragNone)
{
//	setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DirectRendering)));

	QGLFormat format = QGLFormat::defaultFormat();
	format.setDepth(false);
	format.setSampleBuffers(true);
	setViewport(new QGLWidget(format));

	for (unsigned int i = 0; i < 3; i++)
	{
		m_creationMenuSignalMapper[i] = nullptr;
	}

	updateCreationMenu();

	//
	QGraphicsScene* pScene = new QGraphicsScene(this);

	pScene->setSceneRect(0, 0, 1600, 1600);
	setScene(pScene);

	centerOn(400, 400);

	setCacheMode(CacheBackground);
	setViewportUpdateMode(BoundingRectViewportUpdate);
	setViewportUpdateMode(MinimalViewportUpdate);
//	setViewportUpdateMode(FullViewportUpdate);
	setRenderHint(QPainter::Antialiasing);

//	setTransformationAnchor(AnchorViewCenter);
	setTransformationAnchor(AnchorUnderMouse);
	setMouseTracking(true);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	scale(m_scale, m_scale);

	setMinimumSize(400, 400);
}


void ShaderNodeViewWidget::updateCreationMenu()
{
	// create list of available Ops
	std::vector<std::string> aOpNames;
	m_pHost->getListOfNodeNames(aOpNames);

	std::vector<std::string>::const_iterator itName = aOpNames.begin();
	for (; itName != aOpNames.end(); ++itName)
	{
		const std::string& name = *itName;

		std::string category;
		std::string itemName;
		splitInTwo(name, category, itemName, "/");

		// find the category for the menu

		std::map<std::string, NodeCreationMenuCategory*>::iterator itFind = m_aMenuCategories.find(category);
		NodeCreationMenuCategory* pMenuCat = nullptr;
		if (itFind == m_aMenuCategories.end())
		{
			// we haven't got this one yet, so create a new one
			pMenuCat = new NodeCreationMenuCategory();

			m_aMenuCategories[category] = pMenuCat;
		}
		else
		{
			pMenuCat = (*itFind).second;
		}

		QAction* pNewAction = new QAction(itemName.c_str(), this);
		pMenuCat->nodeItems.emplace_back(NodeCreationMenuItem(category, itemName, pNewAction));
	}

	m_aMenuCategoriesIndexed.resize(m_aMenuCategories.size());

	unsigned int index = 0;
	std::map<std::string, NodeCreationMenuCategory*>::iterator itCat = m_aMenuCategories.begin();
	for (; itCat != m_aMenuCategories.end(); ++itCat)
	{
		NodeCreationMenuCategory* pCat = (*itCat).second;

		pCat->index = index;

		if (index < 3)
		{
			QSignalMapper* pNewSignalMapper = new QSignalMapper(this);

			// TODO: actually, this *could* be done with a single mapping using offset indices...

			if (index == 0)
			{
				QObject::connect(pNewSignalMapper, SIGNAL(mapped(int)), this, SLOT(menuNewNode0(int)));
			}
			else if (index == 1)
			{
				QObject::connect(pNewSignalMapper, SIGNAL(mapped(int)), this, SLOT(menuNewNode1(int)));
			}
			else if (index == 2)
			{
				QObject::connect(pNewSignalMapper, SIGNAL(mapped(int)), this, SLOT(menuNewNode2(int)));
			}

			m_aMenuCategoriesIndexed[index] = pCat;

			unsigned int menuIndex = 0;

			// now set up the mappings for each QAction for each menu item to the signal mapper
			std::vector<NodeCreationMenuItem>::iterator itNodeItem = pCat->nodeItems.begin();
			for (; itNodeItem != pCat->nodeItems.end(); ++itNodeItem)
			{
				NodeCreationMenuItem& menuItem = *itNodeItem;

				connect(menuItem.pAction, SIGNAL(triggered()), pNewSignalMapper, SLOT(map()));
				pNewSignalMapper->setMapping(menuItem.pAction, menuIndex++);
			}

			m_creationMenuSignalMapper[index] = pNewSignalMapper;

			index++;
		}
	}
}

ShaderNodeViewWidget::~ShaderNodeViewWidget()
{

}

void ShaderNodeViewWidget::createUIItemsFromNodesCollection()
{
	if (!m_pNodesCollection)
		return;

	// clear existing items
	QGraphicsScene* pScene = scene();
	pScene->clear();

	m_pSelectedItem = nullptr;

	m_aConnections.clear();
	m_aNodes.clear();

	// for the moment, use a naive two-pass approach...
	std::map<unsigned int, ShaderNodeUI*> aCreatedUINodes;

	std::vector<ShaderNode*>& nodesArray = m_pNodesCollection->getNodes();

	// create the UI nodes first
	std::vector<ShaderNode*>::iterator itNodes = nodesArray.begin();
	for (; itNodes != nodesArray.end(); ++itNodes)
	{
		ShaderNode* pNode = *itNodes;

		ShaderNodeUI* pNewUINode = new ShaderNodeUI(pNode);

		unsigned int nodeID = pNode->getNodeID();

		aCreatedUINodes[nodeID] = pNewUINode;

		pScene->addItem(pNewUINode);
		m_aNodes.emplace_back(pNewUINode);
	}

	// then go back and add connections for them
	itNodes = nodesArray.begin();
	for (; itNodes != nodesArray.end(); ++itNodes)
	{
		ShaderNode* pNode = *itNodes;

		unsigned int thisNodeID = pNode->getNodeID();

		std::map<unsigned int, ShaderNodeUI*>::iterator itFindMain = aCreatedUINodes.find(thisNodeID);
		if (itFindMain == aCreatedUINodes.end())
		{
			// something's gone wrong
			continue;
		}

		ShaderNodeUI* pThisUINode = (*itFindMain).second;

		// connect all input connections to their source output ports...
		const std::vector<ShaderNode::InputShaderPort>& aInputPorts = pNode->getInputPorts();
		unsigned int inputIndex = 0;
		std::vector<ShaderNode::InputShaderPort>::const_iterator itInputPort = aInputPorts.begin();
		for (; itInputPort != aInputPorts.end(); ++itInputPort, inputIndex++)
		{
			const ShaderNode::InputShaderPort& inputPort = *itInputPort;

			if (inputPort.connection.nodeID == ShaderNode::kNodeConnectionNone)
				continue;

			// otherwise, connect it to the output port on the source node
			std::map<unsigned int, ShaderNodeUI*>::iterator itFindConnectionNode = aCreatedUINodes.find(inputPort.connection.nodeID);
			if (itFindConnectionNode != aCreatedUINodes.end())
			{
				ShaderNodeUI* pOutputUINode = (*itFindConnectionNode).second;

				ShaderConnectionUI* pConnection = new ShaderConnectionUI(pOutputUINode, pThisUINode);
				pConnection->setSourceNodePortIndex(inputPort.connection.portIndex);
				pConnection->setDestinationNodePortIndex(inputIndex);

				pScene->addItem(pConnection);

				m_aConnections.emplace_back(pConnection);

				pThisUINode->setInputPortConnectionItem(inputIndex, pConnection);

				pOutputUINode->addOutputPortConnectionItem(inputPort.connection.portIndex, pConnection);
			}
		}
	}
}

void ShaderNodeViewWidget::wheelEvent(QWheelEvent* event)
{
	return;
	float scaleAmount = powf(1.15f, event->delta());
	qreal scaleFactor = transform().scale(scaleAmount, scaleAmount).mapRect(QRectF(0, 0, 1, 1)).width();

	if (scaleFactor < 0.07 || scaleFactor > 5.0)
		return;

	if (m_scale * scaleFactor < 0.1f || m_scale * scaleFactor > 10.0f)
		return;

	m_scale *= scaleFactor;

	scale(scaleFactor, scaleFactor);
}

void ShaderNodeViewWidget::mousePressEvent(QMouseEvent* event)
{
	m_lastMousePos = getEventPos(event);
	m_lastMouseScenePos = mapToScene(event->pos());

	if (event->button() == Qt::RightButton)
	{
		showRightClickMenu(event);
		return;
	}

	QPointF cursorScenePos = mapToScene(event->pos().x(), event->pos().y());

	bool selectedSomething = false;

	const bool doQuick = false;
	if (doQuick)
	{
		QTransform dummy;
		QGraphicsItem* pHitItem = scene()->itemAt(event->pos(), dummy);

//		if (pHitItem->)

		if (m_pSelectedItem)
		{
			m_pSelectedItem->setSelected(false);
		}

		if (pHitItem)
		{
			m_selectionType = eSelectionNode;
			pHitItem->setSelected(true);
			m_pSelectedItem = pHitItem;
		}
	}
	else
	{
		if (m_pSelectedItem)
		{
			m_pSelectedItem->setSelected(false);
		}

		// doing it this way (while inefficient as no acceleration structure is used) means we only get
		// the Nodes. Using scene.itemAt() returns children of QGraphicsItem object (like the text items)
		std::vector<ShaderNodeUI*>::iterator itNodes = m_aNodes.begin();
		for (; itNodes != m_aNodes.end(); ++itNodes)
		{
			ShaderNodeUI* pTestNode = *itNodes;

			ShaderNodeUI::SelectionInfo selInfo;

			if (pTestNode->doesContain(cursorScenePos, selInfo))
			{
				if (selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionNode)
				{
					m_pSelectedItem = pTestNode;
					m_pSelectedItem->setSelected(true);
					selectedSomething = true;
					m_selectionType = eSelectionNode;
				}
				else if (selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionInputPort)
				{
					selectedSomething = true;
					m_selectionType = eSelectionEdgeInput;

					// see if the selected port currently has a connection attached...
					if (pTestNode->getInputPortConnectionItem(selInfo.portIndex))
					{
						// it has, so we need to modify an existing connection
						m_selConnectionDragType = eSelConDragExistingInput;
						m_pInteractionConnectionItem = pTestNode->getInputPortConnectionItem(selInfo.portIndex);
						m_pInteractionConnectionItem->setTempMousePos(cursorScenePos);
						m_pInteractionConnectionItem->setDestinationNode(nullptr);
						m_pInteractionConnectionItem->setDestinationNodePortIndex(-1);

						// set the connection on the node we're disconnecting from to nullptr
						pTestNode->setInputPortConnectionItem(selInfo.portIndex, nullptr);

						// fix up backing data structure
						pTestNode->getActualNode()->setInputPortConnection(selInfo.portIndex, -1, -1);

						ShaderNode* pSrcNode = m_pInteractionConnectionItem->getSourceNode()->getActualNode();
						pSrcNode->removeOutputPortConnection(m_pInteractionConnectionItem->getSourceNodePortIndex(), pTestNode->getActualNode()->getNodeID(),
															 selInfo.portIndex);

						m_pSelectedItem = m_pInteractionConnectionItem;
					}
					else
					{
						// it hasn't, so create a new temporary one...
						m_selConnectionDragType = eSelConDragNewInput;
						m_pInteractionConnectionItem = new ShaderConnectionUI(nullptr, pTestNode);
						m_pInteractionConnectionItem->setTempMousePos(cursorScenePos);
						m_pInteractionConnectionItem->setDestinationNodePortIndex(selInfo.portIndex);
						m_pSelectedItem = m_pInteractionConnectionItem;
						scene()->addItem(m_pInteractionConnectionItem);
					}
				}
				else if (selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionOutputPort)
				{
					selectedSomething = true;
					m_selectionType = eSelectionEdgeOutput;

					unsigned int existingOutputConnections = pTestNode->getOutputPortConnectionCount(selInfo.portIndex);
					// see if the selected port currently has a connection attached...
					if (existingOutputConnections > 0)
					{
						// if it has, given that we're allowed to have an output port connected to multiple different
						// input ports/nodes, allow creating a new one by default, or if shift is held down and only one
						// connection exists, drag existing connection.

						if (existingOutputConnections == 1 && event->modifiers() & Qt::ShiftModifier)
						{
							// so modify the existing connection
							m_selConnectionDragType = eSelConDragExistingOutput;
							m_pInteractionConnectionItem = pTestNode->getSingleOutputPortConnectionItem(selInfo.portIndex);
							m_pInteractionConnectionItem->setTempMousePos(cursorScenePos);
							m_pInteractionConnectionItem->setSourceNode(nullptr);
							m_pInteractionConnectionItem->setSourceNodePortIndex(-1);

							// set the connection on the node we're disconnecting from to nullptr
							pTestNode->addOutputPortConnectionItem(selInfo.portIndex, nullptr);

							m_pSelectedItem = m_pInteractionConnectionItem;

							// fix up backing data structure
							pTestNode->getActualNode()->removeOutputPortConnection(selInfo.portIndex,
																		 m_pInteractionConnectionItem->getDestinationNode()->getActualNode()->getNodeID(),
																		 m_pInteractionConnectionItem->getDestinationNodePortIndex());

							ShaderNode* pDestNode = m_pInteractionConnectionItem->getDestinationNode()->getActualNode();
							pDestNode->setInputPortConnection(m_pInteractionConnectionItem->getDestinationNodePortIndex(), -1, -1);

							break;
						}
					}

					// otherwise (no connections, or more than one), just create a new connection for dragging
					m_selConnectionDragType = eSelConDragNewOutput;
					m_pInteractionConnectionItem = new ShaderConnectionUI(pTestNode, nullptr);
					m_pInteractionConnectionItem->setTempMousePos(cursorScenePos);
					m_pInteractionConnectionItem->setSourceNodePortIndex(selInfo.portIndex);
					pTestNode->addOutputPortConnectionItem(selInfo.portIndex, m_pInteractionConnectionItem);
					m_pSelectedItem = m_pInteractionConnectionItem;
					scene()->addItem(m_pInteractionConnectionItem);
				}
				break;
			}
		}
	}

	if (!selectedSomething)
	{
		// if we didn't select a node, see if we selected a connection...
		// TODO: this isn't really that efficient, might want to have another look at using
		//       scene()->itemAt(), although that'll involve a lot of re-jigging how things are done...

		// build a unique list of connections

		std::set<ShaderConnectionUI*> aConnections;

		std::vector<ShaderNodeUI*>::iterator itNodes = m_aNodes.begin();
		for (; itNodes != m_aNodes.end(); ++itNodes)
		{
			ShaderNodeUI* pTestNode = *itNodes;
			pTestNode->getConnectionItems(aConnections);
		}

		// the final closest connection info ends up specified in here...
		ShaderConnectionUI::SelectionInfo connSelInfo;

		float closestConnectionHitDistance = 25.0f;
		bool haveConnection = false;

		std::set<ShaderConnectionUI*>::iterator itConnection = aConnections.begin();
		for (; itConnection != aConnections.end(); ++itConnection)
		{
			ShaderConnectionUI* pConn = *itConnection;

			// ignore connections which aren't fully-connected
			if (pConn->getDestinationNode() == nullptr || pConn->getSourceNode() == nullptr)
				continue;

			if (pConn->didHit(cursorScenePos, connSelInfo, closestConnectionHitDistance))
			{
				haveConnection = true;
				// don't break out, as we want the closest connection...
			}
		}

		if (haveConnection)
		{
			selectedSomething = true;

			m_pInteractionConnectionItem = connSelInfo.pConnection;
			m_pInteractionConnectionItem->setTempMousePos(cursorScenePos);

			m_pSelectedItem = m_pInteractionConnectionItem;

			// so we need to disconnect the connection at the end closest to where it was clicked...
			if (connSelInfo.wasSource)
			{
				m_selectionType = eSelectionEdgeOutput;

				// disconnect the output port side (source)
				m_selConnectionDragType = eSelConDragExistingOutput;

				ShaderNodeUI* pOldSourceNode = m_pInteractionConnectionItem->getSourceNode();
				unsigned int oldSourcePortIndex = m_pInteractionConnectionItem->getSourceNodePortIndex();
				m_pInteractionConnectionItem->setSourceNode(nullptr);
				m_pInteractionConnectionItem->setSourceNodePortIndex(-1);

				// set the connection on the node we're disconnecting from to nullptr
				pOldSourceNode->removeOutputPortConnectionItem(oldSourcePortIndex, m_pInteractionConnectionItem);

				// fix up backing data structure - both source and destination
				ShaderNode* pActualSourceNode = pOldSourceNode->getActualNode();
				ShaderNode* pActualDestNode = m_pInteractionConnectionItem->getDestinationNode()->getActualNode();

				unsigned int destNodePortID = m_pInteractionConnectionItem->getDestinationNodePortIndex();

				pActualSourceNode->removeOutputPortConnection(oldSourcePortIndex,
															  pActualDestNode->getNodeID(), destNodePortID);

				pActualDestNode->setInputPortConnection(destNodePortID, -1, -1);
			}
			else
			{
				m_selectionType = eSelectionEdgeInput;

				// disconnect the input port side (destination)
				m_selConnectionDragType = eSelConDragExistingInput;

				ShaderNodeUI* pOldDestinationNode = m_pInteractionConnectionItem->getDestinationNode();
				unsigned int oldDestinationPortIndex = m_pInteractionConnectionItem->getSourceNodePortIndex();
				m_pInteractionConnectionItem->setDestinationNode(nullptr);
				m_pInteractionConnectionItem->setDestinationNodePortIndex(-1);

				// set the connection on the node we're disconnecting from to nullptr
				pOldDestinationNode->setInputPortConnectionItem(oldDestinationPortIndex, nullptr);

				// fix up backing data structure - both source and destination
				ShaderNode* pActualDestNode = pOldDestinationNode->getActualNode();
				ShaderNode* pActualSourceNode = m_pInteractionConnectionItem->getSourceNode()->getActualNode();

				unsigned int sourceNodePortID = m_pInteractionConnectionItem->getSourceNodePortIndex();

				pActualSourceNode->removeOutputPortConnection(sourceNodePortID,
															  pActualDestNode->getNodeID(), oldDestinationPortIndex);

				pActualDestNode->setInputPortConnection(oldDestinationPortIndex, -1, -1);
			}
		}
	}


	if (!selectedSomething && m_pSelectedItem)
	{
		m_pSelectedItem->setSelected(false);
		m_selectionType = eSelectionNone;
		m_pSelectedItem = nullptr;
	}

	if (event->modifiers() & Qt::ALT || event->button() == Qt::MiddleButton)
	{
		m_mouseMode = eMousePan;

		return;
	}

	if (m_pSelectedItem)
	{
		m_mouseMode = eMouseDrag;
		m_pSelectedItem->setSelected(true);
	}
}

void ShaderNodeViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	QPointF sceneMousePos = mapToScene(event->pos());
	QPointF cursorPosition = getEventPos(event);

	if (m_mouseMode == eMouseNone)
	{

	}
	else if (m_mouseMode == eMousePan)
	{
//		float deltaMoveX =
		QPointF deltaMove = mapToScene(event->pos()) - m_lastMouseScenePos;

//		QGraphicsView::mouseMoveEvent(event);
		translate(deltaMove.rx(), deltaMove.ry());

//		update();
	}
	else if (m_mouseMode == eMouseDrag && m_pSelectedItem)
	{
		QPointF deltaMove = sceneMousePos - m_lastMouseScenePos;

		if (m_selectionType == eSelectionNode)
		{
			// TODO: is this the most efficient way of doing things?
			//       build up a cumulative delta during move and do it on mouse release instead?
			ShaderNodeUI* pNode = static_cast<ShaderNodeUI*>(m_pSelectedItem);
			pNode->movePos(deltaMove.x(), deltaMove.y());
		}
		else if (m_selectionType == eSelectionEdgeInput)
		{
			if (m_selConnectionDragType == eSelConDragNewInput || m_selConnectionDragType == eSelConDragExistingInput)
			{
				ShaderConnectionUI* pConnection = static_cast<ShaderConnectionUI*>(m_pSelectedItem);

				QPointF temp = sceneMousePos;
				pConnection->publicPrepareGeometryChange();
				pConnection->setTempMousePos(temp);
			}
		}
		else if (m_selectionType == eSelectionEdgeOutput)
		{
			if (m_selConnectionDragType == eSelConDragNewOutput || m_selConnectionDragType == eSelConDragExistingOutput)
			{
				ShaderConnectionUI* pConnection = static_cast<ShaderConnectionUI*>(m_pSelectedItem);

				QPointF temp = sceneMousePos;
				pConnection->publicPrepareGeometryChange();
				pConnection->setTempMousePos(temp);
			}
		}
	}

	m_lastMousePos = getEventPos(event);
	m_lastMouseScenePos = sceneMousePos;

	QGraphicsView::mousePressEvent(event);
}

void ShaderNodeViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	QPointF cursorScenePos = mapToScene(event->pos().x(), event->pos().y());

	if (m_selectionType == eSelectionNone)
		return;

	bool didConnectNewConnection = false;
	bool didReconnectExistingConnection = false;

	if (m_selConnectionDragType != eSelConDragNone &&
			m_pSelectedItem != nullptr)
	{
		// we should be currently dragging a connection from a port, so work out what port we might be being
		// dropped on on a source node...

		// work out where we dropped...
		std::vector<ShaderNodeUI*>::iterator itNodes = m_aNodes.begin();
		for (; itNodes != m_aNodes.end(); ++itNodes)
		{
			ShaderNodeUI* pTestNode = *itNodes;

			if (!pTestNode->isActive())
				continue;

			ShaderNodeUI::SelectionInfo selInfo;

			if (!pTestNode->doesContain(cursorScenePos, selInfo))
				continue;

			ShaderPortVariableType existingPortType = ePortTypeAny;

			// check we're not trying to connect to the same node which isn't allowed...
			if (m_pInteractionConnectionItem)
			{
				// get hold of currently existing node, which should be the other end of the connection
				ShaderNodeUI* pOtherNode = m_pInteractionConnectionItem->getSourceNode();
				if (!pOtherNode)
				{
					pOtherNode = m_pInteractionConnectionItem->getDestinationNode();
				}

				if (pOtherNode == pTestNode)
					continue;

				ShaderPortVariableType thisPortType = ePortTypeAny;

				if (selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionOutputPort)
				{
					thisPortType = pTestNode->getActualNode()->getOutputPort(selInfo.portIndex).type;
					existingPortType = pOtherNode->getActualNode()->getInputPort(m_pInteractionConnectionItem->getDestinationNodePortIndex()).type;
				}
				else if (selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionInputPort)
				{
					thisPortType = pTestNode->getActualNode()->getInputPort(selInfo.portIndex).type;
					existingPortType = pOtherNode->getActualNode()->getOutputPort(m_pInteractionConnectionItem->getSourceNodePortIndex()).type;
				}

				bool enforceConnectionTypes = true;

				if (thisPortType == ePortTypeAny || existingPortType == ePortTypeAny)
				{
					enforceConnectionTypes = false;
				}

				if (enforceConnectionTypes && existingPortType != thisPortType)
				{
					continue;
				}
			}

			if (m_selConnectionDragType == eSelConDragNewOutput && selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionInputPort)
			{
				// we're currently dragging from an output port (src, right side) on the source node, and have been dropped
				// onto an input port on the destination (left side)

				// see if the selected port currently has a connection attached...
				if (pTestNode->getInputPortConnectionItem(selInfo.portIndex))
				{
					// it has, so we won't do anything for the moment
					// TODO: if Shift is held down, swap the connection with the new one?
				}
				else
				{
					// it hasn't, so connect the current interactive one which should still be
					// connected at the other end...

					m_pInteractionConnectionItem->setDestinationNode(pTestNode);
					m_pInteractionConnectionItem->setDestinationNodePortIndex(selInfo.portIndex);

					// connect to destination
					pTestNode->setInputPortConnectionItem(selInfo.portIndex, m_pInteractionConnectionItem);

					// connect to source
					ShaderNodeUI* pSrcNode = m_pInteractionConnectionItem->getSourceNode();
					unsigned int srcNodePortID = m_pInteractionConnectionItem->getSourceNodePortIndex();
					pSrcNode->addOutputPortConnectionItem(srcNodePortID, m_pInteractionConnectionItem);

					didConnectNewConnection = true;
					// leak m_pInteractionConnectionItem for the moment...

					// fix up the backing data structure

					ShaderNode* pActualSrcNode = pSrcNode->getActualNode();
					ShaderNode* pActualDstNode = pTestNode->getActualNode();

					pActualSrcNode->addOutputPortConnection(srcNodePortID, pActualDstNode->getNodeID(), selInfo.portIndex);
					pActualDstNode->setInputPortConnection(selInfo.portIndex, pActualSrcNode->getNodeID(), srcNodePortID);
				}
			}
			else if (m_selConnectionDragType == eSelConDragExistingInput && selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionInputPort)
			{
				// we're dragging the input connection (left side of node) from an existing connection to a new one, so we're modifying
				// where the new destination of this connection should be

				m_pInteractionConnectionItem->setDestinationNode(pTestNode);
				m_pInteractionConnectionItem->setDestinationNodePortIndex(selInfo.portIndex);

				pTestNode->setInputPortConnectionItem(selInfo.portIndex, m_pInteractionConnectionItem);

				ShaderNodeUI* pSrcNode = m_pInteractionConnectionItem->getSourceNode();
				unsigned int srcNodePortID = m_pInteractionConnectionItem->getSourceNodePortIndex();
				pSrcNode->addOutputPortConnectionItem(srcNodePortID, m_pInteractionConnectionItem);

				// alter backing data structure
				ShaderNode* pActualSrcNode = pSrcNode->getActualNode();
				ShaderNode* pActualDstNode = pTestNode->getActualNode();

				pActualSrcNode->addOutputPortConnection(srcNodePortID, pActualDstNode->getNodeID(), selInfo.portIndex);
				pActualDstNode->setInputPortConnection(selInfo.portIndex, pActualSrcNode->getNodeID(), srcNodePortID);

				didReconnectExistingConnection = true;
			}
			else if (m_selConnectionDragType == eSelConDragNewInput && selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionOutputPort)
			{
				// we're currently dragging from an input port (destination, left side) on destination node,
				// and have been dropped onto an output port on the source (right side)

				// we support multiple connections on output ports
				m_pInteractionConnectionItem->setSourceNode(pTestNode);
				m_pInteractionConnectionItem->setSourceNodePortIndex(selInfo.portIndex);

				// connect source
				pTestNode->addOutputPortConnectionItem(selInfo.portIndex, m_pInteractionConnectionItem);

				// connect to destination
				ShaderNodeUI* pDstNode = m_pInteractionConnectionItem->getDestinationNode();
				unsigned int dstNodePortID = m_pInteractionConnectionItem->getDestinationNodePortIndex();
				pDstNode->setInputPortConnectionItem(dstNodePortID, m_pInteractionConnectionItem);

				didConnectNewConnection = true;
				// leak m_pInteractionConnectionItem for the moment...

				// fix up the backing data structure

				ShaderNode* pActualSrcNode = pTestNode->getActualNode();
				ShaderNode* pActualDstNode = pDstNode->getActualNode();

				pActualSrcNode->addOutputPortConnection(selInfo.portIndex, pActualDstNode->getNodeID(), dstNodePortID);
				pActualDstNode->setInputPortConnection(dstNodePortID, pActualSrcNode->getNodeID(), selInfo.portIndex);
			}
			else if (m_selConnectionDragType == eSelConDragExistingOutput && selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionOutputPort)
			{
				// we're dragging the output connection (right side of node) from an existing connection to a new one, so we're modifying
				// where the new source of this connection should be

				m_pInteractionConnectionItem->setSourceNode(pTestNode);
				m_pInteractionConnectionItem->setSourceNodePortIndex(selInfo.portIndex);

				pTestNode->addOutputPortConnectionItem(selInfo.portIndex, m_pInteractionConnectionItem);

				// connect to destination
				ShaderNodeUI* pDstNode = m_pInteractionConnectionItem->getDestinationNode();
				unsigned int dstNodePortID = m_pInteractionConnectionItem->getDestinationNodePortIndex();
				pDstNode->setInputPortConnectionItem(dstNodePortID, m_pInteractionConnectionItem);

				// fix up the backing data structure

				ShaderNode* pActualSrcNode = pTestNode->getActualNode();
				ShaderNode* pActualDstNode = pDstNode->getActualNode();

				pActualSrcNode->addOutputPortConnection(selInfo.portIndex, pActualDstNode->getNodeID(), dstNodePortID);
				pActualDstNode->setInputPortConnection(dstNodePortID, pActualSrcNode->getNodeID(), selInfo.portIndex);

				didReconnectExistingConnection = true;
			}
			break;

			m_selectionType = eSelectionNone;
		}
	}

	if (m_selectionType != eSelectionNode)
	{
		m_pSelectedItem = nullptr;
		m_selectionType = eSelectionNone;
		m_selConnectionDragType = eSelConDragNone;
	}

	m_mouseMode = eMouseNone;

	if (m_pInteractionConnectionItem)
	{
		m_pInteractionConnectionItem->setSelected(false);
	}

	// clean up any undropped connections...
	// TODO: this is pretty crap...
	if (didConnectNewConnection)
	{
		// m_pIneractionConnectionItem has now been made into a full connection
	}
	else if (didReconnectExistingConnection)
	{
		// an existing connection was re-connected to another node/port
	}
	else
	{
		// TODO: need to work out what to do with this
		// seems really bad having these allocated per-mouse down on ports... - lots of scope for
		// memory leaks...
		if (m_pInteractionConnectionItem)
		{
			// this connection is going to be discarded as it's been unconnected (dragged from a port to an empty location)

			// we need to change the connection state within the nodes, so they don't think they're connected any more
			ShaderNodeUI* pDstNode = m_pInteractionConnectionItem->getDestinationNode();
			if (pDstNode)
			{
				unsigned int originalPortIndex = m_pInteractionConnectionItem->getDestinationNodePortIndex();
				pDstNode->setInputPortConnectionItem(originalPortIndex, nullptr);
				m_pInteractionConnectionItem->setDestinationNode(nullptr);
			}

			ShaderNodeUI* pSrcNode = m_pInteractionConnectionItem->getSourceNode();
			if (pSrcNode)
			{
				unsigned int originalPortIndex = m_pInteractionConnectionItem->getSourceNodePortIndex();
				pSrcNode->addOutputPortConnectionItem(originalPortIndex, nullptr);
				m_pInteractionConnectionItem->setSourceNode(nullptr);
			}

			// just deleting the object is good enough, as QGraphicsItem's destructor calls
			// QGraphicsScene's removeItem() function, so we don't need to do it ourselves..
			delete m_pInteractionConnectionItem;
		}
	}

	m_pInteractionConnectionItem = nullptr;

	if (didConnectNewConnection || didReconnectExistingConnection)
	{
		// trigger a re-draw, so any just dropped connections snap into place...
		invalidateScene();
	}

	QGraphicsView::mouseReleaseEvent(event);
}

void ShaderNodeViewWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	m_lastMousePos = getEventPos(event);
	m_lastMouseScenePos = mapToScene(event->pos());

	QPointF cursorScenePos = mapToScene(event->pos().x(), event->pos().y());

	bool selectedSomething = false;

	if (m_pSelectedItem)
	{
		m_pSelectedItem->setSelected(false);
	}

	ShaderNodeUI* pClickedNode = nullptr;

	// doing it this way (while inefficient as no acceleration structure is used) means we only get
	// the Nodes. Using scene.itemAt() returns children of QGraphicsItem object (like the text items)
	std::vector<ShaderNodeUI*>::iterator itNodes = m_aNodes.begin();
	for (; itNodes != m_aNodes.end(); ++itNodes)
	{
		ShaderNodeUI* pTestNode = *itNodes;

		ShaderNodeUI::SelectionInfo selInfo;

		if (pTestNode->doesContain(cursorScenePos, selInfo))
		{
			if (selInfo.selectionType == ShaderNodeUI::SelectionInfo::eSelectionNode)
			{
				m_pSelectedItem = pTestNode;
				m_pSelectedItem->setSelected(true);
				selectedSomething = true;
				m_selectionType = eSelectionNode;

				pClickedNode = pTestNode;
			}
		}
	}

	if (!selectedSomething && m_pSelectedItem)
	{
		m_pSelectedItem->setSelected(false);
		m_selectionType = eSelectionNone;
		m_pSelectedItem = nullptr;

		return;
	}

	// do stuff with the node that was double-clicked

	ShaderNode* pNode = pClickedNode->getActualNode();

	ParametersInterface* pNodeParametersInterface  = pNode->getOpParametersInterface();

	ParametersInterface* pParamInterface = dynamic_cast<ParametersInterface*>(m_pSelectedItem);

	if (pNodeParametersInterface)
	{
		m_pHost->showParametersPanelForOp(pNodeParametersInterface);
	}
}

void ShaderNodeViewWidget::showRightClickMenu(QMouseEvent* event)
{
	// work out what type of menu we should show
	std::vector<std::string> aNodeCategories;
	m_pHost->getAllowedCreationCategories(aNodeCategories);

	QMenu menu(this);

	QMenu* pNewNodeMenu = menu.addMenu("New node...");

	std::vector<std::string>::iterator itCats = aNodeCategories.begin();
	for (; itCats != aNodeCategories.end(); ++itCats)
	{
		const std::string& category = *itCats;
		std::map<std::string, NodeCreationMenuCategory*>::iterator itFind = m_aMenuCategories.find(category);
		if (itFind == m_aMenuCategories.end())
		{
			continue;
		}

		NodeCreationMenuCategory* pMenuCat = (*itFind).second;

		QMenu* pCatMenu = pNewNodeMenu->addMenu(category.c_str());

		std::vector<NodeCreationMenuItem>::iterator itMenu = pMenuCat->nodeItems.begin();
		for (; itMenu != pMenuCat->nodeItems.end(); ++itMenu)
		{
			NodeCreationMenuItem& menuItem = *itMenu;
			pCatMenu->addAction(menuItem.pAction);
		}
	}

	menu.addSeparator();

	menu.exec(event->globalPos());
}

QPointF ShaderNodeViewWidget::getEventPos(QMouseEvent* event) const
{
#ifdef IMAGINE_QT_5
	return event->localPos();
#else
	return event->posF();
#endif
}

void ShaderNodeViewWidget::createNewNodeMenu(unsigned int categoryIndex, int menuIndex)
{
	NodeCreationMenuCategory* pCat = m_aMenuCategoriesIndexed[categoryIndex];

	NodeCreationMenuItem& item = pCat->nodeItems[menuIndex];

	ShaderOp* pNewOp = m_pHost->createNewShaderOp(item.category, item.name);

	ShaderNode* pNewNode = new ShaderNode(pNewOp);

	pNewNode->setName(pNewOp->getOpTypeName());

	m_pNodesCollection->addNode(pNewNode);

	ShaderNodeUI* pNewUINode = new ShaderNodeUI(pNewNode);
	pNewUINode->setCustomPos(m_lastMouseScenePos.rx(), m_lastMouseScenePos.ry());

	m_aNodes.emplace_back(pNewUINode);
	scene()->addItem(pNewUINode);
}

void ShaderNodeViewWidget::menuNewNode0(int index)
{
	createNewNodeMenu(0, index);
}

void ShaderNodeViewWidget::menuNewNode1(int index)
{
	createNewNodeMenu(1, index);
}

void ShaderNodeViewWidget::menuNewNode2(int index)
{
	createNewNodeMenu(2, index);
}

} // namespace Imagine
