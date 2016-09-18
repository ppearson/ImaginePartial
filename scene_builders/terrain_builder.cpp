/*
 Imagine
 Copyright 2012-2014 Peter Pearson.

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

#include "terrain_builder.h"

#include "objects/mesh.h"
#include "geometry/geometry_instance.h"
#include "geometry/editable_geometry_instance.h"
#include "geometry/standard_geometry_instance.h"

#include "utils/file_helpers.h"

#include "image/image_1f.h"

#include "textures/image/image_texture_1f.h"
#include "textures/image/image_texture_factory.h"

#include "textures/procedural_2d/noise_2d.h"

#include "scene_builders/ocean_sim.h"

#include "io/file_io_registry.h"
#include "io/image_reader.h"

namespace Imagine
{

const char* heightSourceOptions[] = { "Image", "Noise", "Ocean", 0 };

TerrainBuilder::TerrainBuilder() : SceneBuilder(), m_width(100), m_depth(100), m_Xdivisions(100), m_Ydivisions(100), m_editableGeo(false),
	m_maxHeight(4.0f), m_heightSource(eImage), m_normalise(false), m_heightIgnoreThreshold(31000.0f)
{
}

unsigned char TerrainBuilder::getSceneBuilderTypeID()
{
	return 4;
}

std::string TerrainBuilder::getSceneBuilderDescription()
{
	return "Terrain";
}

void TerrainBuilder::buildParameters(Parameters& parameters, unsigned int flags)
{
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("width", "width", &m_width, eParameterUInt, 1, 20000, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("depth", "depth", &m_depth, eParameterUInt, 1, 20000, eParameterScrubButton));

	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("x_divisions", "X divisions", &m_Xdivisions, eParameterUInt, 1, 16384, eParameterScrubButton));
	parameters.addParameter(new RangeParameter<unsigned int, unsigned int>("y_divisions", "Y divisions", &m_Ydivisions, eParameterUInt, 1, 16384, eParameterScrubButton));

	parameters.addParameter(new BasicParameter<bool>("editable_geo", "Editable geo", &m_editableGeo, eParameterBool, 0));

	parameters.addParameter(new RangeParameter<float, float>("max_height", "Max Height", &m_maxHeight, eParameterFloat, 0.0f, 1000.0f, eParameterScrubButton));

	parameters.addParameter(new EnumParameter("height_source", "height source", (unsigned char*)&m_heightSource, heightSourceOptions));

	parameters.addParameter(new BasicParameter<std::string>("height_map_path", "Height map path", &m_heightMapPath, eParameterFile));

	parameters.addParameter(new BasicParameter<bool>("normalise", "Normalise", &m_normalise, eParameterBool, 0));
	parameters.addParameter(new RangeParameter<float, float>("height_ignore_threshold", "Height ignore thresh.", &m_heightIgnoreThreshold, eParameterFloat, 0.0f, 65500.0f, eParameterScrubButton));
}

void TerrainBuilder::createScene(Scene& scene)
{
	Mesh* pNewMesh = new Mesh();

	Texture* pHeightMapTexture = NULL;

	if (m_heightSource == eImage && !m_heightMapPath.empty())
	{
		// load in the texture
		unsigned int requiredFlags = Image::IMAGE_CHANNELS_1 | Image::IMAGE_FORMAT_NATIVE | Image::IMAGE_FLAGS_BRIGHTNESS | Image::IMAGE_NO_CACHING;

		pHeightMapTexture = ImageTextureFactory::createGreyscaleImageTextureForImageFromPath(m_heightMapPath, requiredFlags);
	}
	else if (m_heightSource == eNoise)
	{
		Noise2D* pNoiseTexture = new Noise2D();
		pHeightMapTexture = pNoiseTexture;
	}
	else if (m_heightSource == eOcean)
	{
		Vector2 wind(16.1f, 31.1f);
		OceanSim sim(m_Xdivisions, 0.005f, m_width, wind, 4242);

		Image1f* pHeightMapImage = sim.generateDisplacementImage(1.0f);

		pHeightMapTexture = new ImageTexture1f(pHeightMapImage, pHeightMapImage->getWidth(), pHeightMapImage->getHeight(), 1.0f, 1.0f);
	}

	if (!pHeightMapTexture)
	{
		return;
	}

	if (m_normalise && m_heightSource == eImage && pHeightMapTexture != NULL)
	{
		// work out min and max
		float minVal = 35000.0f;
		float maxVal = -1000.0f;

		ImageTexture1f* pTypedTexture = static_cast<ImageTexture1f*>(pHeightMapTexture);
		Image1f* pImage = pTypedTexture->getImagePtr();

		unsigned int imageHeight = pImage->getHeight();
		unsigned int imageWidth = pImage->getWidth();

		for (unsigned int y = 0; y < imageHeight; y++)
		{
			const float* pVal = pImage->floatRowPtr(y);

			for (unsigned int x = 0; x < imageWidth; x++)
			{
				const float& val = *pVal++;

				// detect and ignore edited values (See SRTM_edit_rules.pdf spec)
				if (val < m_heightIgnoreThreshold)
				{
					minVal = std::min(minVal, val);
					maxVal = std::max(maxVal, val);
				}
			}
		}

		float range = maxVal - minVal;
		float invRange = 1.0f;
		if (invRange != 0.0f)
		{
			invRange = 1.0f / range;
		}

		// now go through and normalise between 0.0-1.0
		for (unsigned int y = 0; y < imageHeight; y++)
		{
			float* pVal = pImage->floatRowPtr(y);

			for (unsigned int x = 0; x < imageWidth; x++)
			{
				float& val = *pVal++;

				if (val >= m_heightIgnoreThreshold)
				{
					// if so, we don't know what it really should be, so we've got to guess based on surrounding pixels...
					// for the moment, just use an adjacent value

					unsigned int cloneCoordsX = x;
					unsigned int cloneCoordsY = y;

					// TODO: make this more robust
					if (x > 1)
					{
						cloneCoordsX --;
					}
					else if (y > 1)
					{
						cloneCoordsY --;
					}

					float cloneValue = pImage->floatAt(cloneCoordsX, cloneCoordsY);
					val = cloneValue;
				}
				else
				{
					val -= minVal;
					val *= invRange;
				}
			}
		}
	}

	float halfWidth = (float)m_width / 2.0f;
	float halfDepth = (float)m_depth / 2.0f;

	float uIncrease = 1.0f / (float)m_Xdivisions;
	float vIncrease = 1.0f / (float)m_Ydivisions;

	float xIncrease = (float)m_width / (float)m_Xdivisions;
	float zIncrease = (float)m_depth / (float)m_Ydivisions;

	GeometryInstanceGathered* pNewGeoInstance = NULL;
	EditableGeometryInstance* pEditableGeoInstance = NULL;
	StandardGeometryInstance* pStandardGeoInstance = NULL;

	if (m_editableGeo)
	{
		pEditableGeoInstance = new EditableGeometryInstance();
		std::deque<Point>& aPoints = pEditableGeoInstance->getPoints();
		std::deque<UV>& aUVs = pEditableGeoInstance->getUVs();

		HitResult tempResult;

		for (unsigned int x = 0; x <= m_Xdivisions; x++)
		{
			float u = 1.0f - uIncrease * (float)x;

			float xVal = -halfWidth + (xIncrease * x);

			for (unsigned int z = 0; z <= m_Ydivisions; z++)
			{
				float v = vIncrease * (float)z;

				float zVal = -halfDepth + (zIncrease * z);

				UV uv(u, v);
				aUVs.push_back(uv);

				float height = 0.0f;

				tempResult.uv = uv;

				height = pHeightMapTexture->getFloatBlend(tempResult, 0);
				height *= m_maxHeight;

				Point newPoint(xVal, height, zVal);
				aPoints.push_back(newPoint);
			}
		}
		pEditableGeoInstance->setHasPerVertexUVs(true);

		pNewGeoInstance = pEditableGeoInstance;
	}
	else
	{
		pStandardGeoInstance = new StandardGeometryInstance();

		std::vector<Point>& aPoints = pStandardGeoInstance->getPoints();
		std::vector<UV>& aUVs = pStandardGeoInstance->getUVs();

		unsigned int reserveAmount = (m_Xdivisions + 1) * (m_Ydivisions + 1);
		aPoints.reserve(reserveAmount);
		aUVs.reserve(reserveAmount);

		HitResult tempResult;

		for (unsigned int x = 0; x <= m_Xdivisions; x++)
		{
			float u = 1.0f - uIncrease * (float)x;

			float xVal = -halfWidth + (xIncrease * x);

			for (unsigned int z = 0; z <= m_Ydivisions; z++)
			{
				float v = vIncrease * (float)z;

				float zVal = -halfDepth + (zIncrease * z);

				UV uv(u, v);
				aUVs.push_back(uv);

				float height = 0.0f;

				tempResult.uv = uv;

				height = pHeightMapTexture->getFloatBlend(tempResult, 0);
				height *= m_maxHeight;

				Point newPoint(xVal, height, zVal);
				aPoints.push_back(newPoint);
			}
		}

		pStandardGeoInstance->setHasPerVertexUVs(true);

		pNewGeoInstance = pStandardGeoInstance;
	}

	if (pHeightMapTexture)
	{
		delete pHeightMapTexture;
	}

	// now make the faces

	if (m_editableGeo)
	{
		std::deque<Face>& aFaces = pEditableGeoInstance->getFaces();

		for (unsigned int x = 0; x < m_Xdivisions; x++)
		{
			for (unsigned int z = 0; z < m_Ydivisions; z++)
			{
				unsigned int cornerIndex = (x * (m_Ydivisions + 1)) + z;

				unsigned int vertex0 = cornerIndex + 1;
				unsigned int vertex1 = cornerIndex;
				unsigned int vertex2 = cornerIndex + (m_Ydivisions + 1);
				unsigned int vertex3 = cornerIndex + (m_Ydivisions + 1) + 1;

				Face newFace(vertex3, vertex2, vertex1, vertex0);

				newFace.calculateNormal(pEditableGeoInstance);

				newFace.addUV(vertex3);
				newFace.addUV(vertex2);
				newFace.addUV(vertex1);
				newFace.addUV(vertex0);

				aFaces.push_back(newFace);
			}
		}
	}
	else
	{
		std::vector<uint32_t>& aPolyOffsets = pStandardGeoInstance->getPolygonOffsets();
		std::vector<uint32_t>& aPolyIndices = pStandardGeoInstance->getPolygonIndices();

		unsigned int numFaces = m_Xdivisions * m_Ydivisions;
		aPolyOffsets.reserve(numFaces + 1);
		aPolyIndices.reserve(numFaces * 4);

		for (unsigned int x = 0; x < m_Xdivisions; x++)
		{
			for (unsigned int z = 0; z < m_Ydivisions; z++)
			{
				unsigned int cornerIndex = (x * (m_Ydivisions + 1)) + z;

				unsigned int vertex0 = cornerIndex + 1;
				unsigned int vertex1 = cornerIndex;
				unsigned int vertex2 = cornerIndex + (m_Ydivisions + 1);
				unsigned int vertex3 = cornerIndex + (m_Ydivisions + 1) + 1;

				aPolyIndices.push_back(vertex3);
				aPolyIndices.push_back(vertex2);
				aPolyIndices.push_back(vertex1);
				aPolyIndices.push_back(vertex0);
			}
		}

		unsigned int startIndexCount = 4;
		for (unsigned int i = 0; i < m_Xdivisions * m_Ydivisions; i++)
		{
			aPolyOffsets.push_back(startIndexCount);
			startIndexCount += 4;
		}
	}

//	pGeoInstance->setHashValue(hashValue);

	pNewMesh->setGeometryInstance(pNewGeoInstance);

	pNewMesh->setName("Terrain1");

	pNewMesh->setDefaultMaterial();

	addObject(scene, pNewMesh);
}

bool TerrainBuilder::controlChanged(const std::string& name, PostChangedActions& postChangedActions)
{
	if (name == "height_map_path")
	{
		if (m_heightMapPath.find("hgt") != std::string::npos)
		{
			m_normalise = true;

			postChangedActions.addRefreshItem("normalise");

			return true;
		}
	}
	return false;
}

} // namespace Imagine

namespace
{
	Imagine::SceneBuilder* createTerrainBuilder()
	{
		return new Imagine::TerrainBuilder();
	}

	const bool registered = Imagine::SceneBuilderFactory::instance().registerSceneBuilder(4, "Terrain", createTerrainBuilder);
}
