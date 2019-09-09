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

#ifndef ACCELERATION_STRUCTURE_H
#define ACCELERATION_STRUCTURE_H

#include <vector>

#include "core/boundary_box.h"

#include "utils/tagged_pointer.h"

namespace Imagine
{

class RenderTriangleHolder;
class Ray;
struct SelectionHitResult;
struct HitResult;
class Object;
class TriangleFast;
class TriangleMin;
class TriangleZero;
class TriangleZeroMB;
class Shape;
class SphereShapeCompact;
class Texture;
class Partitioner;
class AccelSettings;

struct AccelStructureConfig
{
	AccelStructureConfig(unsigned int type = 0)
	{
		kdtreeIntersectCost = 20.0f;
		kdtreeTraverseCost = 15.0f;
		kdtreeEmptyBonus = 0.9f;
		
		bvhIntersectCost = 2.0f; // 2.0f is triangle

		maxDepth = 24;
		switch (type)
		{
			case 0:
			default:
				leafNodeThreshold = 6;
				break;
			case 1:
				leafNodeThreshold = 3;
				break;
			case 2:
				leafNodeThreshold = 2;
				break;
		}

		fastLevels = 0;

		checkAllAxes = false;
		goodPartitioning = true;

		clip = true;
		useCachedBoundsForClipping = false;
		conserveMemory = false;
		chunkedParallelBuild = false;

		motionBlur = false;
		shutterOpen = 0.0f;
		shutterClose = 1.0f;
	}

	static unsigned int calculateMaxDepthBVH(unsigned int numItems);

	static void applyAccelSettingsToConfig(const AccelSettings& accelSettings, AccelStructureConfig& config, unsigned int itemCount);

	float			kdtreeIntersectCost;
	float			kdtreeTraverseCost;
	float			kdtreeEmptyBonus;
	float			bvhIntersectCost;

	unsigned int	maxDepth;
	unsigned int	leafNodeThreshold;
	unsigned int	fastLevels;

	bool			goodPartitioning;
	bool			checkAllAxes;
	bool			clip;
	bool			useCachedBoundsForClipping;
	bool			conserveMemory;
	bool			chunkedParallelBuild;

	bool			motionBlur;
	// these are deltas between the time samples
	float			shutterOpen;
	float			shutterClose;
};

// Object Holders

template<typename T>
class AccelerationOHPointer
{
public:
	AccelerationOHPointer()
	{
	}

	void setRenderTriangleHolder(const RenderTriangleHolder* pRTH)
	{

	}

	void clear()
	{
		m_aObjects.clear();
		std::vector<T*>().swap(m_aObjects);
	}

	bool isPointerType() const
	{
		return true;
	}

	const T* getConstPtr(unsigned int index) const
	{
		return m_aObjects[index];
	}

	std::vector<T*>* getObjectPtrVector()
	{
		return &m_aObjects;
	}

	std::vector<T>* getObjectVector()
	{
		return NULL;
	}

	std::vector<const Object*>* getExtraVector()
	{
		return NULL;
	}

	inline bool didHitObject(const unsigned int index, const Ray& ray, float& t, HitResult& result) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObject(ray, t, result);
	}

	inline bool doesOcclude(const unsigned int index, const Ray& ray) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOcclude(ray);
	}

	inline bool didHitObjectLazy(const unsigned int index, const Ray& ray, float& t, SelectionHitResult& result, unsigned int subLevel) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectLazy(ray, t, result, subLevel);
	}

	inline bool didHitObjectAlpha(const unsigned int index, const Ray& ray, float& t, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectAlpha(ray, t, result, alphaTexture, pRTH);
	}

	inline bool doesOccludeAlpha(const unsigned int index, const Ray& ray, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOccludeAlpha(ray, result, alphaTexture, pRTH);
	}

	size_t getMainSize() const
	{
		return m_aObjects.size();
	}

	size_t getExtraSize() const
	{
		return 0;
	}

	size_t getMemorySize() const
	{
		return m_aObjects.capacity() * sizeof(T*);
	}

protected:
	std::vector<T*>		m_aObjects;
};

template<typename T>
class AccelerationOHItem
{
public:
	AccelerationOHItem()
	{
	}

