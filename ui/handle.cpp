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

#include "handle.h"

#include <QtOpenGL/QGLWidget>

#include "core/quaternion.h"

#include "opengl_ex.h"

#include "core/point.h"
#include "core/normal.h"

#include "utils/maths/maths.h"
#include "objects/primitives/cylinder.h"
#include "objects/primitives/cone.h"
#include "objects/primitives/cube.h"
#include "objects/primitives/torus.h"
#include "objects/primitives/plane.h"

#include "view_context.h"

namespace Imagine
{

Cylinder* Handle::pObjectXTranslateHandleLine = NULL;
Cylinder* Handle::pObjectYTranslateHandleLine = NULL;
Cylinder* Handle::pObjectZTranslateHandleLine = NULL;

Cone* Handle::pObjectXTranslateHandleHead = NULL;
Cone* Handle::pObjectYTranslateHandleHead = NULL;
Cone* Handle::pObjectZTranslateHandleHead = NULL;

Plane* Handle::pObjectXYTranslateHandlePlane = NULL;
Plane* Handle::pObjectXZTranslateHandlePlane = NULL;
Plane* Handle::pObjectYZTranslateHandlePlane = NULL;

Torus* Handle::pObjectXRotateHandleLoop = NULL;
Torus* Handle::pObjectYRotateHandleLoop = NULL;
Torus* Handle::pObjectZRotateHandleLoop = NULL;

Cylinder* Handle::pFaceNormalTranslateHandleLine = NULL;
Cone* Handle::pFaceNormalTranslateHandleHead = NULL;
Cube* Handle::pFaceExtrude = NULL;

Cylinder* Handle::pFaceTranslateXLine = NULL;
Cone* Handle::pFaceTranslateXHead = NULL;
Cylinder* Handle::pFaceTranslateYLine = NULL;
Cone* Handle::pFaceTranslateYHead = NULL;

Cube* Handle::pFaceScaleX = NULL;
Cube* Handle::pFaceScaleY = NULL;
Cube* Handle::pFaceScaleUniform = NULL;

static const float kObjectRotateHandleThickness = 0.02f;
static const unsigned int kObjectRotateHandleSegments = 36;
static unsigned int kObjectRotateHandleSides = 10;
const float kObjectRotateHandleRadius = 1.0f;
const float kFaceRotateHandleRadius = 0.45f;

Handle::Handle()
{
}

Handle::~Handle()
{
}

void Handle::initAxisHandleObjects()
{
	// assumes none of them have been created yet

	Handle::pObjectXTranslateHandleLine = new Cylinder(1.0f, 0.03f, 6, 1);
	Handle::pObjectYTranslateHandleLine = new Cylinder(1.0f, 0.03f, 6, 1);
	Handle::pObjectZTranslateHandleLine = new Cylinder(1.0f, 0.03f, 6, 1);

	Handle::pObjectXTranslateHandleHead = new Cone(0.2f, 0.07f, 8);
	Handle::pObjectYTranslateHandleHead = new Cone(0.2f, 0.07f, 8);
	Handle::pObjectZTranslateHandleHead = new Cone(0.2f, 0.07f, 8);

	Cylinder* pX = Handle::pObjectXTranslateHandleLine;
	Cylinder* pY = Handle::pObjectYTranslateHandleLine;
	Cylinder* pZ = Handle::pObjectZTranslateHandleLine;

	pX->setObjectID(eXTranslate);
	pX->setPosition(Point(0.6f, 0.0f, 0.0f));
	pX->rotate(0.0f, 0.0f, 90.0f);
	pX->constructGeometry();

	pY->setObjectID(eYTranslate);
	pY->setPosition(Point(0.0f, 0.6f, 0.0f));
	pY->constructGeometry();

	pZ->setObjectID(eZTranslate);
	pZ->setPosition(Point(0.0f, 0.0f, 0.6f));
	pZ->rotate(-90.0f, 0.0f, 0.0f);
	pZ->constructGeometry();
	
	static const float kPlaneHandleSize = 0.2f;
	
	Handle::pObjectXYTranslateHandlePlane = new Plane(kPlaneHandleSize, kPlaneHandleSize);
	Handle::pObjectXZTranslateHandlePlane = new Plane(kPlaneHandleSize, kPlaneHandleSize);
	Handle::pObjectYZTranslateHandlePlane = new Plane(kPlaneHandleSize, kPlaneHandleSize);
	
	Plane* pTXY = Handle::pObjectXYTranslateHandlePlane;
	
	pTXY->setObjectID(eXYTranslate);
	pTXY->setPosition(Vector(0.5f, 0.5f, 0.0f));
	pTXY->rotate(90.0f, 0.0f, 0.0f);
	pTXY->constructGeometry();
	
	Plane* pTXZ = Handle::pObjectXZTranslateHandlePlane;
	
	pTXZ->setObjectID(eXZTranslate);
	pTXZ->setPosition(Vector(0.5f, 0.0f, 0.5f));
	pTXZ->constructGeometry();
	
	Plane* pTYZ = Handle::pObjectYZTranslateHandlePlane;
	
	pTYZ->setObjectID(eYZTranslate);
	pTYZ->setPosition(Vector(0.0f, 0.5f, 0.5f));
	pTYZ->rotate(0.0f, 0.0f, 90.0f);
	pTYZ->constructGeometry();
	
	// rotate

	Cone *pXHead = Handle::pObjectXTranslateHandleHead;
	Cone *pYHead = Handle::pObjectYTranslateHandleHead;
	Cone *pZHead = Handle::pObjectZTranslateHandleHead;

	pXHead->setObjectID(eXTranslate);
	pXHead->setPosition(Point(1.1f, 0.0f, 0.0f));
	pXHead->rotate(0.0f, 0.0f, -90.0f);
	pXHead->constructGeometry();

	pYHead->setObjectID(eYTranslate);
	pYHead->setPosition(Point(0.0f, 1.1f, 0.0f));
	pYHead->constructGeometry();

	pZHead->setObjectID(eZTranslate);
	pZHead->setPosition(Point(0.0f, 0.0f, 1.1f));
	pZHead->rotate(90.0f, 0.0f, 0.0f);
	pZHead->constructGeometry();

	/// object rotate

	Torus* pXRotate = new Torus(kObjectRotateHandleThickness, kObjectRotateHandleRadius, kObjectRotateHandleSegments, kObjectRotateHandleSides);
	Torus* pYRotate = new Torus(kObjectRotateHandleThickness, kObjectRotateHandleRadius, kObjectRotateHandleSegments, kObjectRotateHandleSides);
	Torus* pZRotate = new Torus(kObjectRotateHandleThickness, kObjectRotateHandleRadius, kObjectRotateHandleSegments, kObjectRotateHandleSides);

	Handle::pObjectXRotateHandleLoop = pXRotate;
	Handle::pObjectYRotateHandleLoop = pYRotate;
	Handle::pObjectZRotateHandleLoop = pZRotate;

	pXRotate->setObjectID(eXRotate);
	pXRotate->rotate(0.0f, 0.0f, -90.0f);
	pXRotate->constructGeometry();

	pYRotate->setObjectID(eYRotate);
	pYRotate->rotate(0.0f, 0.0f, 0.0f);
	pYRotate->constructGeometry();

	pZRotate->setObjectID(eZRotate);
	pZRotate->rotate(90.0f, 0.0f, 0.0f);
	pZRotate->constructGeometry();

	/// face

	Handle::pFaceNormalTranslateHandleLine = new Cylinder(1.0f, 0.03f, 6, 1);
	Handle::pFaceNormalTranslateHandleHead = new Cone(0.2f, 0.07f, 8);
	Handle::pFaceExtrude = new Cube(0.05f);

	Cylinder* pFaceNormalLine = Handle::pFaceNormalTranslateHandleLine;
	pFaceNormalLine->setObjectID(eFaceNormalTranslate);
	pFaceNormalLine->setPosition(Point(0.0f, 0.6f, 0.0f));
	pFaceNormalLine->constructGeometry();

	Cone* pFaceNormalHead = Handle::pFaceNormalTranslateHandleHead;
	pFaceNormalHead->setObjectID(eFaceNormalTranslate);
	pFaceNormalHead->setPosition(Point(0.0f, 1.1f, 0.0f));
	pFaceNormalHead->constructGeometry();

	Cube* pFaceExtrude = Handle::pFaceExtrude;
	pFaceExtrude->setObjectID(eFaceExtrude);
	pFaceExtrude->setPosition(Point(0.0f, 1.4f, 0.0f));
	pFaceExtrude->constructGeometry();

	Handle::pFaceTranslateXLine = new Cylinder(1.0f, 0.03f, 6, 1);
	Handle::pFaceTranslateYLine = new Cylinder(1.0f, 0.03f, 6, 1);

	Cylinder* pFaceTranslateXLine = Handle::pFaceTranslateXLine;
	pFaceTranslateXLine->setObjectID(eFaceTranslateX);
	pFaceTranslateXLine->setRotation(Vector(0.0f, 0.0f, -90.0f));
	pFaceTranslateXLine->setPosition(Point(0.6f, 0.0f, 0.0f));
	pFaceTranslateXLine->constructGeometry();

	Cylinder* pFaceTranslateYLine = Handle::pFaceTranslateYLine;
	pFaceTranslateYLine->setObjectID(eFaceTranslateY);
	pFaceTranslateYLine->setRotation(Vector(90.0f, 0.0f, 0.0f));
	pFaceTranslateYLine->setPosition(Point(0.0f, 0.0f, 0.6f));
	pFaceTranslateYLine->constructGeometry();

	Handle::pFaceTranslateXHead = new Cone(0.2f, 0.07f, 8);
	Handle::pFaceTranslateYHead = new Cone(0.2f, 0.07f, 8);

	Cone* pFaceTranslateXHead = Handle::pFaceTranslateXHead;
	pFaceTranslateXHead->setObjectID(eFaceTranslateX);
	pFaceTranslateXHead->setRotation(Vector(0.0f, 0.0f, -90.0f));
	pFaceTranslateXHead->setPosition(Point(1.1f, 0.0f, 0.0f));
	pFaceTranslateXHead->constructGeometry();

	Cone* pFaceTranslateYHead = Handle::pFaceTranslateYHead;
	pFaceTranslateYHead->setObjectID(eFaceTranslateY);
	pFaceTranslateYHead->setRotation(Vector(90.0f, 0.0f, 0.0f));
	pFaceTranslateYHead->setPosition(Point(0.0f, 0.0f, 1.1f));
	pFaceTranslateYHead->constructGeometry();

	//

	Handle::pFaceScaleX = new Cube(0.05f);
	Handle::pFaceScaleY = new Cube(0.05f);
	Handle::pFaceScaleUniform = new Cube(0.05f);
	Cube* pFaceScaleX = Handle::pFaceScaleX;
	pFaceScaleX->setObjectID(eFaceScaleX);
	pFaceScaleX->setPosition(Point(1.4f, 0.0f, 0.0f));
	pFaceScaleX->setRotation(Vector(-90.0f, 0.0f, 0.0f));
	pFaceScaleX->constructGeometry();

	Cube* pFaceScaleY = Handle::pFaceScaleY;
	pFaceScaleY->setObjectID(eFaceScaleY);
	pFaceScaleY->setPosition(Point(0.0f, 0.0f, 1.4f));
	pFaceScaleY->constructGeometry();

	Cube* pFaceScaleUniform = Handle::pFaceScaleUniform;
	pFaceScaleUniform->setObjectID(eFaceScaleUniform);
	pFaceScaleUniform->setPosition(Point(1.4f, 0.0f, 1.4f));
	pFaceScaleUniform->constructGeometry();
}

void Handle::drawObjectTranslateAxisForDisplay()
{
	if (!Handle::pObjectXTranslateHandleLine)
		initAxisHandleObjects();

	Cone* pXHead = Handle::pObjectXTranslateHandleHead;
	Cone* pYHead = Handle::pObjectYTranslateHandleHead;
	Cone* pZHead = Handle::pObjectZTranslateHandleHead;
	
	Plane* pXYTranslatePlane = Handle::pObjectXYTranslateHandlePlane;
	Plane* pXZTranslatePlane = Handle::pObjectXZTranslateHandlePlane;
	Plane* pYZTranslatePlane = Handle::pObjectYZTranslateHandlePlane;
	

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	EditHandle editHandle = ViewContext::instance().getEditHandle();

	// if we've only got one, then the mouse must be down on one, so only draw that
	if (editHandle >= eXTranslate && editHandle <= eYZTranslate)
	{
		if (editHandle == eXTranslate)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
			OpenGLEx::drawLine(Point(0.0f, 0.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
			pXHead->drawForDisplay();
		}
		else if (editHandle == eYTranslate)
		{
			glColor3f(0.0f, 1.0f, 0.0f);
			OpenGLEx::drawLine(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
			pYHead->drawForDisplay();
		}
		else if (editHandle == eZTranslate)
		{
			glColor3f(0.0f, 0.0f, 1.0f);
			OpenGLEx::drawLine(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
			pZHead->drawForDisplay();
		}
		else if (editHandle == eXYTranslate)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
			pXYTranslatePlane->drawForDisplay();
		}
		else if (editHandle == eXZTranslate)
		{
			glColor3f(0.0f, 1.0f, 0.0f);
			pXZTranslatePlane->drawForDisplay();
		}
		else if (editHandle == eYZTranslate)
		{
			glColor3f(0.0f, 0.0f, 1.0f);
			pYZTranslatePlane->drawForDisplay();
		}
	}
	else
	{
		// draw them all
		glColor3f(1.0f, 0.0f, 0.0f);
		OpenGLEx::drawLine(Point(0.0f, 0.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		pXHead->drawForDisplay();
		
		pXYTranslatePlane->drawForDisplay();

		glColor3f(0.0f, 1.0f, 0.0f);
		OpenGLEx::drawLine(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
		pYHead->drawForDisplay();
		
		pXZTranslatePlane->drawForDisplay();
		

		glColor3f(0.0f, 0.0f, 1.0f);
		OpenGLEx::drawLine(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
		pZHead->drawForDisplay();
		
		pYZTranslatePlane->drawForDisplay();
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void Handle::drawObjectTranslateAxisForSelection()
{
	if (!Handle::pObjectXTranslateHandleLine)
		initAxisHandleObjects();

	Cylinder* pXLine = Handle::pObjectXTranslateHandleLine;
	Cylinder* pYLine = Handle::pObjectYTranslateHandleLine;
	Cylinder* pZLine = Handle::pObjectZTranslateHandleLine;

	Cone* pXHead = Handle::pObjectXTranslateHandleHead;
	Cone* pYHead = Handle::pObjectYTranslateHandleHead;
	Cone* pZHead = Handle::pObjectZTranslateHandleHead;
	
	Plane* pXYTranslatePlane = Handle::pObjectXYTranslateHandlePlane;
	Plane* pXZTranslatePlane = Handle::pObjectXZTranslateHandlePlane;
	Plane* pYZTranslatePlane = Handle::pObjectYZTranslateHandlePlane;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	pXLine->drawForSelection();
	pXHead->drawForSelection();
	pYLine->drawForSelection();
	pYHead->drawForSelection();
	pZLine->drawForSelection();
	pZHead->drawForSelection();
	
	pXYTranslatePlane->drawForSelection();
	pXZTranslatePlane->drawForSelection();
	pYZTranslatePlane->drawForSelection();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void Handle::drawObjectRotateAxisForDisplay()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glLineWidth(2.0f);

	EditHandle editHandle = ViewContext::instance().getEditHandle();

	// if we've only got one, then the mouse must be down on one, so only draw that
	if (editHandle >= eXRotate && editHandle <= eZRotate)
	{
		if (editHandle == eXRotate)
		{
			drawObjectRotateXAxis(kObjectRotateHandleRadius);
		}
		else if (editHandle == eYRotate)
		{
			drawObjectRotateYAxis(kObjectRotateHandleRadius);
		}
		else if (editHandle == eZRotate)
		{
			drawObjectRotateZAxis(kObjectRotateHandleRadius);
		}
	}
	else
	{
		drawObjectRotateXAxis(kObjectRotateHandleRadius);
		drawObjectRotateYAxis(kObjectRotateHandleRadius);
		drawObjectRotateZAxis(kObjectRotateHandleRadius);
	}

	glLineWidth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void Handle::drawObjectRotateAxisForSelection()
{
	if (!Handle::pObjectXTranslateHandleLine)
		initAxisHandleObjects();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	pObjectXRotateHandleLoop->drawForSelection();
	pObjectYRotateHandleLoop->drawForSelection();
	pObjectZRotateHandleLoop->drawForSelection();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void Handle::drawObjectRotateXAxis(float radius)
{
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	Point prev(0.0f, 0.0f, radius);
	for (float r = 0.0f; r <= 2.0f * kPI; r += 0.1f)
	{
		float z = radius * cos(r);
		float y = radius * sin(r);

		glVertex3f(prev.x, prev.y, prev.z);

		prev = Point(0.0f, y, z);
		glVertex3f(prev.x, prev.y, prev.z);
	}
	glVertex3f(0.0f, 0.0f, radius);
	glEnd();
}

void Handle::drawObjectRotateYAxis(float radius)
{
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	Point prev(radius, 0.0f, 0.0f);
	for (float r = 0.0f; r <= 2.0f * kPI; r += 0.1f)
	{
		float x = radius * cos(r);
		float z = radius * sin(r);

		glVertex3f(prev.x, prev.y, prev.z);

		prev = Point(x, 0.0f, z);
		glVertex3f(prev.x, prev.y, prev.z);
	}
	glVertex3f(radius, 0.0f, 0.0f);
	glEnd();
}

void Handle::drawObjectRotateZAxis(float radius)
{
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINE_LOOP);
	Point prev(radius, 0.0f, 0.0f);
	for (float r = 0.0f; r <= 2.0f * kPI; r += 0.1f)
	{
		float x = radius * cos(r);
		float y = radius * sin(r);

		glVertex3f(prev.x, prev.y, prev.z);

		prev = Point(x, y, 0.0f);
		glVertex3f(prev.x, prev.y, prev.z);
	}
	glVertex3f(radius, 0.0f, 0.0f);
	glEnd();
}

///

void Handle::drawBoundaryBox(const BoundaryBox& bb)
{
	const Point& min = bb.getMinimum();
	const Point& max = bb.getMaximum();

	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xAAAA);

	glBegin(GL_LINE_LOOP);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(min.x, max.y, min.z);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z);
		glVertex3f(min.x, min.y, max.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z);
		glVertex3f(min.x, max.y, min.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z);
	glEnd();

	glDisable(GL_LINE_STIPPLE);
}

void Handle::drawFaceHandleForDisplay(const Point& position, const Normal& normal, const BoundaryBox& bb)
{
	if (!Handle::pObjectXTranslateHandleLine)
		initAxisHandleObjects();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// draw bbox around face
//	Handle::drawBoundaryBox(bb);

	glColor3f(1.0f, 1.0f, 0.0f);

	glPushMatrix();

	Matrix4 m;
	m.setTranslation(position.x, position.y, position.z);

	Quaternion rotateQ = Quaternion::fromDirections(Normal(0.0f, 1.0f, 0.0f), normal);
	Matrix4 rotateM = rotateQ.toMatrix();
	m *= rotateM;

	GLfloat glM[16];
	m.copyToOpenGL(glM);

	glMultMatrixf(glM);

	Handle::pFaceNormalTranslateHandleHead->drawForDisplay();
	OpenGLEx::drawLine(Point(0.0f, 0.1f, 0.0f), Point(0.0f, 1.1f, 0.0f));
	Handle::pFaceExtrude->drawForDisplay();

	glColor3f(1.0f, 0.0f, 0.0f);
	Handle::pFaceTranslateXHead->drawForDisplay();
	OpenGLEx::drawLine(Point(0.1f, 0.0f, 0.0f), Point(1.1f, 0.0f, 0.0f));
	Handle::pFaceScaleX->drawForDisplay();
	glColor3f(0.0f, 1.0f, 0.0f);
	Handle::pFaceTranslateYHead->drawForDisplay();
	OpenGLEx::drawLine(Point(0.0f, 0.0f, 0.1f), Point(0.0f, 0.0f, 1.1f));
	Handle::pFaceScaleY->drawForDisplay();
	glColor3f(0.0f, 0.0f, 1.0f);
	Handle::pFaceScaleUniform->drawForDisplay();

	drawObjectRotateXAxis(kFaceRotateHandleRadius);
	drawObjectRotateYAxis(kFaceRotateHandleRadius);
	drawObjectRotateZAxis(kFaceRotateHandleRadius);

	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void Handle::drawFaceHandleForSelection(const Point& position, const Normal& normal, const BoundaryBox& bb)
{
	if (!Handle::pObjectXTranslateHandleLine)
		initAxisHandleObjects();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glPushMatrix();

	Matrix4 m;
	m.setTranslation(position.x, position.y, position.z);

	Quaternion rotateQ = Quaternion::fromDirections(Normal(0.0f, 1.0f, 0.0f), normal);
	Matrix4 rotateM = rotateQ.toMatrix();
	m *= rotateM;

	GLfloat glM[16];
	m.copyToOpenGL(glM);

	glMultMatrixf(glM);

	Handle::pFaceNormalTranslateHandleLine->drawForSelection();
	Handle::pFaceNormalTranslateHandleHead->drawForSelection();
	Handle::pFaceExtrude->drawForSelection();

	Handle::pFaceTranslateXHead->drawForSelection();
	Handle::pFaceTranslateYHead->drawForSelection();

	Handle::pFaceScaleX->drawForSelection();
	Handle::pFaceScaleY->drawForSelection();
	Handle::pFaceScaleUniform->drawForSelection();

	glPushName(eFaceRotateX);
	drawObjectRotateXAxis(kFaceRotateHandleRadius);
	glPopName();
	glPushName(eFaceRotateY);
	drawObjectRotateYAxis(kFaceRotateHandleRadius);
	glPopName();
	glPushName(eFaceRotateZ);
	drawObjectRotateZAxis(kFaceRotateHandleRadius);
	glPopName();

	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

} // namespace Imagine
