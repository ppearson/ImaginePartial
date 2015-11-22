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

#ifndef IMAGE_READER_TGA_H
#define IMAGE_READER_TGA_H

#include "io/image_reader.h"

class ImageReaderTGA : public ImageReader
{
public:
	ImageReaderTGA();

	virtual Image* readColourImage(const std::string& filePath, unsigned int requiredTypeFlags);

	// reads in a float image for either brightness (bump mapping) or alpha
	virtual Image* readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags);

	virtual bool supportsByteOnly() const
	{
		return true;
	}

protected:

	typedef struct
	{
	   char  idLength;
	   char  colourMapType;
	   char  dataTypeCode;
	   short int colourMapOrigin;
	   short int colourMapLength;
	   char  colourMapDepth;
	   short int xOrigin;
	   short int yOrigin;
	   short width;
	   short height;
	   char  bitsPerPixel;
	   char  imageDescriptor;
	} TGAHeader;

	typedef struct
	{
	   unsigned char r,g,b,a;
	} TGAPixel;

	struct TGAInfra
	{
		FILE*			pFile;
		TGAHeader		header;
		TGAPixel*		pBuffer;

		bool			flipY;
	};

	bool readData(const std::string& filePath, TGAInfra& infra);
	void extractPixelValues(unsigned char* pixel, TGAPixel* finalPixels, unsigned int bytes);
};

#endif // IMAGE_READER_TGA_H
