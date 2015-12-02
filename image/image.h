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

#ifndef IMAGE_H
#define IMAGE_H

#include "utils/ref.h"

#include "core/hash.h"

class Image : public RefCountCallback
{
public:
	Image(unsigned int width, unsigned int height) : RefCountCallback(), m_recipeHash(0), m_width(width), m_height(height)
	{
	}

	enum ImageType
	{
		IMAGE_CHANNELS_1				= 1 << 0,
		IMAGE_CHANNELS_3				= 1 << 1,

		IMAGE_FORMAT_FLOAT				= 1 << 2,
		IMAGE_FORMAT_HALF				= 1 << 3,
		IMAGE_FORMAT_BYTE				= 1 << 4,

		IMAGE_FORMAT_NATIVE				= 1 << 5,

		IMAGE_FLAGS_BRIGHTNESS			= 1 << 6,
		IMAGE_FLAGS_ALPHA				= 1 << 7,
		IMAGE_FLAGS_EXACT				= 1 << 8,

		// Post-modifier with Op...
		IMAGE_FLAGS_INVERT				= 1 << 10,

		IMAGE_FLAGS_LOAD_MIPMAPS		= 1 << 12,
		IMAGE_FLAGS_GENERATE_MIPMAPS	= 1 << 13,

		// hacks for the moment...
		IMAGE_CONSTRAINTS_MIPMAP_LEVEL_MINUS1	= 1 << 15,
		IMAGE_CONSTRAINTS_MIPMAP_LEVEL_MINUS2	= 1 << 16,

		IMAGE_NO_CACHING				= 1 << 24
	};

	virtual ~Image()
	{
	}

	virtual unsigned int getImageType() const = 0;

	void removeReferenceFromImageCache();

	unsigned int getWidth() const { return m_width; }
	unsigned int getHeight() const { return m_height; }

	HashValue getRecipeHash() const { return m_recipeHash; }
	void setRecipeHash(HashValue hash) { m_recipeHash = hash; }

	virtual size_t getImageMemorySize() const { return 0; }

protected:
	HashValue			m_recipeHash;

	unsigned int		m_width;
	unsigned int		m_height;
};

#endif // IMAGE_H
