/*
 Imagine
 Copyright 2011-2020 Peter Pearson.

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

#include <cstdio>

#include <png.h>

#include "image/output_image.h"

#include "colour/colour_space.h"

#include "global_context.h"

#include "utils/io/data_conversion.h"
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
	
	// be very cheeky and assume we want to write 16-bit PNGs if full float precision was requested.
	bool save16Bit = (flags & ImageWriter::FLOAT32);

	png_byte colourType = (channels & ImageWriter::ALPHA) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
	png_byte bitDepth = save16Bit ? 16 : 8;

	pPNG = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!pPNG)
		return false;

	pInfo = png_create_info_struct(pPNG);
	if (!pInfo)
	{
		png_destroy_write_struct(&pPNG, nullptr);
		return false;
	}

	/// pre-process the file

	FILE* pFile = fopen(filePath.c_str(), "wb");
	if (!pFile)
	{
		png_destroy_write_struct(&pPNG, &pInfo);
		GlobalContext::instance().getLogger().error("Error writing PNG file: %s", filePath.c_str());
		return false;
	}

	png_byte** pRows = new png_byte*[height * sizeof(png_byte*)];
	if (!pRows)
	{
		png_destroy_write_struct(&pPNG, &pInfo);
		fclose(pFile);
		return false;
	}

	if (setjmp(png_jmpbuf(pPNG)))
	{
		delete [] pRows;
		png_destroy_write_struct(&pPNG, &pInfo);
		fclose(pFile);
		return false;
	}
	
	png_init_io(pPNG, pFile);

	png_set_IHDR(pPNG, pInfo, width, height, bitDepth, colourType, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(pPNG, pInfo);
	
	size_t pixelBytes = save16Bit ? 2 : 1;
	size_t byteCount = width * pixelBytes;
	
	if (channels & ImageWriter::ALPHA)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			uint8_t* row = new uint8_t[byteCount * 4];	
			pRows[y] = row;
			const Colour4f* pRow = image.colourRowPtr(y);
			
			if (save16Bit)
			{
				uint16_t* typedRow = (uint16_t*)row;
				for (unsigned int x = 0; x < width; x++)
				{
					float r = ColourSpace::convertLinearToSRGBAccurate(pRow->r);
					float g = ColourSpace::convertLinearToSRGBAccurate(pRow->g);
					float b = ColourSpace::convertLinearToSRGBAccurate(pRow->b);
	
					uint16_t red = clamp(r) * 65535;
					uint16_t green = clamp(g) * 65535;
					uint16_t blue = clamp(b) * 65535;
					uint16_t alpha = clamp(pRow->a) * 65535;
	
					// TODO: this reversing should only be done on marchs which need it...
					*typedRow++ = reverseUInt16Bytes(red);
					*typedRow++ = reverseUInt16Bytes(green);
					*typedRow++ = reverseUInt16Bytes(blue);
					*typedRow++ = reverseUInt16Bytes(alpha);
	
					pRow++;
				}
			}
			else
			{
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
	}
	else
	{
		for (unsigned int y = 0; y < height; y++)
		{
			uint8_t* row = new uint8_t[byteCount * 3];	
			pRows[y] = row;
			const Colour4f* pRow = image.colourRowPtr(y);
			
			if (save16Bit)
			{
				uint16_t* typedRow = (uint16_t*)row;
				for (unsigned int x = 0; x < width; x++)
				{
					float r = ColourSpace::convertLinearToSRGBAccurate(pRow->r);
					float g = ColourSpace::convertLinearToSRGBAccurate(pRow->g);
					float b = ColourSpace::convertLinearToSRGBAccurate(pRow->b);
	
					uint16_t red = clamp(r) * 65535;
					uint16_t green = clamp(g) * 65535;
					uint16_t blue = clamp(b) * 65535;
	
					// TODO: this reversing should only be done on marchs which need it...
					*typedRow++ = reverseUInt16Bytes(red);
					*typedRow++ = reverseUInt16Bytes(green);
					*typedRow++ = reverseUInt16Bytes(blue);
	
					pRow++;
				}
			}
			else
			{
				for (unsigned int x = 0; x < width; x++)
				{
					float r = ColourSpace::convertLinearToSRGBAccurate(pRow->r);
					float g = ColourSpace::convertLinearToSRGBAccurate(pRow->g);
					float b = ColourSpace::convertLinearToSRGBAccurate(pRow->b);
	
					unsigned char red = clamp(r) * 255;
					unsigned char green = clamp(g) * 255;
					unsigned char blue = clamp(b) * 255;
	
					*row++ = red;
					*row++ = green;
					*row++ = blue;
	
					pRow++;
				}
			}
		}
	}
	
	png_write_image(pPNG, pRows);

	for (unsigned int y = 0; y < height; y++)
	{
		delete [] pRows[y];
	}
	delete [] pRows;

	///
	
	png_write_end(pPNG, pInfo); // info ptr isn't really needed, but...

	png_destroy_write_struct(&pPNG, &pInfo);

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
