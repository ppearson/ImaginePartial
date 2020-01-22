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

#include "shader_node_view_items.h"

#include <cmath>
#include <cstdio>

#include <QColor>
#include <QPen>
#include <QLineF>
#include <QPainter>
#include <QFontMetrics>

#include <QRectF>
#include <QSizeF>

#include "utils/maths/maths.h"

#include "shading/shader_node.h"

namespace Imagine
{

static const float kNodeHeaderHeight = 24.0f;

static const float kNodePortRadius = 6.0f;
static const float kNodeFirstPortHeight = 48.0f;
static const float kNodePortSpacing = 24.0f;

ShaderNodeUI::ShaderNodeUI(const std::string& name, ShaderNode* pActualNode)
	: QGraphicsItem(), m_pActualNode(pActualNode), m_name(name)
{
	setCacheMode(DeviceCoordinateCache);
	setFlags(QGraphicsItem::ItemIsSelectable);
	setZValue(0.0);

	initStateFromShaderNode();
}

ShaderNodeUI::ShaderNodeUI(ShaderNode* pActualNode) : QGraphicsItem(), m_pActualNode(pActualNode)
{
	setCacheMode(DeviceCoordinateCache);
	setFlags(QGraphicsItem::ItemIsSelectable);
	setZValue(0.0);

	initStateFromShaderNode();

	m_name = pActualNode->getName();

	setPos(pActualNode->getPosX(), pActualNode->getPosY());
}

ShaderNodeUI::~ShaderNodeUI()
{

}

void ShaderNodeUI::initStateFromShaderNode()
{
	if (!m_pActualNode)
		return;

	unsigned int numInputs = m_pActualNode->getInputPorts().size();
	m_aInputPortConnectionItems.resize(numInputs);
	for (unsigned int i = 0; i < numInputs; i++)
	{
		m_aInputPortConnectionItems[i] = nullptr;
	}

	unsigned int numOutputs = m_pActualNode->getOutputPorts().size();
	m_aOutputPortConnectionItems.resize(numOutputs);

	float firstOutputPortHeight = kNodeFirstPortHeight +
			((float)m_pActualNode->getInputPorts().size() * (kNodePortSpacing));

	float height = firstOutputPortHeight + (float)m_pActualNode->getOutputPorts().size() * kNodePortSpacing;

	m_nodeSize = QSizeF(230.0f, height);
}

ShaderNode* ShaderNodeUI::getActualNode()
{
	return m_pActualNode;
}

void ShaderNodeUI::setCustomPos(int x, int y)
{
	QGraphicsItem::setPos(x, y);
	if (m_pActualNode)
	{
		m_pActualNode->setPos(x, y);
	}
}

void ShaderNodeUI::movePos(int x, int y)
{
	QGraphicsItem::moveBy(x, y);
	if (m_pActualNode)
	{
		float newX = m_pActualNode->getPosX() + x;
		float newY = m_pActualNode->getPosY() + y;
		m_pActualNode->setPos(newX, newY);
	}
}

QRectF ShaderNodeUI::boundingRect() const
{
	QRectF localRect(QPointF(), QSizeF(m_nodeSize.width() + (kNodePortRadius * 2.0f), m_nodeSize.height()));
	return localRect;
}

bool ShaderNodeUI::contains(const QPointF& point) const
{
	return boundingRect().contains(point);
}

bool ShaderNodeUI::containsSceneSpace(const QPointF& point) const
{
	QRectF localRect = boundingRect();
	localRect.moveTo(pos().x(), pos().y());

	return localRect.contains(point);
}

void ShaderNodeUI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	if (!isSelected())
	{
		painter->setPen(QPen(QColor(0, 0, 0)));
	}
	else
	{
		painter->setPen(QPen(QColor(0, 0, 255)));
	}

	// normal node
	painter->setBrush(QBrush(QColor(192, 192, 192)));

	QRectF mainNodeRect(QPointF(kNodePortRadius, 0.0f), QPointF(m_nodeSize.width(), m_nodeSize.height()));
	painter->drawRoundedRect(mainNodeRect, 5.0f, 5.0f);

	QFont font;
	QFontMetrics fontMetrics(font);

	painter->setPen(QPen(QColor(0, 0, 0)));

