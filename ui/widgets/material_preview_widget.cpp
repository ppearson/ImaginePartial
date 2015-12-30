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

#include "material_preview_widget.h"

#include <QPainter>
#include <QMenu>
#include <QContextMenuEvent>

#include "objects/camera.h"
#include "objects/primitives/sphere.h"
#include "objects/primitives/cube.h"
#include "objects/primitives/plane.h"
#include "lights/point_light.h"
#include "lights/area_light.h"
#include "lights/physical_sky.h"

#include "materials/standard_material.h"
#include "textures/procedural_2d/checkerboard.h"

#include "raytracer/raytracer.h"
#include "image/output_image.h"

#include "global_context.h"

#include "utils/params.h"

MaterialPreviewRenderThread::MaterialPreviewRenderThread(SceneInterface* pScene, MaterialPreviewWidget* pWidget)
	: QThread(NULL), m_pScene(pScene), m_pImage(NULL), m_pRaytracer(NULL), m_pWidget(pWidget)
{
	unsigned int threads = GlobalContext::instance().getRenderThreads();
	threads /= 2;

	// we want progressive rendering
	m_pRaytracer = new Raytracer(*m_pScene, threads, false);
	m_pRaytracer->setPreview(true);

	m_wasCancelled = false;
}

MaterialPreviewRenderThread::~MaterialPreviewRenderThread()
{
	wait();

	if (m_pRaytracer)
	{
		m_pRaytracer = NULL;
		delete m_pRaytracer;
	}
}

void MaterialPreviewRenderThread::renderScene(OutputImage* pImage)
{
	m_pImage = pImage;

	// TODO: this still doesn't really work - concurrent renders from scrubbing sliders still happens...

	if (m_pRaytracer->isActive())
	{
		m_pRaytracer->terminate();
		wait();
	}

	start();
}

void MaterialPreviewRenderThread::renderSceneBlock(OutputImage* pImage)
{
	m_pImage = pImage;

	m_pImage->clearImage();

	m_pRaytracer->initialise(m_pImage, *m_pSettings);

	m_pRaytracer->setExtraChannels(0);
	m_pRaytracer->setAmbientColour(m_pScene->getAmbientColour());

	m_pRaytracer->renderScene(1.0f, m_pSettings);

	emit renderFinished();
}

void MaterialPreviewRenderThread::stop()
{
	if (m_pRaytracer)
	{
		m_pRaytracer->terminate();
	}
}

void MaterialPreviewRenderThread::run()
{
	m_pImage->clearImage();

	m_pRaytracer->initialise(m_pImage, *m_pSettings);

	m_pRaytracer->setExtraChannels(0);
	m_pRaytracer->setAmbientColour(m_pScene->getAmbientColour());

	m_pRaytracer->renderScene(1.0f, m_pSettings);

	if (!m_pRaytracer->wasCancelled())
	{
		emit renderFinished();
	}
	else
	{
		emit renderCancelled();
	}
}

/////

MaterialPreviewWidget::MaterialPreviewWidget(QWidget* parent) : QWidget(parent), SceneInterface(),
	m_pQImage(NULL), m_pRawImage(NULL), m_pDisplayImage(NULL), m_pCurrentMaterial(NULL), m_lastWidth(0), m_lastHeight(0)
{
	setupScene();

	m_pUpdateThread = new MaterialPreviewRenderThread(this, this);
	m_pUpdateThread->setParams(&m_renderSettings);
	m_pRefreshTimer = new QTimer(this);

	m_pUpdateThread->connect(m_pUpdateThread, SIGNAL(renderFinished()), this, SLOT(renderFinished()));
	m_pUpdateThread->connect(m_pUpdateThread, SIGNAL(renderCancelled()), this, SLOT(renderCancelled()));

	connect(m_pRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshOutput()));

	// stuff for menu

	m_actionObjectSphere = new QAction("Sphere", this);
	m_actionObjectCube = new QAction("Cube", this);

	m_actionObjectSphere->setCheckable(true);
	m_actionObjectCube->setCheckable(true);

	connect(m_actionObjectSphere, SIGNAL(triggered()), this, SLOT(menuObjectSphere()));
	connect(m_actionObjectCube, SIGNAL(triggered()), this, SLOT(menuObjectCube()));

	m_actionLightPoint = new QAction("Point Light", this);
	m_actionLightArea = new QAction("Area Light", this);
	m_actionLightDistant = new QAction("Distant Light", this);
	m_actionLightEnvironment = new QAction("Environment Light", this);

	m_actionLightPoint->setCheckable(true);
	m_actionLightArea->setCheckable(true);
	m_actionLightEnvironment->setCheckable(true);

	connect(m_actionLightPoint, SIGNAL(triggered()), this, SLOT(menuLightPoint()));
	connect(m_actionLightArea, SIGNAL(triggered()), this, SLOT(menuLightArea()));
	connect(m_actionLightEnvironment, SIGNAL(triggered()), this, SLOT(menuLightEnvironment()));

	m_actionCheckeredBackground = new QAction("Background", this);
	m_actionGlobalIllumination = new QAction("Global Illumination", this);

	m_actionCheckeredBackground->setCheckable(true);
	m_actionGlobalIllumination->setCheckable(true);

	connect(m_actionCheckeredBackground, SIGNAL(triggered()), this, SLOT(menuBackground()));
	connect(m_actionGlobalIllumination, SIGNAL(triggered()), this, SLOT(menuGlobalIllumination()));
}

