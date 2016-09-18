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

#ifndef IMAGE_TEXTURE_COMMON_H
#define IMAGE_TEXTURE_COMMON_H

#include <vector>

#include "colour/colour3f.h"

#include "utils/hints.h"
#include "utils/time_counter.h"

namespace Imagine
{

class ImageTextureFileHandle
{
public:
	ImageTextureFileHandle() : m_isOpen(false)
	{

	}

	virtual ~ImageTextureFileHandle()
	{

	}

	void setOpen(bool open) { m_isOpen = open; }

	bool isOpen() { return m_isOpen; }

	// will be called by ImageTextureCache when it wants to free file handles...
	// technically, we could just rely on the destructor to do the work of closing the file,
	// but this is nicer and provides the ability to give some feedback
	virtual bool close()
	{
		return true;
	}


protected:
	// TODO: not used currently, but we need to do something about existing and to-all-
	//       intents-and-purposes valid from a code point-of-view ImageTextureFileHandles thinking
	//       they are valid, but the internal reader-specific file handle actually not being
	//       valid - NFS issues are a good example of this, so this might be a mechanism to allow
	//       to do sweeps of these orphaned handles in the future...
	bool			m_isOpen;
};

class ImageTextureCustomData
{
public:
	ImageTextureCustomData()
	{

	}

	virtual ~ImageTextureCustomData()
	{

	}
};

// represents one image, or one image's mipmap level

// TODO: currently doesn't cope with data window / ROI, although TBH I hope it's not needed, although
//       overscan for projection type textures might get interesting...
class ImageTextureItemDetails
{
public:
	ImageTextureItemDetails() : m_width(0), m_height(0), m_floatWidth(0.0f), m_floatHeight(0.0f), m_tileWidth(0),
		m_tileHeight(0), m_tileCountX(0), m_tileCountY(0)
	{

	}

	ImageTextureItemDetails(unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
		: m_width(width), m_height(height), m_floatWidth(0.0f), m_floatHeight(0.0f), m_tileWidth(tileWidth),
		  m_tileHeight(tileHeight), m_tileCountX(0), m_tileCountY(0)
	{

	}

	void setWidth(unsigned int width)
	{
		m_width = width;
		m_floatWidth = (float)width;
	}

	void setHeight(unsigned int height)
	{
		m_height = height;
		m_floatHeight = (float)height;
	}

	unsigned int getWidth() const { return m_width; }
	unsigned int getHeight() const { return m_height; }

	float getFloatWidth() const { return m_floatWidth; }
	float getFloatHeight() const { return m_floatHeight; }

	void setTileWidth(unsigned int tileWidth) { m_tileWidth = tileWidth; }
	void setTileHeight(unsigned int tileHeight) { m_tileHeight = tileHeight; }

	unsigned int getTileWidth() const { return m_tileWidth; }
	unsigned int getTileHeight() const { return m_tileHeight; }

	void setTileCountX(unsigned int tileCountX) { m_tileCountX = tileCountX; }
	void setTileCountY(unsigned int tileCountY) { m_tileCountY = tileCountY; }

	unsigned int getTileCountX() const { return m_tileCountX; }
	unsigned int getTileCountY() const { return m_tileCountY; }

protected:
	unsigned int		m_width;
	unsigned int		m_height;
	float				m_floatWidth;
	float				m_floatHeight;

	unsigned short		m_tileWidth;
	unsigned short		m_tileHeight;

	unsigned short		m_tileCountX;
	unsigned short		m_tileCountY;

	// TODO: we currently assume channel count and data type is consistent for all levels
	//       which is currently a reasonable assumption, but we might be able to add more flexibility
	//       in the future for doing higher levels at lower precision.
};

class ImageTextureDetails
{
public:
	ImageTextureDetails() : m_pCustomData(NULL), m_fullWidth(0), m_fullHeight(0), m_flipY(false),
		m_isTiled(false), m_isMipmapped(false), m_isConstant(false), m_needsPerThreadFileHandles(true),
		m_channelCount(0), m_dataType(eUnknown), m_wrapMode(eClamp)
	{

	}

	~ImageTextureDetails()
	{
		if (m_pCustomData)
		{
			delete m_pCustomData;
			m_pCustomData = NULL;
		}
	}

	enum ImageDataType
	{
		eUnknown,
		eFloat,
		eHalf,
		eUInt32,
		eUInt16,
		eUInt8,
		eInt32,
		eInt16,
		eInt8
	};

	enum ImageWrapMode
	{
		eClamp,
		eBlack,
		ePeriodic
	};

	void setFilePath(const std::string& filePath) { m_filePath = filePath; }
	const std::string& getFilePath() const { return m_filePath; }

	// this assumes it is not set before hand which should always be the case
	void setCustomData(const ImageTextureCustomData* pData) { m_pCustomData = pData; }
	const ImageTextureCustomData* getCustomData() const { return m_pCustomData; }

