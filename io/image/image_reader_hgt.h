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

#ifndef IMAGEREADERHGT_H
#define IMAGEREADERHGT_H

#include "io/image_reader.h"

namespace Imagine
{

class ImageReaderHGT : public ImageReader
{
public:
	ImageReaderHGT();

	virtual Image* readColourImage(const std::string& filePath, unsigned int requiredTypeFlags)
	{
		return nullptr;
	}

	// only interested in greyscale for heightmaps...
	virtual Image* readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags);

protected:
};

} // namespace Imagine

#endif // IMAGEREADERHGT_H
