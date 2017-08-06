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

#include "image_reader_jpeg.h"

#include <jpeglib.h>

#include "global_context.h"

#include "image/image_1f.h"
#include "image/image_1b.h"
#include "image/image_colour3f.h"
#include "image/image_colour3b.h"

#include "colour/colour_space.h"

namespace Imagine
{

ImageReaderJPEG::ImageReaderJPEG()
{
}

Image* ImageReaderJPEG::readColourImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile)
	{
		GlobalContext::instance().getLogger().error("Can't open file: %s", filePath.c_str());
		return NULL;
	}

	struct jpeg_decompress_struct cinfo;

	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, pFile);
	jpeg_read_header(&cinfo, TRUE);
	if (jpeg_start_decompress(&cinfo) != TRUE)
	{
		GlobalContext::instance().getLogger().error("Cannot open file: %s", filePath.c_str());
		fclose(pFile);
		return NULL;
	}

	int depth = cinfo.output_components;

	unsigned int width = cinfo.output_width;
	unsigned int height = cinfo.output_height;

	unsigned char** pScanlines = new unsigned char*[height];

	if (!pScanlines)
	{
		GlobalContext::instance().getLogger().error("Cannot allocate memory to read file: %s", filePath.c_str());
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(pFile);
		return NULL;
	}

	ImageColour3f* pImage3f = NULL;
	ImageColour3b* pImage3b = NULL;

	const bool makeFloat = !(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE);

	if (makeFloat)
	{
		pImage3f = new ImageColour3f(width, height, false);
	}
	else
	{
		pImage3b = new ImageColour3b(width, height, false);
	}

	if (!pImage3f && !pImage3b)
	{
		GlobalContext::instance().getLogger().error("Can't allocate memory for image...");
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(pFile);

		delete [] pScanlines;

		return NULL;
	}

	// read all the scanlines
	for (unsigned int i = 0; i < height; i++)
	{
		// TODO: this could fail too...
		pScanlines[i] = new unsigned char[width * depth];
	}

	unsigned int linesRead = 0;

	while (linesRead < height)
		linesRead += jpeg_read_scanlines(&cinfo, &pScanlines[linesRead], height - linesRead);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	static const float inv255 = 1.0f / 255.0f;

	unsigned char* pScanlineBuffer;

	if (makeFloat)
	{
		for (unsigned int i = 0; i < height; i++)
		{
			pScanlineBuffer = pScanlines[i];
			// need to flip the height round...
			unsigned int y = height - i - 1;

			Colour3f* pImageRow = pImage3f->colourRowPtr(y);

			if (depth == 3)
			{
				for (unsigned int x = 0; x < width; x++)
				{
					unsigned char red = *pScanlineBuffer++;
					unsigned char green = *pScanlineBuffer++;
					unsigned char blue = *pScanlineBuffer++;

					pImageRow->r = ColourSpace::convertSRGBToLinearLUT(red);
					pImageRow->g = ColourSpace::convertSRGBToLinearLUT(green);
					pImageRow->b = ColourSpace::convertSRGBToLinearLUT(blue);

					pImageRow++;
				}
			}
			else if (depth == 1)
			{
				// TODO: why is this here?
				for (unsigned int x = 0; x < width; x++)
				{
					unsigned char gray = *pScanlineBuffer++;

					float value = float(gray) * inv255;

					pImageRow->r = value;
					pImageRow->g = value;
					pImageRow->b = value;

					pImageRow++;
				}
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < height; i++)
		{
			pScanlineBuffer = pScanlines[i];
			// need to flip the height round...
			unsigned int y = height - i - 1;

			Colour3b* pImageRow = pImage3b->colour3bRowPtr(y);

			if (depth == 3)
			{
				for (unsigned int x = 0; x < width; x++)
				{
					pImageRow->r = *pScanlineBuffer++;
					pImageRow->g = *pScanlineBuffer++;
					pImageRow->b = *pScanlineBuffer++;

					pImageRow++;
				}
			}
			else if (depth == 1)
			{
				// TODO: why is this here?
				for (unsigned int x = 0; x < width; x++)
				{
					unsigned char gray = *pScanlineBuffer++;

					// TODO: this will still be colour corrected...

					pImageRow->r = gray;
					pImageRow->g = gray;
					pImageRow->b = gray;

					pImageRow++;
				}
			}
		}
	}

	for (unsigned i = 0; i < height; i++)
	{
		unsigned char* pLine = pScanlines[i];
		delete [] pLine;
	}

	delete [] pScanlines;

	fclose(pFile);

	Image* pFinalImage = (makeFloat) ? static_cast<Image*>(pImage3f) : static_cast<Image*>(pImage3b);

	return pFinalImage;
}


