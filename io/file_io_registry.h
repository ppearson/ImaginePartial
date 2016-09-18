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

#ifndef FILE_IO_REGISTRY_H
#define FILE_IO_REGISTRY_H

#include <string>
#include <map>
#include <set>

namespace Imagine
{

class GeoReader;
class GeoWriter;

class SceneReader;

class ImageReader;
class ImageWriter;

class VolumeReader;

class FileIORegistry
{
public:
	static FileIORegistry& instance()
	{
		static FileIORegistry singleton;
		return singleton;
	}

	typedef GeoReader* (*CreateGeoReaderCallback)();
	typedef GeoWriter* (*CreateGeoWriterCallback)();

	typedef SceneReader* (*CreateSceneReaderCallback)();

	typedef ImageReader* (*CreateImageReaderCallback)();
	typedef ImageWriter* (*CreateImageWriterCallback)();

	typedef VolumeReader* (*CreateVolumeReaderCallback)();

protected:
	typedef std::map<std::string, CreateGeoReaderCallback>		GeoReaderCallbacks;
	typedef std::map<std::string, CreateGeoWriterCallback>		GeoWriterCallbacks;
	typedef std::map<std::string, CreateSceneReaderCallback>	SceneReaderCallbacks;
	typedef std::map<std::string, CreateImageReaderCallback>	ImageReaderCallbacks;
	typedef std::map<std::string, CreateImageWriterCallback>	ImageWriterCallbacks;
	typedef std::map<std::string, CreateVolumeReaderCallback>	VolumeReaderCallbacks;

public:
	bool registerGeoReader(const std::string& extension, CreateGeoReaderCallback createReaderCB);
	bool registerGeoWriter(const std::string& extension, CreateGeoWriterCallback createWriterCB);
	bool registerSceneReader(const std::string& extension, CreateSceneReaderCallback createReaderCB);
	bool registerImageReader(const std::string& extension, CreateImageReaderCallback createReaderCB,
							 bool supportsPartialReads = false);
	// extensions separated by ";" char
	bool registerImageReaderMultipleExtensions(const std::string& extensions, CreateImageReaderCallback createReaderCB,
											   bool supportsPartialReads = false);
	bool registerImageWriter(const std::string& extension, CreateImageWriterCallback createWriterCB);
	bool registerVolumeReader(const std::string& extension, CreateVolumeReaderCallback createReaderCB);

	GeoReader* createGeometryReaderForExtension(const std::string& extension) const;
	GeoWriter* createGeometryWriterForExtension(const std::string& extension) const;
	SceneReader* createSceneReaderForExtension(const std::string& extension) const;
	ImageReader* createImageReaderForExtension(const std::string& extension) const;
	ImageWriter* createImageWriterForExtension(const std::string& extension) const;
	VolumeReader* createVolumeReaderForExtension(const std::string& extension) const;

	bool doesImageReaderSupportPartialReads(const std::string& extension) const;

	template <typename T>
	static std::string getQtFileBrowserFilterForCallbackMap(const std::map<std::string, T>& map1)
	{
		std::string finalString;
		/*
		std::map<std::string, T>::const_iterator it = map1.begin();
		unsigned int count = 0;
		for (; it != map1.end(); ++it, count++)
		{
			const std::string& extension = (*it).first;

			if (count > 0)
			{
				finalString += " ";
			}

			finalString += "*." + extension;
		}
*/
		return finalString;
	}

	std::string getQtFileBrowserFilterForRegisteredGeometryReaders() const;
	std::string getQtFileBrowserFilterForRegisteredSceneReaders() const;
	std::string getQtFileBrowserFilterForRegisteredImageReaders() const;
	std::string getQtFileBrowserFilterForRegisteredImageWriters() const;
	std::string getQtFileBrowserFilterForRegisteredVolumeReaders() const;

protected:
	GeoReaderCallbacks			m_geoReaders;
	GeoWriterCallbacks			m_geoWriters;

	SceneReaderCallbacks		m_sceneReaders;

	ImageReaderCallbacks		m_imageReaders;

	// list of extensions that can support partial reading for texture caching
	std::set<std::string>		m_imageReadersPartialRead;

	ImageWriterCallbacks		m_imageWriters;

	VolumeReaderCallbacks		m_volumeReaders;

private:
	FileIORegistry()
	{ }

	FileIORegistry(const FileIORegistry& vc);

	FileIORegistry& operator=(const FileIORegistry& vc);
};

} // namespace Imagine

#endif // FILE_IO_REGISTRY_H
