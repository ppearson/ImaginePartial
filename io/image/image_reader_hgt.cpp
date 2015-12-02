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

#include "image_reader_hgt.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "image/image_1f.h"

#include "utils/maths/maths.h"

ImageReaderHGT::ImageReaderHGT()
{

}

Image* ImageReaderHGT::readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile)
	{
		return NULL;
	}

	// work out the res based on file size... - only way there seems to be to do this.
	struct stat fileInfo;
	stat(filePath.c_str(), &fileInfo);
	int fileSize = fileInfo.st_size;

//	unsigned int numRows = 1201;
//	unsigned int numRows = 3601;

	unsigned int numRows = (fileSize == 2884802) ? 1201 : 3601;

	unsigned int imageWidth = numRows;
	unsigned int imageHeight = numRows;

	// TODO: fix lat/long aspect ratio distortion - probably best to do it in a dedicated reader which
	//       creates geo at the same time rather than here...

	Image1f* pNewImage = new Image1f(imageWidth, imageHeight, false);

	if (!pNewImage)
	{
		fclose(pFile);
		return NULL;
	}

	// TODO: which axis is first?
	for (unsigned int x = 0; x < numRows; x++)
	{
//		float* pDst = pNewImage->floatRowPtr()
		for (unsigned int y = 0; y < numRows; y++)
		{
			unsigned char mainVal = 0;
			unsigned char secVal = 0;

			fread(&mainVal, 1, 1, pFile);
			fread(&secVal, 1, 1, pFile);

			float elevation = (float)((float)mainVal * 256.0f) + (float)secVal;
			pNewImage->floatAt(x, y) = elevation;
		}
	}

	fclose(pFile);

	return pNewImage;
}


namespace
{
	ImageReader* createImageReaderHGT()
	{
		return new ImageReaderHGT();
	}

	const bool registered = FileIORegistry::instance().registerImageReader("hgt", createImageReaderHGT);
}
