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

#ifndef SCENE_BUILDER_H
#define SCENE_BUILDER_H

#include "scene_builder_factory.h"

#include "parameter.h"

namespace Imagine
{

class Scene;
class Object;

class SceneBuilder : public ParametersInterface
{
public:
	SceneBuilder();
	virtual ~SceneBuilder();

	virtual unsigned char getSceneBuilderTypeID() = 0;
	virtual std::string getSceneBuilderDescription() = 0;

	virtual void buildParameters(Parameters& parameters, unsigned int flags) = 0;
	virtual bool controlChanged(const std::string& name, PostChangedActions& postChangedActions)
	{
		return false;
	}

	virtual void createScene(Scene& scene) = 0;

	void addObject(Scene& scene, Object* pObject);

	virtual bool hasParameterUndo() const { return false; }
};

} // namespace Imagine

#endif // SCENE_BUILDER_H
