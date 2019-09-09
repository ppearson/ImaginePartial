/*
 Imagine
 Copyright 2017-2019 Peter Pearson.

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

#ifndef POINT_CLOUD_BUILDER_H
#define POINT_CLOUD_BUILDER_H

#include "scene_builder.h"

namespace Imagine
{

class PointCloudBuilder : public SceneBuilder
{
public:
	PointCloudBuilder();

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);
	virtual bool controlChanged(const std::string& name, PostChangedActions& postChangedActions);

	virtual void createScene(Scene& scene);

protected:
	float			m_pointRadius;
	unsigned char	m_type;
	
	unsigned int	m_distribution;
	unsigned int	m_boundShape;
	
	unsigned int	m_numberOfPoints;
	
	float			m_sampleRadius;
	
	unsigned char	m_savePointCloudType;
	std::string		m_savePath;
};

} // namespace Imagine

#endif // POINT_CLOUD_BUILDER_H
