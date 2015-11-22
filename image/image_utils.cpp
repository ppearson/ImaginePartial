/*
 Imagine
 Copyright 2014-2015 Peter Pearson.

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

#include "image_utils.h"

#include "image_colour3f.h"
#include "image_colour3h.h"
#include "image_1f.h"
#include "image_1h.h"

ImageUtils::ImageUtils()
{
}

bool ImageUtils::flipImageVertically(Image* pImage)
{
	unsigned int scanlines = pImage->getHeight();

	// only need to do half of them, as we're swapping top and bottom
	unsigned int numRows = scanlines / 2;

	unsigned int width = pImage->getWidth();

	bool done = false;

	if (pImage->getImageType() == (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_FLOAT))
	{
		ImageColour3f* pImageColour3f = static_cast<ImageColour3f*>(pImage);
		flipImageVertically(pImageColour3f, scanlines, numRows, width, width * sizeof(Colour3f));
		done = true;
	}
	else if (pImage->getImageType() == (Image::IMAGE_CHANNELS_3 | Image::IMAGE_FORMAT_HALF))
	{
		ImageColour3h* pImageColour3h = static_cast<ImageColour3h*>(pImage);
		flipImageVertically(pImageColour3h, scanlines, numRows, width, width * sizeof(Colour3h));
		done = true;
	}
	else if (pImage->getImageType() == (Image::IMAGE_CHANNELS_1 | Image::IMAGE_FORMAT_FLOAT))
	{
		Image1f* pImage1f = static_cast<Image1f*>(pImage);
		flipImageVertically(pImage1f, scanlines, numRows, width, width * sizeof(float));
		done = true;
	}
	else if (pImage->getImageType() == (Image::IMAGE_CHANNELS_1 | Image::IMAGE_FORMAT_HALF))
	{
		Image1h* pImage1h = static_cast<Image1h*>(pImage);
		flipImageVertically(pImage1h, scanlines, numRows, width, width * sizeof(half));
		done = true;
	}

	return done;
}

void ImageUtils::flipImageTileVertically(unsigned char* pData, unsigned int pixelStride, unsigned int width, unsigned int height)
{
	unsigned int tilePixelWidth = pixelStride * width;
	unsigned char* pTempScanline = new unsigned char[tilePixelWidth];

	// only need to do half of them, as we're swapping top and bottom
	unsigned int numRows = height / 2;

	unsigned int targetScanline = height - 1;

	for (unsigned int i = 0; i < numRows; i++)
	{
		unsigned char* pSrc = pData + (i * tilePixelWidth);

		memcpy(pTempScanline, pSrc, tilePixelWidth);

		unsigned char* pDst = pData + (targetScanline * tilePixelWidth);

		// copy bottom to top
		memcpy(pSrc, pDst, tilePixelWidth);

		// copy temp to bottom
		memcpy(pDst, pTempScanline, tilePixelWidth);

		targetScanline --;
	}

	if (pTempScanline)
	{
		delete [] pTempScanline;
		pTempScanline = NULL;
	}
}

void ImageUtils::flipImageVertically(ImageColour3f* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes)
{
	Colour3f* pTempScanline = new Colour3f[width];

	unsigned int targetScanline = scanlines - 1;

	for (unsigned int i = 0; i < numRows; i++)
	{
		Colour3f* pSrc = pImage->colourRowPtr(i);

		memcpy(pTempScanline, pSrc, scanlineBytes);

		Colour3f* pDst = pImage->colourRowPtr(targetScanline);

		// copy bottom to top
		memcpy(pSrc, pDst, scanlineBytes);

		// copy temp to bottom
		memcpy(pDst, pTempScanline, scanlineBytes);

		targetScanline --;
	}

	if (pTempScanline)
	{
		delete [] pTempScanline;
		pTempScanline = NULL;
	}
}

void ImageUtils::flipImageVertically(ImageColour3h* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes)
{
	Colour3h* pTempScanline = new Colour3h[width];

	unsigned int targetScanline = scanlines - 1;

	for (unsigned int i = 0; i < numRows; i++)
	{
		Colour3h* pSrc = pImage->colour3hRowPtr(i);

		memcpy(pTempScanline, pSrc, scanlineBytes);

		Colour3h* pDst = pImage->colour3hRowPtr(targetScanline);

		// copy bottom to top
		memcpy(pSrc, pDst, scanlineBytes);

		// copy temp to bottom
		memcpy(pDst, pTempScanline, scanlineBytes);

		targetScanline --;
	}

	if (pTempScanline)
	{
		delete [] pTempScanline;
		pTempScanline = NULL;
	}
}

void ImageUtils::flipImageVertically(Image1f* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes)
{
	float* pTempScanline = new float[width];

	unsigned int targetScanline = scanlines - 1;

	for (unsigned int i = 0; i < numRows; i++)
	{
		float* pSrc = pImage->floatRowPtr(i);

		memcpy(pTempScanline, pSrc, scanlineBytes);

		float* pDst = pImage->floatRowPtr(targetScanline);

		// copy bottom to top
		memcpy(pSrc, pDst, scanlineBytes);

		// copy temp to bottom
		memcpy(pDst, pTempScanline, scanlineBytes);

		targetScanline --;
	}

	if (pTempScanline)
	{
		delete [] pTempScanline;
		pTempScanline = NULL;
	}
}

void ImageUtils::flipImageVertically(Image1h* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes)
{
	half* pTempScanline = new half[width];

	unsigned int targetScanline = scanlines - 1;

	for (unsigned int i = 0; i < numRows; i++)
	{
		half* pSrc = pImage->halfRowPtr(i);

		memcpy(pTempScanline, pSrc, scanlineBytes);

		half* pDst = pImage->halfRowPtr(targetScanline);

		// copy bottom to top
		memcpy(pSrc, pDst, scanlineBytes);

		// copy temp to bottom
		memcpy(pDst, pTempScanline, scanlineBytes);

		targetScanline --;
	}

	if (pTempScanline)
	{
		delete [] pTempScanline;
		pTempScanline = NULL;
	}
}

