/*
 Imagine
 Copyright 2013-2014 Peter Pearson.

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

#ifndef INSTANCE_SURFACE_SCATTER_BUILDER_H
#define INSTANCE_SURFACE_SCATTER_BUILDER_H

#include "scene_builder.h"

#include <vector>

#include "core/point.h"
#include "core/boundary_box.h"

namespace Imagine
{

class RNG;
class CompoundObject;
class Object;
class Material;

class InstanceSurfaceScatterBuilder : public SceneBuilder
{
public:
	InstanceSurfaceScatterBuilder();

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);

	virtual void createScene(Scene& scene);


protected:
	void generateCandidateStartPoints(std::vector<Point>& points) const;

	struct TestPosInfo
	{
		TestPosInfo() : pNewCO(nullptr), pCurrentSelectedObject(nullptr), pSrcCOForBakedInstances(nullptr),
			 altMaterialIndex(0), createdCount(0), testedCount(0), switchCount(0)
		{

		}

		bool randomlyPickObject;
		bool doSwitchover;
		bool shouldMakeBakedInstances;

		bool scaleVariation;

		unsigned int numSelectedItems;

		BoundaryBox shapeBBox;
		float heightOffset;

		CompoundObject*	pNewCO;
		Object*			pCurrentSelectedObject;

		CompoundObject* pSrcCOForBakedInstances;
		std::vector<CompoundObject*> aMultipleSrcCO;

		std::vector<Material*>	aMaterials;
		unsigned int	altMaterialIndex;

		unsigned int	createdCount;
		unsigned int	testedCount;

		// used to control switchover
		unsigned int	switchCount;

		char szName[32];
	};

	bool testPositionPickAndAddObject(Scene& scene, const Point& position, RNG& rng, TestPosInfo& posInfo);

protected:
	float			m_width;
	float			m_depth;

	float			m_raycastStartHeight;

	unsigned int	m_targetInstanceCount;
	bool			m_exactNumber;

	unsigned int	m_distribution;

	float			m_surfaceYOffset;

	float			m_minimumCutoff;

	float			m_uniformScaleVariation;

	bool			m_alignToSurface;
	bool			m_randomYRotation;

	bool			m_drawAsBBox;

	bool			m_useBakedInstances;

	bool			m_addToGroup;

	bool			m_randomlyUseMultipleSelection;

	unsigned int	m_lastObjectSwitchover;

	bool			m_alternatingMaterials;
	unsigned int	m_numMaterials;
};

} // namespace Imagine

#endif // INSTANCE_SURFACE_SCATTER_BUILDER_H
