/*
 Imagine
 Copyright 2015-2016 Peter Pearson.

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

#ifndef PLANKSTRUCTUREBUILDER_H
#define PLANKSTRUCTUREBUILDER_H

#include "scene_builder.h"

#include <vector>

namespace Imagine
{

class Vector;
class StandardGeometryInstance;
class Material;

class PlankStructureBuilder : public SceneBuilder
{
public:
	PlankStructureBuilder();

	enum StructureType
	{
		eSimpleTower1,
		eSquareBuilding1
	};

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);

	virtual void createScene(Scene& scene);


protected:
	void createNewPlank(std::vector<Object*>& aPlanks, const Vector& pos, const Vector& rot) const;
	void createNewPlank(std::vector<Object*>& aPlanks, const Vector& pos) const;

	void createSimpleTower1(std::vector<Object*>& aPlanks, Scene& scene);
	void createSquareBuilding(std::vector<Object*>& aPlanks, Scene& scene);

protected:
	unsigned int			m_width;
	unsigned int			m_depth;

	StructureType			m_structureType;
	unsigned int			m_layers;
	bool					m_makeGroup;

	float					m_variance;
	float					m_gap;


	// for mesh sizes only
	float					m_plankLengthScale;
	float					m_plankHeightScale;
	float					m_plankWidthScale;

	std::vector<StandardGeometryInstance*>	m_aSourcePlanks;
};

} // namespace Imagine

#endif // PLANKSTRUCTUREBUILDER_H
