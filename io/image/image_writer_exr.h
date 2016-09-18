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

#ifndef IMAGE_WRITER_EXR_H
#define IMAGE_WRITER_EXR_H

#include "io/image_writer.h"

namespace Imagine
{

class ImageWriterEXR : public ImageWriter
{
public:
	ImageWriterEXR();

	virtual bool writeImage(const std::string& filePath, const OutputImage& image, unsigned int channels, unsigned int flags);

	template <typename T>
	bool writeStandardImage(const std::string& filePath, const OutputImage& image, unsigned int channels, bool fullFloat);

	bool writeDeepImage(const std::string& filePath, const OutputImage& image, unsigned int channels);
};

} // namespace Imagine

#endif // IMAGE_WRITER_EXR_H