	// draw header line
	painter->drawLine(QPointF(kNodePortRadius, kNodeHeaderHeight), QPointF(m_nodeSize.width(), kNodeHeaderHeight));

	// draw node name in the header...
	QSizeF textSize = fontMetrics.size(0, m_name.c_str());

	QPointF rectCentre(m_nodeSize.width() / 2.0, kNodeHeaderHeight / 2.0);
	QPointF textPos = rectCentre;
	textPos += QPointF(-textSize.width() / 2.0, textSize.height() / 2.0 - 2.0f);

	painter->drawText(textPos.x(), textPos.y(), m_name.c_str());

	// draw input ports
	const std::vector<ShaderNode::InputShaderPort>& aInputPorts = m_pActualNode->getInputPorts();
	for (unsigned int i = 0; i < aInputPorts.size(); i++)
	{
		const ShaderNode::InputShaderPort& port = aInputPorts[i];

		float heightOffset = (float)i * kNodePortSpacing;
		// draw outer main circle
		painter->setBrush(QBrush(QColor(232, 232, 64)));
		painter->drawEllipse(QPointF(kNodePortRadius, kNodeFirstPortHeight + heightOffset), kNodePortRadius, kNodePortRadius);

		QSizeF portNameTextSize = fontMetrics.size(0, port.name.c_str());

		QPointF portTextPos(kNodePortRadius * 3.0f, kNodeFirstPortHeight + heightOffset);
		portTextPos += QPointF(0.0f, portNameTextSize.height() / 4.0f);

		painter->drawText(portTextPos.x(), portTextPos.y(), port.name.c_str());
	}

	float outputPortStartHeight = ((float)aInputPorts.size() * kNodePortSpacing) + kNodeFirstPortHeight;

	// draw output ports
	const std::vector<ShaderNode::OutputShaderPort>& aOutputPorts = m_pActualNode->getOutputPorts();
	for (unsigned int i = 0; i < aOutputPorts.size(); i++)
	{
		const ShaderNode::OutputShaderPort& port = aOutputPorts[i];

		float heightOffset = (float)i * kNodePortSpacing;
		painter->setBrush(QBrush(QColor(128, 128, 232)));
		painter->drawEllipse(QPointF(m_nodeSize.width(), outputPortStartHeight + heightOffset), kNodePortRadius, kNodePortRadius);

		QSizeF portNameTextSize = fontMetrics.size(0, port.name.c_str());

		QPointF portTextPos(m_nodeSize.width() - kNodePortRadius * 3.0f, outputPortStartHeight + heightOffset);
		portTextPos -= QPointF(portNameTextSize.width(), 0.0f);
		portTextPos += QPointF(0.0f, portNameTextSize.height() / 4.0f);

		painter->drawText(portTextPos.x(), portTextPos.y(), port.name.c_str());
	}
}

QVariant ShaderNodeUI::itemChange(GraphicsItemChange change, const QVariant& value)
{
	return QGraphicsItem::itemChange(change, value);
}

void ShaderNodeUI::publicPrepareGeometryChange()
{
	prepareGeometryChange();
}

QPointF ShaderNodeUI::getInputPortPosition(unsigned int portIndex) const
{
	float portOffset = (float)portIndex * kNodePortSpacing;
	QPointF finalPosition(kNodePortRadius, kNodeFirstPortHeight + portOffset);
	finalPosition += pos();

	return finalPosition;
}

QPointF ShaderNodeUI::getOutputPortPosition(unsigned int portIndex) const
{
	float outputPortStartHeight = ((float)m_pActualNode->getInputPorts().size() * kNodePortSpacing) + kNodeFirstPortHeight;
	float portOffset = (float)portIndex * kNodePortSpacing;
	QPointF finalPosition(m_nodeSize.width(), outputPortStartHeight + portOffset);
	finalPosition += pos();

	return finalPosition;
}