	void setRenderTriangleHolder(const RenderTriangleHolder* pRTH)
	{
	}

	void clear()
	{
		m_aObjects.clear();
		std::vector<T>().swap(m_aObjects);
	}

	bool isPointerType() const
	{
		return false;
	}

	const T* getConstPtr(unsigned int index) const
	{
		return &m_aObjects[index];
	}

	std::vector<T*>* getObjectPtrVector()
	{
		return NULL;
	}

	std::vector<T>* getObjectVector()
	{
		return &m_aObjects;
	}

	std::vector<const Object*>* getExtraVector()
	{
		return NULL;
	}

	inline bool didHitObject(const unsigned int index, const Ray& ray, float& t, HitResult& result) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObject(ray, t, result);
	}

	inline bool doesOcclude(const unsigned int index, const Ray& ray) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOcclude(ray);
	}

	inline bool didHitObjectLazy(const unsigned int index, const Ray& ray, float& t, SelectionHitResult& result, unsigned int subLevel) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectLazy(ray, t, result, subLevel);
	}

	inline bool didHitObjectAlpha(const unsigned int index, const Ray& ray, float& t, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectAlpha(ray, t, result, alphaTexture, pRTH);
	}

	inline bool doesOccludeAlpha(const unsigned int index, const Ray& ray, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOccludeAlpha(ray, result, alphaTexture, pRTH);
	}

	size_t getMainSize() const
	{
		return m_aObjects.size();
	}

	size_t getExtraSize() const
	{
		return 0;
	}

	size_t getMemorySize() const
	{
		return 0; // should be counted elsewhere...
	}

protected:
	std::vector<T>		m_aObjects;
};

template<typename T>
class AccelerationOHItemCompactTriangle
{
public:
	AccelerationOHItemCompactTriangle() : m_pRTH(NULL)
	{
	}

	void setRenderTriangleHolder(const RenderTriangleHolder* pRTH)
	{
		m_pRTH = pRTH;
	}

	void clear()
	{
		m_aObjects.clear();
		std::vector<T>().swap(m_aObjects);
	}

	bool isPointerType() const
	{
		return false;
	}

	const T* getConstPtr(unsigned int index) const
	{
		return &m_aObjects[index];
	}

	std::vector<T*>* getObjectPtrVector()
	{
		return NULL;
	}

	std::vector<T>* getObjectVector()
	{
		return &m_aObjects;
	}

	std::vector<const Object*>* getExtraVector()
	{
		return NULL;
	}

	inline bool didHitObject(const unsigned int index, const Ray& ray, float& t, HitResult& result) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObject(ray, t, result, m_pRTH);
	}

	inline bool doesOcclude(const unsigned int index, const Ray& ray) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOcclude(ray, m_pRTH);
	}

	inline bool didHitObjectLazy(const unsigned int index, const Ray& ray, float& t, SelectionHitResult& result, unsigned int subLevel) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectLazy(ray, t, result, subLevel, m_pRTH);
	}

	inline bool didHitObjectAlpha(const unsigned int index, const Ray& ray, float& t, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectAlpha(ray, t, result, alphaTexture, m_pRTH);
	}

	inline bool doesOccludeAlpha(const unsigned int index, const Ray& ray, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOccludeAlpha(ray, result, alphaTexture, m_pRTH);
	}

	size_t getMainSize() const
	{
		return m_aObjects.size();
	}

	size_t getExtraSize() const
	{
		return 0;
	}

	size_t getMemorySize() const
	{
		return 0; // should be counted elsewhere...
	}

protected:
	std::vector<T>		m_aObjects;

	const RenderTriangleHolder* m_pRTH;
};

// has support for Object* vector internally as well for baked geometry instances
template<typename T>
class AccelerationOHItemTriangleCombined
{
public:
	AccelerationOHItemTriangleCombined()
	{
	}

	void setRenderTriangleHolder(const RenderTriangleHolder* pRTH)
	{

	}

	void clear()
	{
		m_aObjects.clear();
		std::vector<T>().swap(m_aObjects);
	}

	bool isPointerType() const
	{
		return false;
	}