Image* ImageReaderJPEG::readColourImageAndByteCopy(const std::string& filePath, ImageColour3b* pImageColour3b, unsigned int requiredTypeFlags)
{
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile)
	{
		GlobalContext::instance().getLogger().error("Can't open file: %s", filePath.c_str());
		return NULL;
	}

	struct jpeg_decompress_struct cinfo;

	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, pFile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	int channels = cinfo.output_components;

	unsigned int width = cinfo.output_width;
	unsigned int height = cinfo.output_height;

	unsigned char** pScanlines = new unsigned char*[height];

	ImageColour3f* pImage = new ImageColour3f(width, height, false);

	if (pImage)
	{
		if (!pImageColour3b->initialise(width, height))
		{
			delete [] pScanlines;

			delete pImage;

			fclose(pFile);

			return NULL;
		}

		// read all the scanlines
		for (unsigned int i = 0; i < height; i++)
		{
			// TODO: this could fail...
			pScanlines[i] = new unsigned char[width * channels];
		}

		unsigned int linesRead = 0;

		while (linesRead < height)
			linesRead += jpeg_read_scanlines(&cinfo, &pScanlines[linesRead], height - linesRead);

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		const float inv255 = 1.0f / 255.0f;

		unsigned char* pScanlineBuffer;

		for (unsigned int i = 0; i < height; i++)
		{
			pScanlineBuffer = pScanlines[i];
			// need to flip the height round...
			unsigned int y = height - i - 1;

			Colour3f* pImageRow = pImage->colourRowPtr(y);
			Colour3b* pByteRow = pImageColour3b->colour3bRowPtr(y);

			if (channels == 3)
			{
				for (unsigned int x = 0; x < width; x++)
				{
					unsigned char red = *pScanlineBuffer++;
					unsigned char green = *pScanlineBuffer++;
					unsigned char blue = *pScanlineBuffer++;

					Colour3b newColour(red, green, blue);

					*pByteRow = newColour;

					pImageRow->r = ColourSpace::convertSRGBToLinearLUT(red);
					pImageRow->g = ColourSpace::convertSRGBToLinearLUT(green);
					pImageRow->b = ColourSpace::convertSRGBToLinearLUT(blue);

					pImageRow++;
					pByteRow++;
				}
			}
			else if (channels == 1)
			{
				for (unsigned int x = 0; x < width; x++)
				{
					unsigned char gray = *pScanlineBuffer++;

					Colour3b newColour(gray, gray, gray);

					*pByteRow = newColour;

					float value = float(gray) * inv255;

					pImageRow->r = value;
					pImageRow->g = value;
					pImageRow->b = value;

					pImageRow++;
					pByteRow++;
				}
			}
		}
	}

	for (unsigned i = 0; i < height; i++)
	{
		unsigned char* pLine = pScanlines[i];
		delete [] pLine;
	}

	delete [] pScanlines;

	fclose(pFile);

	return pImage;
}