// this isn't the most efficient way of doing things, as we don't get to use
// QGraphicsScene's acceleration structures, but it means we don't have to mess around with
// subobjects
bool ShaderNodeUI::doesContain(const QPointF& point, SelectionInfo& selInfo) const
{
	QPointF localPosition = point - pos();

//	fprintf(stderr, "DC: (%f, %f)\n", point.x(), point.y());
//	fprintf(stderr, "doesContain(): (%f, %f)\n", localPosition.x(), localPosition.y());

	// check overall bounds of entire node first
	QRectF localRect = boundingRect();
	if (!localRect.contains(localPosition))
		return false;

	// now work out what approximate region we're in
	QPointF inputPortTopLeft(0.0f, kNodeFirstPortHeight - kNodePortRadius);
	QPointF inputPortBottomRight(kNodePortRadius * 2.0, kNodeFirstPortHeight + (float)m_pActualNode->getInputPorts().size() * kNodePortSpacing);
	QRectF inputPortRegion(inputPortTopLeft, inputPortBottomRight);
	if (inputPortRegion.contains(localPosition))
	{
		// it's an input port

		// work out which one
		for (unsigned int i = 0; i < m_pActualNode->getInputPorts().size(); i++)
		{
			QPointF portPos(kNodePortRadius, kNodeFirstPortHeight + (float)i * kNodePortSpacing);
			QPointF dist1 = portPos - localPosition;
			float length = dist1.manhattanLength();

			if (length <= kNodePortRadius * 2.0f)
			{
				selInfo.portIndex = i;
//				fprintf(stderr, "Input port: %u\n", i);

				// annoying we have to const cast this, but...
				selInfo.pNode = const_cast<ShaderNodeUI*>(this);
				selInfo.selectionType = SelectionInfo::eSelectionInputPort;
				return true;
			}
		}

		return false;
	}

	float firstOutputPortHeight = kNodeFirstPortHeight +
			((float)m_pActualNode->getInputPorts().size() * (kNodePortSpacing));
	QPointF outputPortTopLeft(m_nodeSize.width() - kNodePortRadius, firstOutputPortHeight - kNodePortRadius);
	QPointF outputPortBottomRight(m_nodeSize.width() + kNodePortRadius,
								  firstOutputPortHeight + (float)m_pActualNode->getOutputPorts().size() * kNodePortSpacing);
	QRectF outputPortRegion(outputPortTopLeft, outputPortBottomRight);
	if (outputPortRegion.contains(localPosition))
	{
		// it's an output port

		// work out which one
		for (unsigned int i = 0; i < m_pActualNode->getOutputPorts().size(); i++)
		{
			QPointF portPos(m_nodeSize.width() + kNodePortRadius, firstOutputPortHeight + (float)i * kNodePortSpacing);
			QPointF dist1 = portPos - localPosition;
			float length = dist1.manhattanLength();

			if (length <= kNodePortRadius * 2.0f)
			{
				selInfo.portIndex = i;
//				fprintf(stderr, "Output port: %u\n", i);

				// annoying we have to const cast this, but...
				selInfo.pNode = const_cast<ShaderNodeUI*>(this);
				selInfo.selectionType = SelectionInfo::eSelectionOutputPort;
				return true;
			}
		}

		return false;
	}

	QRectF mainNodeRegion(QPointF(kNodePortRadius, 0.0f), QPointF(m_nodeSize.width(), m_nodeSize.height()));
	if (mainNodeRegion.contains(localPosition))
	{
		// it's the main node

		selInfo.selectionType = SelectionInfo::eSelectionNode;
		return true;
	}

	return true;
}

void ShaderNodeUI::setInputPortConnectionItem(unsigned int index, ShaderConnectionUI* pItem)
{
	m_aInputPortConnectionItems[index] = pItem;

	// set the internal connection output nodes as well...
	if (pItem && pItem->getSourceNode() && pItem->getSourceNodePortIndex() != -1)
	{
		ShaderNode* pActualSourceNode = pItem->getSourceNode()->getActualNode();
		std::vector<ShaderNode::OutputShaderPort>& outPorts = pActualSourceNode->getOutputPorts();

		unsigned int sourcePortIndex = pItem->getSourceNodePortIndex();


	}
}

ShaderConnectionUI* ShaderNodeUI::getInputPortConnectionItem(unsigned int index)
{
	return m_aInputPortConnectionItems[index];
}

unsigned int ShaderNodeUI::getOutputPortConnectionCount(unsigned int portIndex) const
{
	std::vector<ShaderNode::OutputShaderPort>& outputPorts = m_pActualNode->getOutputPorts();

	const ShaderNode::OutputShaderPort& outputPort = outputPorts[portIndex];

	return outputPort.connections.size();
}

