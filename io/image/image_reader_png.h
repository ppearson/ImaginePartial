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

#ifndef IMAGE_READER_PNG_H
#define IMAGE_READER_PNG_H

#include "io/image_reader.h"

namespace Imagine
{

struct PNGInfra;

class ImageReaderPNG : public ImageReader
{
public:
	ImageReaderPNG();

	enum ImageType
	{
		eInvalid,
		eRGBA,
		eA
	};

	virtual Image* readColourImage(const std::string& filePath, unsigned int requiredTypeFlags);
	virtual Image* readColourImageAndByteCopy(const std::string& filePath, ImageColour3b* pImageColour3b, unsigned int requiredTypeFlags);

	virtual Image* readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags);

	virtual bool supportsByteOnly() const
	{
		return true;
	}

protected:
	ImageType readData(const std::string& filePath, PNGInfra& infra, bool wantAlpha);

};

} // namespace Imagine

#endif // IMAGE_READER_PNG_H
