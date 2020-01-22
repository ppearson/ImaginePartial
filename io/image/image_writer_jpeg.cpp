/*
 Imagine
 Copyright 2018 Peter Pearson.

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

#include "image_writer_jpeg.h"

#include <cstdio>

#include <jpeglib.h>

#include "image/output_image.h"

#include "colour/colour_space.h"

#include "global_context.h"

#include "utils/maths/maths.h"

namespace Imagine
{

ImageWriterJPEG::ImageWriterJPEG() : ImageWriter()
{
	
}

bool ImageWriterJPEG::writeImage(const std::string& filePath, const OutputImage& image, unsigned int channels, unsigned int flags)
{
	unsigned int width = image.getWidth();
	unsigned int height = image.getHeight();
	
	struct jpeg_compress_struct compressInfo;
	struct jpeg_error_mgr jerr;
	
	jpeg_create_compress(&compressInfo);
	compressInfo.err = jpeg_std_error(&jerr);
	
	float quality = 0.95f;	
	
	FILE* pFile = fopen(filePath.c_str(), "wb");
	if (!pFile)
	{
		jpeg_destroy_compress(&compressInfo);
		GlobalContext::instance().getLogger().error("Error writing file: %s", filePath.c_str());
		return false;
	}
	
	jpeg_stdio_dest(&compressInfo, pFile);
	
	compressInfo.image_width = width;
	compressInfo.image_height = height;
	compressInfo.input_components = 3;
	compressInfo.in_color_space = JCS_RGB;
	
	jpeg_set_defaults(&compressInfo);
	
	// set actual options we want...
	int intQuality = (int)(quality * 100.0f);
	jpeg_set_quality(&compressInfo, intQuality, TRUE);
	
	unsigned int chromaSubSamplingType = 0;
	// 0 = 4:1:1, 1 = 4:2:2, 2 = 4:4:4
	compressInfo.comp_info[0].h_samp_factor = (chromaSubSamplingType < 2) ? 2 : 1;
	compressInfo.comp_info[0].v_samp_factor = (chromaSubSamplingType < 1) ? 2 : 1;
	
	jpeg_start_compress(&compressInfo, TRUE);
	
	// allocate for only one scanline currently
	unsigned char* pTempRow = new unsigned char[width * 3];
	
	JSAMPROW rowPointer[1];
	
	for (unsigned int y = 0; y < height; y++)
	{
		const Colour4f* pRow = image.colourRowPtr(y);
		
		unsigned char* pTempScanline = pTempRow;

		for (unsigned int x = 0; x < width; x++)
		{
			float r = ColourSpace::convertLinearToSRGBAccurate(pRow->r);
			float g = ColourSpace::convertLinearToSRGBAccurate(pRow->g);
			float b = ColourSpace::convertLinearToSRGBAccurate(pRow->b);

			unsigned char red = clamp(r) * 255;
			unsigned char green = clamp(g) * 255;
			unsigned char blue = clamp(b) * 255;

			*pTempScanline++ = red;
			*pTempScanline++ = green;
			*pTempScanline++ = blue;

			pRow++;
		}
		
		rowPointer[0] = pTempRow;
		jpeg_write_scanlines(&compressInfo, rowPointer, 1);
	}
	
	jpeg_finish_compress(&compressInfo);
	
	delete [] pTempRow;
	
	fclose(pFile);
	
	jpeg_destroy_compress(&compressInfo);
	
	return true;	
}

} // namespace Imagine

namespace
{
	Imagine::ImageWriter* createImageWriterJPEG()
	{
		return new Imagine::ImageWriterJPEG();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerImageWriter("jpg", createImageWriterJPEG);
}

