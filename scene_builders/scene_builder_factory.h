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

#ifndef SCENE_BUILDER_FACTORY_H
#define SCENE_BUILDER_FACTORY_H

#include <map>
#include <string>

class SceneBuilder;

class SceneBuilderFactory
{
public:
	static SceneBuilderFactory& instance()
	{
		static SceneBuilderFactory singleton;
		return singleton;
	}

	typedef SceneBuilder* (*CreateSceneBuilderCallback)();

protected:
	typedef std::map<unsigned char, CreateSceneBuilderCallback>		SceneBuilderCallbacks;

public:
	typedef std::map<unsigned char, std::string>					SceneBuilderNames;

public:
	bool registerSceneBuilder(unsigned char id, std::string description, CreateSceneBuilderCallback createSceneBuilderCB);

	SceneBuilder* createSceneBuilderForTypeID(unsigned char typeID);

	SceneBuilderNames::iterator sceneBuilderNamesBegin() { return m_names.begin(); }
	SceneBuilderNames::iterator sceneBuilderNamesEnd() { return m_names.end(); }

protected:
	SceneBuilderCallbacks			m_sceneBuilders;
	SceneBuilderNames				m_names;

private:
	SceneBuilderFactory()
	{ }

	SceneBuilderFactory(const SceneBuilderFactory& vc);

	SceneBuilderFactory& operator=(const SceneBuilderFactory& vc);
};

#endif // SCENE_BUILDER_FACTORY_H
