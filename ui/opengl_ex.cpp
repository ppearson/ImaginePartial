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

#include "opengl_ex.h"

#include <QtOpenGL/QGLWidget>

#include "raytracer/raytracer_common.h"

#include "core/point.h"
#include "textures/texture.h"

#include "textures/image/image_texture_3b.h"

#include "core/matrix4.h"

#include "utils/maths/maths.h"

#include "colour/colour_space.h"

#include "global_context.h"

namespace Imagine
{

void OpenGLEx::drawSquareOnFlatPlane(const Point& point, float radius)
{
	Point points[4];
	points[0] = Point(point.x - radius, point.y, point.z - radius);
	points[1] = Point(point.x + radius, point.y, point.z - radius);
	points[2] = Point(point.x + radius, point.y, point.z + radius);
	points[3] = Point(point.x - radius, point.y, point.z + radius);

	for (unsigned int i = 0; i < 4; i++)
		glVertex3f(points[i].x, points[i].y, points[i].z);
}

OpenGLExTextSampler::OpenGLExTextSampler(const ImageColour3b* pImage, float uScaleF, float vScaleF) :
	m_pImage(pImage), m_scale(false)
{
	m_width = pImage->getWidth();
	m_height = pImage->getHeight();

	m_floatWidth = (float)(m_width - 1);
	m_floatHeight = (float)(m_height - 1);

	// make a note of the max source resolution we've got, so we can accurately decide what image sizes to generate
	// for OpenGL
//	m_maxSourceResolution = std::max(width, height);

	if (uScaleF != 1.0f || vScaleF != 1.0f)
	{
		m_scale = true;
		m_UScaleFactor = 1.0f / uScaleF;
		m_VScaleFactor = 1.0f / vScaleF;
	}
}

Colour3b OpenGLExTextSampler::getColourBlend(float u, float v) const
{
	Colour3b colour;

	if (m_pImage)
	{
		if (m_scale)
		{
			u *= m_UScaleFactor;
			v *= m_VScaleFactor;
		}

		u = uvWrap(u);
		v = uvWrap(v);

		unsigned int x = (unsigned int)(m_floatWidth * u);
		unsigned int y = (unsigned int)(m_floatHeight * v);
		unsigned int xOther = (x == m_width - 1) ? x - 1 : x + 1;
		unsigned int yOther = (y == m_height - 1) ? y - 1 : y + 1;

		float alpha = u * m_floatWidth - x;
		float beta = v * m_floatHeight - y;

		Colour3b fColour0 = m_pImage->colour3bAt(x, y);
		Colour3b fColour1 = m_pImage->colour3bAt(xOther, y);
		Colour3b fColour2 = m_pImage->colour3bAt(xOther, yOther);
		Colour3b fColour3 = m_pImage->colour3bAt(x, yOther);

		Colour3b item0 = (fColour3 * beta) + fColour0 * (1.0f - beta);
		Colour3b item1 = (fColour2 * beta) + fColour1 * (1.0f - beta);

		Colour3b blendedColour = (item1 * alpha + (item0 * (1.0f - alpha)));
		return blendedColour;
	}

	return colour;
}

// resize any existing image to the format we want for OpenGL preview
ImageColour3b* OpenGLEx::makeImageColour3bFromTexture(Texture* pTexture, unsigned int width, unsigned int height)
{
	if (!pTexture || pTexture->disableGLPreviews())
		return NULL;

	ImageColour3b* pImage = new ImageColour3b();
	if (!pImage)
		return NULL;

	if (!pImage->initialise(width, height))
		return NULL;

	float glWidth = 1.0f / (float)width;
	float glHeight = 1.0f / (float)height;

	bool isColour3bAlready = (pTexture->getImageTypeFlags() & (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_BYTE));
	isColour3bAlready = false;
	if (isColour3bAlready)
	{
		// TODO: version without any unnecessary colour conversion
	}
	else
	{
		unsigned int threads = GlobalContext::instance().getWorkerThreads();
		if (threads == 0)
		{
			threads = 1;
		}

		bool doThreads = true;
		if (doThreads && threads > 1)
		{
			ObjectGLTextureBakerWorker worker(threads);
			worker.bakeTextureToImage(pImage, pTexture);
		}
		else
		{
			HitResult tempResult;
			for (unsigned int v = 0; v < height; v++)
			{
				tempResult.uv.v = glHeight * (float)v;

				Colour3b* pByteBuffer = pImage->colour3bRowPtr(v);
				for (unsigned int u = 0; u < width; u++)
				{
					tempResult.uv.u = glWidth * (float)u;

					Colour3f colour = pTexture->getColourBlend(tempResult, 0);

					ColourSpace::convertLinearToSRGBFast(colour);

					colour.clamp();

					unsigned char r = colour.r * 255;
					unsigned char g = colour.g * 255;
					unsigned char b = colour.b * 255;

					Colour3b newColour(r, g, b);
					*pByteBuffer = newColour;

					pByteBuffer++;
				}
			}
		}
	}

	return pImage;
}

// resample an existing ByteImage to the size the OpenGL view needs
bool OpenGLEx::resampleByteImageForTexture(Texture* pTexture, unsigned int maxGLTextureSize)
{
	const ImageColour3b* pImage = pTexture->getTempDiffuseOpenGLImage();

	bool isByteImage = (pTexture->getImageTypeFlags() & (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_BYTE)) ==
			(Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_BYTE);
	if (!pImage && isByteImage)
	{
		const ImageTexture3b* pTypeImage = static_cast<const ImageTexture3b*>(pTexture);
		pImage = pTypeImage->getRawImagePointer();
	}

	if (!pImage || !pImage->isValidImage()) // TODO: this second check shouldn't be needed - why is pImage actually non-NULL but with bad data?
		return false;

	unsigned int textureSourceMax = pTexture->getMaxSourceResolution();

	unsigned int textureSizeToUse;
	// 0 means infinite, so use the max resolution possible for this
	if (textureSourceMax == 0 || textureSourceMax >= maxGLTextureSize)
	{
		textureSizeToUse = maxGLTextureSize;
	}
	else if (textureSourceMax >= maxGLTextureSize / 2)
	{
		textureSizeToUse = maxGLTextureSize / 2;
	}
	else if (textureSourceMax >= maxGLTextureSize / 4)
	{
		textureSizeToUse = maxGLTextureSize / 4;
	}
	else
	{
		textureSizeToUse = maxGLTextureSize / 8;
	}

	const unsigned int width = textureSizeToUse;
	const unsigned int height = textureSizeToUse;

	// if it's already that size, we don't need to do anything...
	if (width == pImage->getWidth() && height == pImage->getHeight())
		return true;

	OpenGLExTextSampler sampler(pImage, pTexture->getScaleU(), pTexture->getScaleV());

	float glWidth = 1.0f / (float)width;
	float glHeight = 1.0f / (float)height;

	ImageColour3b* pResampledImage = new ImageColour3b();
	if (!pResampledImage || !pResampledImage->initialise(width, height))
	{
		return false;
	}

	for (unsigned int v = 0; v < height; v++)
	{
		float y = glHeight * (float)v;

		Colour3b* pResampledRow = pResampledImage->colour3bRowPtr(v);
		for (unsigned int u = 0; u < width; u++)
		{
			float x = glWidth * (float)u;

			Colour3b colour = sampler.getColourBlend(x, y);

			*pResampledRow = colour;

			pResampledRow++;
		}
	}

	pTexture->setTempDiffuseOpenGLImage(pResampledImage);

	return true;
}

bool OpenGLEx::generateTexture(Texture* pTexture, unsigned int& textureID)
{
	GlobalContext& gc = GlobalContext::instance();

	unsigned int maxTextureSize = gc.getMax3DViewTextureSize();

	bool createdThisTime = false;
	const ImageColour3b* pImage = pTexture->getTempDiffuseOpenGLImage();

	// see if we are a Byte image already

	unsigned int expectedFlags = (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_BYTE);

	bool isByteImage = (pTexture->getImageTypeFlags() & (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_BYTE)) == expectedFlags;
	if (!pImage && isByteImage)
	{
		const ImageTexture3b* pTypeImage = static_cast<const ImageTexture3b*>(pTexture);
		pImage = pTypeImage->getRawImagePointer();
	}

	// if we haven't got one already (code path from Material Editor), create one...
	if (!pImage)
	{
		createdThisTime = true;

		unsigned int textureSourceMax = pTexture->getMaxSourceResolution();

		unsigned int textureSizeToUse;
		// 0 means infinite, so use the max resolution possible for this
		if (textureSourceMax == 0 || textureSourceMax >= maxTextureSize)
		{
			textureSizeToUse = maxTextureSize;
		}
		else if (textureSourceMax >= maxTextureSize / 2)
		{
			textureSizeToUse = maxTextureSize / 2;
		}
		else if (textureSourceMax >= maxTextureSize / 4)
		{
			textureSizeToUse = maxTextureSize / 4;
		}
		else
		{
			textureSizeToUse = maxTextureSize / 8;
		}

		unsigned int width = textureSizeToUse;
		unsigned int height = textureSizeToUse;

		pImage = makeImageColour3bFromTexture(pTexture, width, height);
	}
	else
	{
		// otherwise, make sure that the dimensions of the existing texture are good for OpenGL...
		// if we're loading a file, MaterialTextureLoadGLWorker will have done this for us, but when using the Material
		// Editor, that code path isn't used, so we need to do this for that case
		if (resampleByteImageForTexture(pTexture, maxTextureSize))
		{
			// see if there's a new temp image - if so, the image was resized, so use that instead
			const ImageColour3b* pResizedTempImage = pTexture->getTempDiffuseOpenGLImage();
			if (pResizedTempImage)
				pImage = pTexture->getTempDiffuseOpenGLImage();
		}
		else
		{
			// for whatever reason, we couldn't create a new resized image for OpenGL, so delete the original image
			// and set it to NULL
			pTexture->setTempDiffuseOpenGLImage(NULL);

			pImage = NULL;
		}
	}

	if (!pImage)
		return false;

	// allocate the GL texture
	GLuint texture;
	glGenTextures(1, &texture);
	// bind the texture immediately, so that it's set as a 2D texture
	glBindTexture(GL_TEXTURE_2D, texture);

	if (gc.shouldCreate3DViewMipmaps())
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		
//		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pImage->getWidth(), pImage->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, pImage->colour3bRowPtr(0)->getStartPointer());

	pTexture->setTempDiffuseOpenGLImage(NULL); // delete the cached image...

	textureID = texture;

	glBindTexture(GL_TEXTURE_2D, 0); // unbind it again now we've created it, so it isn't drawn until needed.

	if (createdThisTime)
		delete pImage;

	return true;
}

void OpenGLEx::deleteTexture(unsigned int textureID)
{
	glDeleteTextures(1, &textureID);
}

void OpenGLEx::drawLine(const Point& point0, const Point& point1)
{
	glBegin(GL_LINES);

	glVertex3f(point0.x, point0.y, point0.z);
	glVertex3f(point1.x, point1.y, point1.z);

	glEnd();
}

void OpenGLEx::perspective(double fov, double aspectRatio, double near, double far)
{
	double height = tan(radians(fov * 0.5)) * near;

	double width = height * aspectRatio;

	glFrustum(-width, width, -height, height, near, far);
}

void OpenGLEx::pickMatrix(double x, double y, double width, double height, int* viewport)
{
	float sx = viewport[2] / width;
	float sy = viewport[3] / height;
	float tx = (viewport[2] + 2.0 * (viewport[0] - x)) / width;
	float ty = (viewport[3] + 2.0 * (viewport[1] - y)) / height;

	float matrix[16];
	memset(&matrix, 0, 16 * sizeof(float));

	matrix[0] = sx;
	matrix[5] = sy;
	matrix[10] = 1.0f;
	matrix[12] = tx;
	matrix[13] = ty;
	matrix[15] = 1.0f;

	glMultMatrixf(matrix);
}

bool OpenGLEx::unproject(float x, float y, float z, double* pModelView, double* pProjection, int32_t* viewport, Point& objectCoordinates)
{
	Matrix4 model;
	model.setFromArray(pModelView, true);

	Matrix4 projection;
	projection.setFromArray(pProjection, true);

	Matrix4 final = projection;
	final *= model;

	final = final.inverse();

	Point position;

	// transform to normalised coordinates (-1 to 1)

	position.x = (x - (float)viewport[0]) / (float)viewport[2] * 2.0f - 1.0f;
	position.y = (y - (float)viewport[1]) / (float)viewport[3] * 2.0f - 1.0f;
	position.z = z * 2.0f - 1.0f;

/*
	position.x = x / (float)viewport[2];
	position.y = y / (float)viewport[3];
	position.z = z;

	position *= 2.0f;
	position -= 1.0f;
*/

	Point pointInView = final.transform(position); // this should handle scaling the position correctly by .w ...
	objectCoordinates = pointInView;

	return true;
}

bool OpenGLEx::unprojectCombined(float x, float y, double* pModelView, double* pProjection, int32_t* viewport,
								 Point& objectCoordinatesNear, Point& objectCoordinatesFar)
{
	float model[16];
	for (unsigned int i = 0; i < 16; i++)
		model[i] = (float)pModelView[i];

	float projection[16];
	for (unsigned int i = 0; i < 16; i++)
		projection[i] = (float)pProjection[i];

	float res1[16];
	mulMatrices(model, projection, res1);

	float final[16];
	invertMatrix(res1, final);

	float pos[4];
	pos[0] = (x - (float)viewport[0]) / (float)viewport[2] * 2.0f - 1.0f;
	pos[1] = (y - (float)viewport[1]) / (float)viewport[3] * 2.0f - 1.0f;
	pos[2] = 0.0f * 2.0f - 1.0f;
	pos[3] = 1.0f;

	float outNear[4];
	mulMatrixVec4f(final, pos, outNear);

	if (outNear[3] == 0.0f)
		return false;

	outNear[0] /= outNear[3];
	outNear[1] /= outNear[3];
	outNear[2] /= outNear[3];

	objectCoordinatesNear = Point(outNear[0], outNear[1], outNear[2]);

	pos[2] = 1.0f * 2.0f - 1.0f;
	float outFar[4];

	mulMatrixVec4f(final, pos, outFar);

	if (outFar[3] == 0.0f)
		return false;

	outFar[0] /= outFar[3];
	outFar[1] /= outFar[3];
	outFar[2] /= outFar[3];

	objectCoordinatesFar = Point(outFar[0], outFar[1], outFar[2]);

	return true;
}

void OpenGLEx::mulMatrixVec4f(const float matrix[16], const float vec4[4], float out[4])
{
	for (unsigned int i = 0; i < 4; i++)
	{
		out[i] = vec4[0] * matrix[0 * 4 + i] +
				 vec4[1] * matrix[1 * 4 + i] +
				 vec4[2] * matrix[2 * 4 + i] +
				 vec4[3] * matrix[3 * 4 + i];
	}
}

void OpenGLEx::mulMatrices(const float matrixA[16], const float matrixB[16], float matrixOut[16])
{
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			matrixOut[i * 4 + j] = matrixA[i * 4 + 0] * matrixB[0 * 4 + j]
								 + matrixA[i * 4 + 1] * matrixB[1 * 4 + j]
								 + matrixA[i * 4 + 2] * matrixB[2 * 4 + j]
								 + matrixA[i * 4 + 3] * matrixB[3 * 4 + j];
		}
	}
}

