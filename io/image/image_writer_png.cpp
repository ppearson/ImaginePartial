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

#include "image_writer_png.h"

#include <stdio.h>

#include <png.h>

#include "image/output_image.h"

#include "colour/colour_space.h"

#include "global_context.h"

#include "utils/maths/maths.h"

namespace Imagine
{

ImageWriterPNG::ImageWriterPNG() : ImageWriter()
{
}

bool ImageWriterPNG::writeImage(const std::string& filePath, const OutputImage& image, unsigned int channels, unsigned int flags)
{
	unsigned int width = image.getWidth();
	unsigned int height = image.getHeight();

	png_structp pPNG;
	png_infop pInfo;

	png_byte colourType = (channels & ImageWriter::ALPHA) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
	png_byte bitDepth = 8;

	pPNG = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pPNG)
		return false;

	pInfo = png_create_info_struct(pPNG);
	if (!pInfo)
	{
		png_destroy_write_struct(&pPNG, NULL);
		return false;
	}

	/// pre-process the file

	FILE* pFile = fopen(filePath.c_str(), "wb");
	if (!pFile)
	{
		png_destroy_write_struct(&pPNG, NULL);
		GlobalContext::instance().getLogger().error("Error writing file: %s", filePath.c_str());
		return false;
	}

	png_byte** pRows = new png_byte*[height * sizeof(png_byte*)];
	if (!pRows)
	{
		png_destroy_write_struct(&pPNG, NULL);
		fclose(pFile);
		return false;
	}

	if (setjmp(png_jmpbuf(pPNG)))
	{
		delete [] pRows;
		png_destroy_write_struct(&pPNG, NULL);
		fclose(pFile);
		return false;
	}

	if (channels & ImageWriter::ALPHA)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			uint8_t* row = new uint8_t[width * 4];
			if (!row)
			{
				delete [] pRows;
				// TODO: we're leaking stuff here...
				fclose(pFile);
				return false;
			}

			pRows[y] = row;
			const Colour4f* pRow = image.colourRowPtr(y);

			for (unsigned int x = 0; x < width; x++)
			{
				float r = ColourSpace::convertLinearToSRGBAccurate(pRow->r);
				float g = ColourSpace::convertLinearToSRGBAccurate(pRow->g);
				float b = ColourSpace::convertLinearToSRGBAccurate(pRow->b);

				unsigned char red = clamp(r) * 255;
				unsigned char green = clamp(g) * 255;
				unsigned char blue = clamp(b) * 255;
				unsigned char alpha = clamp(pRow->a) * 255;

				*row++ = red;
				*row++ = green;
				*row++ = blue;
				*row++ = alpha;

				pRow++;
			}
		}
	}
	else
	{
		for (unsigned int y = 0; y < height; y++)
		{
			uint8_t* row = new uint8_t[width * 3];
			if (!row)
			{
				delete [] pRows;

				// TODO: we're leaking stuff here...
				fclose(pFile);
				return false;
			}

			pRows[y] = row;
			const Colour4f* pRow = image.colourRowPtr(y);

			for (unsigned int x = 0; x < width; x++)
			{
				float r = ColourSpace::convertLinearToSRGBAccurate(pRow->r);
				float g = ColourSpace::convertLinearToSRGBAccurate(pRow->g);
				float b = ColourSpace::convertLinearToSRGBAccurate(pRow->b);

				unsigned char red = r * 255;
				unsigned char green = g * 255;
				unsigned char blue = b * 255;

				*row++ = red;
				*row++ = green;
				*row++ = blue;

				pRow++;
			}
		}
	}

	///



	png_init_io(pPNG, pFile);

	png_set_IHDR(pPNG, pInfo, width, height, bitDepth, colourType, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(pPNG, pInfo);

	png_write_image(pPNG, pRows);

	png_write_end(pPNG, NULL);

	for (unsigned int y = 0; y < height; y++)
	{
		delete [] pRows[y];
	}
	delete [] pRows;

	png_destroy_write_struct(&pPNG, NULL);

	fclose(pFile);

	return true;
}

} // namespace Imagine

namespace
{
	Imagine::ImageWriter* createImageWriterPNG()
	{
		return new Imagine::ImageWriterPNG();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerImageWriter("png", createImageWriterPNG);
}
