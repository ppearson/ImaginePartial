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

#include "data_chart_builder.h"

#include <fstream>
#include <cstring>
#include <cstdio>
#include <set>

#include "objects/mesh.h"
#include "objects/compound_object.h"

#include "global_context.h"

#include "geometry/standard_geometry_instance.h"

#include "materials/standard_material.h"

#include "scene.h"

namespace Imagine
{

const char* dataChartBuilderTypeOptions[] = { "3D Surface plot mesh", "3D Bar chart grid", "2D Bar chart", 0 };
const char* dataChartBuilderDataTypeOptions[] = { "CSV: flattened list {int,int,float}", "CSV: flattened list {int,float}", 0 };
const char* dataChartBuilderGapTypeOptions[] = { "Ratio", "Absolute", 0 };

static uint32_t kChartBuilderCuboidPolyIndices[26] = { 0, 1, 2, 3, 7, 6, 5, 4, 3, 2, 6, 7, 2, 1, 5, 6, 1, 0, 4, 5, 0, 3, 7, 4 };
static uint32_t kChartBuilderCuboidPolyOffsets[6] = { 4, 8, 12, 16, 20, 24 };

DataChartBuilder::DataChartBuilder() : m_type(1), m_dataType(0), m_autoDetectCounts(true),
    m_width(20.0f), m_depth(20.0f), m_height(10.0f),
    m_normaliseHeight(false),
    m_xItems(32), m_yItems(32), m_zItems(32),
    m_gapType(0), m_gapValue(0.1f)
{

}

unsigned char DataChartBuilder::getSceneBuilderTypeID()
{
	return 14;
}

std::string DataChartBuilder::getSceneBuilderDescription()
{
	return "Data Chart";
}

void DataChartBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new EnumParameter("type", "type", (unsigned char*)&m_type, dataChartBuilderTypeOptions));
	parameters.addParameter(new EnumParameter("dataType", "data type", (unsigned char*)&m_dataType, dataChartBuilderDataTypeOptions));
	
	parameters.addParameter(new BasicParameter<std::string>("data_file_path", "data file", &m_dataFile, eParameterFile));
	parameters.addParameter(new BasicParameter<bool>("autodetect_counts", "autodetect counts", &m_autoDetectCounts, eParameterBool));
	
	parameters.addParameter(new RangeParameter<float, float>("width", "width", &m_width, eParameterFloat, 1.0f, 2000.0f, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<float, float>("depth", "depth", &m_depth, eParameterFloat, 1.0f, 2000.0f, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<float, float>("height", "height", &m_height, eParameterFloat, 1.0f, 2000.0f, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("normalise_height", "normalise height", &m_normaliseHeight, eParameterBool));
	
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("xItems", "x data items", &m_xItems,
																		   eParameterUInt, 1, 4096, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("yItems", "y data items", &m_yItems,
																		   eParameterUInt, 1, 4096, eParameterScrubButton));

	parameters.addParameter(new EnumParameter("gapType", "gap type", (unsigned char*)&m_gapType, dataChartBuilderGapTypeOptions));
	parameters.addParameter(new RangeParameter<float, float>("gapValue", "gap value", &m_gapValue, eParameterFloat, 0.0f, 5.0f, eParameterScrubButtonFine));
	
	parameters.setInitiallyHiddenParameter("height");
}

bool DataChartBuilder::controlChanged(const std::string& name, PostChangedActions& postChangedActions)
{
	if (name == "data_file_path")
	{
		unsigned int numItemsX = 0;
		unsigned int numItemsY = 0;
		if (countDataDimensions(m_dataFile, numItemsX, numItemsY))
		{
			m_xItems = numItemsX;
			m_yItems = numItemsY;
			
			postChangedActions.addRefreshItem("xItems");
			postChangedActions.addRefreshItem("yItems");
		}
	}
	else if (name == "type")
	{
		if (m_type == 2)
		{
			// 2D graph only
			m_dataType = 1;
			
			postChangedActions.addHideItem("depth");
			postChangedActions.addHideItem("yItems");
		}
		else
		{
			// 3D types
			m_dataType = 0;
			
			postChangedActions.addShowItem("depth");
			postChangedActions.addShowItem("yItems");
		}
		
		// This is less verbose, but also less obvious what the logic is from outside...
/*		
		postChangedActions.addVisibilityChangeItem("gapType", m_type != 0);
		postChangedActions.addVisibilityChangeItem("gapType", m_type != 0);
*/		
		if (m_type == 0)
		{
			postChangedActions.addHideItem("gapType");
			postChangedActions.addHideItem("gapValue");
		}
		else
		{
			postChangedActions.addShowItem("gapType");
			postChangedActions.addShowItem("gapValue");
		}
		
		postChangedActions.addRefreshItem("dataType");
	}
	else if (name == "normalise_height")
	{
		if (m_normaliseHeight)
		{
			postChangedActions.addShowItem("height");
		}
		else
		{
			postChangedActions.addHideItem("height");
		}
	}
	
	return true;
}

void DataChartBuilder::createScene(Scene& scene)
{
	if (m_type == 0)
	{
		create3DSurfacePlot(scene);
	}
	else if (m_type == 1)
	{
		create3DBarChart(scene);
	}
	else if (m_type == 2)
	{
		create2DBarChart(scene);
	}
}

void DataChartBuilder::create3DSurfacePlot(Scene& scene)
{
	if (m_dataFile.empty())
		return;
	
	std::vector<float> aHeightData;
	
	loadHeightData(m_dataFile, aHeightData);
	
	if (aHeightData.size() != (m_xItems * m_yItems))
	{
		GlobalContext::instance().getLogger().error("Invalid number of items in data file. Was expecting: %u, had: %zu.", (m_xItems * m_yItems), aHeightData.size());
		return;
	}
	
	float halfWidth = m_width / 2.0f;
	float halfDepth = m_depth / 2.0f;

	// Note: y here means depth, as opposed to height...
	
	float xIncrease = m_width / (float)(m_xItems - 1);
	float yIncrease = m_depth / (float)(m_yItems - 1);

	StandardGeometryInstance* pStandardGeoInstance = new StandardGeometryInstance();

	std::vector<Point>& aPoints = pStandardGeoInstance->getPoints();

	unsigned int reserveAmount = (m_xItems + 1) * (m_yItems + 1);
	aPoints.reserve(reserveAmount);

	unsigned int count = 0;

	for (unsigned int x = 0; x < m_xItems; x++)
	{
		float xVal = -halfWidth + (xIncrease * x);

		for (unsigned int y = 0; y < m_yItems; y++)
		{
			float yVal = -halfDepth + (yIncrease * y);

			float height = aHeightData[count++];

			Point newPoint(xVal, height, yVal);
			aPoints.emplace_back(newPoint);
		}
	}

	std::vector<uint32_t>& aPolyOffsets = pStandardGeoInstance->getPolygonOffsets();
	std::vector<uint32_t>& aPolyIndices = pStandardGeoInstance->getPolygonIndices();

	unsigned int numFaces = m_xItems * m_yItems;
	aPolyOffsets.reserve(numFaces + 1);
	aPolyIndices.reserve(numFaces * 4);

	unsigned int actualXQuads = m_xItems - 1;
	unsigned int actualYQuads = m_yItems - 1;

	for (unsigned int x = 0; x < actualXQuads; x++)
	{
		for (unsigned int y = 0; y < actualYQuads; y++)
		{
			unsigned int cornerIndex = (x * (m_yItems)) + y;

			unsigned int vertex0 = cornerIndex + 1;
			unsigned int vertex1 = cornerIndex;
			unsigned int vertex2 = cornerIndex + (m_yItems);
			unsigned int vertex3 = cornerIndex + (m_yItems) + 1;

			aPolyIndices.emplace_back(vertex3);
			aPolyIndices.emplace_back(vertex2);
			aPolyIndices.emplace_back(vertex1);
			aPolyIndices.emplace_back(vertex0);
		}
	}

	unsigned int startIndexCount = 4;
	for (unsigned int i = 0; i < actualXQuads * actualYQuads; i++)
	{
		aPolyOffsets.emplace_back(startIndexCount);
		startIndexCount += 4;
	}

	Mesh* pNewMesh = new Mesh();

	pNewMesh->setGeometryInstance(pStandardGeoInstance);

	pNewMesh->setName("Plot");

	pNewMesh->setDefaultMaterial();

	addObject(scene, pNewMesh);
}

void DataChartBuilder::create3DBarChart(Scene& scene)
{
	if (m_dataFile.empty())
		return;

	std::vector<float> aHeightData;

	loadHeightData(m_dataFile, aHeightData);

	if (aHeightData.size() != (m_xItems * m_yItems))
	{
		GlobalContext::instance().getLogger().error("Invalid number of items in data file. Was expecting: %u, had: %zu.", (m_xItems * m_yItems), aHeightData.size());
		return;
	}

	float halfWidth = m_width / 2.0f;
	float halfDepth = m_depth / 2.0f;

	// Note: y here means depth, as opposed to height...
	
	unsigned int numGapsX = m_xItems - 1;
	unsigned int numGapsY = m_yItems - 1;

	float xIncrease = m_width / (float)numGapsX;
	float yIncrease = m_depth / (float)numGapsY;

	float gapAmountX = 0.0f;
	float gapAmountY = 0.0f;

	if (m_gapValue > 0.0f && m_gapType == 0)
	{
		float gapRatio = std::min(m_gapValue, 0.999f);

		gapAmountX = xIncrease * gapRatio;
		gapAmountY = yIncrease * gapRatio;
	}
	else if (m_gapValue > 0.0f && m_gapType == 1)
	{
		gapAmountX = m_gapValue;
		gapAmountY = m_gapValue;
	}

	float cuboidWidths = xIncrease - gapAmountX;
	float cuboidDepths = yIncrease - gapAmountY;

	CompoundObject* pCO = new CompoundObject();

	unsigned int count = 0;

	for (unsigned int x = 0; x < m_xItems; x++)
	{
		float xVal = -halfWidth + (xIncrease * x);

		for (unsigned int y = 0; y < m_yItems; y++)
		{
			float yVal = -halfDepth + (yIncrease * y);

			float height = aHeightData[count++];

			// offset to centre of height
			Vector newPosition(xVal, height / 2.0f, yVal);

			Object* pNewBar = createBarCuboid(cuboidWidths, cuboidDepths, height, newPosition);
			
			pNewBar->setName("Bar_" + std::to_string(x) + "_" + std::to_string(y));

			pCO->addObject(pNewBar);
		}
	}

	pCO->updateBoundaryBox();

	pCO->setName("ChartGroup1");

	pCO->setType(CompoundObject::eBaked);

	addObject(scene, pCO);
}

void DataChartBuilder::create2DBarChart(Scene& scene)
{
	if (m_dataFile.empty())
		return;

	std::vector<float> aHeightData;

	loadHeightData(m_dataFile, aHeightData);

	if (aHeightData.size() != m_xItems)
	{
		GlobalContext::instance().getLogger().error("Invalid number of items in data file. Was expecting: %u, had: %zu.", m_xItems, aHeightData.size());
		return;
	}

	float halfWidth = m_width / 2.0f;

	unsigned int numGapsX = m_xItems - 1;

	float xIncrease = m_width / (float)numGapsX;

	float gapAmountX = 0.0f;

	///////

	if (m_gapValue > 0.0f && m_gapType == 0)
	{
		float gapRatio = std::min(m_gapValue, 0.999f);

		gapAmountX = xIncrease * gapRatio;
	}
	else if (m_gapValue > 0.0f && m_gapType == 1)
	{
		gapAmountX = m_gapValue;
	}

	float cuboidWidths = xIncrease - gapAmountX;

	CompoundObject* pCO = new CompoundObject();

	unsigned int count = 0;

	for (unsigned int x = 0; x < m_xItems; x++)
	{
		float xVal = -halfWidth + (xIncrease * x);

		float height = aHeightData[count++];

		// offset to centre of height
		Vector newPosition(xVal, height / 2.0f, 0.0f);

		Object* pNewBar = createBarCuboid(cuboidWidths, cuboidWidths, height, newPosition);
		
		pNewBar->setName("Bar_" + std::to_string(x));

		pCO->addObject(pNewBar);
	}

	pCO->updateBoundaryBox();

	pCO->setName("ChartGroup1");

	pCO->setType(CompoundObject::eBaked);

	addObject(scene, pCO);
}

Object* DataChartBuilder::createBarCuboid(float width, float depth, float height, const Vector& position) const
{
	StandardGeometryInstance* pNewGeoInstance = new StandardGeometryInstance();

	std::vector<Point>& aPoints = pNewGeoInstance->getPoints();

	float xVal = width * 0.5f;
	float yVal = height * 0.5f;
	float zVal = depth * 0.5f;

	aPoints.emplace_back(Point(-xVal, yVal, zVal));
	aPoints.emplace_back(Point(xVal, yVal, zVal));
	aPoints.emplace_back(Point(xVal, yVal, -zVal));
	aPoints.emplace_back(Point(-xVal, yVal, -zVal));

	aPoints.emplace_back(Point(-xVal, -yVal, zVal));
	aPoints.emplace_back(Point(xVal, -yVal, zVal));
	aPoints.emplace_back(Point(xVal, -yVal, -zVal));
	aPoints.emplace_back(Point(-xVal, -yVal, -zVal));

	std::vector<uint32_t>& aPolyIndices = pNewGeoInstance->getPolygonIndices();
	std::vector<uint32_t>& aPolyOffsets = pNewGeoInstance->getPolygonOffsets();

	std::copy(kChartBuilderCuboidPolyIndices, kChartBuilderCuboidPolyIndices + 24, std::back_inserter(aPolyIndices));
	std::copy(kChartBuilderCuboidPolyOffsets, kChartBuilderCuboidPolyOffsets + 6, std::back_inserter(aPolyOffsets));

	unsigned int geoBuildFlags = GeometryInstance::GEO_BUILD_TESSELATE | GeometryInstance::GEO_BUILD_CALC_VERT_NORMALS |
	                                GeometryInstance::GEO_BUILD_CALC_BBOX;

	pNewGeoInstance->setGeoBuildFlags(geoBuildFlags);

	pNewGeoInstance->buildRawGeometryData(geoBuildFlags);

	Mesh* pNewMesh = new Mesh();

	pNewMesh->setGeometryInstance(pNewGeoInstance);

	pNewMesh->transform().position().setFromVector(position);
	
	pNewMesh->setDefaultMaterial();

	return pNewMesh;
}

bool DataChartBuilder::loadHeightData(const std::string& filePath, std::vector<float>& aHeightData) const
{
	std::fstream fileStream(filePath.c_str(), std::ios::in);

	char buf[256];
	memset(buf, 0, 256);
	
	float maxHeight = -2000.0f;
	float minHeight = 2000.0f;
	
	// TODO: this isn't completely robust, but it works well enough for well-formed and valid data

	while (fileStream.getline(buf, 256))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		unsigned int temp1;
		unsigned int temp2;
		float zValue;

		if (m_dataType == 0)
		{
			sscanf(buf, "%u,%u,%f", &temp1, &temp2, &zValue);
		}
		else if (m_dataType == 1)
		{
			sscanf(buf, "%u,%f", &temp1, &zValue);
		}
		
		maxHeight = std::max(zValue, maxHeight);
		minHeight = std::min(zValue, minHeight);

		aHeightData.emplace_back(zValue);
	}

	fileStream.close();

	if (m_normaliseHeight)
	{
		// scale the height values so that we're normalised to our selected height
		// TODO: this doesn't account for lowest starting offsets, so it's not as useful as it could be...

		float heightScale = m_height / maxHeight;

		for (float& hVal : aHeightData)
		{
			hVal *= heightScale;
		}
	}
	
	if (!aHeightData.empty())
	{
		GlobalContext::instance().getLogger().notice("Height data imported for chart. Max height: %g, Min height: %g", maxHeight, minHeight);
		
		return true;
	}

	return false;
}

bool DataChartBuilder::countDataDimensions(const std::string& filePath, unsigned int& xItems, unsigned int& yItems) const
{
	std::fstream fileStream(filePath.c_str(), std::ios::in);

	char buf[256];
	memset(buf, 0, 256);
	
	std::set<unsigned int> aXItems;
	std::set<unsigned int> aYItems;	
	
	// TODO: we could also pre-calculate the height?

	while (fileStream.getline(buf, 256))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		unsigned int xVal;
		unsigned int yVal;
		float zValue;

		if (m_dataType == 0)
		{
			sscanf(buf, "%u,%u,%f", &xVal, &yVal, &zValue);
			
			aXItems.insert(xVal);
			aYItems.insert(yVal);
		}
		else if (m_dataType == 1)
		{
			sscanf(buf, "%u,%f", &xVal, &zValue);
			
			aXItems.insert(xVal);
		}
	}
	
	if (m_dataType == 0)
	{
		if (!aXItems.empty() && !aYItems.empty())
		{
			xItems = aXItems.size();
			yItems = aYItems.size();
			return true;
		}
	}
	else if (m_dataType == 1)
	{
		if (!aXItems.empty())
		{
			xItems = aXItems.size();
			return true;
		}
	}
	return false;
}

} // namespace Imagine

namespace
{
    Imagine::SceneBuilder* createDataChartBuilder()
	{
		return new Imagine::DataChartBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(14, "Data Chart", createDataChartBuilder);
}
