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

#include "scene_builder.h"

#include "object.h"
#include "scene.h"

namespace Imagine
{

SceneBuilder::SceneBuilder() : ParametersInterface()
{
}

SceneBuilder::~SceneBuilder()
{
}

void SceneBuilder::addObject(Scene& scene, Object* pObject)
{
	pObject->constructGeometry();
	scene.addObject(pObject, false, true); // don't set name, but set object ID
}

} // namespace Imagine
