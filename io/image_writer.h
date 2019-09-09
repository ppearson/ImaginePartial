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

#ifndef IMAGE_WRITER_H
#define IMAGE_WRITER_H

#include <string>

#include "file_io_registry.h"

namespace Imagine
{

class OutputImage;

class ImageWriter
{
public:
	ImageWriter();
	virtual ~ImageWriter();

	enum ChannelFlags
	{
		RGB =		1 << 0,
		ALPHA =		1 << 1,
		DEPTH =		1 << 2,
		NORMALS =	1 << 3,
		WPP =		1 << 4,
		SHADOWS =	1 << 5,

		DEEP =		1 << 6,

		RGBA = RGB | ALPHA,
		ALL = RGB | ALPHA | DEPTH | NORMALS | WPP | SHADOWS
	};

	enum WriteFlags
	{
		FLOAT32		=	1 << 0
	};

	virtual bool writeImage(const std::string& filePath, const OutputImage& image, unsigned int channels, unsigned int flags) = 0;
};

} // namespace Imagine

#endif // IMAGE_WRITER_H
