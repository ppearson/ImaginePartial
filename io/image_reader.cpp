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

#include "image_reader.h"

#include "image/image_colour3f.h"
#include "image/image_colour3h.h"
#include "image/image_1f.h"
#include "image/image_1h.h"
#include "image/image_colour3b.h"
#include "colour/colour_space.h"

namespace Imagine
{

ImageReader::ImageReader()
{
}

ImageReader::~ImageReader()
{
}

Image* ImageReader::readColourImageAndByteCopy(const std::string& filePath, ImageColour3b* pImageColour3b, unsigned int requiredTypeFlags)
{
	Image* pColourImage = readColourImage(filePath, requiredTypeFlags);
	if (!pColourImage)
		return nullptr;

	// see if we've got a byte format image already...
	if (pColourImage->getImageType() & Image::IMAGE_FORMAT_BYTE)
	{
		// just return the main image without initialising the Byte copy...
		return pColourImage;
	}

	if (!pImageColour3b)
		return pColourImage;

	// otherwise, work out if it's half format or full float
	if (pColourImage->getImageType() & Image::IMAGE_FORMAT_FLOAT)
	{
		ImageColour3f* pColour3fImage = static_cast<ImageColour3f*>(pColourImage);

		unsigned width = pColour3fImage->getWidth();
		unsigned height = pColour3fImage->getHeight();

		pImageColour3b->initialise(width, height);

		for (unsigned int y = 0; y < height; y++)
		{
			Colour3f* pColourRow = pColour3fImage->colourRowPtr(y);
			Colour3b* pByteRow = pImageColour3b->colour3bRowPtr(y);

			for (unsigned int x = 0; x < width; x++)
			{
				Colour3f colour = *pColourRow;
				ColourSpace::convertLinearToSRGBFast(colour);

				colour.clamp();

				*pByteRow = Colour3b((unsigned char)(colour.r * 255.0f), (unsigned char)(colour.g * 255.0f), (unsigned char)(colour.b * 255.0f));

				pColourRow++;
				pByteRow++;
			}
		}
	}
	else if (pColourImage->getImageType() & Image::IMAGE_FORMAT_HALF)
	{
		ImageColour3h* pColour3hImage = static_cast<ImageColour3h*>(pColourImage);

		unsigned width = pColour3hImage->getWidth();
		unsigned height = pColour3hImage->getHeight();

		pImageColour3b->initialise(width, height);

		for (unsigned int y = 0; y < height; y++)
		{
			Colour3h* pColourRow = pColour3hImage->colour3hRowPtr(y);
			Colour3b* pByteRow = pImageColour3b->colour3bRowPtr(y);

			for (unsigned int x = 0; x < width; x++)
			{
				Colour3h colour = *pColourRow;
				ColourSpace::convertLinearToSRGBFast(colour);

				colour.clamp();

				*pByteRow = Colour3b((unsigned char)(colour.r * 255.0f), (unsigned char)(colour.g * 255.0f), (unsigned char)(colour.b * 255.0f));

				pColourRow++;
				pByteRow++;
			}
		}
	}

	return pColourImage;
}

Image* ImageReader::readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	Image* pColourImage = readColourImage(filePath, requiredTypeFlags);
	if (!pColourImage)
		return nullptr;

	if (pColourImage->getImageType() & Image::IMAGE_FORMAT_FLOAT)
	{
		ImageColour3f* pColour3fImage = dynamic_cast<ImageColour3f*>(pColourImage);
		if (!pColour3fImage)
			return nullptr;

		Image1f* pFloatImage = new Image1f(*pColour3fImage, requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS);
		if (pFloatImage)
		{
			delete pColourImage;
			return pFloatImage;
		}
	}
	else if (pColourImage->getImageType() & Image::IMAGE_FORMAT_HALF)
	{
		ImageColour3h* pColour3hImage = dynamic_cast<ImageColour3h*>(pColourImage);
		if (!pColour3hImage)
			return nullptr;

		Image1h* pHalfImage = new Image1h(*pColour3hImage, requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS);
		if (pHalfImage)
		{
			delete pColourImage;
			return pHalfImage;
		}
	}

	return nullptr;
}

} // namespace Imagine
