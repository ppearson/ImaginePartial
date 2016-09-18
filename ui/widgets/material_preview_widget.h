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

#ifndef MATERIAL_PREVIEW_WIDGET_H
#define MATERIAL_PREVIEW_WIDGET_H

#include <QWidget>
#include <QThread>
#include <QTimer>

class Widget;
class OutputImage;
class Object;
class Material;
class Camera;
class QImage;

#include "scene_interface.h"
#include "raytracer/raytracer.h"

#include "utils/threads/mutex.h"

namespace Imagine
{

class MaterialPreviewWidget;

class MaterialPreviewRenderThread : public QThread
{
	Q_OBJECT
public:
	MaterialPreviewRenderThread(SceneInterface* pScene, MaterialPreviewWidget* pWidget);
	virtual ~MaterialPreviewRenderThread();

	// run in a thread...
	void renderScene(OutputImage* pImage);
	// modal block
	void renderSceneBlock(OutputImage* pImage);

	void setParams(Params* pParams) { m_pSettings = pParams; }

	void stop();

signals:
	void renderFinished();
	void renderCancelled();

public slots:

protected:
	void run();

protected:
	// we don't own any of these...
	SceneInterface*			m_pScene;
	OutputImage*			m_pImage;
	Raytracer*				m_pRaytracer;
	Params*					m_pSettings;
	MaterialPreviewWidget*	m_pWidget;

	bool					m_wasCancelled;
};

class MaterialPreviewWidget : public QWidget, public SceneInterface
{
	Q_OBJECT
public:
	MaterialPreviewWidget(QWidget* parent = 0);
	virtual ~MaterialPreviewWidget();

	enum PreviewObjectType
	{
		eSphere,
		eCube,
		ePlane
	};

	enum PreviewLightType
	{
		eSinglePoint,
		eSingleArea,
		eDistant,
		eEnvironment
	};

	struct SceneSetup
	{
		SceneSetup() : globalIllumination(false), backgroundCheckerboard(false), objectType(eSphere), lightType(eSinglePoint)
		{
		}

		bool				globalIllumination;
		bool				backgroundCheckerboard;
		PreviewObjectType	objectType;
		PreviewLightType	lightType;
	};

	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;

	void setupScene();
	void setupObjectsAndLights();

	virtual void paintEvent(QPaintEvent* event);
	virtual void contextMenuEvent(QContextMenuEvent* event);

	void updateRaytracer();

	void renderImage(Material* pMaterial);
	void showImage(const OutputImage* pImage);

	void cancelRender();

	virtual void initCamera(bool picking);

	virtual RenderBackgroundType getBackgroundType() const
	{
		return eBackgroundNone;
	}

	virtual const TextureParameters& getBackgroundTexture() const
	{
		return m_background;
	}

	virtual Colour3f getBackgroundColour() const { return Colour3f(); }
	virtual Colour3f getAmbientColour() const { return Colour3f(); }

public slots:
	void refreshOutput();
	void renderFinished();
	void renderCancelled();

	void menuObjectSphere();
	void menuObjectCube();

	void menuLightPoint();
	void menuLightArea();
	void menuLightEnvironment();

	void menuBackground();
	void menuGlobalIllumination();

protected:
	QImage*				m_pQImage;
	OutputImage*		m_pRawImage;
	OutputImage*		m_pDisplayImage;

	Material*			m_pCurrentMaterial;
	Object*				m_pCurrentObject;

	Params				m_renderSettings;

	//

	MaterialPreviewRenderThread*	m_pUpdateThread;
	QTimer*				m_pRefreshTimer;

	SceneSetup			m_sceneSetup;

	Mutex				m_lock;

	TextureParameters	m_background;

	unsigned int		m_lastWidth;
	unsigned int		m_lastHeight;

	//

	QAction*			m_actionObjectSphere;
	QAction*			m_actionObjectCube;

	QAction*			m_actionLightPoint;
	QAction*			m_actionLightArea;
	QAction*			m_actionLightDistant;
	QAction*			m_actionLightEnvironment;

	QAction*			m_actionCheckeredBackground;
	QAction*			m_actionGlobalIllumination;
};

} // namespace Imagine

#endif // MATERIAL_PREVIEW_WIDGET_H
