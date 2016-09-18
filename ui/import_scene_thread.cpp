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

#include "import_scene_thread.h"

#include <vector>

#include "object.h"
#include "materials/material.h"

#include "utils/string_helpers.h"
#include "utils/file_helpers.h"

#include "io/file_io_registry.h"

#include "io/scene_reader.h"

#include <algorithm>

#include "scene.h"

namespace Imagine
{

ImportSceneThread::ImportSceneThread(Scene& scene, QObject* parent) : QThread(parent), m_scene(scene)
{
}

ImportSceneThread::~ImportSceneThread()
{
	wait();
}

void ImportSceneThread::importScene(const std::vector<std::string>& aPaths, const SceneReaderOptions& options)
{
	m_paths = aPaths;

	m_importOptions = options;

	start();
}

void ImportSceneThread::run()
{
	// this needs to get cleaned up by the receiver...
	std::vector<Object*>* aObjects = new std::vector<Object*>;
	std::vector<Material*>* aMaterials = new std::vector<Material*>;

	SceneReaderResults results;

	std::vector<std::string>::iterator it = m_paths.begin();
	std::vector<std::string>::iterator itEnd = m_paths.end();
	for (; it != itEnd; ++it)
	{
		const std::string& path = *it;

		std::string extension = FileHelpers::getFileExtension(path);

		SceneReader* pReader = FileIORegistry::instance().createSceneReaderForExtension(extension);

		if (!pReader)
			continue; // don't support this format

		pReader->readFile(path, m_importOptions, results);

		delete pReader;
		pReader = NULL;
	}

	// add results objects to final ones on heap...

	std::vector<Object*>::iterator itObject = results.objects.begin();
	for (; itObject != results.objects.end(); ++itObject)
	{
		Object* pObject = *itObject;
		aObjects->push_back(pObject);
	}

	std::vector<Material*>::iterator itMat = results.materials.begin();
	for (; itMat != results.materials.end(); ++itMat)
	{
		Material* pMaterial = *itMat;
		aMaterials->push_back(pMaterial);
	}

	emit importedScene(true, aObjects, aMaterials);
}

} // namespace Imagine
