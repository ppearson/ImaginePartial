/*
 Imagine
 Copyright 2014-2015 Peter Pearson.

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

#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

namespace Imagine
{

class ImageColour3f;
class ImageColour3h;
class Image1f;
class Image1h;
class Image;

class ImageUtils
{
public:
	ImageUtils();

	static bool flipImageVertically(Image* pImage);
	static void flipImageTileVertically(unsigned char* pData, unsigned int pixelStride, unsigned int width, unsigned int height);

protected:

	static void flipImageVertically(ImageColour3f* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes);
	static void flipImageVertically(ImageColour3h* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes);

	static void flipImageVertically(Image1f* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes);
	static void flipImageVertically(Image1h* pImage, unsigned int scanlines, unsigned int numRows, unsigned int width, unsigned int scanlineBytes);

};

} // namespace Imagine

#endif // IMAGE_UTILS_H
