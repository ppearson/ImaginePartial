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

#include "image_widget.h"

#include <QPainter>
#include <QWheelEvent>
#include <QMenu>
#include <QFileDialog>

#include <algorithm>

#include "image/output_image.h"
#include "utils/file_helpers.h"

#include "io/image_writer.h"

namespace Imagine
{

ImageWidget::ImageWidget(QWidget *parent) : QAbstractScrollArea(parent), m_pQImage(NULL), m_pRawImage(NULL), m_pDisplayImage(NULL),
	m_zoomLevel(1.0f), m_gotDimensions(false), m_displayChannel(eRGB), m_gain(1.0f), m_haveNormalisedRawImage(false)
{
	adjustScrollbars();
	viewport()->update();

	m_saveEvent = new QAction("Save Image...", this);
	m_saveFullFloatEvent = new QAction("Save Image (float 32)...", this);
	m_saveNormalsEvent = new QAction("Save Image with extra AOVs...", this);
	m_saveNormalsFullFloatEvent = new QAction("Save Image with extra AOVs (float 32)...", this);

	m_saveDeepEvent = new QAction("Save Deep Image...", this);

	connect(m_saveEvent, SIGNAL(triggered()), this, SLOT(saveImage()));
	connect(m_saveFullFloatEvent, SIGNAL(triggered()), this, SLOT(saveImageFullFloat()));
	connect(m_saveNormalsEvent, SIGNAL(triggered()), this, SLOT(saveImageExtraAOVs()));
	connect(m_saveNormalsFullFloatEvent, SIGNAL(triggered()), this, SLOT(saveImageExtraAOVsFullFloat()));
	connect(m_saveDeepEvent, SIGNAL(triggered()), this, SLOT(saveImageDeep()));
}

ImageWidget::~ImageWidget()
{
	if (m_pQImage)
	{
		delete m_pQImage;
		m_pQImage = NULL;
	}

	if (m_pRawImage)
	{
		delete m_pRawImage;
		m_pRawImage = NULL;
	}

	if (m_pDisplayImage)
	{
		delete m_pDisplayImage;
		m_pDisplayImage = NULL;
	}
}

void ImageWidget::paintEvent(QPaintEvent* event)
{
	if (!m_pQImage)
		return;

	QPixmap pixmap = QPixmap::fromImage(*m_pQImage);

	QPainter painter(viewport());

	QSize viewSize = viewport()->size();

	if (m_workArea.width() < viewSize.width() && m_workArea.height() < viewSize.height())
	{
		// don't need scroll bars

		int widthDiff = viewSize.width() - m_workArea.width();
		int heightDiff = viewSize.height() - m_workArea.height();

		painter.translate(widthDiff / 2, heightDiff / 2);
	}
	else if (m_workArea.width() > viewSize.width() && m_workArea.height() > viewSize.height())
	{
		int y = yOffset();
		int x = xOffset();
		painter.translate(-x, -y);
	}
	else
	{
		// work out which one

		int x = 0;
		int y = 0;

		if (m_workArea.width() < viewSize.width())
		{
			x = (viewSize.width() - m_workArea.width()) / 2;
		}
		else
		{
			x = -xOffset();
		}

		if (m_workArea.height() < viewSize.height())
		{
			y = (viewSize.height() - m_workArea.height()) / 2;
		}
		else
		{
			y = -yOffset();
		}

		painter.translate(x, y);
	}

	painter.scale(m_zoomLevel, m_zoomLevel);

	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	painter.drawPixmap(1.0f, 1.0f, pixmap);

	drawLastTiles(painter);

	painter.end();
}

void ImageWidget::adjustScrollbars()
{
	if (!m_gotDimensions)
		return;

	m_workArea = QSize(m_window.width() * m_zoomLevel, m_window.height() * m_zoomLevel);
	m_transform  = QTransform(m_zoomLevel, 0.0f, 0.0f, m_zoomLevel, 0, 0);

	QSize viewSize = viewport()->size();

	verticalScrollBar()->setPageStep(viewSize.height());
	horizontalScrollBar()->setPageStep(viewSize.width());
	verticalScrollBar()->setRange(0, std::max(0, m_workArea.height() - viewSize.height()));
	horizontalScrollBar()->setRange(0, std::max(0, m_workArea.width() - viewSize.width()));

	viewport()->update();
}

void ImageWidget::wheelEvent(QWheelEvent* event)
{
	if (event->delta() < 1.0f)
		setZoom(m_transform.m11() - 0.05f);
	else
		setZoom(m_transform.m11() + 0.05f);
}

void ImageWidget::resizeEvent(QResizeEvent* e)
{
	adjustScrollbars();
}

void ImageWidget::keyPressEvent(QKeyEvent* event)
{
	if (event == QKeySequence::ZoomIn)
	{
		setZoom(m_transform.m11() + 0.1f);
		event->accept();
		return;
	}

	if (event == QKeySequence::ZoomOut)
	{
		setZoom(m_transform.m11() - 0.1f);
		event->accept();
		return;
	}

	bool redrawImage = false;

	if (event->key() == Qt::Key_A)
	{
		toggleDisplayImageEnum(eA);
		redrawImage = true;
	}

	if (event->key() == Qt::Key_R)
	{
		toggleDisplayImageEnum(eR);
		redrawImage = true;
	}

	if (event->key() == Qt::Key_G)
	{
		toggleDisplayImageEnum(eG);
		redrawImage = true;
	}

	if (event->key() == Qt::Key_B)
	{
		toggleDisplayImageEnum(eB);
		redrawImage = true;
	}

	if (event->modifiers() & Qt::AltModifier)
	{
		bool modified = false;
		if (event->key() == Qt::Key_Up)
		{
			m_gain += 0.05f;
			modified = true;
		}
		else if (event->key() == Qt::Key_Down)
		{
			m_gain -= 0.05f;
			modified = true;
		}
		else if (event->key() == Qt::Key_Right)
		{
			m_gain = 1.0f;
			modified = true;
		}

		if (modified)
		{
			redrawImage = true;
		}
	}

	if (redrawImage)
	{
		event->accept();
		convertImageValues();
		this->viewport()->repaint();
		return;
	}

	event->ignore();
}

void ImageWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu* m = new QMenu(this);
	m->addAction(m_saveEvent);
	m->addAction(m_saveFullFloatEvent);
	m->addSeparator();

