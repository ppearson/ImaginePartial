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

#ifndef POINTS_GRID_H
#define POINTS_GRID_H

#include "points_lookup_accel.h"

#include "core/boundary_box.h"

#include "utils/slab_allocator.h"

namespace Imagine
{

class PointsGrid : public PointsLookupAccel
{
public:
	PointsGrid();
	virtual ~PointsGrid();
	
	class SubCell
	{
	public:
		SubCell()
		{
			
		}
		
		void addPoint(uint32_t index)
		{
			m_pointIndices.emplace_back(index);
		}
		
		const std::vector<uint32_t>& getIndices() const
		{
			return m_pointIndices;
		}

	protected:
		std::vector<uint32_t>	m_pointIndices;
	};
	

	virtual void build(const std::vector<PointColour3f>& points, float lookupSize);

	virtual Colour3f lookupColour(const Point& worldSpacePosition, float filterRadius) const;

protected:	
	void freeCells();
	
	void getSubCellIndicesForPoint(const Point& point, unsigned int& i, unsigned int& j, unsigned int& k) const;
	
	
protected:
	BoundaryBox					m_bbox;

	std::vector<PointColour3f>	m_aPoints;
	
	std::vector<SubCell*>		m_aSubCells;
	
	FixedSlabAllocator		m_subCellMemory;
	
	float					m_overallResX;
	float					m_overallResY;
	float					m_overallResZ;
	
	// currently, the cell size is the same in all 3 dimensions...
	float					m_cellSize;

	// these are worked out based on the overall bbox and the cell size...
	uint32_t				m_cellCountX;
	uint32_t				m_cellCountY;
	uint32_t				m_cellCountZ;
	uint32_t				m_cellCountXY;
};

} // namespace Imagine

#endif // POINTS_GRID_H
