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

#ifndef IMPORT_SCENE_THREAD_H
#define IMPORT_SCENE_THREAD_H

#include <string>
#include <vector>

#include <QThread>

#include "io/scene_reader.h"

class Scene;

class ImportSceneThread : public QThread
{
	Q_OBJECT
public:
	ImportSceneThread(Scene& scene, QObject* parent = 0);
	virtual ~ImportSceneThread();

	void importScene(const std::vector<std::string>& aPaths, const SceneReaderOptions& options);

signals:
	void importedScene(bool success, std::vector<Object*>* aObjects, std::vector<Material*>* pMaterials);

protected:
	virtual void run();

	Scene&						m_scene;
	std::vector<std::string>	m_paths;
	SceneReaderOptions			m_importOptions;
};

#endif // IMPORT_SCENE_THREAD_H