	const T* getConstPtr(unsigned int index) const
	{
		return &m_aObjects[index];
	}

	const Object* getConstExtraObjectPtr(unsigned int index) const
	{
		return m_aExtraObjects[index];
	}

	std::vector<T*>* getObjectPtrVector()
	{
		return NULL;
	}

	std::vector<T>* getObjectVector()
	{
		return &m_aObjects;
	}

	std::vector<const Object*>* getExtraVector()
	{
		return &m_aExtraObjects;
	}

	inline bool didHitObject(const unsigned int index, const Ray& ray, float& t, HitResult& result) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObject(ray, t, result);
	}

	inline bool doesOcclude(const unsigned int index, const Ray& ray) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOcclude(ray);
	}

	inline bool didHitObjectLazy(const unsigned int index, const Ray& ray, float& t, SelectionHitResult& result, unsigned int subLevel) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectLazy(ray, t, result, subLevel);
	}

	inline bool didHitObjectAlpha(const unsigned int index, const Ray& ray, float& t, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectAlpha(ray, t, result, alphaTexture, pRTH);
	}

	inline bool doesOccludeAlpha(const unsigned int index, const Ray& ray, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOccludeAlpha(ray, result, alphaTexture, pRTH);
	}

	size_t getMainSize() const
	{
		return m_aObjects.size();
	}

	size_t getExtraSize() const
	{
		return m_aExtraObjects.size();
	}

	size_t getMemorySize() const
	{
		return 0; // should be counted elsewhere...
	}

protected:
	std::vector<T>		m_aObjects;

	std::vector<const Object*>	m_aExtraObjects;
};

// has support for Object* vector internally as well for baked geometry instances
template<typename T>
class AccelerationOHItemCompactTriangleCombined
{
public:
	AccelerationOHItemCompactTriangleCombined() : m_pRTH(NULL)
	{
	}

	void setRenderTriangleHolder(const RenderTriangleHolder* pRTH)
	{
		m_pRTH = pRTH;
	}

	void clear()
	{
		m_aObjects.clear();
		std::vector<T>().swap(m_aObjects);
	}

	bool isPointerType() const
	{
		return false;
	}

	const T* getConstPtr(unsigned int index) const
	{
		return &m_aObjects[index];
	}

	const Object* getConstExtraObjectPtr(unsigned int index) const
	{
		return m_aExtraObjects[index];
	}

	std::vector<T*>* getObjectPtrVector()
	{
		return NULL;
	}

	std::vector<T>* getObjectVector()
	{
		return &m_aObjects;
	}

	std::vector<const Object*>* getExtraVector()
	{
		return &m_aExtraObjects;
	}

	inline bool didHitObject(const unsigned int index, const Ray& ray, float& t, HitResult& result) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObject(ray, t, result, m_pRTH);
	}

	inline bool doesOcclude(const unsigned int index, const Ray& ray) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOcclude(ray, m_pRTH);
	}

	inline bool didHitObjectLazy(const unsigned int index, const Ray& ray, float& t, SelectionHitResult& result, unsigned int subLevel) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectLazy(ray, t, result, subLevel, m_pRTH);
	}

	inline bool didHitObjectAlpha(const unsigned int index, const Ray& ray, float& t, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->didHitObjectAlpha(ray, t, result, alphaTexture, m_pRTH);
	}

	inline bool doesOccludeAlpha(const unsigned int index, const Ray& ray, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		const T* pItem = getConstPtr(index);
		return pItem->doesOccludeAlpha(ray, result, alphaTexture, m_pRTH);
	}

	size_t getMainSize() const
	{
		return m_aObjects.size();
	}

	size_t getExtraSize() const
	{
		return m_aExtraObjects.size();
	}

	size_t getMemorySize() const
	{
		return 0; // should be counted elsewhere...
	}

protected:
	std::vector<T>		m_aObjects;

	std::vector<const Object*>	m_aExtraObjects;

	const RenderTriangleHolder* m_pRTH;
};

// for zero-overhead - passes in index, can't be used with baking & alpha
template<typename T>
class AccelerationOHItemZeroTriangle
{
public:
	AccelerationOHItemZeroTriangle() : m_pRTH(NULL)
	{
	}