void ShaderNodeUI::addOutputPortConnectionItem(unsigned int portIndex, ShaderConnectionUI* pItem)
{
	// set the UI-only state
	std::vector<ShaderConnectionUI*>& outputPortConnections = m_aOutputPortConnectionItems[portIndex];
	outputPortConnections.emplace_back(pItem);
}

void ShaderNodeUI::removeOutputPortConnectionItem(unsigned int portIndex, ShaderConnectionUI* pItem)
{
	if (!pItem)
		return;

	// find the output port which is currently connected to the ShaderConnectionUI's destination node port
	// and remove it
	std::vector<ShaderConnectionUI*>::iterator itConn = m_aOutputPortConnectionItems[portIndex].begin();
	for (; itConn != m_aOutputPortConnectionItems[portIndex].end(); ++itConn)
	{
		ShaderConnectionUI* pTestConn = *itConn;

		if (pTestConn == pItem)
		{
			m_aOutputPortConnectionItems[portIndex].erase(itConn);
			break;
		}
	}
}

// assumes the UI-side of things does have one valid connection for this port
ShaderConnectionUI* ShaderNodeUI::getSingleOutputPortConnectionItem(unsigned int index)
{
	std::vector<ShaderConnectionUI*>& outputPortConnections = m_aOutputPortConnectionItems[index];

	return outputPortConnections[0];
}

void ShaderNodeUI::getConnectionItems(std::set<ShaderConnectionUI*>& aConnectionItems) const
{
	std::vector<ShaderConnectionUI*>::const_iterator itConnection = m_aInputPortConnectionItems.begin();
	for (; itConnection != m_aInputPortConnectionItems.end(); ++itConnection)
	{
		const ShaderConnectionUI* pConn = *itConnection;

		if (pConn)
		{
			aConnectionItems.insert(const_cast<ShaderConnectionUI*>(pConn));
		}
	}

	// as we only want fully-connected items, we only have to process the inputs...
}

////

ShaderConnectionUI::ShaderConnectionUI(ShaderNodeUI* pSrc) : QGraphicsLineItem(nullptr), m_pSrcNode(pSrc), m_pDstNode(nullptr),
	m_srcNodePortIndex(-1), m_dstNodePortIndex(-1)
{
//	setCacheMode(QGraphicsItem::NoCache);
	setFlag(ItemIsMovable);
	setFlags(QGraphicsItem::ItemIsSelectable);
	setFlag(ItemSendsGeometryChanges);
	setZValue(1.0);
}

ShaderConnectionUI::ShaderConnectionUI(ShaderNodeUI* pSrc, ShaderNodeUI* pDst) : QGraphicsLineItem(nullptr), m_pSrcNode(pSrc), m_pDstNode(pDst),
	m_srcNodePortIndex(-1), m_dstNodePortIndex(-1)
{
	setFlag(ItemIsMovable);
	setFlags(QGraphicsItem::ItemIsSelectable);
	setFlag(ItemSendsGeometryChanges);
	setZValue(1.0);
}

ShaderConnectionUI::~ShaderConnectionUI()
{

}

void ShaderConnectionUI::publicPrepareGeometryChange()
{
	prepareGeometryChange();
}

QRectF ShaderConnectionUI::boundingRect() const
{
	QRectF rect;

	if (m_pSrcNode && m_pDstNode)
	{
		// we're connected at both ends
		QPointF srcPoint = m_pSrcNode->getOutputPortPosition(m_srcNodePortIndex);

		QPointF dstPoint = m_pDstNode->getInputPortPosition(m_dstNodePortIndex);

		float width = std::abs(dstPoint.x() - srcPoint.x());
		float height = std::abs(dstPoint.y() - srcPoint.y());
		rect = QRectF(srcPoint, QSizeF(width, height));
		rect.adjust(-5, -5, 5, 5);
//		rect.normalized();
	}
	else
	{
		// we're only partially attached, and so one end is being dragged with the mouse...
		QPointF attachedPoint;
		if (m_pSrcNode && m_srcNodePortIndex != -1)
		{
			attachedPoint = m_pSrcNode->getOutputPortPosition(m_srcNodePortIndex);
		}
		else if (m_pDstNode && m_dstNodePortIndex != -1)
		{
			attachedPoint = m_pDstNode->getInputPortPosition(m_dstNodePortIndex);
		}

		QPointF floatingPoint = m_tempMousePos;

		rect = QRectF(attachedPoint, QSizeF(floatingPoint.x() - attachedPoint.x(), floatingPoint.y() - attachedPoint.y()));
		rect.adjust(-5, -5, 5, 5);
//		rect.normalized();
	}

	return rect;
}

