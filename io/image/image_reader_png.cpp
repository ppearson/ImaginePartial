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

#include "image_reader_png.h"

#include <png.h>

#include "image/image_colour3f.h"
#include "image/image_colour3b.h"
#include "image/image_1f.h"
#include "image/image_1b.h"

#include "global_context.h"

#include "colour/colour_space.h"

namespace Imagine
{

struct PNGInfra
{
	FILE*			pFile;
	png_structp		pPNG;
	png_infop		pInfo;
	png_uint_32		width;
	png_uint_32		height;
	png_bytepp		pRows;
};

ImageReaderPNG::ImageReaderPNG() : ImageReader()
{
}

ImageReaderPNG::ImageType ImageReaderPNG::readData(const std::string& filePath, PNGInfra& infra, bool wantAlpha)
{
	if (filePath.empty())
		return eInvalid;

	infra.pFile = fopen(filePath.c_str(), "rb");
	if (!infra.pFile)
	{
		GlobalContext::instance().getLogger().error("Error opening file: %s", filePath.c_str());
		return eInvalid;
	}

	unsigned char sig[8];

	// check the signature
	fread(sig, 1, 8, infra.pFile);
	if (!png_check_sig(sig, 8))
	{
		GlobalContext::instance().getLogger().error("Cannot open file: %s - not a valid PNG file.", filePath.c_str());
		fclose(infra.pFile);
		return eInvalid;
	}

	infra.pPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!infra.pPNG)
	{
		fclose(infra.pFile);
		return eInvalid;
	}

	infra.pInfo = png_create_info_struct(infra.pPNG);
	if (!infra.pInfo)
	{
		png_destroy_read_struct(&infra.pPNG, NULL, NULL);
		fclose(infra.pFile);
		return eInvalid;
	}

	if (setjmp(png_jmpbuf(infra.pPNG)))
	{
		png_destroy_read_struct(&infra.pPNG, &infra.pInfo, NULL);
		fclose(infra.pFile);
		return eInvalid;
	}

	png_init_io(infra.pPNG, infra.pFile);
	png_set_sig_bytes(infra.pPNG, 8);
	png_read_info(infra.pPNG, infra.pInfo);

	int colorType;
	int bitDepth, interlaceType, compressionType;

	png_get_IHDR(infra.pPNG, infra.pInfo, &infra.width, &infra.height, &bitDepth, &colorType, &interlaceType, &compressionType, NULL);

