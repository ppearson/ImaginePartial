/*
 Imagine
 Copyright 2020 Peter Pearson.

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

#ifndef DATA_CHART_BUILDER_H
#define DATA_CHART_BUILDER_H

#include "scene_builder.h"

#include <vector>
#include <string>

namespace Imagine
{

class Vector;
class StandardGeometryInstance;
class Material;

class DataChartBuilder : public SceneBuilder
{
public:
	DataChartBuilder();

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);
	
	virtual bool controlChanged(const std::string& name, PostChangedActions& postChangedActions);

	virtual void createScene(Scene& scene);

	
protected:
	void create3DSurfacePlot(Scene& scene);
	void create3DBarChart(Scene& scene);
	void create2DBarChart(Scene& scene);

	Object* createBarCuboid(float width, float depth, float height, const Vector& position) const;

	bool loadHeightData(const std::string& filePath, std::vector<float>& aHeightData) const;
	
	bool countDataDimensions(const std::string& filePath, unsigned int& xItems, unsigned int& yItems) const;

protected:

	unsigned char				m_type;
	unsigned char				m_dataType;

	std::string					m_dataFile;
	bool						m_autoDetectCounts;

	float						m_width;
	float						m_depth;
	float						m_height;

	bool						m_normaliseHeight;

	unsigned int				m_xItems;
	unsigned int				m_yItems;
	unsigned int				m_zItems;

	unsigned char				m_gapType;
	float						m_gapValue;
};

} // namespace Imagine

#endif // DATA_CHART_BUILDER_H
