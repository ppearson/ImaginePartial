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

#include "image_writer_exr.h"

#include <stdio.h>
#include <vector>

#include "image/output_image.h"
#include "colour/colour_space.h"

#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfStringAttribute.h>
#include <half.h>

#define USE_OPENEXR2 1
#define USE_DEEPTILE 0

#if USE_OPENEXR2
#include "image/image_deep_colour4f.h"

#include <ImfPartType.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfDeepScanLineOutputFile.h>
#include <ImfDeepScanLineOutputPart.h>
#include <ImfDeepTiledOutputFile.h>
#include <ImfDeepTiledOutputPart.h>
#endif

using namespace Imath;

ImageWriterEXR::ImageWriterEXR()
{
}

bool ImageWriterEXR::writeImage(const std::string& filePath, const OutputImage& image, unsigned int channels, unsigned int flags)
{
	if (channels & ImageWriter::DEEP)
	{
		writeDeepImage(filePath, image, channels);
	}
	else
	{
		if (flags & ImageWriter::FLOAT32)
		{
			writeStandardImage<float>(filePath, image, channels, true);
		}
		else
		{
			writeStandardImage<half>(filePath, image, channels, false);
		}
	}

	return true;
}

template <typename T>
bool ImageWriterEXR::writeStandardImage(const std::string& filePath, const OutputImage& image, unsigned int channels, bool fullFloat)
{
	unsigned int width = image.getWidth();
	unsigned int height = image.getHeight();

	Imf::Header header(width, height);

	Imf::StringAttribute sourceAttribute;
	sourceAttribute.value() = "Created with Imagine 0.98";
	header.insert("comments", sourceAttribute);

	Imf::PixelType pixelType = (fullFloat) ? Imf::FLOAT : Imf::HALF;

	if (channels & ImageWriter::RGB)
	{
		header.channels().insert("R", Imf::Channel(pixelType));
		header.channels().insert("G", Imf::Channel(pixelType));
		header.channels().insert("B", Imf::Channel(pixelType));
	}

	if (channels & ImageWriter::ALPHA)
		header.channels().insert("A", Imf::Channel(pixelType));

	// if we haven't got any depth data, don't bother
	if (!(image.components() & COMPONENT_DEPTH))
		channels = channels & ~ImageWriter::DEPTH;

	if (channels & ImageWriter::DEPTH)
		header.channels().insert("Z", Imf::Channel(pixelType));

	// if we haven't got any normal data, don't bother
	if (!(image.components() & COMPONENT_NORMAL))
		channels = channels & ~ImageWriter::NORMALS;

	if (channels & ImageWriter::NORMALS)
	{
		header.channels().insert("normal.X", Imf::Channel(pixelType));
		header.channels().insert("normal.Y", Imf::Channel(pixelType));
		header.channels().insert("normal.Z", Imf::Channel(pixelType));
	}

	// if we haven't got any wpp data, don't bother
	if (!(image.components() & COMPONENT_WPP))
		channels = channels & ~ImageWriter::WPP;

	if (channels & ImageWriter::WPP)
	{
		header.channels().insert("wpp.X", Imf::Channel(pixelType));
		header.channels().insert("wpp.Y", Imf::Channel(pixelType));
		header.channels().insert("wpp.Z", Imf::Channel(pixelType));
	}

	// if we haven't got shadows, don't write them
	if (!(image.components() & COMPONENT_SHADOWS))
		channels = channels & ~ImageWriter::SHADOWS;

	if (channels & ImageWriter::SHADOWS)
	{
		// cope with Nuke's limitations by putting ".r" on the end so it shows up in channel list...
		header.channels().insert("shadows.r", Imf::Channel(pixelType));
	}

	unsigned int rgbStride = (channels & ImageWriter::ALPHA) ? 4 : 3;

	T* rgba = NULL;
	if (channels & ImageWriter::RGB)
		rgba = new T[width * height * rgbStride];
	T* pDepth = NULL;
	if (channels & ImageWriter::DEPTH)
		pDepth = new T[width * height];
	T* pNormal = NULL;
	if (channels & ImageWriter::NORMALS)
		pNormal = new T[width * height * 3];
	T* pWPP = NULL;
	if (channels & ImageWriter::WPP)
		pWPP = new T[width * height * 3];
	T* pShadows = NULL;
	if (channels & ImageWriter::SHADOWS)
		pShadows = new T[width * height];

	const Colour4f* pRow = NULL;
	const float* pDepthRow = NULL;
	const Colour3f* pNormalRow = NULL;
	const Colour3f* pWPPRow = NULL;
	const float* pShadowsRow = NULL;

	for (unsigned int y = 0; y < height; y++)
	{
		pRow = image.colourRowPtr(y);

		if (channels & ImageWriter::DEPTH)
			pDepthRow = image.depthRowPtr(y);

		if (channels & ImageWriter::NORMALS)
			pNormalRow = image.normalRowPtr(y);

		if (channels & ImageWriter::WPP)
			pWPPRow = image.wppRowPtr(y);

		if (channels & ImageWriter::SHADOWS)
			pShadowsRow = image.shadowsRowPtr(y);

		unsigned int rgbStartPos = width * y * rgbStride;
		unsigned int normalStartPos = width * y * 3;
		unsigned int wppStartPos = width * y * 3;

		for (unsigned int x = 0; x < width; x++)
		{
			unsigned int pixelPos = rgbStartPos + (x * rgbStride);

			if (channels & ImageWriter::RGB)
			{
				float red = ColourSpace::convertSRGBToLinearAccurate(pRow->r);
				float green = ColourSpace::convertSRGBToLinearAccurate(pRow->g);
				float blue = ColourSpace::convertSRGBToLinearAccurate(pRow->b);

				rgba[pixelPos++] = red;
				rgba[pixelPos++] = green;
				rgba[pixelPos++] = blue;
			}
			else
			{
				pixelPos += 3;
			}

			if (channels & ImageWriter::ALPHA)
				rgba[pixelPos++] = pRow->a;

			if (channels & ImageWriter::DEPTH)
			{
				pDepth[(width * y) + x] = *pDepthRow++;
			}

			if (channels & ImageWriter::NORMALS && pNormalRow)
			{
				const Colour3f& normal = *pNormalRow;

				unsigned int normalPixelPos = normalStartPos + (x * 3);

				pNormal[normalPixelPos++] = normal.r;
				pNormal[normalPixelPos++] = normal.g;
				pNormal[normalPixelPos++] = normal.b;

				pNormalRow++;
			}

			if (channels & ImageWriter::WPP && pWPPRow)
			{
				const Colour3f& wpp = *pWPPRow;

				unsigned int wppPixelPos = wppStartPos + (x * 3);

				pWPP[wppPixelPos++] = wpp.r;
				pWPP[wppPixelPos++] = wpp.g;
				pWPP[wppPixelPos++] = wpp.b;

				pWPPRow++;
			}

			if (channels & ImageWriter::SHADOWS)
			{
				pShadows[(width * y) + x] = *pShadowsRow++;
			}

			pRow++;
		}
	}

	Imf::FrameBuffer fb;
	if (channels & ImageWriter::RGB)
	{
		fb.insert("R", Imf::Slice(pixelType, (char *)rgba, rgbStride*sizeof(T), rgbStride*width*sizeof(T)));
		fb.insert("G", Imf::Slice(pixelType, (char *)rgba + sizeof(T), rgbStride*sizeof(T), rgbStride*width*sizeof(T)));
		fb.insert("B", Imf::Slice(pixelType, (char *)rgba + 2 * sizeof(T), rgbStride*sizeof(T), rgbStride*width*sizeof(T)));
	}

	if (channels & ImageWriter::ALPHA)
		fb.insert("A", Imf::Slice(pixelType, (char *)rgba+3*sizeof(T), rgbStride*sizeof(T), rgbStride*width*sizeof(T)));

	if (channels & ImageWriter::DEPTH)
	{
		fb.insert("Z", Imf::Slice(pixelType, (char *)&(*pDepth), sizeof(T), width * sizeof(T)));
	}

	if (channels & ImageWriter::NORMALS)
	{
		fb.insert("normal.X", Imf::Slice(pixelType, (char *)pNormal, 3 * sizeof(T), 3 * width * sizeof(T)));
		fb.insert("normal.Y", Imf::Slice(pixelType, (char *)pNormal + sizeof(T), 3*sizeof(T), 3*width*sizeof(T)));
		fb.insert("normal.Z", Imf::Slice(pixelType, (char *)pNormal + 2 * sizeof(T), 3*sizeof(T), 3*width*sizeof(T)));
	}

	if (channels & ImageWriter::WPP)
	{
		fb.insert("wpp.X", Imf::Slice(pixelType, (char *)pWPP, 3 * sizeof(T), 3 * width * sizeof(T)));
		fb.insert("wpp.Y", Imf::Slice(pixelType, (char *)pWPP + sizeof(T), 3*sizeof(T), 3*width*sizeof(T)));
		fb.insert("wpp.Z", Imf::Slice(pixelType, (char *)pWPP + 2 * sizeof(T), 3*sizeof(T), 3*width*sizeof(T)));
	}

	if (channels & ImageWriter::SHADOWS)
	{
		fb.insert("shadows.r", Imf::Slice(pixelType, (char *)&(*pShadows), sizeof(T), width * sizeof(T)));
	}

	Imf::OutputFile file(filePath.c_str(), header);
	file.setFrameBuffer(fb);
	file.writePixels(height);

	if (rgba)
		delete [] rgba;
	if (pDepth)
		delete [] pDepth;
	if (pNormal)
		delete [] pNormal;
	if (pWPP)
		delete [] pWPP;
	if (pShadows)
		delete [] pShadows;

	return true;
}