	if (colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(infra.pPNG);

	if (png_get_valid(infra.pPNG, infra.pInfo, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(infra.pPNG);
	}

	if (bitDepth == 16)
		png_set_strip_16(infra.pPNG);

	ImageType type = eInvalid;

	if (!wantAlpha)
	{
		// add black Alpha
		if ((colorType & PNG_COLOR_MASK_ALPHA) == 0)
			png_set_add_alpha(infra.pPNG, 0xFF, PNG_FILLER_AFTER);

		if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(infra.pPNG);
		else if (colorType != PNG_COLOR_TYPE_RGB && colorType != PNG_COLOR_TYPE_RGB_ALPHA)
			return eInvalid;

		type = eRGBA;
	}
	else
	{
		if (colorType == PNG_COLOR_TYPE_RGB_ALPHA)
			type = eRGBA;
		else if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			type = eA;
		else
			return eInvalid;
	}

	png_read_update_info(infra.pPNG, infra.pInfo);

	infra.pRows = new png_bytep[infra.height * sizeof(png_bytep)];

	png_set_rows(infra.pPNG, infra.pInfo, infra.pRows);

	for (unsigned int i = 0; i < infra.height; i++)
	{
		infra.pRows[i] = new png_byte[png_get_rowbytes(infra.pPNG, infra.pInfo)];
	}

	png_read_image(infra.pPNG, infra.pRows);
	png_read_end(infra.pPNG, infra.pInfo);

	return type;
}

Image* ImageReaderPNG::readColourImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	PNGInfra infra;
	if (readData(filePath, infra, false) != eRGBA)
		return NULL;

	ImageColour3f* pImage3f = NULL;
	ImageColour3b* pImage3b = NULL;

	const bool makeFloat = !(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE);

	if (makeFloat)
	{
		pImage3f = new ImageColour3f(infra.width, infra.height, false);
	}
	else
	{
		pImage3b = new ImageColour3b(infra.width, infra.height, false);
	}

	if (pImage3f || pImage3b)
	{
		if (makeFloat)
		{
			// convert to linear float
			for (unsigned int i = 0; i < infra.height; i++)
			{
				png_byte* pLineData = infra.pRows[i];

				// need to flip the height round...
				unsigned int y = infra.height - i - 1;

				Colour3f* pImageRow = pImage3f->colourRowPtr(y);

				for (unsigned int x = 0; x < infra.width; x++)
				{
					unsigned char red = *pLineData++;
					unsigned char green = *pLineData++;
					unsigned char blue = *pLineData++;
					pLineData++;

					pImageRow->r = ColourSpace::convertSRGBToLinearLUT(red);
					pImageRow->g = ColourSpace::convertSRGBToLinearLUT(green);
					pImageRow->b = ColourSpace::convertSRGBToLinearLUT(blue);

					pImageRow++;
				}
			}
		}
		else
		{
			// copy the raw data across directly
			for (unsigned int i = 0; i < infra.height; i++)
			{
				png_byte* pLineData = infra.pRows[i];

				// need to flip the height round...
				unsigned int y = infra.height - i - 1;

				Colour3b* pImageRow = pImage3b->colour3bRowPtr(y);

				for (unsigned int x = 0; x < infra.width; x++)
				{
					unsigned char red = *pLineData++;
					unsigned char green = *pLineData++;
					unsigned char blue = *pLineData++;
					pLineData++;

					pImageRow->r = red;
					pImageRow->g = green;
					pImageRow->b = blue;

					pImageRow++;
				}
			}
		}
	}

	png_destroy_read_struct(&infra.pPNG, &infra.pInfo, (png_infopp)NULL);

	for (unsigned int y = 0; y < infra.height; y++)
	{
		delete [] infra.pRows[y];
	}
	delete [] infra.pRows;

	fclose(infra.pFile);

	Image* pFinalImage = (makeFloat) ? static_cast<Image*>(pImage3f) : static_cast<Image*>(pImage3b);

	return pFinalImage;
}

Image* ImageReaderPNG::readColourImageAndByteCopy(const std::string& filePath, ImageColour3b* pImageColour3b, unsigned int requiredTypeFlags)
{
	PNGInfra infra;
	if (readData(filePath, infra, false) != eRGBA)
		return NULL;

	ImageColour3f* pImage = new ImageColour3f(infra.width, infra.height, false);
	if (pImage)
	{
		if (!pImageColour3b->initialise(infra.width, infra.height))
		{
			png_destroy_read_struct(&infra.pPNG, &infra.pInfo, (png_infopp)NULL);

			for (unsigned int y = 0; y < infra.height; y++)
			{
				delete [] infra.pRows[y];
			}
			delete [] infra.pRows;

			fclose(infra.pFile);

			return NULL;
		}

		// copy data across...
		for (unsigned int i = 0; i < infra.height; i++)
		{
			png_byte* pLineData = infra.pRows[i];

			// need to flip the height round...
			unsigned int y = infra.height - i - 1;

			Colour3f* pImageRow = pImage->colourRowPtr(y);
			Colour3b* pByteRow = pImageColour3b->colour3bRowPtr(y);

			for (unsigned int x = 0; x < infra.width; x++)
			{
				unsigned char red = *pLineData++;
				unsigned char green = *pLineData++;
				unsigned char blue = *pLineData++;
				pLineData++;

				Colour3b newColour(red, green, blue);

				*pByteRow = newColour;

				pImageRow->r = ColourSpace::convertSRGBToLinearLUT(red);
				pImageRow->g = ColourSpace::convertSRGBToLinearLUT(green);
				pImageRow->b = ColourSpace::convertSRGBToLinearLUT(blue);

				pImageRow++;
				pByteRow++;
			}
		}
	}

	png_destroy_read_struct(&infra.pPNG, &infra.pInfo, (png_infopp)NULL);

	for (unsigned int y = 0; y < infra.height; y++)
	{
		delete [] infra.pRows[y];
	}
	delete [] infra.pRows;

	fclose(infra.pFile);

	return pImage;
}

Image* ImageReaderPNG::readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	PNGInfra infra;
	ImageType actualType = readData(filePath, infra, requiredTypeFlags & Image::IMAGE_FLAGS_ALPHA);

	if (actualType == eInvalid)
		return NULL;

	const bool makeFloat = !(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE);

	Image1f* pImage1f = NULL;
	Image1b* pImage1b = NULL;

	if (makeFloat)
	{
		pImage1f = new Image1f(infra.width, infra.height, false);
	}
	else
	{
		pImage1b = new Image1b(infra.width, infra.height, false);
	}

