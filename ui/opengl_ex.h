/*
 Imagine
 Copyright 2011-2014 Peter Pearson.

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

#ifndef OPENGL_EX_H
#define OPENGL_EX_H

#include "image/image_colour3b.h"

#include "utils/threads/thread_pool.h"

namespace Imagine
{

class Point;
class Texture;

class OpenGLExTextSampler
{
public:
	OpenGLExTextSampler(const ImageColour3b* pImage, float uScaleF = 1.0f, float vScaleF = 1.0f);

	Colour3b getColourBlend(float u, float v) const;

protected:
	const ImageColour3b*	m_pImage;

	unsigned int	m_width;
	unsigned int	m_height;

	float			m_floatWidth;
	float			m_floatHeight;

	bool			m_scale;
	float			m_UScaleFactor;
	float			m_VScaleFactor;
};

class OpenGLEx
{
public:
	static void drawSquareOnFlatPlane(const Point& point, float radius);

	static ImageColour3b* makeImageColour3bFromTexture(Texture* pTexture, unsigned int width, unsigned int height);

	static bool resampleByteImageForTexture(Texture* pTexture, unsigned int maxGLTextureSize);

	static bool generateTexture(Texture* pTexture, unsigned int& textureID);
	static void deleteTexture(unsigned int textureID);

	static void drawLine(const Point& point0, const Point& point1);

	// GLU replacement functions
	static void perspective(double fov, double aspectRatio, double near, double far);
	static void pickMatrix(double x, double y, double width, double height, int* viewport);

	static bool unproject(float x, float y, float z, double* pModelView, double* pProjection, int32_t* viewport, Point& objectCoordinates);
	static bool unprojectCombined(float x, float y, double* pModelView, double* pProjection, int32_t* viewport,
								  Point& objectCoordinatesNear, Point& objectCoordinatesFar);

	static void mulMatrixVec4f(const float matrix[16], const float vec4[4], float out[4]);
	static void mulMatrices(const float matrixA[16], const float matrixB[16], float matrixOut[16]);
	static void invertMatrix(const float matrixIn[16], float matrixOut[16]);
};

class ObjectGLTextureBakerTask : public ThreadPoolTask
{
public:
	ObjectGLTextureBakerTask(ImageColour3b* pTargetImage, Texture* pTexture, unsigned int startLine, unsigned int endLine) : m_pTargetImage(pTargetImage), m_pTexture(pTexture),
		m_startLine(startLine), m_endLine(endLine)
	{
	}

	virtual ~ObjectGLTextureBakerTask() { }

	ImageColour3b* getTargetImage() { return m_pTargetImage; }
	Texture* getTexture() { return m_pTexture; }

	unsigned int getStartLine() const { return m_startLine; }
	unsigned int getEndLine() const { return m_endLine; }

protected:
	ImageColour3b*	m_pTargetImage;
	Texture*		m_pTexture;

	unsigned int	m_startLine;
	unsigned int	m_endLine;
};

class ObjectGLTextureBakerWorker : public ThreadPool
{
public:
	ObjectGLTextureBakerWorker(unsigned int threads);
	virtual ~ObjectGLTextureBakerWorker() { }

	void bakeTextureToImage(ImageColour3b* pTargetImage, Texture* pTexture);

protected:
	virtual bool doTask(ThreadPoolTask* pTask, unsigned int threadID);

};


} // namespace Imagine

#endif // OPENGL_EX_H