	void setRenderTriangleHolder(const RenderTriangleHolder* pRTH)
	{
		m_pRTH = pRTH;
	}

	void clear()
	{
		m_aObjects.clear();
		std::vector<T>().swap(m_aObjects);
	}

	void clearUnneededTriangles()
	{
		clear();
	}

	bool isPointerType() const
	{
		return false;
	}

	const T* getConstPtr(unsigned int index) const
	{
		return &m_aObjects[index];
	}

	std::vector<T*>* getObjectPtrVector()
	{
		return NULL;
	}

	std::vector<T>* getObjectVector()
	{
		return &m_aObjects;
	}

	std::vector<const Object*>* getExtraVector()
	{
		return NULL;
	}

	inline bool didHitObject(const unsigned int index, const Ray& ray, float& t, HitResult& result) const
	{
		return T::didHitObject(ray, t, result, m_pRTH, index);
	}

	inline bool doesOcclude(const unsigned int index, const Ray& ray) const
	{
		return T::doesOcclude(ray, m_pRTH, index);
	}

	inline bool didHitObjectLazy(const unsigned int index, const Ray& ray, float& t, SelectionHitResult& result, unsigned int subLevel) const
	{
		return T::didHitObjectLazy(ray, t, result, subLevel, m_pRTH, index);
	}

	inline bool didHitObjectAlpha(const unsigned int index, const Ray& ray, float& t, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		return T::didHitObjectAlpha(ray, t, result, alphaTexture, m_pRTH, index);
	}

	inline bool doesOccludeAlpha(const unsigned int index, const Ray& ray, HitResult& result, const Texture* alphaTexture, const RenderTriangleHolder* pRTH) const
	{
		return T::doesOccludeAlpha(ray, result, alphaTexture, m_pRTH, index);
	}

	size_t getMainSize() const
	{
		return m_aObjects.size();
	}

	size_t getExtraSize() const
	{
		return 0;
	}

	size_t getMemorySize() const
	{
		return 0; // should be counted elsewhere...
	}

protected:
	// TODO: shouldn't really need this, but it's currently needed at build time...
	std::vector<T>		m_aObjects;

	const RenderTriangleHolder* m_pRTH;
};

template<typename T, typename OH>
class AccelerationStructure
{
public:
	AccelerationStructure() : m_pRenderTriangleHolder(NULL)
	{
	}

	virtual ~AccelerationStructure()
	{
	}

	// this must match the type in AccelSettings
	virtual unsigned int getType() const = 0;

	virtual bool isTrianglePacketType() const
	{
		return false;
	}

	float getObjectSurfaceArea(const Object* pObject) const;
	float getObjectSurfaceArea(const TriangleFast* pTriangle) const;
	float getObjectSurfaceArea(const TriangleMin* pTriangle) const;
	float getObjectSurfaceArea(const TriangleZero* pTriangle) const;
	float getObjectSurfaceArea(const TriangleZeroMB* pTriangle) const;
	float getObjectSurfaceArea(const Shape* pShape) const;
	float getObjectSurfaceArea(const SphereShapeCompact* pShape) const;

	BoundaryBox getObjectBoundaryBoxByIndex(unsigned int index) const
	{
		const T* pObject = m_objectHolder.getConstPtr(index);
		return getObjectBoundaryBox(pObject, index);
	}

	BoundaryBox getObjectBoundaryBox(const Object* pObject, unsigned int index) const;
	BoundaryBox getObjectBoundaryBox(const TriangleFast* pTriangle, unsigned int index) const;
	BoundaryBox getObjectBoundaryBox(const TriangleMin* pTriangle, unsigned int index) const;
	BoundaryBox getObjectBoundaryBox(const TriangleZero* pTriangle, unsigned int index) const;
	BoundaryBox getObjectBoundaryBox(const TriangleZeroMB* pTriangle, unsigned int index) const;
	BoundaryBox getObjectBoundaryBox(const Shape* pShape, unsigned int index) const;
	BoundaryBox getObjectBoundaryBox(const SphereShapeCompact* pShape, unsigned int index) const;

