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

#include "image_reader_tga.h"

#include "image/image_colour3f.h"
#include "image/image_colour3b.h"
#include "image/image_1f.h"
#include "image/image_1b.h"

#include "global_context.h"

#include "colour/colour_space.h"

namespace Imagine
{

ImageReaderTGA::ImageReaderTGA() : ImageReader()
{
}

Image* ImageReaderTGA::readColourImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	TGAInfra infra;

	if (!readData(filePath, infra))
		return nullptr;

	ImageColour3f* pImage3f = nullptr;
	ImageColour3b* pImage3b = nullptr;

	const bool makeFloat = !(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE);

	if (makeFloat)
	{
		pImage3f = new ImageColour3f(infra.header.width, infra.header.height, false);
	}
	else
	{
		pImage3b = new ImageColour3b(infra.header.width, infra.header.height, false);
	}

	TGAPixel* pScanlineBuffer = infra.pBuffer;

	if (makeFloat)
	{
		// now convert to float in image...
		for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
		{
			// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
			// in which case there's no need to do this...
			unsigned int y = i;
			if (!infra.flipY)
			{
				y = (unsigned int)infra.header.height - i - 1;
			}

			Colour3f* pImageRow = pImage3f->colourRowPtr(y);

			for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
			{
				unsigned char red = pScanlineBuffer->r;
				unsigned char green = pScanlineBuffer->g;
				unsigned char blue = pScanlineBuffer->b;

				pImageRow->r = ColourSpace::convertSRGBToLinearLUT(red);
				pImageRow->g = ColourSpace::convertSRGBToLinearLUT(green);
				pImageRow->b = ColourSpace::convertSRGBToLinearLUT(blue);

				pImageRow++;
				pScanlineBuffer++;
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
		{
			// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
			// in which case there's no need to do this...
			unsigned int y = i;
			if (!infra.flipY)
			{
				y = (unsigned int)infra.header.height - i - 1;
			}

			Colour3b* pImageRow = pImage3b->colour3bRowPtr(y);

			for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
			{
				pImageRow->r = pScanlineBuffer->r;
				pImageRow->g = pScanlineBuffer->g;
				pImageRow->b = pScanlineBuffer->b;

				pImageRow++;
				pScanlineBuffer++;
			}
		}
	}

	delete [] infra.pBuffer;

	Image* pFinalImage = (makeFloat) ? static_cast<Image*>(pImage3f) : static_cast<Image*>(pImage3b);

	return pFinalImage;
}

// reads in a float image for either brightness (bump mapping) or alpha
Image* ImageReaderTGA::readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	TGAInfra infra;

	if (!readData(filePath, infra))
		return nullptr;

	const bool makeFloat = !(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE);

	Image1f* pImage1f = nullptr;
	Image1b* pImage1b = nullptr;

	if (makeFloat)
	{
		pImage1f = new Image1f(infra.header.width, infra.header.height, false);
	}
	else
	{
		pImage1b = new Image1b(infra.header.width, infra.header.height, false);
	}

	TGAPixel* pScanlineBuffer = infra.pBuffer;

	if (makeFloat)
	{
		if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
		{
			// now convert to float in image...
			for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
			{
				// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
				// in which case there's no need to do this...
				unsigned int y = i;
				if (!infra.flipY)
				{
					y = (unsigned int)infra.header.height - i - 1;
				}

				float* pFloatRow = pImage1f->floatRowPtr(y);

				for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
				{
					unsigned char red = pScanlineBuffer->r;
					unsigned char green = pScanlineBuffer->g;
					unsigned char blue = pScanlineBuffer->b;

					float r = ColourSpace::convertSRGBToLinearLUT(red);
					float g = ColourSpace::convertSRGBToLinearLUT(green);
					float b = ColourSpace::convertSRGBToLinearLUT(blue);

					Colour3f finalColour(r, g, b);

					*pFloatRow = finalColour.brightness();

					pFloatRow++;
					pScanlineBuffer++;
				}
			}

		}
		else if (requiredTypeFlags & Image::IMAGE_FLAGS_ALPHA)
		{
			// otherwise, alpha

			// now convert to float in image...
			for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
			{
				// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
				// in which case there's no need to do this...
				unsigned int y = i;
				if (!infra.flipY)
				{
					y = (unsigned int)infra.header.height - i - 1;
				}

				float* pFloatRow = pImage1f->floatRowPtr(y);

				if (infra.header.bitsPerPixel == 24)
				{
					// use r as we don't have an alpha channel
					for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
					{
						unsigned char alpha = pScanlineBuffer->r;

						*pFloatRow = alpha;

						pFloatRow++;
						pScanlineBuffer++;
					}
				}
				else
				{
					for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
					{
						unsigned char alpha = pScanlineBuffer->a;

						*pFloatRow = alpha;

						pFloatRow++;
						pScanlineBuffer++;
					}
				}
			}
		}
		else // EXACT - for displacement
		{
			for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
			{
				// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
				// in which case there's no need to do this...
				unsigned int y = i;
				if (!infra.flipY)
				{
					y = (unsigned int)infra.header.height - i - 1;
				}

				float* pFloatRow = pImage1f->floatRowPtr(y);

				for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
				{
					unsigned char red = pScanlineBuffer->r;

					*pFloatRow = (float)red / 255.0f;

					pFloatRow++;
					pScanlineBuffer++;
				}
			}
		}
	}
	else
	{
		if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
		{
			// now convert to float in image...
			for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
			{
				// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
				// in which case there's no need to do this...
				unsigned int y = i;
				if (!infra.flipY)
				{
					y = (unsigned int)infra.header.height - i - 1;
				}

				unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

				for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
				{
					unsigned char red = pScanlineBuffer->r;
					unsigned char green = pScanlineBuffer->g;
					unsigned char blue = pScanlineBuffer->b;

					Colour3b finalColour(red, green, blue);

					*pUCharRow = finalColour.brightness();

					pUCharRow++;
					pScanlineBuffer++;
				}
			}

		}
		else if (requiredTypeFlags & Image::IMAGE_FLAGS_ALPHA)
		{
			// otherwise, alpha

			// now convert to float in image...
			for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
			{
				// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
				// in which case there's no need to do this...
				unsigned int y = i;
				if (!infra.flipY)
				{
					y = (unsigned int)infra.header.height - i - 1;
				}

				unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

				if (infra.header.bitsPerPixel == 24)
				{
					// use r as we don't have an alpha channel
					for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
					{
						unsigned char alpha = pScanlineBuffer->r;

						*pUCharRow = alpha;

						pUCharRow++;
						pScanlineBuffer++;
					}
				}
				else
				{
					for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
					{
						unsigned char alpha = pScanlineBuffer->a;

						*pUCharRow = alpha;

						pUCharRow++;
						pScanlineBuffer++;
					}
				}
			}
		}
		else // EXACT - for displacement
		{
			for (unsigned int i = 0; i < (unsigned int)infra.header.height; i++)
			{
				// need to flip the y scanline round by default to flip the Y, unless TGA's header says it's inverted,
				// in which case there's no need to do this...
				unsigned int y = i;
				if (!infra.flipY)
				{
					y = (unsigned int)infra.header.height - i - 1;
				}

				unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

				for (unsigned int x = 0; x < (unsigned int)infra.header.width; x++)
				{
					unsigned char red = pScanlineBuffer->r;

					*pUCharRow = red;

					pUCharRow++;
					pScanlineBuffer++;
				}
			}
		}
	}

	delete [] infra.pBuffer;

	Image* pFinalImage = (makeFloat) ? static_cast<Image*>(pImage1f) : static_cast<Image*>(pImage1b);

	return pFinalImage;
}

bool ImageReaderTGA::readData(const std::string& filePath, TGAInfra& infra)
{
	infra.pFile = fopen(filePath.c_str(), "rb");
	if (!infra.pFile)
	{
		GlobalContext::instance().getLogger().error("Error reading file: %s", filePath.c_str());
		return false;
	}

	infra.header.idLength = fgetc(infra.pFile);
	infra.header.colourMapType = fgetc(infra.pFile);
	infra.header.dataTypeCode = fgetc(infra.pFile);
	fread(&infra.header.colourMapOrigin, 2, 1, infra.pFile);
	fread(&infra.header.colourMapLength, 2, 1, infra.pFile);
	infra.header.colourMapDepth = fgetc(infra.pFile);
	fread(&infra.header.xOrigin, 2, 1, infra.pFile);
	fread(&infra.header.yOrigin, 2, 1, infra.pFile);
	fread(&infra.header.width, 2, 1, infra.pFile);
	fread(&infra.header.height, 2, 1, infra.pFile);
	infra.header.bitsPerPixel = fgetc(infra.pFile);
	infra.header.imageDescriptor = fgetc(infra.pFile);

	// check what we currently support
	if (infra.header.dataTypeCode != 2 && infra.header.dataTypeCode != 10)
	{
		fclose(infra.pFile);
		GlobalContext::instance().getLogger().error("Can't handle this TGA image type for file: %s", filePath.c_str());
		return false;
	}

	infra.flipY = !(infra.header.imageDescriptor & 0x20);

	if (infra.header.bitsPerPixel != 16 && infra.header.bitsPerPixel != 24 && infra.header.bitsPerPixel != 32)
	{
		fclose(infra.pFile);
		GlobalContext::instance().getLogger().error("Can only handle pixel depths of 16,24,32...");
		return false;
	}

	if (infra.header.colourMapType != 0 && infra.header.colourMapType != 1)
	{
		fclose(infra.pFile);
		GlobalContext::instance().getLogger().error("Can't handle this colourmap type...");
		return false;
	}

	infra.pBuffer = new TGAPixel[infra.header.width * infra.header.height];
	memset(infra.pBuffer, 0, infra.header.width * infra.header.height * sizeof(TGAPixel));

	unsigned char p[5];

	size_t bytesToRead = infra.header.bitsPerPixel / 8;
	size_t skipDataLength = infra.header.idLength + (infra.header.colourMapType * infra.header.colourMapLength);
	// skip unneeded data
	fseek(infra.pFile, skipDataLength, SEEK_CUR);

	unsigned int n = 0;
	while (n < (unsigned int)infra.header.width * (unsigned int)infra.header.height)
	{
		// if uncompressed
		if (infra.header.dataTypeCode == 2)
		{
			if (fread(p, 1, bytesToRead, infra.pFile) != bytesToRead)
			{
				GlobalContext::instance().getLogger().error("Can't read TGA file: %s...", filePath.c_str());
				fclose(infra.pFile);
				delete [] infra.pBuffer;
				return false;
			}
			extractPixelValues(p, &(infra.pBuffer[n]), bytesToRead);
			n++;
		}
		else if (infra.header.dataTypeCode == 10) // compressed
		{
			if (fread(p, 1, bytesToRead + 1, infra.pFile) != bytesToRead + 1)
			{
				GlobalContext::instance().getLogger().error("Can't read TGA file: %s...", filePath.c_str());
				fclose(infra.pFile);
				delete [] infra.pBuffer;
				return false;
			}

			unsigned int j = p[0] & 0x7f;
			extractPixelValues(&(p[1]), &(infra.pBuffer[n]), bytesToRead);
			n++;

			if (p[0] & 0x80) // RLE chunk
			{
				for (unsigned int i = 0; i < j; i++)
				{
					extractPixelValues(&(p[1]), &(infra.pBuffer[n]), bytesToRead);
					n++;
				}
			}
			else // normal
			{
				for (unsigned int i = 0; i < j; i++)
				{
					if (fread(p, 1, bytesToRead, infra.pFile) != bytesToRead)
					{
						GlobalContext::instance().getLogger().error("Can't read TGA file: %s...", filePath.c_str());
						fclose(infra.pFile);
						delete [] infra.pBuffer;
						return false;
					}
					extractPixelValues(p, &(infra.pBuffer[n]), bytesToRead);
					n++;
				}
			}
		}
	}

	fclose(infra.pFile);
	return true;
}

void ImageReaderTGA::extractPixelValues(const unsigned char* pixel, TGAPixel* finalPixels, unsigned int bytes)
{
	switch (bytes)
	{
		case 4:
		default:
			finalPixels->r = pixel[2];
			finalPixels->g = pixel[1];
			finalPixels->b = pixel[0];
			finalPixels->a = pixel[3];
			break;
		case 3:
			finalPixels->r = pixel[2];
			finalPixels->g = pixel[1];
			finalPixels->b = pixel[0];
			finalPixels->a = 255;
			break;
		case 2:
			finalPixels->r = (pixel[1] & 0x7c) << 1;
			finalPixels->g = ((pixel[1] & 0x03) << 6) | ((pixel[0] & 0xe0) >> 2);
			finalPixels->b = (pixel[0] & 0x1f) << 3;
			finalPixels->a = (pixel[1] & 0x80);
			break;
	}
}


} // namespace Imagine

namespace
{
	Imagine::ImageReader* createImageReaderTGA()
	{
		return new Imagine::ImageReaderTGA();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerImageReader("tga", createImageReaderTGA);
}
