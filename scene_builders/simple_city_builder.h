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

#ifndef SIMPLE_CITY_BUILDER_H
#define SIMPLE_CITY_BUILDER_H

#include "scene_builder.h"

namespace Imagine
{

class SimpleCityBuilder : public SceneBuilder
{
public:
	SimpleCityBuilder();

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);

	virtual void createScene(Scene& scene);

protected:
	unsigned int	m_width;
	unsigned int	m_depth;

	unsigned int	m_numberOfBuildings;

	unsigned int	m_maxStories;

	unsigned int	m_distribution;
};

} // namespace Imagine

#endif // SIMPLE_CITY_BUILDER_H
