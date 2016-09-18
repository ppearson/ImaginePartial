/*
 Imagine
 Copyright 2013 Peter Pearson.

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

#ifndef SCENE_READER_H
#define SCENE_READER_H

#include <string>
#include <vector>

#include "file_io_registry.h"

namespace Imagine
{

class Object;
class Material;

struct SceneReaderOptions
{
	SceneReaderOptions()
	{
		importCameras = false;
		importMaterials = false;
		createMaterialsFromFacesets = true;
		processFacesets = true;
		setItemsAsBBox = false;
		useInstances = true;
	}

	bool			importCameras;
	bool			importMaterials;
	bool			createMaterialsFromFacesets;
	bool			processFacesets;
	bool			setItemsAsBBox;
	bool			useInstances;
};

struct SceneReaderResults
{
	std::vector<Object*>	objects;
	std::vector<Material*>	materials;
};

class SceneReader
{
public:
	SceneReader()
	{
	}

	virtual ~SceneReader()
	{
	}

	virtual bool readFile(const std::string& path, const SceneReaderOptions& options, SceneReaderResults& results) = 0;
};

} // namespace Imagine

#endif // SCENE_READER_H