bool ImageWriterEXR::writeDeepImage(const std::string& filePath, const OutputImage& image, unsigned int channels)
{
#if USE_OPENEXR2

	unsigned int width = image.getWidth();
	unsigned int height = image.getHeight();

	Imf::Header header(width, height);
	header.setName("Main");

	header.channels().insert("A", Imf::Channel(Imf::HALF));
	header.channels().insert("R", Imf::Channel(Imf::HALF));
	header.channels().insert("G", Imf::Channel(Imf::HALF));
	header.channels().insert("B", Imf::Channel(Imf::HALF));

	header.channels().insert("Z", Imf::Channel(Imf::HALF));

	Imf::StringAttribute sourceAttribute;
	sourceAttribute.value() = "Created with Imagine 0.98";
	header.insert("comments", sourceAttribute);

#if USE_DEEPTILE
	header.setType(Imf::DEEPTILE);
	header.setTileDescription(Imf::TileDescription());
#else
	header.setType(Imf::DEEPSCANLINE);
#endif

	// we need this...
	header.compression() = Imf::ZIPS_COMPRESSION;
//	header.compression() = Imf::NO_COMPRESSION;

	unsigned int* pSampleCount = new unsigned int[width * height];

	// work out total number of samples for all pixels
	unsigned int totalSamples = 0;
	unsigned int pixelIndex = 0;
	for (unsigned int y = 0; y < height; y++)
	{
		DeepValues* pDeepValues = image.getDeepImage()->colourRowPtr(y);

		for (unsigned int x = 0; x < width; x++)
		{
			totalSamples += pDeepValues->numSamples;

			pSampleCount[pixelIndex++] = pDeepValues->numSamples;

			pDeepValues++;
		}
	}

	half* pAlphaFullSamples = new half[totalSamples];
	half* pRedFullSamples = new half[totalSamples];
	half* pGreenFullSamples = new half[totalSamples];
	half* pBlueFullSamples = new half[totalSamples];
	half* pZFullSamples = new half[totalSamples];

	const DeepValues* pDeepValues = NULL;

	// accumulate the data for writing...
	std::vector<half*> aAlphaPointers(width * height);
	std::vector<half*> aRedPointers(width * height);
	std::vector<half*> aGreenPointers(width * height);
	std::vector<half*> aBluePointers(width * height);
	std::vector<half*> aZPointers(width * height);

	pixelIndex = 0;
	unsigned int fullSampleIndex = 0;
	for (unsigned int y = 0; y < height; y++)
	{
		pDeepValues = image.getDeepImage()->colourRowPtr(y);

		const DeepValues* pDeepPixelValues = pDeepValues;

		for (unsigned int x = 0; x < width; x++)
		{
			unsigned int numSamples = pDeepPixelValues->numSamples;

			half* pAlphaSampleStart = pAlphaFullSamples + fullSampleIndex;
			aAlphaPointers[pixelIndex] = pAlphaSampleStart;
			half* pRedSampleStart = pRedFullSamples + fullSampleIndex;
			aRedPointers[pixelIndex] = pRedSampleStart;
			half* pGreenSampleStart = pGreenFullSamples + fullSampleIndex;
			aGreenPointers[pixelIndex] = pGreenSampleStart;
			half* pBlueSampleStart = pBlueFullSamples + fullSampleIndex;
			aBluePointers[pixelIndex] = pBlueSampleStart;

			half* pZSampleStart = pZFullSamples + fullSampleIndex;
			aZPointers[pixelIndex] = pZSampleStart;

			DeepSample* pDeepSample = pDeepPixelValues->samples;

			// now blit the data over
			for (unsigned int s = 0; s < numSamples; s++)
			{
				*pRedSampleStart++ = pDeepSample->colour.r;
				*pGreenSampleStart++ = pDeepSample->colour.g;
				*pBlueSampleStart++ = pDeepSample->colour.b;

				*pAlphaSampleStart++ = pDeepSample->colour.a;

				*pZSampleStart++ = pDeepSample->depth;

				pDeepSample++;
			}

			pDeepPixelValues++;

			fullSampleIndex += numSamples;
			pixelIndex ++;
		}
	}

	Imf::DeepFrameBuffer frameBuffer;

	// write the sample count
	frameBuffer.insertSampleCountSlice(Imf::Slice(Imf::UINT, (char*)pSampleCount, sizeof(unsigned int), width * sizeof(unsigned int)));

	frameBuffer.insert("A", Imf::DeepSlice(Imf::HALF, (char*)(&aAlphaPointers[0]), sizeof(half*), sizeof(half*) * width, sizeof(half)));
	frameBuffer.insert("R", Imf::DeepSlice(Imf::HALF, (char*)(&aRedPointers[0]), sizeof(half*), sizeof(half*) * width, sizeof(half)));
	frameBuffer.insert("G", Imf::DeepSlice(Imf::HALF, (char*)(&aGreenPointers[0]), sizeof(half*), sizeof(half*) * width, sizeof(half)));
	frameBuffer.insert("B", Imf::DeepSlice(Imf::HALF, (char*)(&aBluePointers[0]), sizeof(half*), sizeof(half*) * width, sizeof(half)));

	frameBuffer.insert("Z", Imf::DeepSlice(Imf::HALF, (char*)(&aZPointers[0]), sizeof(half*), sizeof(half*) * width, sizeof(half)));

	const bool useMultipart = true;

	if (useMultipart)
	{
#if !USE_DEEPTILE
		Imf::MultiPartOutputFile file(filePath.c_str(), &header, 1);
		Imf::DeepScanLineOutputPart part(file, 0);

		try
		{
			part.setFrameBuffer(frameBuffer);
			part.writePixels(height);
		}
		catch (...)
		{
			int one = 5;
		}
#else
		Imf::MultiPartOutputFile file(filePath.c_str(), &header, 1);
		Imf::DeepTiledOutputPart part(file, 0);

		try
		{
			part.setFrameBuffer(frameBuffer);

			for (int y = 0; y < part.numYTiles(); y++)
			{
				for (int x = 0; x < part.numXTiles(); x++)
				{
					part.writeTile(x, y, 0);
				}
			}
		}
		catch (...)
		{
			int one = 5;
		}
#endif
	}
	else
	{
#if !USE_DEEPTILE
		Imf::DeepScanLineOutputFile file(filePath.c_str(), header);

		try
		{
			file.setFrameBuffer(frameBuffer);
			file.writePixels(height);
		}
		catch (...)
		{
			int one = 5;
		}
#else
		Imf::DeepTiledOutputFile file(filePath.c_str(), header);

		try
		{
			file.setFrameBuffer(frameBuffer);

			file.writeTiles(0, file.numXTiles() - 1, 0, file.numYTiles() - 1);
/*
			for (int y = 0; y < file.numYTiles(); y++)
			{
				for (int x = 0; x < file.numXTiles(); x++)
				{
					file.writeTile(x, y, 0);
				}
			}
*/
		}
		catch (...)
		{
			int one = 5;
		}
#endif
	}

	delete [] pSampleCount;

	delete [] pRedFullSamples;
	delete [] pGreenFullSamples;
	delete [] pBlueFullSamples;
	delete [] pAlphaFullSamples;

	delete [] pZFullSamples;
#endif
	return true;
}

namespace
{
	ImageWriter* createImageWriterEXR()
	{
		return new ImageWriterEXR();
	}

	const bool registered = FileIORegistry::instance().registerImageWriter("exr", createImageWriterEXR);
}