	void setFullWidth(unsigned int fullWidth) { m_fullWidth = (float)fullWidth; }
	void setFullHeight(unsigned int fullHeight) { m_fullHeight = (float)fullHeight; }

	float getFullWidth() const { return m_fullWidth; }
	float getFullHeight() const { return m_fullHeight; }

	void setFlipY(bool flipY) { m_flipY = flipY; }
	bool isFlipY() const { return m_flipY; }

	void setIsTiled(bool isTiled) { m_isTiled = isTiled; }
	bool isTiled() const { return m_isTiled; }

	void setMipmapped(bool mipmapped) { m_isMipmapped = mipmapped; }
	bool isMipmapped() const { return m_isMipmapped; }

	// whether the entire image is constant or not
	void setIsConstant(bool isConstant) { m_isConstant = isConstant; }
	bool isConstant() const { return m_isConstant; }

	void setNeedsPerThreadFileHandles(bool needsPerThreadFileHandles) { m_needsPerThreadFileHandles = needsPerThreadFileHandles; }
	bool isNeedsPerThreadFileHandles() const { return m_needsPerThreadFileHandles; }

	void setChannelCount(unsigned int channelCount) { m_channelCount = channelCount; }
	unsigned int getChannelCount() const { return m_channelCount; }

	void setDataType(ImageDataType dataType) { m_dataType = dataType; }
	ImageDataType getDataType() const { return m_dataType; }

	void setWrapMode(ImageWrapMode wrapMode) { m_wrapMode = wrapMode; }
	ImageWrapMode getWrapMode() const { return m_wrapMode; }

	const std::vector<ImageTextureItemDetails>& getMipmaps() const { return m_aMipmaps; }
	std::vector<ImageTextureItemDetails>& getMipmaps() { return m_aMipmaps; }

protected:
	std::string					m_filePath;

	// optional custom data that an image reader can set, and then get access to later when reading tiles.
	// once this is set, ImageTextureCache owns it and will free it with the ImageTextureItem
	const ImageTextureCustomData*	m_pCustomData;

	// don't really need these, as they're duplicates of the base level mipmap's items, but
	// storing them as float prevents us having to cast when working out texture filtering regions...
	float						m_fullWidth;
	float						m_fullHeight;

	bool						m_flipY;

	bool						m_isTiled;
	bool						m_isMipmapped;
	bool						m_isConstant;

	bool						m_needsPerThreadFileHandles;

	unsigned short				m_channelCount;

	ImageDataType				m_dataType;
	ImageWrapMode				m_wrapMode; // TODO: support separate s/t wrap modes?

	std::vector<ImageTextureItemDetails>	m_aMipmaps;
};

// these classes are used and passed in to FileReaders intentionally, as it allows more flexibility
// in terms of locking and controlling who and how updates what in the main cache, and and prevents
// having to pass in a non-const ImageTextureDetails

// input parameter
class ImageTextureTileReadParams
{
public:
	__finline ImageTextureTileReadParams(const ImageTextureDetails& textureDetails, unsigned int mmLevel, unsigned int tX,
							   unsigned int tY, size_t pSize, unsigned char* pD) : m_imageDetails(textureDetails), mipmapLevel(mmLevel),
								tileX(tX), tileY(tY), pixelSize(pSize), pData(pD), m_pExistingFileHandle(NULL), m_allowedToLeaveFileHandleOpen(false),
								m_wantStats(true)
	{

	}

	__finline const ImageTextureDetails& getImageDetails() const { return m_imageDetails; }

	void setExistingFileHandle(ImageTextureFileHandle* pExistingFileHandle) { m_pExistingFileHandle = pExistingFileHandle; }
	ImageTextureFileHandle* getExistingFileHandle() const { return m_pExistingFileHandle; }

	void setAllowedToLeaveFileHandleOpen(bool leaveOpen) { m_allowedToLeaveFileHandleOpen = leaveOpen; }
	bool isAllowedToLeaveFileHandleOpen() const { return m_allowedToLeaveFileHandleOpen; }

	// whether we want timing stats or not (there's a slight overhead to using the timers)
	void setWantStats(bool wantStats) { m_wantStats = wantStats; }
	bool wantStats() const { return m_wantStats; }

protected:
	const ImageTextureDetails&		m_imageDetails;

public:
	unsigned int mipmapLevel;
	unsigned int tileX;
	unsigned int tileY;

	size_t		pixelSize; // size of one pixel (data type * num channels) in bytes

	// passed in pointer to memory which will be big enough for the tile dimensions and the image type / num channels
	// Note: it is important that all channels are read fully and the data copied to pData. ImageTextureCache is responsible
	//       for partial reading of values and determining what channels are actually needed for each request
	// TODO: this (all channels being allocated for) may change in the future...
	unsigned char* pData;

protected:
	// existing file handle which might optionally be set (non-NULL) that an image reader can cast to its own
	// derived class to use to get custom data/file handles in order to read tile data from existing file
	// handles
	ImageTextureFileHandle*		m_pExistingFileHandle;