	BoundaryBox getObjectBoundaryBoxForMotionBlur(const Object* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
	BoundaryBox getObjectBoundaryBoxForMotionBlur(const TriangleFast* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
	BoundaryBox getObjectBoundaryBoxForMotionBlur(const TriangleMin* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
	BoundaryBox getObjectBoundaryBoxForMotionBlur(const TriangleZero* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
	BoundaryBox getObjectBoundaryBoxForMotionBlur(const TriangleZeroMB* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
	BoundaryBox getObjectBoundaryBoxForMotionBlur(const Shape* pObject, unsigned int index, float shutterOpen, float shutterClose) const;
	BoundaryBox getObjectBoundaryBoxForMotionBlur(const SphereShapeCompact* pObject, unsigned int index, float shutterOpen, float shutterClose) const;

	void getObjectBoundaryBoxesForMotionBlur(const TriangleZeroMB* pObject, unsigned int index, float shutterOpen, float shutterClose,
											 BoundaryBox& bboxT0, BoundaryBox& bboxT1) const;

	BoundaryBox getObjectClippedBoundaryBox(const Object* pObject, unsigned int index, const BoundaryBox& clipBB) const;
	BoundaryBox getObjectClippedBoundaryBox(const TriangleFast* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;
	BoundaryBox getObjectClippedBoundaryBox(const TriangleMin* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;
	BoundaryBox getObjectClippedBoundaryBox(const TriangleZero* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;
	BoundaryBox getObjectClippedBoundaryBox(const TriangleZeroMB* pTriangle, unsigned int index, const BoundaryBox& clipBB) const;
	BoundaryBox getObjectClippedBoundaryBox(const Shape* pObject, unsigned int index, const BoundaryBox& clipBB) const;
	BoundaryBox getObjectClippedBoundaryBox(const SphereShapeCompact* pObject, unsigned int index, const BoundaryBox& clipBB) const;

	const T* getConstPtr(unsigned int index) const
	{
		return m_objectHolder.getConstPtr(index);
	}

	std::vector<T*>* getObjectPtrVector()
	{
		return m_objectHolder.getObjectPtrVector();
	}

	std::vector<T>* getObjectVector()
	{
		return m_objectHolder.getObjectVector();
	}

	std::vector<const Object*>* getExtraVector()
	{
		return m_objectHolder.getExtraVector();
	}

	bool isOHPointerType() const
	{
		return m_objectHolder.isPointerType();
	}

	void clearUnneededTriangles()
	{
		m_objectHolder.clearUnneededTriangles();
	}

	//

	virtual void clear() = 0;

	virtual bool didHitObject(const Ray& ray, float& t, HitResult& result) = 0;

	virtual bool didHitObjectAlpha(const Ray& ray, float& t, HitResult& result, const Texture* alphaTexture)
	{
		return false;
	}

	virtual bool getHitObjectLazy(const Ray& ray, float& t, SelectionHitResult& result, unsigned int subLevel)
	{
		return false;
	}

	virtual bool doesOcclude(const Ray& ray) const = 0;

	virtual bool doesOccludeAlpha(const Ray& ray, HitResult& result, const Texture* alphaTexture) const
	{
		return false;
	}

	virtual void compileFromObjectPointers(std::vector<T*>& objects, const AccelStructureConfig& config)
	{

	}

	virtual void compileFromObjectPointersMotionBlur(std::vector<T*>& objects, const AccelStructureConfig& config, float shutterOpen, float shutterClose)
	{

	}

	virtual void compile(std::vector<T>& objects, const AccelStructureConfig& config)
	{

	}

	// designed to be used when m_aObjects has been externally set up already...
	virtual void compileFromInternalObjects(const AccelStructureConfig& config, bool motionBlur = false)
	{

	}

	virtual size_t getMemoryUsage(bool includeContents) const
	{
		return 0;
	}

	size_t getMainSize() const
	{
		return m_objectHolder.getMainSize();
	}

	size_t getExtraSize() const
	{
		return m_objectHolder.getExtraSize();
	}

protected:
	const RenderTriangleHolder*	m_pRenderTriangleHolder;

	OH							m_objectHolder;
};

} // namespace Imagine

#endif // ACCELERATION_STRUCTURE_H
