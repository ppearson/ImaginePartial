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

#ifndef HANDLE_H
#define HANDLE_H

class Cylinder;
class Cone;
class Cube;
class Torus;

class Material;
class BoundaryBox;
class Point;
class Normal;

class Handle
{
public:
	Handle();
	~Handle();

	static void initAxisHandleObjects();

	static void drawObjectTranslateAxisForDisplay();
	static void drawObjectTranslateAxisForSelection();
	static void drawObjectRotateAxisForDisplay();
	static void drawObjectRotateAxisForSelection();

	static void drawObjectRotateXAxis(float radius);
	static void drawObjectRotateYAxis(float radius);
	static void drawObjectRotateZAxis(float radius);

	static void drawBoundaryBox(const BoundaryBox& bb);

	static void drawFaceHandleForDisplay(const Point& position, const Normal& normal, const BoundaryBox& bb);
	static void drawFaceHandleForSelection(const Point& position, const Normal& normal, const BoundaryBox& bb);

protected:
	///// translate handles

	static Cylinder* pObjectXTranslateHandleLine;
	static Cylinder* pObjectYTranslateHandleLine;
	static Cylinder* pObjectZTranslateHandleLine;

	static Cone* pObjectXTranslateHandleHead;
	static Cone* pObjectYTranslateHandleHead;
	static Cone* pObjectZTranslateHandleHead;

	static Torus* pObjectXRotateHandleLoop;
	static Torus* pObjectYRotateHandleLoop;
	static Torus* pObjectZRotateHandleLoop;

	///// face handles

	static Cylinder* pFaceNormalTranslateHandleLine;
	static Cone* pFaceNormalTranslateHandleHead;
	static Cube* pFaceExtrude;

	static Cylinder* pFaceTranslateXLine;
	static Cone* pFaceTranslateXHead;
	static Cylinder* pFaceTranslateYLine;
	static Cone* pFaceTranslateYHead;

	static Cube* pFaceScaleX;
	static Cube* pFaceScaleY;
	static Cube* pFaceScaleUniform;
};

#endif // HANDLE_H