	// this will be true if the Reader is allowed to "leak" the file handle, such that it can
	// be re-used later. If it's false, the reader must open and close a handle to a file within
	// the same call.
	// If a reader wants to "leak" a file handle, it must set the pointer in ImageTextureTileReadResults
	// to a non-NULL value, after which ImageTextureCache then owns and controls the handle object
	bool	m_allowedToLeaveFileHandleOpen;

	// whether timing stats during the readImageTile() call are wanted...
	bool	m_wantStats;
};

// output parameter
class ImageTextureTileReadResults
{
public:
	__finline ImageTextureTileReadResults() : m_pNewFileHandle(NULL), m_existingFileHandleUpdated(false), m_openedFile(false),
		m_tileWasConstant(false), m_pConstantData(NULL), m_rawBytesRead(0)
	{
	}

	void setNewFileHandle(ImageTextureFileHandle* pFileHandle, bool updated = false)
	{
		m_pNewFileHandle = pFileHandle;
		m_existingFileHandleUpdated = updated;
	}

	ImageTextureFileHandle* getNewFileHandle() { return m_pNewFileHandle; }

	bool wasFileHandleUpdated() const { return m_existingFileHandleUpdated; }

	// this assumes passed in data will be 1x1 res in size
	void setTileConstant(unsigned char* pConstantData)
	{
		m_tileWasConstant = true;
		m_pConstantData = pConstantData;
	}

	void setFileOpenedThisRequest()
	{
		m_openedFile = true;
	}

	bool wasFileOpenedThisRequest() const
	{
		return m_openedFile;
	}

	unsigned char* getConstantData()
	{
		if (!m_tileWasConstant || !m_pConstantData)
			return NULL;

		return m_pConstantData;
	}

	// time taken to open files so as to be ready to read pixel data / metadata
	TimerCounter& getFileOpenTimer() { return m_fileOpenTimer; }
	const TimerCounter& getFileOpenTimer() const { return m_fileOpenTimer; }
	// time taken to prepare to read pixel data - this is really optional, and is mainly here for timing TIFF's
	// TIFFSetDirectory() to switch mipmap levels, but can also be used to count per-read setup before each tile read
	TimerCounter& getFileSeekTimer() { return m_fileSeekTimer; }
	const TimerCounter& getFileSeekTimer() const { return m_fileSeekTimer; }
	// time taken to actually read pixel data into memory (including any decompression / bit shuffling)
	TimerCounter& getFileReadTimer() { return m_fileReadTimer; }
	const TimerCounter& getFileReadTimer() const { return m_fileReadTimer; }

protected:
	// we don't own this - a reader may optionally create a new one if allowed, and will set this pointer in that case.
	// ImageTextureCache will then take the pointer and own it from that point on, possibly passing it in to
	ImageTextureFileHandle*		m_pNewFileHandle;
	// this can be set to true if m_pNewFileHandle is valid, but contains an existing item ImageTextureCache knows about
	// but the internal file handle has been updated
	bool						m_existingFileHandleUpdated;

	bool						m_openedFile; // whether the reader opened the file or not for this request

	// these are optional...
	// indicates the tile asked for was actually constant, and the passed in memory therefore wasn't used
	// and can be de-allocated
	bool						m_tileWasConstant;

	// pointer to heap allocated constant value. Readers should allow this to "leak", as the ImageTextureCache will own this
	// and use it like a block of regular (non-constant) tile pixel data, and manage it.
	// If a file reader supports determining if tile data is constant this is recommended (if determining this is fast),
	// as ImageTextureCache slightly prioritises (in terms of keeping them) these allocations over regular tile data
	// when freeing/paging tile pixel data.
	// It's currently assumed that the number of channels and data type is identical to that of normal tile blocks, just
	// for one single pixel.
	// Note: Image reader's shouldn't use this method for entire images which are constant if they know that up front -
	//       the best approach there currently is to just register the image as 1x1 in size, with no mipmap levels and
	//       set the constant property to true. ImageTextureCache will recognise this when paging tiles and try not to
	//       drop those ones as much (if ever)
	unsigned char*		m_pConstantData; // we don't own this

	// not really too happy about this, but alternative is providing callbacks back to ImageTextureCache to start/stop
	// timers which the Reader calls, which seems rather convoluted and doesn't have any more guarentees in terms of enforcement
	TimerCounter		m_fileOpenTimer;
	TimerCounter		m_fileSeekTimer;
	TimerCounter		m_fileReadTimer;

	size_t				m_rawBytesRead; // optional counter for number of compressed (not final) bytes read off disk / from network
};

} // namespace Imagine

#endif // IMAGE_TEXTURE_COMMON_H