	if (pImage1f || pImage1b)
	{
		if (makeFloat)
		{
			if (actualType == eRGBA)
			{
				if (requiredTypeFlags & Image::IMAGE_FLAGS_ALPHA)
				{
					// we just want the alpha channel from the RGBA
					for (unsigned int i = 0; i < infra.height; i++)
					{
						png_byte* pLineData = infra.pRows[i];

						// need to flip the height round...
						unsigned int y = infra.height - i - 1;

						float* pFloatRow = pImage1f->floatRowPtr(y);

						for (unsigned int x = 0; x < infra.width; x++)
						{
							pLineData++;
							pLineData++;
							pLineData++;
							unsigned char alpha = *pLineData++;

							*pFloatRow = (alpha == 0) ? 0 : alpha / 255.0f;

							pFloatRow++;
						}
					}
				}
				else if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
				{
					// otherwise, get the brightness of the RGB
					// copy data across...
					for (unsigned int i = 0; i < infra.height; i++)
					{
						png_byte* pLineData = infra.pRows[i];

						// need to flip the height round...
						unsigned int y = infra.height - i - 1;

						float* pFloatRow = pImage1f->floatRowPtr(y);

						for (unsigned int x = 0; x < infra.width; x++)
						{
							unsigned char red = *pLineData++;
							unsigned char green = *pLineData++;
							unsigned char blue = *pLineData++;
							pLineData++;

							float r = ColourSpace::convertSRGBToLinearLUT(red);
							float g = ColourSpace::convertSRGBToLinearLUT(green);
							float b = ColourSpace::convertSRGBToLinearLUT(blue);

							Colour3f finalColour(r, g, b);

							*pFloatRow = finalColour.brightness();

							pFloatRow++;
						}
					}
				}
				else // exact - assumes greyscale for displacement
				{
					for (unsigned int i = 0; i < infra.height; i++)
					{
						png_byte* pLineData = infra.pRows[i];

						// need to flip the height round...
						unsigned int y = infra.height - i - 1;

						float* pFloatRow = pImage1f->floatRowPtr(y);

						for (unsigned int x = 0; x < infra.width; x++)
						{
							unsigned char red = *pLineData++;
							unsigned char green = *pLineData++;
							unsigned char blue = *pLineData++;
							pLineData++;

							float average = (float)red + (float)green + (float)blue;
							average *= 0.3333333333f;

							average /= 255.0f;

							*pFloatRow = average;

							pFloatRow++;
						}
					}
				}
			}
			else
			{
				// it's only got one channel, so just bring that in as raw value...
				for (unsigned int i = 0; i < infra.height; i++)
				{
					png_byte* pLineData = infra.pRows[i];

					// need to flip the height round...
					unsigned int y = infra.height - i - 1;

					float* pFloatRow = pImage1f->floatRowPtr(y);

					for (unsigned int x = 0; x < infra.width; x++)
					{
						unsigned char value = *pLineData++;

						*pFloatRow = (value == 0) ? 0 : (value * (1.0f / 255.0f));

						pFloatRow++;
					}
				}
			}
		}
		else
		{
			// copy over directly
			if (actualType == eRGBA)
			{
				if (requiredTypeFlags & Image::IMAGE_FLAGS_ALPHA)
				{
					// we just want the alpha channel from the RGBA
					for (unsigned int i = 0; i < infra.height; i++)
					{
						png_byte* pLineData = infra.pRows[i];

						// need to flip the height round...
						unsigned int y = infra.height - i - 1;

						unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

						for (unsigned int x = 0; x < infra.width; x++)
						{
							pLineData++;
							pLineData++;
							pLineData++;
							unsigned char alpha = *pLineData++;

							*pUCharRow = alpha;

							pUCharRow++;
						}
					}
				}
				else if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
				{
					// otherwise, get the brightness of the RGB
					// copy data across...
					for (unsigned int i = 0; i < infra.height; i++)
					{
						png_byte* pLineData = infra.pRows[i];

						// need to flip the height round...
						unsigned int y = infra.height - i - 1;

						unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

						for (unsigned int x = 0; x < infra.width; x++)
						{
							unsigned char red = *pLineData++;
							unsigned char green = *pLineData++;
							unsigned char blue = *pLineData++;
							pLineData++;

							Colour3b finalColour(red, green, blue);

							*pUCharRow = finalColour.brightness();

							pUCharRow++;
						}
					}
				}
				else // exact - assumes greyscale for displacement
				{
					for (unsigned int i = 0; i < infra.height; i++)
					{
						png_byte* pLineData = infra.pRows[i];

						// need to flip the height round...
						unsigned int y = infra.height - i - 1;

						unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

						for (unsigned int x = 0; x < infra.width; x++)
						{
							unsigned char red = *pLineData++;
							unsigned char green = *pLineData++;
							unsigned char blue = *pLineData++;
							pLineData++;

							float average = (float)red + (float)green + (float)blue;
							average *= 0.3333333333f;

							*pUCharRow = (unsigned char)average;

							pUCharRow++;
						}
					}
				}
			}
			else
			{
				// it's only got one channel, so just bring that in as raw value...
				for (unsigned int i = 0; i < infra.height; i++)
				{
					png_byte* pLineData = infra.pRows[i];

					// need to flip the height round...
					unsigned int y = infra.height - i - 1;

					unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

					for (unsigned int x = 0; x < infra.width; x++)
					{
						unsigned char value = *pLineData++;

						*pUCharRow = value;

						pUCharRow++;
					}
				}
			}
		}
	}

	png_destroy_read_struct(&infra.pPNG, &infra.pInfo, (png_infopp)NULL);

	for (unsigned int y = 0; y < infra.height; y++)
	{
		delete [] infra.pRows[y];
	}
	delete [] infra.pRows;

	fclose(infra.pFile);

	Image* pFinalImage = (makeFloat) ? static_cast<Image*>(pImage1f) : static_cast<Image*>(pImage1b);

	return pFinalImage;
}

} // namespace Imagine

namespace
{
	Imagine::ImageReader* createImageReaderPNG()
	{
		return new Imagine::ImageReaderPNG();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerImageReader("png", createImageReaderPNG);
}