	if (m_pRawImage->components() & COMPONENT_DEEP)
	{
		m->addAction(m_saveDeepEvent);
	}
	else if (m_pRawImage->components() & COMPONENT_NORMAL || m_pRawImage->components() & COMPONENT_WPP ||
			 m_pRawImage->components() & COMPONENT_DEPTH || m_pRawImage->components() & COMPONENT_SHADOWS)
	{
		m->addAction(m_saveNormalsEvent);
		m->addAction(m_saveNormalsFullFloatEvent);
	}

	m->move(mapToGlobal(event->pos()));
	m->show();
}

void ImageWidget::setZoom(float zoomLevel)
{
	if (zoomLevel < 0.33f || zoomLevel > 1.0f)
		return;

	m_zoomLevel = zoomLevel;

	adjustScrollbars();
}

void ImageWidget::setLastTiles(const std::queue<TileGrid>& lastTiles)
{
	m_aLastTiles.clear();

	std::queue<TileGrid> aLocalCopy = lastTiles;

	while (!aLocalCopy.empty())
	{
		TileGrid& tg = aLocalCopy.front();
		m_aLastTiles.push_back(tg);

		aLocalCopy.pop();
	}
}

void ImageWidget::drawLastTiles(QPainter& painter)
{
	QPen blackPen(Qt::black);
	painter.setPen(blackPen);
	std::vector<TileGrid>::iterator it = m_aLastTiles.begin();
	for (; it != m_aLastTiles.end(); ++it)
	{
		const TileGrid& tg = *it;

		painter.drawRect(tg.x, tg.y, tg.width + 1, tg.height + 1);
	}

	// now draw stippled white line on top
	QPen whitePen(Qt::white, 1, Qt::DotLine);
	painter.setPen(whitePen);

	it = m_aLastTiles.begin();
	for (; it != m_aLastTiles.end(); ++it)
	{
		const TileGrid& tg = *it;

		painter.drawRect(tg.x, tg.y, tg.width + 1, tg.height + 1);
	}
}

void ImageWidget::showImage(const OutputImage& image, float gamma)
{
	// explicitly copy the raw image
	if (m_pRawImage)
	{
		delete m_pRawImage;
		m_pRawImage = NULL;
	}

	m_pRawImage = new OutputImage(image);

	// only allocate m_pDisplayImage and m_pQImage first time through...
	// TODO: when we have slots, we're going to need to use a hash to control
	//       when to re-create these for different images

	if (!m_pDisplayImage)
	{
		// we haven't allocated yet, so create it first time around
		m_pDisplayImage = new OutputImage(*m_pRawImage);
	}
	else
	{
		// we already have it, so just copy the pixels
		m_pDisplayImage->copyFullImageContents(*m_pRawImage, COMPONENT_SAMPLES);
	}

	if (!m_haveNormalisedRawImage)
	{
		// the raw image hasn't been normalised yet (probably still rendering)
		// so do this to the display image copy
		m_pDisplayImage->normaliseProgressive();
		m_pDisplayImage->applyExposure(gamma);
	}

	m_width = image.getWidth();
	m_height = image.getHeight();

	m_window = QRectF(0.0f, 0.0f, m_width, m_height);
	m_gotDimensions = true;

	if (!m_pQImage)
	{
		m_pQImage = new QImage(m_width, m_height, QImage::Format_RGB32);
	}

	if (!m_pQImage)
		return;

	convertImageValues();

	adjustScrollbars();
}

