/*
 Imagine
 Copyright 2015 Peter Pearson.

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

#ifndef IMAGE_READER_TEST_H
#define IMAGE_READER_TEST_H

#include "io/image_reader.h"

namespace Imagine
{

class ImageReaderTXT : public ImageReader
{
public:
	ImageReaderTXT();

	virtual Image* readColourImage(const std::string& filePath, unsigned int requiredTypeFlags)
	{
		return nullptr;
	}

	virtual Image* readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags)
	{
		return nullptr;
	}

	virtual bool supportsPartialReading() const
	{
		return true;
	}

	virtual bool readImageDetails(const std::string& filePath, ImageTextureDetails& textureDetails) const;

	virtual bool readImageTile(const ImageTextureTileReadParams& readParams, ImageTextureTileReadResults& readResults) const;
};

} // namespace Imagine

#endif // IMAGE_READER_TEST_H
