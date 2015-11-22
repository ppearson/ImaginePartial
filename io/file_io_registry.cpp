/*
 Imagine
 Copyright 2011-2015 Peter Pearson.

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

#include "file_io_registry.h"

#include <vector>

#include "utils/string_helpers.h"

bool FileIORegistry::registerGeoReader(const std::string& extension, CreateGeoReaderCallback createReaderCB)
{
	m_geoReaders.insert(GeoReaderCallbacks::value_type(extension, createReaderCB));
	return true;
}

bool FileIORegistry::registerGeoWriter(const std::string& extension, CreateGeoWriterCallback createWriterCB)
{
	m_geoWriters.insert(GeoWriterCallbacks::value_type(extension, createWriterCB));
	return true;
}

bool FileIORegistry::registerSceneReader(const std::string& extension, CreateSceneReaderCallback createReaderCB)
{
	m_sceneReaders.insert(SceneReaderCallbacks::value_type(extension, createReaderCB));
	return true;
}

bool FileIORegistry::registerImageReader(const std::string& extension, CreateImageReaderCallback createReaderCB)
{
	m_imageReaders.insert(ImageReaderCallbacks::value_type(extension, createReaderCB));
	return true;
}

bool FileIORegistry::registerImageReaderMultipleExtensions(const std::string& extensions, CreateImageReaderCallback createReaderCB)
{
	std::vector<std::string> extensionsItems;

	split(extensions, extensionsItems, ";", 0);

	std::vector<std::string>::const_iterator itExt = extensionsItems.begin();
	for (; itExt != extensionsItems.end(); ++itExt)
	{
		const std::string& extension = *itExt;

		m_imageReaders.insert(ImageReaderCallbacks::value_type(extension, createReaderCB));
	}

	return true;
}

bool FileIORegistry::registerImageWriter(const std::string& extension, CreateImageWriterCallback createWriterCB)
{
	m_imageWriters.insert(ImageWriterCallbacks::value_type(extension, createWriterCB));
	return true;
}

bool FileIORegistry::registerVolumeReader(const std::string& extension, CreateVolumeReaderCallback createReaderCB)
{
	m_volumeReaders.insert(VolumeReaderCallbacks::value_type(extension, createReaderCB));
	return true;
}

GeoReader* FileIORegistry::createGeometryReaderForExtension(const std::string& extension)
{
	GeoReaderCallbacks::iterator itFind = m_geoReaders.find(extension);
	if (itFind != m_geoReaders.end())
		return (itFind->second)();

	return NULL;
}

GeoWriter* FileIORegistry::createGeometryWriterForExtension(const std::string& extension)
{
	GeoWriterCallbacks::iterator itFind = m_geoWriters.find(extension);
	if (itFind != m_geoWriters.end())
		return (itFind->second)();

	return NULL;
}


SceneReader* FileIORegistry::createSceneReaderForExtension(const std::string& extension)
{
	SceneReaderCallbacks::iterator itFind = m_sceneReaders.find(extension);
	if (itFind != m_sceneReaders.end())
		return (itFind->second)();

	return NULL;
}


ImageReader* FileIORegistry::createImageReaderForExtension(const std::string& extension)
{
	ImageReaderCallbacks::iterator itFind = m_imageReaders.find(extension);
	if (itFind != m_imageReaders.end())
		return (itFind->second)();

	return NULL;
}

ImageWriter* FileIORegistry::createImageWriterForExtension(const std::string& extension)
{
	ImageWriterCallbacks::iterator itFind = m_imageWriters.find(extension);
	if (itFind != m_imageWriters.end())
		return (itFind->second)();

	return NULL;
}

VolumeReader* FileIORegistry::createVolumeReaderForExtension(const std::string& extension)
{
	VolumeReaderCallbacks::iterator itFind = m_volumeReaders.find(extension);
	if (itFind != m_volumeReaders.end())
		return (itFind->second)();

	return NULL;
}

std::string FileIORegistry::getQtFileBrowserFilterForRegisteredGeometryReaders() const
{
	std::string finalString;
	GeoReaderCallbacks::const_iterator it = m_geoReaders.begin();
	unsigned int count = 0;
	for (; it != m_geoReaders.end(); ++it, count++)
	{
		const std::string& extension = (*it).first;

		if (count > 0)
		{
			finalString += " ";
		}

		finalString += "*." + extension;
	}

	return finalString;
}

std::string FileIORegistry::getQtFileBrowserFilterForRegisteredSceneReaders() const
{
	std::string finalString;
	SceneReaderCallbacks::const_iterator it = m_sceneReaders.begin();
	unsigned int count = 0;
	for (; it != m_sceneReaders.end(); ++it, count++)
	{
		const std::string& extension = (*it).first;

		if (count > 0)
		{
			finalString += " ";
		}

		finalString += "*." + extension;
	}

	return finalString;
}

std::string FileIORegistry::getQtFileBrowserFilterForRegisteredImageReaders() const
{
	std::string finalString;
	ImageReaderCallbacks::const_iterator it = m_imageReaders.begin();
	unsigned int count = 0;
	for (; it != m_imageReaders.end(); ++it, count++)
	{
		const std::string& extension = (*it).first;

		if (count > 0)
		{
			finalString += " ";
		}

		finalString += "*." + extension;
	}

	return finalString;
}

std::string FileIORegistry::getQtFileBrowserFilterForRegisteredVolumeReaders() const
{
	std::string finalString;
	VolumeReaderCallbacks::const_iterator it = m_volumeReaders.begin();
	unsigned int count = 0;
	for (; it != m_volumeReaders.end(); ++it, count++)
	{
		const std::string& extension = (*it).first;

		if (count > 0)
		{
			finalString += " ";
		}

		finalString += "*." + extension;
	}

	return finalString;
}

