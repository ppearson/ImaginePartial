/*
 Imagine
 Copyright 2011-2020 Peter Pearson.

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

#ifndef IMAGE_WIDGET_H
#define IMAGE_WIDGET_H

#include <QWidget>
#include <QAbstractScrollArea>
#include <QScrollBar>

#include <vector>
#include <queue>

#include "ui/render_view_update_thread.h"

namespace Imagine
{

class OutputImage;

class ImageWidget : public QAbstractScrollArea
{
	Q_OBJECT
public:
	ImageWidget(QWidget* parent = 0);
	virtual ~ImageWidget();

	enum DisplayChannel
	{
		eRGB,
		eR,
		eG,
		eB,
		eA
	};
	
	enum ToneMappingType
	{
		eNone,
		eGamma,
		eReinhard
	};

	inline int xOffset() const { return horizontalScrollBar()->value(); }
	inline int yOffset() const { return verticalScrollBar()->value(); }

	virtual void paintEvent(QPaintEvent* event);
	virtual void adjustScrollbars();
	virtual void wheelEvent(QWheelEvent* event);
	virtual void resizeEvent(QResizeEvent* e);
	virtual void keyPressEvent(QKeyEvent* event);

	virtual void contextMenuEvent(QContextMenuEvent* event);

	void setZoom(float zoomLevel);

	void setLastTiles(const std::queue<TileGrid>& lastTiles);

	void drawLastTiles(QPainter& painter);

	void showImage(const OutputImage& image, float gamma);
	void convertImageValues();
	void toggleDisplayImageEnum(DisplayChannel type);

	void saveImage(unsigned int channels, unsigned int flags);

	void renderFinished(float gamma);

public slots:
	void saveImageRGBA();
	void saveImageRGB();
	void saveImageRGBAFloat32();
	void saveImageRGBFloat32();
	void saveImageExtraAOVs();
	void saveImageExtraAOVsFullFloat();
	void saveImageDeep();

protected:
	QImage*				m_pQImage;
	OutputImage*		m_pRawImage;
	OutputImage*		m_pDisplayImage;

	QAction*			m_saveRGBAEvent;
	QAction*			m_saveRGBEvent;
	QAction*			m_saveRGBAFloat32Event;
	QAction*			m_saveRGBFloat32Event;
	
	QAction*			m_saveNormalsEvent;
	QAction*			m_saveNormalsFloat32Event;
	QAction*			m_saveDeepEvent;

	QTransform			m_transform;
	float				m_zoomLevel;
	QSize				m_workArea;
	QRectF				m_window;
	bool				m_gotDimensions;

	std::vector<TileGrid>	m_aLastTiles;

	DisplayChannel		m_displayChannel;
	float				m_gain;

	unsigned int		m_width;
	unsigned int		m_height;

	bool				m_haveNormalisedRawImage;
	
	ToneMappingType		m_toneMappingType;
};

} // namespace Imagine

#endif // IMAGE_WIDGET_H
