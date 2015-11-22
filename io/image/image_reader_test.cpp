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

#include "image_reader_test.h"

#include "colour/colour3f.h"

ImageReaderTXT::ImageReaderTXT()
{
}

bool ImageReaderTXT::readImageDetails(const std::string& filePath, ImageTextureDetails& textureDetails) const
{
	textureDetails.setChannelCount(3);
	textureDetails.setDataType(ImageTextureDetails::eFloat);

	unsigned int width = 4096 * 2 * 2;
	textureDetails.setFullWidth(width);
	textureDetails.setFullHeight(width);

	textureDetails.setIsTiled(true);
	textureDetails.setMipmapped(true);

	std::vector<ImageTextureItemDetails>& mipmaps = textureDetails.getMipmaps();
	mipmaps.reserve(8);

	unsigned int textureSize = width;
	unsigned int tileSize = 32;
	while (textureSize > 1)
	{
		ImageTextureItemDetails mipmapDetails(textureSize, textureSize, tileSize, tileSize);

		mipmaps.push_back(mipmapDetails);

		textureSize /= 2;

		if (tileSize > textureSize)
		{
			tileSize = textureSize;
		}
	}

	return true;
}

bool ImageReaderTXT::readImageTile(const ImageTextureTileReadParams& readParams, ImageTextureTileReadResults& readResults) const
{
	const ImageTextureDetails& textureDetails = readParams.getImageDetails();
	const ImageTextureItemDetails& mipmapInfo = textureDetails.getMipmaps()[readParams.mipmapLevel];

	unsigned int imageSize = mipmapInfo.getWidth();
	unsigned int tileSize = mipmapInfo.getTileWidth();

	unsigned int numTiles = imageSize / tileSize;

	float ratioX = (float)readParams.tileX / (float)numTiles;
	float ratioY = (float)readParams.tileY / (float)numTiles;

	float maxRat = 1.0f / (float)(textureDetails.getMipmaps().size() - 1);

	float mipMapRatio = maxRat * (float)(readParams.mipmapLevel);

	Colour3f* pColourData = (Colour3f*)readParams.pData;

	for (unsigned int y = 0; y < 32; y++)
	{
		Colour3f* pRow = pColourData + (tileSize * y);

		for (unsigned int x = 0; x < 32; x++)
		{
	//		*pRow = Colour3f(ratioX, ratioY, 0.0f);
			*pRow = Colour3f(mipMapRatio, mipMapRatio, mipMapRatio);
			pRow++;
		}
	}

	return true;
}

namespace
{
	ImageReader* createImageReaderTXT()
	{
		return new ImageReaderTXT();
	}

	const bool registered = FileIORegistry::instance().registerImageReader("txt", createImageReaderTXT);
}
