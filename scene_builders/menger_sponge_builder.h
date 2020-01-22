/*
 Imagine
 Copyright 2019 Peter Pearson.

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

#ifndef MENGER_SPONGE_BUILDER_H
#define MENGER_SPONGE_BUILDER_H

#include "scene_builder.h"

#include <vector>

#include "core/boundary_box.h"

namespace Imagine
{

class Vector;
class Material;
class Object;

class MengerSpongeBuilder : public SceneBuilder
{
public:
	MengerSpongeBuilder();

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);

	virtual void createScene(Scene& scene);

protected:
	void generateSubCubes(std::vector<Object*>& cubes, const BoundaryBox& overallBBox,
						  int levelsRemaining, int levelCount, std::vector<float>& subCubeSizes, std::vector<Point>* savePositions);

protected:
	unsigned int			m_iterations;
	float					m_overallWidth;

	bool					m_makeGroup;

	float					m_gap;
	
	unsigned char			m_savePositionsType;
	std::string				m_savePath;
};

} // namespace Imagine

#endif // MENGER_SPONGE_BUILDER_H