void OpenGLEx::invertMatrix(const float matrixIn[16], float matrixOut[16])
{
	float inv[16];

	inv[0] = matrixIn[5] * matrixIn[10] * matrixIn[15] - matrixIn[5] * matrixIn[11] * matrixIn[14] - matrixIn[9] * matrixIn[6] * matrixIn[15]
	+ matrixIn[9] * matrixIn[7] * matrixIn[14] + matrixIn[13] * matrixIn[6] * matrixIn[11] - matrixIn[13] * matrixIn[7] * matrixIn[10];
	inv[4] = - matrixIn[4] * matrixIn[10] * matrixIn[15] + matrixIn[4] * matrixIn[11] * matrixIn[14] + matrixIn[8] * matrixIn[6] * matrixIn[15]
	- matrixIn[8] * matrixIn[7] * matrixIn[14] - matrixIn[12] * matrixIn[6] * matrixIn[11] + matrixIn[12] * matrixIn[7] * matrixIn[10];
	inv[8] = matrixIn[4] * matrixIn[9] * matrixIn[15] - matrixIn[4] * matrixIn[11] * matrixIn[13] - matrixIn[8] * matrixIn[5] * matrixIn[15]
	+ matrixIn[8] * matrixIn[7] * matrixIn[13] + matrixIn[12] * matrixIn[5] * matrixIn[11] - matrixIn[12] * matrixIn[7] * matrixIn[9];
	inv[12] = - matrixIn[4] * matrixIn[9] * matrixIn[14] + matrixIn[4] * matrixIn[10] * matrixIn[13] + matrixIn[8] * matrixIn[5] * matrixIn[14]
	- matrixIn[8] * matrixIn[6] * matrixIn[13] - matrixIn[12] * matrixIn[5] * matrixIn[10] + matrixIn[12] * matrixIn[6] * matrixIn[9];
	inv[1] = - matrixIn[1] * matrixIn[10] * matrixIn[15] + matrixIn[1] * matrixIn[11] * matrixIn[14] + matrixIn[9] * matrixIn[2] * matrixIn[15]
	- matrixIn[9] * matrixIn[3] * matrixIn[14] - matrixIn[13] * matrixIn[2] * matrixIn[11] + matrixIn[13] * matrixIn[3] * matrixIn[10];
	inv[5] = matrixIn[0] * matrixIn[10] * matrixIn[15] - matrixIn[0] * matrixIn[11] * matrixIn[14] - matrixIn[8] * matrixIn[2] * matrixIn[15]
	+ matrixIn[8] * matrixIn[3] * matrixIn[14] + matrixIn[12] * matrixIn[2] * matrixIn[11] - matrixIn[12] * matrixIn[3] * matrixIn[10];
	inv[9] = - matrixIn[0] * matrixIn[9] * matrixIn[15] + matrixIn[0] * matrixIn[11] * matrixIn[13] + matrixIn[8] * matrixIn[1] * matrixIn[15]
	- matrixIn[8] * matrixIn[3] * matrixIn[13] - matrixIn[12] * matrixIn[1] * matrixIn[11] + matrixIn[12] * matrixIn[3] * matrixIn[9];
	inv[13] = matrixIn[0] * matrixIn[9] * matrixIn[14] - matrixIn[0] * matrixIn[10] * matrixIn[13] - matrixIn[8] * matrixIn[1] * matrixIn[14]
	+ matrixIn[8] * matrixIn[2] * matrixIn[13] + matrixIn[12] * matrixIn[1] * matrixIn[10] - matrixIn[12] * matrixIn[2] * matrixIn[9];
	inv[2] = matrixIn[1] * matrixIn[6] * matrixIn[15] - matrixIn[1] * matrixIn[7] * matrixIn[14] - matrixIn[5] * matrixIn[2] * matrixIn[15]
	+ matrixIn[5] * matrixIn[3] * matrixIn[14] + matrixIn[13] * matrixIn[2] * matrixIn[7] - matrixIn[13] * matrixIn[3] * matrixIn[6];
	inv[6] = - matrixIn[0] * matrixIn[6] * matrixIn[15] + matrixIn[0] * matrixIn[7] * matrixIn[14] + matrixIn[4] * matrixIn[2] * matrixIn[15]
	- matrixIn[4] * matrixIn[3] * matrixIn[14] - matrixIn[12] * matrixIn[2] * matrixIn[7] + matrixIn[12] * matrixIn[3] * matrixIn[6];
	inv[10] = matrixIn[0] * matrixIn[5] * matrixIn[15] - matrixIn[0] * matrixIn[7] * matrixIn[13] - matrixIn[4] * matrixIn[1] * matrixIn[15]
	+ matrixIn[4] * matrixIn[3] * matrixIn[13] + matrixIn[12] * matrixIn[1] * matrixIn[7] - matrixIn[12] * matrixIn[3] * matrixIn[5];
	inv[14] = - matrixIn[0] * matrixIn[5] * matrixIn[14] + matrixIn[0] * matrixIn[6] * matrixIn[13] + matrixIn[4] * matrixIn[1] * matrixIn[14]
	- matrixIn[4] * matrixIn[2] * matrixIn[13] - matrixIn[12] * matrixIn[1] * matrixIn[6] + matrixIn[12] * matrixIn[2] * matrixIn[5];
	inv[3] = - matrixIn[1] * matrixIn[6] * matrixIn[11] + matrixIn[1] * matrixIn[7] * matrixIn[10] + matrixIn[5] * matrixIn[2] * matrixIn[11]
	- matrixIn[5] * matrixIn[3] * matrixIn[10] - matrixIn[9] * matrixIn[2] * matrixIn[7] + matrixIn[9] * matrixIn[3] * matrixIn[6];
	inv[7] = matrixIn[0] * matrixIn[6] * matrixIn[11] - matrixIn[0] * matrixIn[7] * matrixIn[10] - matrixIn[4] * matrixIn[2] * matrixIn[11]
	+ matrixIn[4] * matrixIn[3] * matrixIn[10] + matrixIn[8] * matrixIn[2] * matrixIn[7] - matrixIn[8] * matrixIn[3] * matrixIn[6];
	inv[11] = - matrixIn[0] * matrixIn[5] * matrixIn[11] + matrixIn[0] * matrixIn[7] * matrixIn[9] + matrixIn[4] * matrixIn[1] * matrixIn[11]
	- matrixIn[4] * matrixIn[3] * matrixIn[9] - matrixIn[8] * matrixIn[1] * matrixIn[7] + matrixIn[8] * matrixIn[3] * matrixIn[5];
	inv[15] = matrixIn[0] * matrixIn[5] * matrixIn[10] - matrixIn[0] * matrixIn[6] * matrixIn[9] - matrixIn[4] * matrixIn[1] * matrixIn[10]
	+ matrixIn[4] * matrixIn[2] * matrixIn[9] + matrixIn[8] * matrixIn[1] * matrixIn[6] - matrixIn[8] * matrixIn[2] * matrixIn[5];

	float det = matrixIn[0] * inv[0] + matrixIn[1] * inv[4] + matrixIn[2] * inv[8] + matrixIn[3] * inv[12];

	if (det == 0.0f)
	{
		return;
	}
	det = 1.0f / det;
	for (unsigned int i = 0; i < 16; i++)
	{
		matrixOut[i] = inv[i] * det;
	}
}

