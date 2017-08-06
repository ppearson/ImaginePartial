/*
 Imagine
 Copyright 2012-2014 Peter Pearson.

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

#include "image_reader_hdr.h"

#include <stdio.h>
#include <memory.h>
#include <string.h>

#include "global_context.h"

#include "image/image_colour3f.h"
#include "colour/colour_space.h"

namespace Imagine
{

#define MINELEN 8
#define MAXELEN 0x7fff

static const float kToFloatRcp = 1.0f / 256.0f;

ImageReaderHDR::ImageReaderHDR() : ImageReader()
{
}

Image* ImageReaderHDR::readColourImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile)
	{
		GlobalContext::instance().getLogger().error("Cannot open file: %s", filePath.c_str());
		return NULL;
	}

	char szTemp[64];
	fread(szTemp, 10, 1, pFile);
	if (memcmp(szTemp, "#?RADIANCE", 10) && memcmp(szTemp, "#?RGBE", 6))
	{
		GlobalContext::instance().getLogger().error("Can't open file: %s - doesn't seem to be an .hdr file...", filePath.c_str());
		fclose(pFile);
		return NULL;
	}

	fseek(pFile, 1, SEEK_CUR);

	char thisChar = 0;
	char lastChar = 0;
	unsigned int count = 0;
	while (true)
	{
		lastChar = thisChar;
		thisChar = fgetc(pFile);
		if (thisChar == 0xa && lastChar == 0xa)
			break;
	}

	// get the resolution of the image
	char szResolution[128];
	count = 0;
	unsigned int limit = 0;
	while (true && limit++ < 127)
	{
		thisChar = fgetc(pFile);
		szResolution[count++] = thisChar;
		if (thisChar == 0xa)
			break;
	}

	unsigned int width;
	unsigned int height;

	bool negativeY = false;
	bool foundSize = sscanf(szResolution, "-Y %u +X %u", &height, &width) != 0;

	if (!foundSize)
	{
		foundSize = sscanf(szResolution, "Y %u +X %u", &height, &width) != 0;
		if (foundSize)
		{
			negativeY = true;
		}
		else
		{
			fclose(pFile);
			return NULL;
		}
	}

	RGBE* pScanline = new RGBE[width];
	if (!pScanline)
	{
		GlobalContext::instance().getLogger().error("Cannot allocate memory to read file: %s", filePath.c_str());
		fclose(pFile);
		return NULL;
	}

	ImageColour3f* pImage = new ImageColour3f(width, height, false);
	if (!pImage)
	{
		GlobalContext::instance().getLogger().error("Cannot allocate memory for image from file: %s", filePath.c_str());
		delete [] pScanline;
		fclose(pFile);
		return NULL;
	}

	// now process the scanlines...
	for (unsigned int y = height; y > 0; y--)
	{
		if (!processScanline(pScanline, width, pFile))
			break;

		unsigned int localY = y - 1;

		unsigned int dstRow = negativeY ? height - localY : localY;
		Colour3f* pImageRow = pImage->colourRowPtr(dstRow);
		processColour(pScanline, width, pImageRow);
	}

	delete [] pScanline;
	fclose(pFile);

	return pImage;
}

bool ImageReaderHDR::processScanline(RGBE* pScanline, unsigned int length, FILE* pFile)
{
	if (length < MINELEN || length > MAXELEN)
		return processScanlineOld(pScanline, length, pFile);

	if (fgetc(pFile) != 2)
	{
		fseek(pFile, -1, SEEK_CUR);
		return processScanlineOld(pScanline, length, pFile);
	}

	pScanline[0].G = fgetc(pFile);
	pScanline[0].B = fgetc(pFile);
	unsigned int i = fgetc(pFile);

	if (pScanline[0].G != 2 || pScanline[0].B & 128)
	{
		pScanline[0].R = 2;
		pScanline[0].E = i;
		return processScanlineOld(pScanline, length - 1, pFile);
	}

	for (i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < length; )
		{
			unsigned char code = fgetc(pFile);
			if (code > 128)
			{
				code &= 127;
				unsigned char value = fgetc(pFile);
				while (code--)
				{
					pScanline[j++][i] = value;
				}
			}
			else
			{
				while (code--)
				{
					pScanline[j++][i] = fgetc(pFile);
				}
			}
		}
	}

	return feof(pFile) ? false : true;
}

bool ImageReaderHDR::processScanlineOld(RGBE* pScanline, unsigned int length, FILE* pFile)
{
	unsigned int rShift = 0;

	while (length > 0)
	{
		pScanline[0].R = fgetc(pFile);
		pScanline[0].G = fgetc(pFile);
		pScanline[0].B = fgetc(pFile);
		pScanline[0].E = fgetc(pFile);
		if (feof(pFile))
			return false;

		if (pScanline[0].R == 1 && pScanline[0].G == 1 && pScanline[0].B == 1)
		{
			for (unsigned int i = pScanline[0].E << rShift; i > 0; i--)
			{
				memcpy(&pScanline[0].R, &pScanline[-1].R, 4);
				pScanline++;
				length--;
			}
			rShift += 8;
		}
		else
		{
			pScanline++;
			length --;
			rShift = 0;
		}
	}

	return true;
}

void ImageReaderHDR::processColour(const RGBE* pScanline, unsigned int length, Colour3f* pDestImageRow)
{
	while (length-- > 0)
	{
		const RGBE* pVal = pScanline;

		// retrieve the values ahead-of-time in the order they are in memory, in the hope it
		// helps the pre-fetching...
		float red = (float)pVal->R;
		float green = (float)pVal->G;
		float blue = (float)pVal->B;

		int exposure = pVal->E - 128;

		float exposureScaleValue = ldexpf(1.0f, exposure) * kToFloatRcp;

		pDestImageRow->r = red * exposureScaleValue;
		pDestImageRow->g = green * exposureScaleValue;
		pDestImageRow->b = blue * exposureScaleValue;

		pDestImageRow ++;
		pScanline ++;
	}
}


} // namespace Imagine

namespace
{
	Imagine::ImageReader* createImageReaderHDR()
	{
		return new Imagine::ImageReaderHDR();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerImageReader("hdr", createImageReaderHDR);
}