void ImageWidget::convertImageValues()
{
	QRgb value;

	const float gainValue = m_gain * 255.0f;

	float rMult = gainValue * ((m_displayChannel == eR || m_displayChannel == eRGB) ? 1.0f : 0.0f);
	float gMult = gainValue * ((m_displayChannel == eG || m_displayChannel == eRGB) ? 1.0f : 0.0f);
	float bMult = gainValue * ((m_displayChannel == eB || m_displayChannel == eRGB) ? 1.0f : 0.0f);

	for (unsigned int y = 0; y < m_height; y++)
	{
		if (m_displayChannel == eA)
		{
			for (unsigned int x = 0; x < m_width; x++)
			{
				const Colour4f& rawColour = m_pDisplayImage->colourAt(x, y);

				float finalA = rawColour.a * gainValue;
				int a = std::min((int)(finalA), 255);

				value = qRgb(a, a, a);
				m_pQImage->setPixel(x, y, value);
			}
		}
		else if (m_displayChannel == eRGB)
		{
			for (unsigned int x = 0; x < m_width; x++)
			{
				const Colour4f& rawColour = m_pDisplayImage->colourAt(x, y);

				float finalR = rawColour.r * gainValue;
				float finalG = rawColour.g * gainValue;
				float finalB = rawColour.b * gainValue;

				int red = std::min((int)(finalR), 255);
				int green = std::min((int)(finalG), 255);
				int blue = std::min((int)(finalB), 255);

				value = qRgb(red, green, blue);
				m_pQImage->setPixel(x, y, value);
			}
		}
		else
		{
			// we're one (maybe two in the future?!) of the options
			for (unsigned int x = 0; x < m_width; x++)
			{
				const Colour4f& rawColour = m_pDisplayImage->colourAt(x, y);

				float finalR = rawColour.r * rMult;
				float finalG = rawColour.g * gMult;
				float finalB = rawColour.b * bMult;

				// we can just max to get the single value we want for greyscale...

				float singleValue = std::max(finalR, std::max(finalG, finalB));

				int grey = std::min((int)(singleValue), 255);

				value = qRgb(grey, grey, grey);
				m_pQImage->setPixel(x, y, value);
			}
		}
	}
}

void ImageWidget::toggleDisplayImageEnum(DisplayChannel type)
{
	if (m_displayChannel == type)
		m_displayChannel = eRGB;
	else
		m_displayChannel = type;
}

void ImageWidget::saveImage(unsigned int channels, unsigned int flags)
{
	QFileDialog dialog(parentWidget(), tr("Save Image"), QDir::homePath());
	std::string filenameFilter = FileIORegistry::instance().getQtFileBrowserFilterForRegisteredImageWriters();
	std::string fullFilter = "Image files (" + filenameFilter + ")";
	dialog.setNameFilter(fullFilter.c_str());
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setViewMode(QFileDialog::Detail);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix(tr("png"));
	dialog.setConfirmOverwrite(true);

	if (!dialog.exec())
		 return;

	QString fileName = dialog.selectedFiles()[0];
	std::string path = fileName.toStdString();

	std::string extension = FileHelpers::getFileExtension(path);

	ImageWriter* pWriter = FileIORegistry::instance().createImageWriterForExtension(extension);
	if (!pWriter)
	{
		// file extension not recognised
		return;
	}

	pWriter->writeImage(path, *m_pRawImage, channels, flags);

	delete pWriter;
	pWriter = NULL;
}

void ImageWidget::renderFinished(float gamma)
{
	m_haveNormalisedRawImage = true;

	m_pRawImage->normaliseProgressive();
	m_pRawImage->applyExposure(gamma);
}

void ImageWidget::saveImage()
{
	saveImage(ImageWriter::RGBA, 0);
}

void ImageWidget::saveImageFullFloat()
{
	saveImage(ImageWriter::RGBA, ImageWriter::FLOAT32);
}

void ImageWidget::saveImageExtraAOVs()
{
	saveImage(ImageWriter::ALL | ImageWriter::NORMALS | ImageWriter::WPP | ImageWriter::SHADOWS, 0);
}

void ImageWidget::saveImageExtraAOVsFullFloat()
{
	saveImage(ImageWriter::ALL | ImageWriter::NORMALS | ImageWriter::WPP | ImageWriter::SHADOWS, ImageWriter::FLOAT32);
}

void ImageWidget::saveImageDeep()
{
	saveImage(ImageWriter::DEEP, 0);
}

} // namespace Imagine
