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

#include "import_object_thread.h"

#include "utils/timer.h"
#include "utils/string_helpers.h"
#include "utils/file_helpers.h"

#include "scene.h"

#include "io/file_io_registry.h"

#include <algorithm>

#include "materials/material.h"

namespace Imagine
{

ImportObjectThread::ImportObjectThread(QObject* parent) : QThread(parent), m_pScene(NULL)
{
}

ImportObjectThread::~ImportObjectThread()
{
	wait();
}

void ImportObjectThread::importObjects(const std::vector<std::string>& aPaths, const GeoReaderOptions& options, Scene* pScene)
{
	m_paths = aPaths;
	m_importOptions = options;
	m_pScene = pScene;

	start();
}

void ImportObjectThread::run()
{
	Object* pMesh = NULL;
	// this needs to get cleaned up by the receiver...
	std::vector<Object*>* aObjects = new std::vector<Object*>;
	std::vector<Material*>* aMaterials = new std::vector<Material*>;

	{
//		Timer t1("import object");

		std::vector<std::string>::iterator it = m_paths.begin();
		std::vector<std::string>::iterator itEnd = m_paths.end();
		for (; it != itEnd; ++it)
		{
			const std::string& path = *it;

			std::string extension = FileHelpers::getFileExtension(path);

			GeoReader* pReader = FileIORegistry::instance().createGeometryReaderForExtension(extension);

			if (!pReader)
				continue; // don't support this format

			pReader->setScene(m_pScene);

			bool result = pReader->readFile(path, m_importOptions);

			if (result)
			{
				pMesh = pReader->getNewObject();

				std::vector<Material*>& pNewMats = pReader->getNewMaterials();
				std::vector<Material*>::iterator itMat = pNewMats.begin();
				for (; itMat != pNewMats.end(); ++itMat)
				{
					Material* pMaterial = *itMat;
					aMaterials->push_back(pMaterial);
				}

				aObjects->push_back(pMesh);
			}

			if (pReader)
			{
				delete pReader;
				pReader = NULL;
			}
		}
	}

	emit importedObjects(true, aObjects, aMaterials);
}

} // namespace Imagine