MaterialPreviewWidget::~MaterialPreviewWidget()
{
	if (m_pRefreshTimer)
	{
		m_pRefreshTimer->stop();
	}

	if (m_pUpdateThread)
	{
		m_pUpdateThread->stop();
		m_pUpdateThread->quit();
	}

	if (m_pQImage)
		delete m_pQImage;

	if (m_pDisplayImage)
		delete m_pDisplayImage;

	if (m_pRawImage)
		delete m_pRawImage;

	clearAllObjects();

	if (m_pDefaultCamera)
		delete m_pDefaultCamera;

	if (m_pUpdateThread)
	{
		delete m_pUpdateThread;
		m_pUpdateThread = NULL;
	}

	if (m_pRefreshTimer)
	{
		delete m_pRefreshTimer;
		m_pRefreshTimer = NULL;
	}
}

QSize MaterialPreviewWidget::minimumSizeHint() const
{
	return QSize(100, 100);
}

QSize MaterialPreviewWidget::sizeHint() const
{
	return QSize(100, 100);
}

void MaterialPreviewWidget::setupScene()
{
	m_pDefaultCamera = new Camera();
	m_pDefaultCamera->setPosition(Vector(6.0f, 2.0f, 2.0f));
	m_pDefaultCamera->lookAt(Point(0.0f, 0.0f, 0.0f));

	setupObjectsAndLights();
}

void MaterialPreviewWidget::setupObjectsAndLights()
{
	m_lock.lock();

	clearAllObjects();

	Light* pLight = NULL;

	if (m_sceneSetup.lightType == eSinglePoint)
	{
		pLight = new PointLight();
		pLight->setPosition(Vector(6.0f, 5.0f, 2.0f));
		pLight->setIntensity(2.4f);
	}
	else if (m_sceneSetup.lightType == eSingleArea)
	{
		AreaLight* pAreaLight = new AreaLight();
		pAreaLight->setPosition(Vector(6.0f, 5.0f, 2.0f));
		pAreaLight->setIntensity(2.4f);
		pAreaLight->setVisible(true);
		pAreaLight->setDimensions(3.0f, 3.0f);

		pAreaLight->constructGeometry();

		pLight = static_cast<Light*>(pAreaLight);
	}
	else if (m_sceneSetup.lightType == eEnvironment)
	{
		PhysicalSky* pPSLight = new PhysicalSky();
		pPSLight->setIntensity(2.0);
		pPSLight->setHemiExtend(1);
		pPSLight->setVisible(true);
		pPSLight->setIntensityScales(0.03f, 1.0f);
		pPSLight->generateEnvironmentImage(0.0f);

		pPSLight->setRotation(Vector(0.0f, -178.0f, 0.0f));

		pLight = pPSLight;
	}

	m_aLights.push_back(pLight);

	m_pCurrentObject = NULL;
	if (m_sceneSetup.objectType == eSphere)
	{
		m_pCurrentObject = new Sphere(2.0f, 8, false);
		m_pCurrentObject->setPosition(Vector(0.0f, 0.0f, 0.0f));
		m_pCurrentObject->setRotation(Vector(0.0f, 180.0f, 0.0f));
	}
	else if (m_sceneSetup.objectType == eCube)
	{
		m_pCurrentObject = new Cube(1.3f);
		m_pCurrentObject->setFreeMaterial(false);
		m_pCurrentObject->setPosition(Vector(0.0f, 0.0f, 0.0f));

		m_pCurrentObject->constructGeometry();
	}

	m_aObjects.push_back(m_pCurrentObject);

	if (m_sceneSetup.backgroundCheckerboard)
	{
		StandardMaterial* pMaterial = new StandardMaterial();
		pMaterial->setDiffuseCheckerboard(2.0f);

		pMaterial->setSpecularColour(Colour3f(0.01f));

		Object* pBackplane1 = new Plane(8.0f, 8.0f);
		pBackplane1->constructGeometry();
		pBackplane1->setMaterial(pMaterial);
		pBackplane1->setPosition(Vector(-4.0f, -1.0f, -1.0f));
		pBackplane1->setRotation(Vector(0.0f, 0.0f, -90.0f));

		Object* pBackplane2 = new Plane(8.0f, 8.0f);
		pBackplane2->constructGeometry();
		pBackplane2->setMaterial(pMaterial);
		pBackplane2->setPosition(Vector(0.0f, -1.0f, -5.0f));
		pBackplane2->setRotation(Vector(90.0f, 0.0f, 0.0f));

		Object* pBackplane3 = new Plane(8.0f, 8.0f);
		pBackplane3->constructGeometry();
		pBackplane3->setMaterial(pMaterial);
		pBackplane3->setPosition(Vector(0.0f, -2.0f, -1.0f));
		pBackplane3->setRotation(Vector(0.0f, 0.0f, 0.0f));

		m_aObjects.push_back(pBackplane1);
		m_aObjects.push_back(pBackplane2);
		m_aObjects.push_back(pBackplane3);
	}

	m_lock.unlock();
}

void MaterialPreviewWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu* m = new QMenu(this);

	// set the state of the menus based on the scene setup

	m_actionObjectSphere->setChecked(m_sceneSetup.objectType == eSphere);
	m_actionObjectCube->setChecked(m_sceneSetup.objectType == eCube);

	m_actionLightPoint->setChecked(m_sceneSetup.lightType == eSinglePoint);
	m_actionLightArea->setChecked(m_sceneSetup.lightType == eSingleArea);
	m_actionLightEnvironment->setChecked(m_sceneSetup.lightType == eEnvironment);

	m_actionCheckeredBackground->setChecked(m_sceneSetup.backgroundCheckerboard);
	m_actionGlobalIllumination->setChecked(m_sceneSetup.globalIllumination);

	m->addAction(m_actionObjectSphere);
	m->addAction(m_actionObjectCube);
	m->addSeparator();

	m->addAction(m_actionLightPoint);
	m->addAction(m_actionLightArea);
	m->addAction(m_actionLightEnvironment);

	m->addSeparator();

	m->addAction(m_actionCheckeredBackground);
	m->addAction(m_actionGlobalIllumination);

	m->move(mapToGlobal(event->pos()));
	m->show();
}

void MaterialPreviewWidget::paintEvent(QPaintEvent* event)
{
	if (!m_pQImage)
		return;

	QPixmap pixmap = QPixmap::fromImage(*m_pQImage);

	QPainter painter(this);

	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	painter.drawPixmap(0.0f, 0.0f, pixmap);

	painter.end();
}

void MaterialPreviewWidget::updateRaytracer()
{
	unsigned int width = geometry().width();
	unsigned int height = geometry().height();

	Params settings;
	settings.add("width", width);
	settings.add("height", height);

	// for material preview with lazy textures...
	settings.add("useTextureCaching", true);

	settings.add("integrator", m_sceneSetup.globalIllumination ? 1 : 0);

	unsigned int outputImageFlags = COMPONENT_RGBA;

	if (m_sceneSetup.globalIllumination)
	{
		outputImageFlags |= COMPONENT_SAMPLES;

		settings.add("SamplesPerIteration", 16);
		settings.add("Iterations", 8);
		settings.add("progressive", true);
		settings.add("rbOverall", 3);
		settings.add("preview", true);
	}
	else
	{
		settings.add("rbOverall", 3);
		settings.add("antiAliasing", 2);
	}

	m_renderSettings = settings;

	if (m_pRawImage)
		delete m_pRawImage;

	m_pRawImage = new OutputImage(width, height, outputImageFlags);

	m_lastWidth = width;
	m_lastHeight = height;
}

