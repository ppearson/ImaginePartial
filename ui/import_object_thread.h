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

#ifndef IMPORT_OBJECT_THREAD_H
#define IMPORT_OBJECT_THREAD_H

#include <string>
#include <vector>

#include <QThread>

#include "objects/mesh.h"
#include "io/geo_reader.h"

namespace Imagine
{

class Scene;

class ImportObjectThread : public QThread
{
	Q_OBJECT
public:
	explicit ImportObjectThread(QObject *parent = 0);
	virtual ~ImportObjectThread();

	void importObjects(const std::vector<std::string>& aPaths, const GeoReaderOptions& options, Scene* pScene);

signals:
	void importedObjects(bool success, std::vector<Object*>* aObjects, std::vector<Material*>* pMaterials);

protected:
	virtual void run();

	std::vector<std::string> m_paths;
	GeoReaderOptions	m_importOptions;
	Scene*				m_pScene;
};

} // namespace Imagine

#endif // IMPORT_OBJECT_THREAD_H
