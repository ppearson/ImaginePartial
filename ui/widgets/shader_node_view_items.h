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

#ifndef SHADER_NODE_VIEW_ITEMS_H
#define SHADER_NODE_VIEW_ITEMS_H

#include <string>
#include <set>

#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QWidget>

namespace Imagine
{

class ShaderNode;

class ShaderConnectionUI;

// graphical wrapper of ShaderNode item for GUI purposes
class ShaderNodeUI : public QGraphicsItem
{
public:
	ShaderNodeUI(const std::string& name, ShaderNode* pActualNode = NULL);
	ShaderNodeUI(ShaderNode* pActualNode);

	virtual ~ShaderNodeUI();

	void initStateFromShaderNode();

	ShaderNode* getActualNode();

	void setCustomPos(int x, int y);
	void movePos(int x, int y);

	virtual QRectF boundingRect() const;

	virtual bool contains(const QPointF& point) const;

	virtual bool containsSceneSpace(const QPointF& point) const;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL);

	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

	void publicPrepareGeometryChange();

	QPointF getInputPortPosition(unsigned int portIndex) const;
	QPointF getOutputPortPosition(unsigned int portIndex) const;

	struct SelectionInfo
	{
		SelectionInfo() : pNode(NULL), selectionType(eSelectionNone), portIndex(-1)
		{

		}

		enum Type
		{
			eSelectionNone,
			eSelectionNode,
			eSelectionInputPort,
			eSelectionOutputPort,
			eSelectionResize
		};

		ShaderNodeUI*	pNode;
		Type			selectionType;
		unsigned int	portIndex;
	};

	bool doesContain(const QPointF& point, SelectionInfo& selInfo) const;

	void setInputPortConnectionItem(unsigned int index, ShaderConnectionUI* pItem);
	ShaderConnectionUI* getInputPortConnectionItem(unsigned int index);


	unsigned int getOutputPortConnectionCount(unsigned int portIndex) const;
	void addOutputPortConnectionItem(unsigned int portIndex, ShaderConnectionUI* pItem);
	void removeOutputPortConnectionItem(unsigned int portIndex, ShaderConnectionUI* pItem);
	ShaderConnectionUI* getSingleOutputPortConnectionItem(unsigned int index);

	void getConnectionItems(std::set<ShaderConnectionUI*>& aConnectionItems) const;

protected:
	// we don't own this
	ShaderNode*			m_pActualNode;

	QSizeF				m_nodeSize;

	std::string			m_name; // eventually we shouldn't need this, as it will be pulled from above...

	// these are just pointers to existing connections - they're not owned by this class...
	// it's up to ShaderNodeViewWidget to keep them up-to-date...
	std::vector<ShaderConnectionUI*>	m_aInputPortConnectionItems;

	// TODO: do we even need this now?
	std::vector< std::vector<ShaderConnectionUI*> >	m_aOutputPortConnectionItems;
};


class ShaderConnectionUI : public QGraphicsLineItem
{
public:
	// output
	ShaderConnectionUI(ShaderNodeUI* pSrc);

	ShaderConnectionUI(ShaderNodeUI* pSrc, ShaderNodeUI* pDst);

	virtual ~ShaderConnectionUI();

	void setSourceNode(ShaderNodeUI* pSrcNode) { m_pSrcNode = pSrcNode; }
	void setDestinationNode(ShaderNodeUI* pDstNode) { m_pDstNode = pDstNode; }

	ShaderNodeUI* getSourceNode() const { return m_pSrcNode; }
	ShaderNodeUI* getDestinationNode() const { return m_pDstNode; }

	void setSourceNodePortIndex(unsigned int portIndex) { m_srcNodePortIndex = portIndex; }
	void setDestinationNodePortIndex(unsigned int portIndex) { m_dstNodePortIndex = portIndex; }

	unsigned int getSourceNodePortIndex() const { return m_srcNodePortIndex; }
	unsigned int getDestinationNodePortIndex() const { return m_dstNodePortIndex; }

	void publicPrepareGeometryChange();

	virtual QRectF boundingRect() const;

	virtual bool contains(const QPointF& point) const;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL);

	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

	void setTempMousePos(QPointF pos) { m_tempMousePos = pos; }

	struct SelectionInfo
	{
		SelectionInfo() : pConnection(NULL), wasSource(false)
		{

		}


		bool					wasSource; // whether hit point was closest to source node or destination node
		ShaderConnectionUI*		pConnection;
	};

	bool didHit(const QPointF& pos, SelectionInfo& selInfo, float& closestDistance) const;

	// TODO: this is duplicate of stuff in FlowNodeViewItems...
	float distanceFromLineEnds(const QPointF& point) const;

protected:

protected:
	// we don't own these...
	ShaderNodeUI*		m_pSrcNode;
	ShaderNodeUI*		m_pDstNode;

	// connections always go src -> dst left to right, so srcNode connections are output ports, dstNode connections
	// are input ports...

	unsigned int		m_srcNodePortIndex; // output port index on source node
	unsigned int		m_dstNodePortIndex; // input port index on destination node

	// this isn't great, but...
	QPointF			m_tempMousePos;
};

} // namespace Imagine

#endif // SHADER_NODE_VIEW_ITEMS_H
