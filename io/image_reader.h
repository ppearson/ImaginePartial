/*
 Imagine
 Copyright 2011-2015 Peter Pearson.

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

#ifndef IMAGE_READER_H
#define IMAGE_READER_H

#include <string>

#include "file_io_registry.h"

#include "image/image_texture_common.h"
#include "image/image.h"

class ImageColour3b;

class ImageReader
{
public:
	ImageReader();
	virtual ~ImageReader();

	// these are designed for use with loading entire planar images

	// reads in RGB colour image as floats into linear format
	virtual Image* readColourImage(const std::string& filePath, unsigned int requiredTypeFlags) = 0;

	// reads in RGB colour image as floats into linear format, and also pulls in the sRGB byte copy at the same time,
	// hopefully being quicker (so there's no need to reconvert the linear back to sRGB)...
	virtual Image* readColourImageAndByteCopy(const std::string& filePath, ImageColour3b* pImageColour3b, unsigned int requiredTypeFlags);

	// reads in a greyscale image for either brightness (bump mapping), alpha or exact values
	virtual Image* readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags);

	virtual bool supportsByteOnly() const
	{
		return false;
	}

	virtual bool supportsPartialReading() const
	{
		return false;
	}

	// these are designed for on-demand, lazy-loading (paging) of tiled mipmapped images, although unmipmapped images are supported
	// as are scanline, although in Imagine's usage, these configurations are not recommended in terms of performance

	// designed for lazily-loading textures - this will be called once per image texture path,
	// and the result will then be stored permanently in memory for the remainer of the session
	virtual bool readImageDetails(const std::string& filePath, ImageTextureDetails& textureDetails) const
	{
		return false;
	}

	// will read a given texture tile coordinate for a specified mipmap level - pData must be allocated, and is assumed to
	// be a continguous block of memory representing the actual tile size for the data format
	virtual bool readImageTile(const ImageTextureTileReadParams& readParams, ImageTextureTileReadResults& readResults) const
	{
		return false;
	}
};

#endif // IMAGE_READER_H
