/*
 Imagine
 Copyright 2015-2016 Peter Pearson.

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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <stdio.h>
#include <stdint.h>

// base class for Imagine plugins...

namespace Imagine
{

#define IMAGINE_PLUGIN_API_VERSION 1

#define IMAGINE_PLUGIN_EXPORT // nothing for Linux / OS X, but let's stick it here just in case...

// forward declarations of possible plugin base classes, which plugins have to (in preference)
// return new classes of themselves from static functions named appropriately based on the plugin type

class ImageReader;
class ImageWriter;
class GeoReader;
class GeoWriter;
class SceneReader;
class VolumeReader;

class Plugin
{
public:
	Plugin()
	{
	}

	virtual ~Plugin()
	{
	}

	// for the plugin to fill in, via the macros provided below
	struct PluginDescription
	{
		const char*		name;
		const char*		originalSourceFile;

		unsigned char	type;

		const char*		fileExtension; // this is only needed for file type plugins, and can contain multiple extensions separated by ';'

		unsigned int	pluginVersion;

		unsigned char	apiVersion;
	};

	// main macro for declaring a generic non-file-specific plugin
#define REGISTER_STANDARD_PLUGIN(PLUGIN_CLASS, PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_TYPE)					\
	extern "C"																								\
	{																										\
		IMAGINE_PLUGIN_EXPORT Imagine::Plugin* getPluginClass()												\
		{																									\
		  static PLUGIN_CLASS singleton;																	\
		  return &singleton;																				\
		}																									\
																											\
		IMAGINE_PLUGIN_EXPORT PLUGIN_CLASS* createNewPluginInstance()										\
		{																									\
			return new PLUGIN_CLASS();																		\
		}																									\
																											\
		IMAGINE_PLUGIN_EXPORT void describePlugin(struct Imagine::Plugin::PluginDescription* description)	\
		{																									\
			description->apiVersion = IMAGINE_PLUGIN_API_VERSION;											\
			description->name = PLUGIN_NAME;																\
			description->originalSourceFile = __FILE__;														\
			description->pluginVersion = PLUGIN_VERSION;													\
			description->type = PLUGIN_TYPE;																\
		}																									\
	}

	// main macro for declaring a file-specific single plugin (image reader, image writer, geo reader, etc).
	// PLUGIN_FILEIO_EXTENSION should just be the letters of the extension
#define REGISTER_FILEIO_PLUGIN(PLUGIN_CLASS, BASE_CLASS, PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_TYPE, PLUGIN_FILE_EXTENSION)	\
	extern "C"																												\
	{																														\
		IMAGINE_PLUGIN_EXPORT Imagine::Plugin* getPluginClass()																\
		{																													\
		  static PLUGIN_CLASS singleton;																					\
		  return &singleton;																								\
		}																													\
																															\
		IMAGINE_PLUGIN_EXPORT BASE_CLASS* createFilePluginInstance()														\
		{																													\
			return new PLUGIN_CLASS();																						\
		}																													\
																															\
		IMAGINE_PLUGIN_EXPORT void describePlugin(struct Imagine::Plugin::PluginDescription* description)					\
		{																													\
			description->apiVersion = IMAGINE_PLUGIN_API_VERSION;															\
			description->name = PLUGIN_NAME;																				\
			description->originalSourceFile = __FILE__;																		\
			description->pluginVersion = PLUGIN_VERSION;																	\
			description->type = PLUGIN_TYPE;																				\
			description->fileExtension = PLUGIN_FILE_EXTENSION;																\
		}																													\
	}

	enum PluginType
	{
		ImaginePluginNone = 0, // as default for invalid / none

		// below are file plugin types that just read/write files
		ImaginePluginImageReader = 1,
		ImaginePluginImageWriter = 2,
		ImaginePluginGeoReader = 3,
		ImaginePluginGeoWriter = 4,
		ImaginePluginSceneReader = 5,
		ImaginePluginVolumeReader = 6,

		// below are plugin types that describe or create geometry
		ImaginePluginPrimitive = 7,
		ImaginePluginProcedural = 8,
		ImaginePluginSceneBuilder = 9,

		ImaginePluginOutputDriver = 10, // (basically a display driver)
		ImaginePluginBSDF = 11,
		ImaginePluginTexture = 12,
		ImaginePluginShapeOp = 13
	};


	// above macros will provide these two
	// definition of describe function
	IMAGINE_PLUGIN_EXPORT typedef void (*describeImaginePluginFP_t)(struct PluginDescription* description);

	// TODO: this is pretty poor - other alternatives are providing pure virtual interfaces to everything,
	// or Nuke-style registration of constructors per plugin type which return the correct pointer type...
	IMAGINE_PLUGIN_EXPORT typedef void* (*createImaginePluginClassFP_t)();

	// this is more preferable
	IMAGINE_PLUGIN_EXPORT typedef void* (*createImagineFilePluginClassFP_t)();

	// this is optional and advanced, and is only for specialising multiple plugins in one dynamic library
	IMAGINE_PLUGIN_EXPORT typedef void* (*createImagineFilePluginClassCombinedFP_t)(int read, const char* extension); //

};

} // namespace Imagine

#endif // PLUGIN_H

