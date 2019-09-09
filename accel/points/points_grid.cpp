/*
 Imagine
 Copyright 2017 Peter Pearson.

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

#include "points_grid.h"

#include "utils/item_tracking.h"

namespace Imagine
{

PointsGrid::PointsGrid() : m_subCellMemory(false, false, 32768)
{

}

PointsGrid::~PointsGrid()
{

}

void PointsGrid::build(const std::vector<PointColour3f>& points, float lookupSize)
{
	freeCells();

	m_aPoints = points;

	m_bbox.reset();

	std::vector<PointColour3f>::const_iterator itPoint = m_aPoints.begin();
	for (; itPoint != m_aPoints.end(); ++itPoint)
	{
		const PointColour3f& pointItem = *itPoint;

		m_bbox.includePoint(pointItem.position);
	}

	m_overallResX = m_bbox.getExtent().x;
	m_overallResY = m_bbox.getExtent().y;
	m_overallResZ = m_bbox.getExtent().z;

	m_cellSize = lookupSize;

	float cellCountX = m_overallResX / m_cellSize;
	float cellCountY = m_overallResY / m_cellSize;
	float cellCountZ = m_overallResZ / m_cellSize;

	m_cellCountX = (unsigned int)std::ceil(cellCountX);
	m_cellCountY = (unsigned int)std::ceil(cellCountY);
	m_cellCountZ = (unsigned int)std::ceil(cellCountZ);

	m_cellCountXY = m_cellCountX * m_cellCountY;
	
	m_subCellMemory.reserve(sizeof(SubCell) * m_cellCountXY * m_cellCountZ);

	for (unsigned int i = 0; i < m_cellCountX; i++)
	{
		for (unsigned int j = 0; j < m_cellCountY; j++)
		{
			for (unsigned int k = 0; k < m_cellCountZ; k++)
			{
				SubCell* pNewSubCell = m_subCellMemory.allocSingle<SubCell>();				

				m_aSubCells.push_back(pNewSubCell);
			}
		}
	}

	// now stick the points in the cells

	itPoint = m_aPoints.begin();
	unsigned int count = 0;
	for (; itPoint != m_aPoints.end(); ++itPoint)
	{
		const PointColour3f& pointItem = *itPoint;

		unsigned int cellX;
		unsigned int cellY;
		unsigned int cellZ;

		getSubCellIndicesForPoint(pointItem.position, cellX, cellY, cellZ);

		unsigned int actualIndex = cellX + (cellY * m_cellCountX) + (cellZ * m_cellCountXY);

		SubCell* pSubCell = m_aSubCells[actualIndex];

		pSubCell->addPoint(count++);
	}
}

Colour3f PointsGrid::lookupColour(const Point& worldSpacePosition, float filterRadius) const
{
	if (m_aPoints.empty())
		return m_missingColour;

	unsigned int cellX;
	unsigned int cellY;
	unsigned int cellZ;

	float clampedPositionX = std::max(worldSpacePosition.x, m_bbox.getMinimum().x);
	float clampedPositionY = std::max(worldSpacePosition.y, m_bbox.getMinimum().y);
	float clampedPositionZ = std::max(worldSpacePosition.z, m_bbox.getMinimum().z);

	clampedPositionX = std::min(clampedPositionX, m_bbox.getMaximum().x);
	clampedPositionY = std::min(clampedPositionY, m_bbox.getMaximum().y);
	clampedPositionZ = std::min(clampedPositionZ, m_bbox.getMaximum().z);

	Point clampedPosition(clampedPositionX, clampedPositionY, clampedPositionZ);

	getSubCellIndicesForPoint(clampedPosition, cellX, cellY, cellZ);

	unsigned int actualIndex = cellX + (cellY * m_cellCountX) + (cellZ * m_cellCountXY);

	const SubCell* pSubCell = m_aSubCells[actualIndex];

	const std::vector<uint32_t>& pointIndices = pSubCell->getIndices();

	if (pointIndices.empty())
		return m_missingColour;

	if (filterRadius > 0.0f)
	{
		SmallestNItems<4> items;

		std::vector<uint32_t>::const_iterator itPointIndices = pointIndices.begin();
		for (; itPointIndices != pointIndices.end(); ++itPointIndices)
		{
			uint32_t pointIndex = *itPointIndices;

			const PointColour3f& pointItem = m_aPoints[pointIndex];

			float distance = Point::distance(clampedPosition, pointItem.position);

			items.addItem(distance, pointIndex);
		}

		const float* pFinalSizes = NULL;
		const unsigned int* pFinalIndices = NULL;
		unsigned int numItems = items.getFinalItems(pFinalSizes, pFinalIndices);
		Colour3f finalColour;
		float ratioDistance = pFinalSizes[numItems - 1];
		for (unsigned int i = 0; i < numItems; i++)
		{
			const PointColour3f& pointItem = m_aPoints[pFinalIndices[i]];
			finalColour += pointItem.colour * (pFinalSizes[i] / ratioDistance);
		}

		finalColour /= (float)numItems;
		return finalColour;
	}
	else
	{
		SmallestNItems<2> items;

		std::vector<uint32_t>::const_iterator itPointIndices = pointIndices.begin();
		for (; itPointIndices != pointIndices.end(); ++itPointIndices)
		{
			uint32_t pointIndex = *itPointIndices;

			const PointColour3f& pointItem = m_aPoints[pointIndex];

			float distance = Point::distance(clampedPosition, pointItem.position);

			items.addItem(distance, pointIndex);
		}

		unsigned int closestPointIndex = items.getSmallestItemIndex();
		const PointColour3f& closestPoint = m_aPoints[closestPointIndex];
		return Colour3f(closestPoint.colour.r, closestPoint.colour.g, closestPoint.colour.b);
	}
}

void PointsGrid::freeCells()
{
	m_subCellMemory.freeMem();

	m_aSubCells.clear();
}

void PointsGrid::getSubCellIndicesForPoint(const Point& point, unsigned int& i, unsigned int& j, unsigned int& k) const
{
//	Vector extent = m_bbox.getExtent();

	float offsetX = point.x - m_bbox.getMinimum().x;
	float offsetY = point.y - m_bbox.getMinimum().y;
	float offsetZ = point.z - m_bbox.getMinimum().z;

	float cellX = offsetX / m_cellSize;
	float cellY = offsetY / m_cellSize;
	float cellZ = offsetZ / m_cellSize;

	i = (unsigned int)std::floor(cellX);
	j = (unsigned int)std::floor(cellY);
	k = (unsigned int)std::floor(cellZ);
}

} // namespace Imagine
