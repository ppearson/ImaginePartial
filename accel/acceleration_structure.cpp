/*
 Imagine
 Copyright 2011-2015 Peter Pearson.

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

#include "acceleration_structure.h"

#include "object.h"
#include "shapes/triangle_fast.h"
#include "shapes/triangle_min.h"
#include "shapes/triangle_zero.h"
#include "shapes/triangle_zero_mb.h"
#include "shapes/shape.h"
#include "shapes/sphere_shape_compact.h"

namespace Imagine
{

unsigned int AccelStructureConfig::calculateMaxDepthBVH(unsigned int numItems)
{
	unsigned int maxDepth = 26;
	maxDepth = 63; // 1 less than the max...

	return maxDepth;
}

void AccelStructureConfig::applyAccelSettingsToConfig(const AccelSettings& accelSettings, AccelStructureConfig& config, unsigned int itemCount)
{
	if (accelSettings.getMaxDepth() == 0)
	{
		config.maxDepth = AccelStructureConfig::calculateMaxDepthBVH(itemCount);
	}
	else
	{
		// non-default
		config.maxDepth = accelSettings.getMaxDepth();
	}

	// non-default
	if (accelSettings.getLeafNodeThreshold() != 0)
	{
		config.leafNodeThreshold = accelSettings.getLeafNodeThreshold();
	}

//	config.goodPartitioning = accelSettings.hasGoodPartitioning();
	config.goodPartitioning = false;

	config.checkAllAxes = accelSettings.hasCheckAllAxes();

	config.chunkedParallelBuild = accelSettings.hasChunkedParallelBuild();

	config.clip = accelSettings.hasClipping();
}

// This is a bit messy, but we need to special case triangles as they don't store geometry instances or points
// so we need the geometry instance in order to get the points for the triangle as the different components
// are stored separately for compactness...

template<typename T, typename OH>
float AccelerationStructure<T, OH>::getObjectSurfaceArea(const Object* pObject) const
{
	return 0.0f;
}

template float AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectSurfaceArea(const Object* pObject) const;

template<typename T, typename OH>
float AccelerationStructure<T, OH>::getObjectSurfaceArea(const TriangleFast* pTriangle) const
{
//	return pTriangle->getTransformedBoundaryBox(*this->m_pGeometryInstance);
	return 0.0f;
}

template float AccelerationStructure<TriangleFast, AccelerationOHPointer<TriangleFast> >::getObjectSurfaceArea(const TriangleFast* pTriangle) const;

template<typename T, typename OH>
float AccelerationStructure<T, OH>::getObjectSurfaceArea(const TriangleMin* pTriangle) const
{
//	return pTriangle->getTransformedBoundaryBox(*this->m_pGeometryInstance);
	return 0.0f;
}

template float AccelerationStructure<TriangleMin, AccelerationOHPointer<TriangleMin> >::getObjectSurfaceArea(const TriangleMin* pTriangle) const;


template<typename T, typename OH>
float AccelerationStructure<T, OH>::getObjectSurfaceArea(const Shape* pShape) const
{
	return 0.0f;
}

template float AccelerationStructure<Shape, AccelerationOHPointer<Shape> >::getObjectSurfaceArea(const Shape* pShape) const;

//

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBox(const Object* pObject, unsigned int index) const
{
	return pObject->getTransformedBoundaryBox();
}

template BoundaryBox AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectBoundaryBox(const Object* pObject, unsigned int index) const;


// Triangle shapes pass down the index

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBox(const TriangleFast* pTriangle, unsigned int index) const
{
	return pTriangle->getTransformedBoundaryBox(*this->m_pRenderTriangleHolder, index);
}

template BoundaryBox AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectBoundaryBox(const TriangleFast* pTriangle, unsigned int index) const;
template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHPointer<TriangleFast> >::getObjectBoundaryBox(const TriangleFast* pTriangle, unsigned int index) const;

template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHItem<TriangleFast> >::getObjectBoundaryBox(const TriangleFast* pTriangle, unsigned int index) const;
template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHItemTriangleCombined<TriangleFast> >::getObjectBoundaryBox(const TriangleFast* pTriangle, unsigned int index) const;


template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBox(const TriangleMin* pTriangle, unsigned int index) const
{
	return pTriangle->getTransformedBoundaryBox(*this->m_pRenderTriangleHolder, index);
}

template BoundaryBox AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectBoundaryBox(const TriangleMin* pTriangle, unsigned int index) const;
template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHPointer<TriangleMin> >::getObjectBoundaryBox(const TriangleMin* pTriangle, unsigned int index) const;

template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHItem<TriangleMin> >::getObjectBoundaryBox(const TriangleMin* pTriangle, unsigned int index) const;
template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHItemCompactTriangle<TriangleMin> >::getObjectBoundaryBox(const TriangleMin* pTriangle, unsigned int index) const;
template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHItemCompactTriangleCombined<TriangleMin> >::getObjectBoundaryBox(const TriangleMin* pTriangle, unsigned int index) const;

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBox(const TriangleZero* pTriangle, unsigned int index) const
{
	return TriangleZero::getTransformedBoundaryBox(*this->m_pRenderTriangleHolder, index);
}

template BoundaryBox AccelerationStructure<TriangleZero, AccelerationOHItemZeroTriangle<TriangleZero> >::getObjectBoundaryBox(const TriangleZero* pTriangle, unsigned int index) const;

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBox(const TriangleZeroMB* pTriangle, unsigned int index) const
{
	return TriangleZeroMB::getTransformedBoundaryBox(*this->m_pRenderTriangleHolder, index);
}

template BoundaryBox AccelerationStructure<TriangleZeroMB, AccelerationOHItemZeroTriangle<TriangleZeroMB> >::getObjectBoundaryBox(const TriangleZeroMB* pTriangle, unsigned int index) const;

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBox(const Shape* pShape, unsigned int index) const
{
	return pShape->getTransformedBoundaryBox();
}

template BoundaryBox AccelerationStructure<Shape, AccelerationOHPointer<Shape> >::getObjectBoundaryBox(const Shape* pShape, unsigned int index) const;

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBox(const SphereShapeCompact* pShape, unsigned int index) const
{
	return pShape->getTransformedBoundaryBox();
}

template BoundaryBox AccelerationStructure<SphereShapeCompact, AccelerationOHItem<SphereShapeCompact> >::getObjectBoundaryBox(const SphereShapeCompact* pShape, unsigned int index) const;


template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBoxForMotionBlur(const Object* pObject, unsigned int index, float shutterOpen, float shutterClose) const
{
	return pObject->getTransformedBoundaryBoxForMotionBlur(shutterOpen, shutterClose);
}

template BoundaryBox AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectBoundaryBoxForMotionBlur(const Object* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHPointer<TriangleFast> >::getObjectBoundaryBoxForMotionBlur(const Object* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

//

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBoxForMotionBlur(const TriangleFast* pObject, unsigned int index, float shutterOpen, float shutterClose) const
{
	return getObjectBoundaryBox(pObject, index);
}

template BoundaryBox AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectBoundaryBoxForMotionBlur(const TriangleFast* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHPointer<TriangleFast> >::getObjectBoundaryBoxForMotionBlur(const TriangleFast* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHItem<TriangleFast> >::getObjectBoundaryBoxForMotionBlur(const TriangleFast* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

//

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBoxForMotionBlur(const TriangleMin* pObject, unsigned int index, float shutterOpen, float shutterClose) const
{
	return getObjectBoundaryBox(pObject, index);
}

template BoundaryBox AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectBoundaryBoxForMotionBlur(const TriangleMin* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHPointer<TriangleMin> >::getObjectBoundaryBoxForMotionBlur(const TriangleMin* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

//template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHItem<TriangleMin> >::getObjectBoundaryBoxForMotionBlur(const TriangleMin* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHItemCompactTriangle<TriangleMin> >::getObjectBoundaryBoxForMotionBlur(const TriangleMin* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

//

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBoxForMotionBlur(const TriangleZero* pObject, unsigned int index, float shutterOpen, float shutterClose) const
{
	return TriangleZero::getTransformedBoundaryBox(*this->m_pRenderTriangleHolder, index);
}

template BoundaryBox AccelerationStructure<TriangleZero, AccelerationOHItemZeroTriangle<TriangleZero> >::getObjectBoundaryBoxForMotionBlur(const TriangleZero* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

//

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBoxForMotionBlur(const TriangleZeroMB* pObject, unsigned int index, float shutterOpen, float shutterClose) const
{
	return TriangleZeroMB::getTransformedBoundaryBoxMBShutter(*this->m_pRenderTriangleHolder, index, shutterOpen, shutterClose);
}

template BoundaryBox AccelerationStructure<TriangleZeroMB, AccelerationOHItemZeroTriangle<TriangleZeroMB> >::getObjectBoundaryBoxForMotionBlur(const TriangleZeroMB* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

//

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBoxForMotionBlur(const Shape* pObject, unsigned int index, float shutterOpen, float shutterClose) const
{
	return pObject->getTransformedBoundaryBox();
}

template BoundaryBox AccelerationStructure<Shape, AccelerationOHPointer<Shape> >::getObjectBoundaryBoxForMotionBlur(const Shape* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

//

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectBoundaryBoxForMotionBlur(const SphereShapeCompact* pObject, unsigned int index, float shutterOpen, float shutterClose) const
{
	return pObject->getTransformedBoundaryBox();
}

template BoundaryBox AccelerationStructure<SphereShapeCompact, AccelerationOHItem<SphereShapeCompact> >::getObjectBoundaryBoxForMotionBlur(const SphereShapeCompact* pObject, unsigned int index, float shutterOpen, float shutterClose) const;


////

template<typename T, typename OH>
void AccelerationStructure<T, OH>::getObjectBoundaryBoxesForMotionBlur(const TriangleZeroMB* pObject, unsigned int index, float shutterOpen, float shutterClose,
										 BoundaryBox& bboxT0, BoundaryBox& bboxT1) const
{
	TriangleZeroMB::getTransformedBoundaryBoxesMBShutter(*this->m_pRenderTriangleHolder, index, shutterOpen, shutterClose,
														 bboxT0, bboxT1);
}

template void AccelerationStructure<TriangleZeroMB, AccelerationOHItemZeroTriangle<TriangleZeroMB> >::getObjectBoundaryBoxesForMotionBlur(const TriangleZeroMB* pObject, unsigned int index,
																			  float shutterOpen, float shutterClose, BoundaryBox& bboxT0, BoundaryBox& bboxT1) const;



////

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectClippedBoundaryBox(const Object* pObject, unsigned int index, const BoundaryBox& clipBB) const
{
	return pObject->getTransformedClippedBoundaryBox(clipBB);
}

template BoundaryBox AccelerationStructure<Object, AccelerationOHPointer<Object> >::getObjectClippedBoundaryBox(const Object* pObject, unsigned int index, const BoundaryBox& clipBB) const;


template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectClippedBoundaryBox(const TriangleFast* pTriangle, unsigned int index, const BoundaryBox& clipBB) const
{
	return pTriangle->getClippedBoundaryBox(*this->m_pRenderTriangleHolder, index, clipBB);
}

template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHPointer<TriangleFast> >::getObjectClippedBoundaryBox(const TriangleFast* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;

template BoundaryBox AccelerationStructure<TriangleFast, AccelerationOHItem<TriangleFast> >::getObjectClippedBoundaryBox(const TriangleFast* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;


template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectClippedBoundaryBox(const TriangleMin* pTriangle, unsigned int index, const BoundaryBox& clipBB) const
{
	return pTriangle->getClippedBoundaryBox(*this->m_pRenderTriangleHolder, index, clipBB);
}

template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHPointer<TriangleMin> >::getObjectClippedBoundaryBox(const TriangleMin* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;

template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHItem<TriangleMin> >::getObjectClippedBoundaryBox(const TriangleMin* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;
template BoundaryBox AccelerationStructure<TriangleMin, AccelerationOHItemCompactTriangle<TriangleMin> >::getObjectClippedBoundaryBox(const TriangleMin* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;


template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectClippedBoundaryBox(const TriangleZero* pTriangle, unsigned int index, const BoundaryBox& clipBB) const
{
	return pTriangle->getClippedBoundaryBox(*this->m_pRenderTriangleHolder, index, clipBB);
}

template BoundaryBox AccelerationStructure<TriangleZero, AccelerationOHItemZeroTriangle<TriangleZero> >::getObjectClippedBoundaryBox(const TriangleZero* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;


template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectClippedBoundaryBox(const TriangleZeroMB* pTriangle, unsigned int index, const BoundaryBox& clipBB) const
{
	return pTriangle->getClippedBoundaryBox(*this->m_pRenderTriangleHolder, index, clipBB);
}

template BoundaryBox AccelerationStructure<TriangleZeroMB, AccelerationOHItemZeroTriangle<TriangleZeroMB> >::getObjectClippedBoundaryBox(const TriangleZeroMB* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;


template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectClippedBoundaryBox(const Shape* pShape, unsigned int index, const BoundaryBox& clipBB) const
{
	return pShape->getClippedBoundaryBox(clipBB);
}

template BoundaryBox AccelerationStructure<Shape, AccelerationOHPointer<Shape> >::getObjectClippedBoundaryBox(const Shape* pShpe, unsigned int index, const BoundaryBox& clipBB) const;

template<typename T, typename OH>
BoundaryBox AccelerationStructure<T, OH>::getObjectClippedBoundaryBox(const SphereShapeCompact* pShape, unsigned int index, const BoundaryBox& clipBB) const
{
	return pShape->getClippedBoundaryBox(clipBB);
}

template BoundaryBox AccelerationStructure<SphereShapeCompact, AccelerationOHItem<SphereShapeCompact> >::getObjectClippedBoundaryBox(const SphereShapeCompact* pShpe, unsigned int index, const BoundaryBox& clipBB) const;

} // namespace Imagine
