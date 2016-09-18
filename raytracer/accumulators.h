/*
 Imagine
 Copyright 2013-2015 Peter Pearson.

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

#ifndef ACCUMULATORS_H
#define ACCUMULATORS_H

#include "image/output_image.h"
#include "image/output_image_tile.h"

namespace Imagine
{

class Colour4fStandard
{
public:
	static void addColour(const Colour4f& sample, Colour4f& finalColour)
	{
		finalColour += sample;
	}

	static void accumulateSample(Colour4f& sample, OutputImageTile* pOutputTile, unsigned int x, unsigned int y,
								 const Sample2D& samplePos)
	{
		// do nothing
	}

	static void setTilePixelColour(Colour4f& colour, float invSamplesPerIt, float sampleWeight, OutputImageTile* pOutputTile,
								   unsigned int x, unsigned int y)
	{
		colour *= invSamplesPerIt;

		pOutputTile->colourAt(x, y) = colour;
		pOutputTile->setSamplesAt(x, y, sampleWeight);
	}

	static void addTileToOutputImage(OutputImageTile* pOutputTile, OutputImage* pOutputImage, unsigned int x, unsigned int y,
									 unsigned int tileWidth, unsigned int tileHeight)
	{
		pOutputImage->addColourTile(x, y, tileWidth, tileHeight, 0, 0, *pOutputTile);
	}
};

class Colour4fStandardNoSamples
{
public:
	static void addColour(const Colour4f& sample, Colour4f& finalColour)
	{
		finalColour += sample;
	}

	static void accumulateSample(Colour4f& sample, OutputImageTile* pOutputTile, unsigned int x, unsigned int y,
								 const Sample2D& samplePos)
	{
		// do nothing
	}

	static void setTilePixelColour(Colour4f& colour, float invSamplesPerIt, float sampleWeight, OutputImageTile* pOutputTile,
								   unsigned int x, unsigned int y)
	{
		colour *= invSamplesPerIt;

		pOutputTile->colourAt(x, y) = colour;
	}

	static void addTileToOutputImage(OutputImageTile* pOutputTile, OutputImage* pOutputImage, unsigned int x, unsigned int y,
									 unsigned int tileWidth, unsigned int tileHeight)
	{
		pOutputImage->addColourTileNoSamples(x, y, tileWidth, tileHeight, 0, 0, *pOutputTile);
	}
};

class Colour4fFiltered
{
public:
	static void addColour(const Colour4f& sample, Colour4f& finalColour)
	{
		// do nothing
	}

	static void accumulateSample(Colour4f& sample, OutputImageTile* pOutputTile, unsigned int x, unsigned int y,
								 const Sample2D& samplePos)
	{
		pOutputTile->putFilteredColour(x, y, samplePos, sample);
	}

	static void setTilePixelColour(Colour4f& colour, float invSamplesPerIt, float sampleWeight, OutputImageTile* pOutputTile,
								   unsigned int x, unsigned int y)
	{
		// do nothing
	}

	static void addTileToOutputImage(OutputImageTile* pOutputTile, OutputImage* pOutputImage, unsigned int x, unsigned int y,
									 unsigned int tileWidth, unsigned int tileHeight)
	{
		pOutputImage->addColourTileFilter(x, y, *pOutputTile);
	}
};

class Colour4fFilteredNoSamples
{
public:
	static void addColour(const Colour4f& sample, Colour4f& finalColour)
	{
		// do nothing
	}

	static void accumulateSample(Colour4f& sample, OutputImageTile* pOutputTile, unsigned int x, unsigned int y,
								 const Sample2D& samplePos)
	{
		pOutputTile->putFilteredColourNoSamples(x, y, samplePos, sample);
	}

	static void setTilePixelColour(Colour4f& colour, float invSamplesPerIt, float sampleWeight, OutputImageTile* pOutputTile,
								   unsigned int x, unsigned int y)
	{
		// do nothing
	}

	static void addTileToOutputImage(OutputImageTile* pOutputTile, OutputImage* pOutputImage, unsigned int x, unsigned int y,
									 unsigned int tileWidth, unsigned int tileHeight)
	{
		pOutputImage->addColourTileFilterNoSamples(x, y, *pOutputTile);
	}
};

class Colour4fFilteredClamp
{
public:
	static void addColour(const Colour4f& sample, Colour4f& finalColour)
	{
		// do nothing
	}

	static void accumulateSample(Colour4f& sample, OutputImageTile* pOutputTile, unsigned int x, unsigned int y,
								 const Sample2D& samplePos)
	{
		sample.clamp(2.0f);
		pOutputTile->putFilteredColour(x, y, samplePos, sample);
	}

	static void setTilePixelColour(Colour4f& colour, float invSamplesPerIt, float sampleWeight, OutputImageTile* pOutputTile,
								   unsigned int x, unsigned int y)
	{
		// do nothing
	}

	static void addTileToOutputImage(OutputImageTile* pOutputTile, OutputImage* pOutputImage, unsigned int x, unsigned int y,
									 unsigned int tileWidth, unsigned int tileHeight)
	{
		pOutputImage->addColourTileFilter(x, y, *pOutputTile);
	}
};

class Colour4fFilteredDiscard
{
public:

	Colour4fFilteredDiscard(float luminanceThreshold) : m_luminanceThreshold(luminanceThreshold)
	{

	}

	static void addColour(const Colour4f& sample, Colour4f& finalColour)
	{
		// do nothing
	}

	void accumulateSample(Colour4f& sample, OutputImageTile* pOutputTile, unsigned int x, unsigned int y,
								 const Sample2D& samplePos)
	{
		if (sample.brightness() > m_luminanceThreshold)
			return;

		pOutputTile->putFilteredColour(x, y, samplePos, sample);
	}

	static void setTilePixelColour(Colour4f& colour, float invSamplesPerIt, float sampleWeight, OutputImageTile* pOutputTile,
								   unsigned int x, unsigned int y)
	{
		// do nothing
	}

	static void addTileToOutputImage(OutputImageTile* pOutputTile, OutputImage* pOutputImage, unsigned int x, unsigned int y,
									 unsigned int tileWidth, unsigned int tileHeight)
	{
		pOutputImage->addColourTileFilter(x, y, *pOutputTile);
	}

protected:
	float		m_luminanceThreshold;
};

} // namespace Imagine

#endif // ACCUMULATORS_H
