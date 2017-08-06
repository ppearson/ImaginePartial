/*
 Imagine
 Copyright 2012-2014 Peter Pearson.

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

#ifndef IMAGE_READER_HDR_H
#define IMAGE_READER_HDR_H

#include "io/image_reader.h"

#include <math.h>

namespace Imagine
{

class Colour3f;

class ImageReaderHDR : public ImageReader
{
public:
	ImageReaderHDR();

	struct RGBE
	{
		unsigned char	R;
		unsigned char	G;
		unsigned char	B;
		unsigned char	E;

		inline unsigned char& operator[](unsigned int i)
		{
			return (&R)[i];
		}
	};

	virtual Image* readColourImage(const std::string& filePath, unsigned int requiredTypeFlags);

protected:
	static bool processScanline(RGBE* pScanline, unsigned int length, FILE* pFile);
	static bool processScanlineOld(RGBE* pScanline, unsigned int length, FILE* pFile);

	static void processColour(const RGBE* pScanline, unsigned int length, Colour3f* pDestImageRow);

};

} // namespace Imagine

#endif // IMAGE_READER_HDR_H
