/*
 Imagine
 Copyright 2013-2014 Peter Pearson.

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

#include "image_reader_tiff.h"

#include <tiffio.h>

#include "image/image_1f.h"
#include "image/image_1b.h"
#include "image/image_colour3f.h"
#include "image/image_colour3h.h"
#include "image/image_colour3b.h"

#include "colour/colour_space.h"

#include "utils/file_helpers.h"

// TODO: there's a fair bit of duplicate code here and it could be re-written in a more compact way, but it's written
//       with as few branches as possible (i.e. not within loops) in the hope compilers might be able to vectorise it more easily

ImageReaderTIFF::ImageReaderTIFF()
{
}

bool ImageReaderTIFF::readInfo(TIFF* pTiff, TiffInfo& tiffInfo)
{
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &tiffInfo.imageHeight);
	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &tiffInfo.imageWidth);
	TIFFGetField(pTiff, TIFFTAG_IMAGEDEPTH, &tiffInfo.imageDepth);

	if (tiffInfo.imageHeight == 0 || tiffInfo.imageWidth == 0)
		return false;

	TIFFGetField(pTiff, TIFFTAG_XPOSITION, &tiffInfo.xPos);
	TIFFGetField(pTiff, TIFFTAG_YPOSITION, &tiffInfo.yPos);

	TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &tiffInfo.bitDepth);
	TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &tiffInfo.channelCount);

	TIFFGetField(pTiff, TIFFTAG_ROWSPERSTRIP, &tiffInfo.rowsPerStrip);
	TIFFGetField(pTiff, TIFFTAG_SAMPLEFORMAT, &tiffInfo.sampleFormat);

	TIFFGetField(pTiff, TIFFTAG_ORIENTATION, &tiffInfo.orientation);

	TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &tiffInfo.compression);

	if (TIFFIsTiled(pTiff))
	{
		tiffInfo.isTiled = true;
		TIFFGetField(pTiff, TIFFTAG_TILEWIDTH, &tiffInfo.tileWidth);
		TIFFGetField(pTiff, TIFFTAG_TILELENGTH, &tiffInfo.tileHeight);
		TIFFGetField(pTiff, TIFFTAG_TILEDEPTH, &tiffInfo.tileDepth);

		// get other stuff
//		if (TIFFGetField(pTiff, TIFFTAG_PIXAR_IMAGEFULLLENGTH, &tiffInfo.imageHeight) == 1)
		{

		}

//		if (TIFFGetField(pTiff, TIFFTAG_PIXAR_IMAGEFULLWIDTH, &tiffInfo.imageWidth) == 1)
		{

		}
	}

	return true;
}

Image* ImageReaderTIFF::readColourImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	TIFF* pTiff = TIFFOpen(filePath.c_str(), "r");
	if (!pTiff)
	{
		fprintf(stderr, "Error reading file: %s\n", filePath.c_str());
		return NULL;
	}

	TiffInfo tiffInfo;
	if (!readInfo(pTiff, tiffInfo))
	{
		fprintf(stderr, "Invalid tiff file: %s\n", filePath.c_str());
		TIFFClose(pTiff);
		return NULL;
	}

	if (tiffInfo.bitDepth < 8)
	{
		fprintf(stderr, "Unsupported TIFF format for file: %s\n", filePath.c_str());
		TIFFClose(pTiff);
		return NULL;
	}

	if (!tiffInfo.isTiled)
	{
		return readScanlineColourImage(filePath, pTiff, tiffInfo, requiredTypeFlags);
	}
	else
	{
		return readTiledColourImage(filePath, pTiff, tiffInfo, requiredTypeFlags);
	}
}

Image* ImageReaderTIFF::readScanlineColourImage(const std::string& filePath, TIFF* pTiff, TiffInfo& tiffInfo, unsigned int requiredTypeFlags)
{
	ImageColour3f* pImage3f = NULL;
	ImageColour3h* pImage3h = NULL;
	ImageColour3b* pImage3b = NULL;

	unsigned int bitDepthToCreate = tiffInfo.bitDepth;
	if (!(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE))
	{
		bitDepthToCreate = 32;
	}

	if (bitDepthToCreate == 32)
	{
		pImage3f = new ImageColour3f(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}
	else if (bitDepthToCreate == 16)
	{
		pImage3h = new ImageColour3h(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}
	else if (bitDepthToCreate == 8)
	{
		pImage3b = new ImageColour3b(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}

	// if we didn't allocate any image, bail out
	if (!pImage3b && !pImage3h && !pImage3f)
	{
		fprintf(stderr, "Couldn't allocate memory for new image for file: %s\n", filePath.c_str());
		TIFFClose(pTiff);
		return NULL;
	}

	if (bitDepthToCreate == 8)
	{
		// allocate memory for image - do it the easy way for the moment - we can convert to scanline approach later...

		uint32_t totalImageSize = tiffInfo.imageHeight * tiffInfo.imageWidth;
		uint32_t* pRawBuffer = (uint32_t*)_TIFFmalloc(totalImageSize * sizeof(uint32_t)); // RGBA byte

		bool failed = false;

		if (!pRawBuffer)
		{
			fprintf(stderr, "Couldn't allocate memory to read file: %s\n", filePath.c_str());

			failed = true;
		}

		if (TIFFReadRGBAImage(pTiff, tiffInfo.imageWidth, tiffInfo.imageHeight, pRawBuffer, 0) == 0)
		{
			fprintf(stderr, "Couldn't read image: %s\n", filePath.c_str());

			failed = true;

			_TIFFfree(pRawBuffer);
		}

		if (failed)
		{
			if (pImage3b)
			{
				delete pImage3b;
				pImage3b = NULL;
			}

			TIFFClose(pTiff);
			return NULL;
		}

		unsigned int index = 0;

		for (unsigned int i = 0; i < tiffInfo.imageHeight; i++)
		{
			// we don't seem to need to reverse this for 8-bit...
			unsigned int y = i;

			Colour3b* pImageRow = pImage3b->colour3bRowPtr(y);

			for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
			{
				unsigned char red = TIFFGetR(pRawBuffer[index]);
				unsigned char green = TIFFGetG(pRawBuffer[index]);
				unsigned char blue = TIFFGetB(pRawBuffer[index]);

				pImageRow->r = red;
				pImageRow->g = green;
				pImageRow->b = blue;

				pImageRow++;

				index ++;
			}
		}

		_TIFFfree(pRawBuffer);
	}
	else
	{
		// if we're not 8-bit, it's highly likely the image is compressed, so
		// we need to read images as strips, as TIFFReadScanline() cannot read
		// compressed images.

		unsigned int stripSize = TIFFStripSize(pTiff);

		unsigned int numStrips = TIFFNumberOfStrips(pTiff);

		tdata_t pRawBuffer = _TIFFmalloc(stripSize);

		if (!pRawBuffer)
		{
			fprintf(stderr, "Couldn't allocate memory to read file: %s\n", filePath.c_str());
			if (pImage3f)
			{
				delete pImage3f;
				pImage3f = NULL;
			}

			if (pImage3h)
			{
				delete pImage3h;
				pImage3h = NULL;
			}

			TIFFClose(pTiff);
			return NULL;
		}

		const float invShortConvert = 1.0f / 65536.0f;

		unsigned int targetY = 0;

		for (unsigned int strip = 0; strip < numStrips; strip++)
		{
			TIFFReadEncodedStrip(pTiff, strip, pRawBuffer, (tsize_t)-1);

			for (unsigned int tY = 0; tY < tiffInfo.rowsPerStrip; tY++)
			{
				if (tiffInfo.bitDepth == 16 && bitDepthToCreate == 32)
				{
					// TODO: there's got to be a better way to handle this... Return value of TIFFReadEncodedStrip()?
					if (targetY >= tiffInfo.imageHeight)
						break;

					const uint16* pUShortLine = (uint16*)pRawBuffer + (tY * tiffInfo.imageWidth * tiffInfo.channelCount);

					// reverse Y
					unsigned int actualY = tiffInfo.imageHeight - targetY - 1;
					Colour3f* pImageRow = pImage3f->colourRowPtr(actualY);

					for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
					{
						uint16_t red = *pUShortLine++;
						uint16_t green = *pUShortLine++;
						uint16_t blue = *pUShortLine++;

						// TODO: LUT for ushort format

						pImageRow->r = (float)red * invShortConvert;
						pImageRow->g = (float)green * invShortConvert;
						pImageRow->b = (float)blue * invShortConvert;

						// convert to linear
						ColourSpace::convertSRGBToLinearAccurate(*pImageRow);

						pImageRow++;
					}
				}
				else if (tiffInfo.bitDepth == 16 && bitDepthToCreate == 16)
				{
					// TODO: there's got to be a better way to handle this... Return value of TIFFReadEncodedStrip()?
					if (targetY >= tiffInfo.imageHeight)
						break;

					const uint16_t* pUShortLine = (uint16*)pRawBuffer + (tY * tiffInfo.imageWidth * tiffInfo.channelCount);

					// reverse Y
					unsigned int actualY = tiffInfo.imageHeight - targetY - 1;
					Colour3h* pImageRow = pImage3h->colour3hRowPtr(actualY);

					for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
					{
						uint16_t red = *pUShortLine++;
						uint16_t green = *pUShortLine++;
						uint16_t blue = *pUShortLine++;

						// TODO: LUT for ushort format

						pImageRow->r = (float)red * invShortConvert;
						pImageRow->g = (float)green * invShortConvert;
						pImageRow->b = (float)blue * invShortConvert;

						// convert to linear
						ColourSpace::convertSRGBToLinearAccurate(*pImageRow);

						pImageRow++;
					}
				}
				else
				{
					if (targetY >= tiffInfo.imageHeight)
						break;

					const float* pFloatLine = (float*)pRawBuffer + (tY * tiffInfo.imageWidth * tiffInfo.channelCount);

					// reverse Y
					unsigned int actualY = tiffInfo.imageHeight - targetY - 1;
					Colour3f* pImageRow = pImage3f->colourRowPtr(actualY);

					for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
					{
						pImageRow->r = *pFloatLine++;
						pImageRow->g = *pFloatLine++;
						pImageRow->b = *pFloatLine++;

						pImageRow++;
					}
				}

				targetY += 1;
			}
		}

		_TIFFfree(pRawBuffer);
	}

	TIFFClose(pTiff);

	if (pImage3f)
		return pImage3f;
	if (pImage3h)
		return pImage3h;
	if (pImage3b)
		return pImage3b;

	return NULL;
}

Image* ImageReaderTIFF::readTiledColourImage(const std::string& filePath, TIFF* pTiff, TiffInfo& tiffInfo, unsigned int requiredTypeFlags)
{
	ImageColour3f* pImage3f = NULL;
	ImageColour3h* pImage3h = NULL;
	ImageColour3b* pImage3b = NULL;

	unsigned int bitDepthToCreate = tiffInfo.bitDepth;
	if (!(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE))
	{
		bitDepthToCreate = 32;
	}

	unsigned int dirOffset = 0;
	if (requiredTypeFlags & Image::IMAGE_CONSTRAINTS_MIPMAP_LEVEL_MINUS1)
	{
		dirOffset = 1;
	}
	else if (requiredTypeFlags & Image::IMAGE_CONSTRAINTS_MIPMAP_LEVEL_MINUS2)
	{
		dirOffset = 2;
	}

	if (dirOffset)
	{
		TIFFSetDirectory(pTiff, dirOffset);

		// read the info again to get the correct image dimensions
		readInfo(pTiff, tiffInfo);
	}

	if (bitDepthToCreate == 32)
	{
		pImage3f = new ImageColour3f(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}
	else if (bitDepthToCreate == 16)
	{
		pImage3h = new ImageColour3h(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}
	else if (bitDepthToCreate == 8)
	{
		pImage3b = new ImageColour3b(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}

	// if we didn't allocate any image, bail out
	if (!pImage3b && !pImage3h && !pImage3f)
	{
		fprintf(stderr, "Couldn't allocate memory for new image for file: %s\n", filePath.c_str());
		TIFFClose(pTiff);
		return NULL;
	}

	// allocate memory to store a single tile
	unsigned int tileLineSize = (bitDepthToCreate / 8) * tiffInfo.tileWidth * tiffInfo.channelCount;
	unsigned int tileByteSize = tileLineSize * tiffInfo.tileHeight;

	unsigned char* pTileBuffer = new unsigned char[tileByteSize];

	// work out how many tiles we've got in each direction
	unsigned int tileCountX = tiffInfo.imageWidth / tiffInfo.tileWidth;
	unsigned int tileCountY = tiffInfo.imageHeight / tiffInfo.tileHeight;

	// see if there are any remainders, meaning non-complete tiles...
	bool remainderX = false;
	bool remainderY = false;

	if (tiffInfo.imageWidth % tiffInfo.tileWidth > 0)
	{
		remainderX = true;
		tileCountX += 1;
	}

	if (tiffInfo.imageHeight % tiffInfo.tileHeight > 0)
	{
		remainderY = true;
		tileCountY += 1;
	}

	const float invShortConvert = 1.0f / 65536.0f;

	// we need to read each tile individually and copy it into the destination image - this is not going to be too efficient...
	// TODO: need to work out if tile order makes a difference - rows first or columns?

	for (unsigned int tileX = 0; tileX < tileCountX; tileX++)
	{
		unsigned int tilePosX = tileX * tiffInfo.tileWidth;

		unsigned int localTileWidth = tiffInfo.tileWidth;
		if (remainderX && tileX == (tileCountX - 1))
		{
			localTileWidth = tiffInfo.imageWidth - ((tileCountX - 1) * tiffInfo.tileWidth);
		}

		for (unsigned int tileY = 0; tileY < tileCountY; tileY++)
		{
			unsigned int tilePosY = tileY * tiffInfo.tileHeight;

			TIFFReadTile(pTiff, pTileBuffer, tilePosX, tilePosY, 0, 0);

			unsigned int localTileHeight = tiffInfo.tileHeight;
			if (remainderY && tileY == (tileCountY - 1))
			{
				localTileHeight = tiffInfo.imageHeight - ((tileCountY - 1) * tiffInfo.tileHeight);
			}

			unsigned int localYStartPos = tileY * tiffInfo.tileHeight;

			// now copy the data into our image in the correct position...

			if (bitDepthToCreate == 32)
			{
				// cast to the type the data should be...
				const Colour3f* pSrcTileBuffer = (Colour3f*)pTileBuffer;

				unsigned int localLineSizeBytes = (bitDepthToCreate / 8) * localTileWidth * tiffInfo.channelCount;

				for (unsigned int localY = 0; localY < localTileHeight; localY++)
				{
					// reverse Y
					unsigned int actualY = tiffInfo.imageHeight - (localYStartPos + localY + 1);
					Colour3f* pDst = pImage3f->colourRowPtr(actualY);

					// offset to X pos
					pDst += tilePosX;

					memcpy(pDst, pSrcTileBuffer, localLineSizeBytes);

					// because we're of the correct type, we should be able to just increment
					// by the tile width to get to the next line...
					pSrcTileBuffer += tiffInfo.tileWidth;
				}
			}
			else if (bitDepthToCreate == 16)
			{
				// cast to the type the data should be...
				const uint16_t* pSrcTileBuffer = (uint16_t*)pTileBuffer;

				for (unsigned int localY = 0; localY < localTileHeight; localY++)
				{
					// reverse Y
					unsigned int actualY = tiffInfo.imageHeight - (localYStartPos + localY + 1);
					Colour3h* pDst = pImage3h->colour3hRowPtr(actualY);

					// offset to X pos
					pDst += tilePosX;

					const uint16_t* pLocalSrcTileBuffer = pSrcTileBuffer + (localY * tiffInfo.tileWidth * 3); // need to use tileWidth here

					for (unsigned int localX = 0; localX < localTileWidth; localX++)
					{
						uint16_t red = *pLocalSrcTileBuffer++;
						uint16_t green = *pLocalSrcTileBuffer++;
						uint16_t blue = *pLocalSrcTileBuffer++;

						pDst->r = (float)red * invShortConvert;
						pDst->g = (float)green * invShortConvert;
						pDst->b = (float)blue * invShortConvert;

						// convert to linear
						ColourSpace::convertSRGBToLinearAccurate(*pDst++);
					}
				}
			}
			else if (bitDepthToCreate == 8)
			{
				// cast to the type the data should be...
				const uint8_t* pSrcTileBuffer = (uint8_t*)pTileBuffer;

				for (unsigned int localY = 0; localY < localTileHeight; localY++)
				{
					// reverse Y
					unsigned int actualY = tiffInfo.imageHeight - (localYStartPos + localY + 1);
					Colour3b* pDst = pImage3b->colour3bRowPtr(actualY);

					// offset to X pos
					pDst += tilePosX;

					const uint8_t* pLocalSrcTileBuffer = pSrcTileBuffer + (localY * tiffInfo.tileWidth * 3); // need to use tileWidth here

					for (unsigned int localX = 0; localX < localTileWidth; localX++)
					{
						uint8_t red = *pLocalSrcTileBuffer++;
						uint8_t green = *pLocalSrcTileBuffer++;
						uint8_t blue = *pLocalSrcTileBuffer++;

						pDst->r = red;
						pDst->g = green;
						pDst->b = blue;

						pDst++;
					}
				}
			}
		}
	}

	if (pTileBuffer)
	{
		delete [] pTileBuffer;
		pTileBuffer = NULL;
	}

	TIFFClose(pTiff);

	if (pImage3f)
		return pImage3f;
	if (pImage3h)
		return pImage3h;
	if (pImage3b)
		return pImage3b;

	return NULL;
}

Image* ImageReaderTIFF::readGreyscaleImage(const std::string& filePath, unsigned int requiredTypeFlags)
{
	TIFF* pTiff = TIFFOpen(filePath.c_str(), "r");
	if (!pTiff)
	{
		fprintf(stderr, "Error reading file: %s\n", filePath.c_str());
		return NULL;
	}

	TiffInfo tiffInfo;
	if (!readInfo(pTiff, tiffInfo))
	{
		fprintf(stderr, "Invalid tiff file: %s\n", filePath.c_str());
		TIFFClose(pTiff);
		return NULL;
	}

	if (tiffInfo.bitDepth < 8)
	{
		fprintf(stderr, "Unsupported TIFF format for file: %s\n", filePath.c_str());
		TIFFClose(pTiff);
		return NULL;
	}

	if (!tiffInfo.isTiled)
	{
		return readScanlineGreyscaleImage(filePath, pTiff, tiffInfo, requiredTypeFlags);
	}
	else
	{
		return readTiledGreyscaleImage(filePath, pTiff, tiffInfo, requiredTypeFlags);
	}
}

Image* ImageReaderTIFF::readScanlineGreyscaleImage(const std::string& filePath, TIFF* pTiff, TiffInfo& tiffInfo, unsigned int requiredTypeFlags)
{
	Image1f* pImage1f = NULL;
	Image1b* pImage1b = NULL;

	bool makeFloat = tiffInfo.bitDepth >= 16 || !(requiredTypeFlags & Image::IMAGE_FORMAT_NATIVE);

	if (makeFloat)
	{
		pImage1f = new Image1f(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}
	else
	{
		pImage1b = new Image1b(tiffInfo.imageWidth, tiffInfo.imageHeight, false);
	}

	// if we didn't allocate any image, bail out
	if (!pImage1b && !pImage1f)
	{
		fprintf(stderr, "Couldn't allocate memory for new image for file: %s\n", filePath.c_str());
		TIFFClose(pTiff);
		return NULL;
	}

	if (!makeFloat)
	{
		// allocate memory for image - do it the easy way for the moment - we can convert to scanline approach later...

		uint32_t totalImageSize = tiffInfo.imageHeight * tiffInfo.imageWidth;
		uint32_t* pRawBuffer = (uint32_t*)_TIFFmalloc(totalImageSize * sizeof(uint32_t)); // RGBA byte

		bool failed = false;

		if (!pRawBuffer)
		{
			fprintf(stderr, "Couldn't allocate memory to read file: %s\n", filePath.c_str());

			failed = true;
		}

		if (TIFFReadRGBAImage(pTiff, tiffInfo.imageWidth, tiffInfo.imageHeight, pRawBuffer, 0) == 0)
		{
			fprintf(stderr, "Couldn't read image: %s\n", filePath.c_str());

			failed = true;

			_TIFFfree(pRawBuffer);
		}

		if (failed)
		{
			if (pImage1b)
			{
				delete pImage1b;
				pImage1b = NULL;
			}

			TIFFClose(pTiff);
			return NULL;
		}

		unsigned int index = 0;

		if (requiredTypeFlags & Image::IMAGE_FLAGS_ALPHA)
		{
			for (unsigned int i = 0; i < tiffInfo.imageHeight; i++)
			{
				// for 8-bit, we don't seem to need to flip Y
				unsigned int y = i;

				unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

				for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
				{
					unsigned char alpha;

					if (tiffInfo.channelCount == 1 || tiffInfo.channelCount == 3)
					{
						// it's actually the red channel
						alpha = TIFFGetR(pRawBuffer[index]);
					}
					else
					{
						alpha = TIFFGetA(pRawBuffer[index]);
					}

					*pUCharRow = alpha;

					pUCharRow++;

					index ++;
				}
			}
		}
		else if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
		{
			for (unsigned int i = 0; i < tiffInfo.imageHeight; i++)
			{
				// for 8-bit, we don't seem to need to flip Y
				unsigned int y = i;

				unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

				for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
				{
					unsigned char red = TIFFGetR(pRawBuffer[index]);
					unsigned char green = TIFFGetG(pRawBuffer[index]);
					unsigned char blue = TIFFGetB(pRawBuffer[index]);

					Colour3b finalColour(red, green, blue);

					*pUCharRow = finalColour.brightness();

					pUCharRow++;

					index ++;
				}
			}
		}
		else
		{
			// exact
			for (unsigned int i = 0; i < tiffInfo.imageHeight; i++)
			{
				// for 8-bit, we don't seem to need to flip Y
				unsigned int y = i;

				unsigned char* pUCharRow = pImage1b->uCharRowPtr(y);

				if (tiffInfo.channelCount == 1)
				{
					for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
					{
						unsigned char red = TIFFGetR(pRawBuffer[index]);

						*pUCharRow = red;

						pUCharRow++;

						index ++;
					}
				}
				else
				{
					for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
					{
						unsigned char red = TIFFGetR(pRawBuffer[index]);
						unsigned char green = TIFFGetG(pRawBuffer[index]);
						unsigned char blue = TIFFGetB(pRawBuffer[index]);

						float average = (float)red + (float)green + (float)blue;
						average *= 0.3333333333f;

						*pUCharRow = (unsigned char)average;

						pUCharRow++;

						index ++;
					}
				}
			}
		}

		_TIFFfree(pRawBuffer);
	}
	else
	{
		// if we're not 8-bit, it's highly likely the image is compressed, so
		// we need to read images as strips, as TIFFReadScanline() cannot read
		// compressed images.

		unsigned int stripSize = TIFFStripSize(pTiff);

		unsigned int numStrips = TIFFNumberOfStrips(pTiff);

		tdata_t pRawBuffer = _TIFFmalloc(stripSize);

		if (!pRawBuffer)
		{
			fprintf(stderr, "Couldn't allocate memory to read file: %s\n", filePath.c_str());
			if (pImage1f)
			{
				delete pImage1f;
				pImage1f = NULL;
			}
			TIFFClose(pTiff);
			return NULL;
		}

		const float invShortConvert = 1.0f / 65536.0f;

		unsigned int targetY = 0;

		for (unsigned int strip = 0; strip < numStrips; strip++)
		{
			TIFFReadEncodedStrip(pTiff, strip, pRawBuffer, (tsize_t)-1);

			for (unsigned int tY = 0; tY < tiffInfo.rowsPerStrip; tY++)
			{
				if (tiffInfo.bitDepth == 16)
				{
					// TODO: there's got to be a better way to handle this... Return value of TIFFReadEncodedStrip()?
					if (targetY >= tiffInfo.imageHeight)
						break;

					const uint16* pUShortLine = (uint16*)pRawBuffer + (tY * tiffInfo.imageWidth * tiffInfo.channelCount);

					// flip Y
					unsigned int actualY = tiffInfo.imageHeight - targetY - 1;
					float* pFloatRow = pImage1f->floatRowPtr(actualY);

					if (requiredTypeFlags & Image::IMAGE_FLAGS_ALPHA)
					{
						uint16_t alpha = 0;

						if (tiffInfo.channelCount == 1)
						{
							for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
							{
								alpha = *pUShortLine++;

								*pFloatRow++ = (float)alpha * invShortConvert;
							}
						}
						else if (tiffInfo.channelCount == 3)
						{
							for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
							{
								// get red one
								alpha = *pUShortLine++;
								pUShortLine++;
								pUShortLine++;

								*pFloatRow++ = (float)alpha * invShortConvert;
							}
						}
						else
						{
							for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
							{
								// ignore first 3
								pUShortLine++;
								pUShortLine++;
								pUShortLine++;

								alpha = *pUShortLine++;

								*pFloatRow++ = (float)alpha * invShortConvert;
							}
						}
					}
					else if (requiredTypeFlags & Image::IMAGE_FLAGS_BRIGHTNESS)
					{
						for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
						{
							float brightness = 0.0f;

							if (tiffInfo.channelCount == 1)
							{
								uint16_t rawValue = *pUShortLine++;

								brightness = (float)rawValue * invShortConvert;
							}
							else
							{
								uint16_t red = *pUShortLine++;
								uint16_t green = *pUShortLine++;
								uint16_t blue = *pUShortLine++;

								float fRed = (float)red * invShortConvert;
								float fGreen = (float)green * invShortConvert;
								float fBlue = (float)blue * invShortConvert;

								Colour3f colour(fRed, fGreen, fBlue);

								brightness = colour.brightness();
							}

							*pFloatRow++ = brightness;
						}
					}
					else
					{
						for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
						{
							// exact copy - assume 1 channel for the moment
							float value = 0.0f;

							if (tiffInfo.channelCount == 1)
							{
								uint16_t rawValue = *pUShortLine++;

								value = (float)rawValue * invShortConvert;
							}

							*pFloatRow++ = value;
						}
					}

					pFloatRow++;
				}
				else
				{
					// must be float
					if (targetY >= tiffInfo.imageHeight)
						break;

					const float* pFloatLine = (float*)pRawBuffer + (tY * tiffInfo.imageWidth * tiffInfo.channelCount);

					// flip Y
					unsigned int actualY = tiffInfo.imageHeight - targetY - 1;
					float* pFloatRow = pImage1f->floatRowPtr(actualY);

					if (tiffInfo.channelCount == 1)
					{
						for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
						{
							*pFloatRow = *pFloatLine++;

							pFloatRow++;
						}
					}
					else if (tiffInfo.channelCount == 3)
					{
						for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
						{
							*pFloatRow = *pFloatLine++;
							pFloatLine++;
							pFloatLine++;
						}
					}
					else
					{
						for (unsigned int x = 0; x < tiffInfo.imageWidth; x++)
						{
							*pFloatRow = *pFloatLine++;
							pFloatLine++;
							pFloatLine++;

							pFloatRow++;
						}
					}
				}

				targetY += 1;
			}
		}

		_TIFFfree(pRawBuffer);
	}

	TIFFClose(pTiff);

	Image* pFinalImage = (makeFloat) ? static_cast<Image*>(pImage1f) : static_cast<Image*>(pImage1b);

	return pFinalImage;
}

Image* ImageReaderTIFF::readTiledGreyscaleImage(const std::string& filePath, TIFF* pTiff, TiffInfo& tiffInfo, unsigned int requiredTypeFlags)
{
	return NULL;
}

bool ImageReaderTIFF::readImageDetails(const std::string& filePath, ImageTextureDetails& textureDetails) const
{
	// TODO: implement this at some point for completeness's sake, but given that tiled TIFF reading for texture caching
	//       is really slow (due to statting done to change mipmap level dirs), using TIFFs for texture caching
	//       at production level isn't really a good idea, and EXR is much faster...
	return false;

	if (!FileHelpers::doesFileExist(filePath))
		return false;

	bool isTiled = false;

	if (!isTiled)
	{
		fprintf(stderr, "Reading of non-tiled TIFFs with texture caching enabled is not yet supported.\n");
		return false;
	}

	return true;
}

bool ImageReaderTIFF::readImageTile(const ImageTextureTileReadParams& readParams, ImageTextureTileReadResults& readResults) const
{
	const ImageTextureDetails& textureDetails = readParams.getImageDetails();

	// TODO...

	return false;
}

namespace
{
	ImageReader* createImageReaderTIFF()
	{
		return new ImageReaderTIFF();
	}

	const bool registered = FileIORegistry::instance().registerImageReaderMultipleExtensions("tif;tiff;tex", createImageReaderTIFF);
}