bool ShaderConnectionUI::contains(const QPointF& point) const
{
	if (boundingRect().contains(point))
	{
		return true;
	}

	return false;
}

void ShaderConnectionUI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QPointF srcPoint;
	if (m_pSrcNode && m_srcNodePortIndex != -1)
	{
		srcPoint = m_pSrcNode->getOutputPortPosition(m_srcNodePortIndex);

		// draw black dot indicating it's connected
		painter->setBrush(QBrush(QColor(0, 0, 0)));
		painter->drawEllipse(srcPoint, 2, 2);
	}
	else
	{
		srcPoint = m_tempMousePos;
	}

	QPointF dstPoint;
	if (m_pDstNode && m_dstNodePortIndex != -1)
	{
		dstPoint = m_pDstNode->getInputPortPosition(m_dstNodePortIndex);

		// draw black dot indicating it's connected
		painter->setBrush(QBrush(QColor(0, 0, 0)));
		painter->drawEllipse(dstPoint, 2, 2);
	}
	else
	{
		dstPoint = m_tempMousePos;
	}

	QLineF line(srcPoint, dstPoint);

	if (qFuzzyCompare(line.length(), qreal(0.0)))
		return;

	if (isSelected())
	{
		painter->setBrush(Qt::blue);
		painter->setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	}
	else
	{
		painter->setBrush(Qt::black);
		painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	}

//	QPainterPath path(srcPoint);

//	path.cubicTo(srcPoint + QPointF(10.0f, -10.0f), dstPoint - QPointF(-10.0f, -10.0f), dstPoint);
//	painter->strokePath(path, QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

	painter->drawLine(line);
}

QVariant ShaderConnectionUI::itemChange(GraphicsItemChange change, const QVariant& value)
{
	return QGraphicsItem::itemChange(change, value);
}

bool ShaderConnectionUI::didHit(const QPointF& pos, SelectionInfo& selInfo, float& closestDistance) const
{
	if (boundingRect().contains(pos))
	{
		float distance = distanceFromLineEnds(pos);

		// see if we're close to the line now
		if (distance < closestDistance)
		{
			closestDistance = distance;

			// nasty, but...
			selInfo.pConnection = const_cast<ShaderConnectionUI*>(this);

			// this is duplicate work done already in distanceFromLineEnds(), but...

			QPointF srcDelta = m_pSrcNode->getOutputPortPosition(m_srcNodePortIndex) - pos;
			QPointF dstDelta = m_pDstNode->getInputPortPosition(m_dstNodePortIndex) - pos;

			float srcDistance = srcDelta.manhattanLength();
			float dstDistance = dstDelta.manhattanLength();

			if (srcDistance < dstDistance)
			{
				selInfo.wasSource = true;
			}

			return true;
		}
	}

	return false;
}

// returns distance to nearest end of line
float ShaderConnectionUI::distanceFromLineEnds(const QPointF& point) const
{
	// currently this is only designed to be used with fully-connected (at both ends)
	// connections - no temporary ones
	QPointF srcPoint;
	QPointF dstPoint;

	// these checks should be redundant as it should be done before this function gets called...

	if (m_pSrcNode && m_srcNodePortIndex != -1)
	{
		srcPoint = m_pSrcNode->getOutputPortPosition(m_srcNodePortIndex);
	}

	if (m_pDstNode && m_dstNodePortIndex != -1)
	{
		dstPoint = m_pDstNode->getInputPortPosition(m_dstNodePortIndex);
	}

	QPointF v = srcPoint - point;
	QPointF u = dstPoint - srcPoint;

	// TODO: strictly-speaking, this isn't correct, and should be the real length, but...
	float length = u.manhattanLength();

	float det = (-v.x() * u.x()) + (-v.y() * u.y());
	if (det < 0.0f || det > length)
	{
		u = dstPoint - point;
		return sqrtf(std::min(v.manhattanLength(), u.manhattanLength()));
	}

	det = u.x() * v.y() - u.y() * v.x();
	return sqrtf((det * det) / length);
}

} // namespace Imagine