Image* ImageReaderJPEG::readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile)
	{
		GlobalContext::instance().getLogger().error("Can't open file: %s", filePath.c_str());
		return NULL;
	}

	struct jpeg_decompress_struct cinfo;

	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, pFile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	int channels = cinfo.output_components;

	unsigned int width = cinfo.output_width;
	unsigned int height = cinfo.output_height;

	unsigned char** pScanlines = new unsigned char*[height];

	const bool makeFloat = !(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE);

	Image1f* pImage1f = NULL;
	Image1b* pImage1b = NULL;

	if (makeFloat)
	{
		pImage1f = new Image1f(width, height, false);
	}
	else
	{
		pImage1b = new Image1b(width, height, false);
	}

	if (pImage1f || pImage1b)
	{
		unsigned char* pScanlineBuffer;
		// read all the scanlines
		for (unsigned int i = 0; i < height; i++)
		{
			pScanlines[i] = new unsigned char[width * channels];
		}

		unsigned int linesRead = 0;

		while (linesRead < height)
			linesRead += jpeg_read_scanlines(&cinfo, &pScanlines[linesRead], height - linesRead);

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		if (channels == 3)
		{
			if (makeFloat)
			{
				const float inv255 = 1.0f / 255.0f;
				for (unsigned int i = 0; i < height; i++)
				{
					pScanlineBuffer = pScanlines[i];
					// need to flip the height round...
					unsigned int y = height - i - 1;

					float* pFloatRow = pImage1f->floatRowPtr(y);

					if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
					{
						for (unsigned int x = 0; x < width; x++)
						{
							unsigned char red = *pScanlineBuffer++;
							unsigned char green = *pScanlineBuffer++;
							unsigned char blue = *pScanlineBuffer++;

							float r = ColourSpace::convertSRGBToLinearLUT(red);
							float g = ColourSpace::convertSRGBToLinearLUT(green);
							float b = ColourSpace::convertSRGBToLinearLUT(blue);

							Colour3f finalColour(r, g, b);

							*pFloatRow = finalColour.brightness();

							pFloatRow++;
						}
					}
					else // type EXACT - assumes greyscale for displacement
					{
						for (unsigned int x = 0; x < width; x++)
						{
							unsigned char red = *pScanlineBuffer++;
							unsigned char green = *pScanlineBuffer++;
							unsigned char blue = *pScanlineBuffer++;

							float average = (float)red + (float)green + (float)blue;
							average *= 0.3333333333f;

							average *= inv255;

							*pFloatRow = average;

							pFloatRow++;
						}
					}
				}
			}
			else
			{
				for (unsigned int i = 0; i < height; i++)
				{
					pScanlineBuffer = pScanlines[i];
					// need to flip the height round...
					unsigned int y = height - i - 1;

					unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

					if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
					{
						for (unsigned int x = 0; x < width; x++)
						{
							unsigned char red = *pScanlineBuffer++;
							unsigned char green = *pScanlineBuffer++;
							unsigned char blue = *pScanlineBuffer++;

							Colour3b finalColour(red, green, blue);

							*pUCharRow = finalColour.brightness();

							pUCharRow++;
						}
					}
					else // type EXACT - assumes greyscale for displacement
					{
						for (unsigned int x = 0; x < width; x++)
						{
							unsigned char red = *pScanlineBuffer++;
							unsigned char green = *pScanlineBuffer++;
							unsigned char blue = *pScanlineBuffer++;

							unsigned short average = red + green + blue;
							average *= 0.3333333333f;

							*pUCharRow = average;

							pUCharRow++;
						}
					}
				}
			}
		}
		else // assume 1 for greyscale
		{
			if (makeFloat)
			{
				const float inv255 = (1.0f / 255.0f);
				// we don't need to differenciate between brightness, exact here as they'll all be the same.
				// we also don't need to worry about accurately converting to linear colour space...
				for (unsigned int i = 0; i < height; i++)
				{
					pScanlineBuffer = pScanlines[i];
					// need to flip the height round...
					unsigned int y = height - i - 1;

					float* pFloatRow = pImage1f->floatRowPtr(y);

					for (unsigned int x = 0; x < width; x++)
					{
						unsigned char value = *pScanlineBuffer++;

						*pFloatRow = (float)value * inv255;

						pFloatRow++;
					}
				}
			}
			else
			{
				// we don't need to differenciate between brightness, exact here as they'll all be the same.
				for (unsigned int i = 0; i < height; i++)
				{
					pScanlineBuffer = pScanlines[i];
					// need to flip the height round...
					unsigned int y = height - i - 1;

					unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

					for (unsigned int x = 0; x < width; x++)
					{
						*pUCharRow = *pScanlineBuffer++;

						pUCharRow++;
					}
				}
			}
		}
	}

	for (unsigned i = 0; i < height; i++)
	{
		unsigned char* pLine = pScanlines[i];
		delete [] pLine;
	}

	delete [] pScanlines;

	fclose(pFile);

	Image* pFinalImage = (makeFloat) ? static_cast<Image*>(pImage1f) : static_cast<Image*>(pImage1b);

	return pFinalImage;
}

} // namespace Imagine

namespace
{
	Imagine::ImageReader* createImageReaderJPEG()
	{
		return new Imagine::ImageReaderJPEG();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerImageReaderMultipleExtensions("jpg;jpeg", createImageReaderJPEG);
}