ObjectGLTextureBakerWorker::ObjectGLTextureBakerWorker(unsigned int threads) : ThreadPool(threads, false)
{

}

void ObjectGLTextureBakerWorker::bakeTextureToImage(ImageColour3b* pTargetImage, Texture* pTexture)
{
	unsigned int mainHeight = pTargetImage->getHeight();

	unsigned int tasks = m_numberOfThreads;

	unsigned int segHeight = mainHeight / tasks;

	unsigned int startLine = 0;
	unsigned int endLine = segHeight;

	for (unsigned int i = 0; i < tasks; i++)
	{
		ObjectGLTextureBakerTask* pNewTask = new ObjectGLTextureBakerTask(pTargetImage, pTexture, startLine, endLine);
		addTaskNoLock(pNewTask);

		startLine += segHeight;

		if (i == (tasks - 2))
		{
			// The next run of this loop will be the last one, so set end line to be the remainder of the image space.
			// On the assumption that this threaded worker will never get called with tasks = 1,
			// this should be safe.
			endLine = mainHeight;
		}
		else
		{
			endLine += segHeight;
		}
	}

	startPool(POOL_WAIT_FOR_COMPLETION);
}

bool ObjectGLTextureBakerWorker::doTask(ThreadPoolTask* pTask, unsigned int threadID)
{
	if (!pTask)
		return false;

	ObjectGLTextureBakerTask* pThisTask = static_cast<ObjectGLTextureBakerTask*>(pTask);

	ImageColour3b* pTargetImage = pThisTask->getTargetImage();
	Texture* pTexture = pThisTask->getTexture();

	unsigned int mainWidth = pTargetImage->getWidth();
	unsigned int mainHeight = pTargetImage->getHeight();

	unsigned int startLine = pThisTask->getStartLine();
	unsigned int endLine = pThisTask->getEndLine();

	float glWidth = 1.0f / (float)mainWidth;
	float glHeight = 1.0f / (float)mainHeight;

	HitResult tempResult;
	for (unsigned int v = startLine; v < endLine; v++)
	{
		tempResult.uv.v = glHeight * (float)v;

		Colour3b* pByteBuffer = pTargetImage->colour3bRowPtr(v);
		for (unsigned int u = 0; u < mainWidth; u++)
		{
			tempResult.uv.u = glWidth * (float)u;

			Colour3f colour = pTexture->getColourBlend(tempResult, 0);

			ColourSpace::convertLinearToSRGBFast(colour);

			colour.clamp();

			unsigned char r = colour.r * 255;
			unsigned char g = colour.g * 255;
			unsigned char b = colour.b * 255;

			Colour3b newColour(r, g, b);
			*pByteBuffer = newColour;

			pByteBuffer++;
		}
	}

	return true;
}

} // namespace Imagine