void MaterialPreviewWidget::renderImage(Material* pMaterial)
{
	if (geometry().width() != m_lastWidth || geometry().height() != m_lastHeight)
		updateRaytracer();

	// only set material if valid material specified - otherwise, refresh with current one
	if (pMaterial)
	{
		m_pCurrentMaterial = pMaterial;
	}

	if (!m_pCurrentMaterial)
		return;

	m_aObjects[0]->setMaterial(m_pCurrentMaterial, false); // don't free existing material

	if (m_sceneSetup.globalIllumination)
	{
		m_pUpdateThread->renderScene(m_pRawImage);
		m_pRefreshTimer->start(500);
	}
	else
	{
		// no GI, so just block for the moment, as at least it will be stable...
		m_pUpdateThread->renderSceneBlock(m_pRawImage);
	}
}

void MaterialPreviewWidget::showImage(const OutputImage* pImage)
{
	// create a display copy of the image...

	OutputImage* pNewImage = new OutputImage(*pImage);
	if (m_pDisplayImage)
	{
		delete m_pDisplayImage;
		m_pDisplayImage = NULL;
	}

	m_pDisplayImage = pNewImage;

	if (m_sceneSetup.globalIllumination)
	{
		m_pDisplayImage->normaliseProgressive();
	}

	m_pDisplayImage->applyExposure(1.8f);
	m_pDisplayImage->clamp();

	unsigned int width = m_pDisplayImage->getWidth();
	unsigned int height = m_pDisplayImage->getHeight();

	if (m_pQImage)
		delete m_pQImage;

	m_pQImage = new QImage(width, height, QImage::Format_RGB32);

	QRgb value;

	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			const Colour4f& rawColour = m_pDisplayImage->colourAt(x, y);

			value = qRgb(rawColour.r * 255, rawColour.g * 255, rawColour.b * 255);
			m_pQImage->setPixel(x, y, value);
		}
	}

	update();
}

void MaterialPreviewWidget::cancelRender()
{
	m_pRefreshTimer->stop();

	// if it isn't a GI render (which will take a long time), don't stop the update thread,
	// as there's going to be another call to renderImage() which kicks things off again very soon...
	if (m_sceneSetup.globalIllumination)
	{
		// otherwise stop
		m_pUpdateThread->stop();
	}
}

void MaterialPreviewWidget::initCamera(bool picking)
{
	m_pRenderCamera = m_pDefaultCamera;
}

void MaterialPreviewWidget::refreshOutput()
{
	showImage(m_pRawImage);
}

void MaterialPreviewWidget::renderFinished()
{
	m_pRefreshTimer->stop();

	showImage(m_pRawImage);
}

void MaterialPreviewWidget::renderCancelled()
{
	m_pRefreshTimer->stop();
}

void MaterialPreviewWidget::menuObjectSphere()
{
	m_sceneSetup.objectType = eSphere;

	setupObjectsAndLights();
	updateRaytracer(); // if we change the light, we need to update raytracer, otherwise env light pointer gets cached

	renderImage(NULL);
}

void MaterialPreviewWidget::menuObjectCube()
{
	m_sceneSetup.objectType = eCube;

	setupObjectsAndLights();
	updateRaytracer(); // if we change the light, we need to update raytracer, otherwise env light pointer gets cached

	renderImage(NULL);
}

void MaterialPreviewWidget::menuLightPoint()
{
	m_sceneSetup.lightType = eSinglePoint;

	setupObjectsAndLights();
	updateRaytracer(); // if we change the light, we need to update raytracer, otherwise env light pointer gets cached

	renderImage(NULL);
}

void MaterialPreviewWidget::menuLightArea()
{
	m_sceneSetup.lightType = eSingleArea;

	setupObjectsAndLights();
	updateRaytracer(); // if we change the light, we need to update raytracer, otherwise env light pointer gets cached

	renderImage(NULL);
}

void MaterialPreviewWidget::menuLightEnvironment()
{
	m_sceneSetup.lightType = eEnvironment;

	setupObjectsAndLights();
	updateRaytracer(); // if we change the light, we need to update raytracer, otherwise env light pointer gets cached

	renderImage(NULL);
}

void MaterialPreviewWidget::menuBackground()
{
	m_sceneSetup.backgroundCheckerboard = !m_sceneSetup.backgroundCheckerboard;

	setupObjectsAndLights();
	updateRaytracer(); // if we change the light, we need to update raytracer, otherwise env light pointer gets cached

	renderImage(NULL);
}

void MaterialPreviewWidget::menuGlobalIllumination()
{
	m_sceneSetup.globalIllumination = !m_sceneSetup.globalIllumination;

	updateRaytracer();
	renderImage(NULL);
}
