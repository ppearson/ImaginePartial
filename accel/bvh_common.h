/*
 Imagine
 Copyright 2013-2016 Peter Pearson.

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

#ifndef BVH_COMMON_H
#define BVH_COMMON_H

#include <vector>
#include <string.h>     // for memset
#include <stdint.h>		// for uintptr_t

#include "accel/acceleration_structure.h"
#include "accel/primitives_list.h"
#include "accel/partitioner.h"

#include "core/boundary_box.h"

#include "utils/slab_allocator.h"
#include "utils/threads/mutex.h"

namespace Imagine
{

#define USE_BVHTEMP_TAGGED_POINTER 1

class BVHTempNode
{
public:
	BVHTempNode()
	{
	}

	~BVHTempNode()
	{
#if USE_BVHTEMP_TAGGED_POINTER
		if (isLeaf() && getObjectsPtr())
		{
			delete [] getObjectsPtr();
		}
#else
		if (isLeaf() && leaf.m_pObjects)
			delete [] leaf.m_pObjects;
#endif
	}

#if USE_BVHTEMP_TAGGED_POINTER
	inline void setLeftChild(BVHTempNode* pLeftChild)
	{
		unsigned int tempTag = m_tag & kFlagsMask;
		inner.m_pLeftChild = pLeftChild;
		m_tag |= tempTag;
	}

	inline const uint32_t* getObjectsPtr() const
	{
		return reinterpret_cast<const uint32_t*>(m_tag & kPointerMask);
	}
#endif


	void setInterior(uint32_t axis, const BoundaryBox& bbox, BVHTempNode* pLeftChild, BVHTempNode* pRightChild)
	{
		m_boundaryBox = bbox;

#if USE_BVHTEMP_TAGGED_POINTER
		inner.m_pLeftChild = pLeftChild;
		inner.m_pRightChild = pRightChild;

		m_tag |= axis;
#else
		inner.m_flags = axis;

		inner.m_pLeftChild = pLeftChild;
		inner.m_pRightChild = pRightChild;
#endif
	}

	void setLeaf(const std::vector<uint32_t>& objects, const BoundaryBox& bbox)
	{
		m_boundaryBox = bbox;

#if USE_BVHTEMP_TAGGED_POINTER
		leaf.m_objectsCount = objects.size();
		leaf.m_pObjects = new uint32_t[leaf.m_objectsCount];
		memcpy(leaf.m_pObjects, &objects[0], leaf.m_objectsCount * sizeof(uint32_t));
		m_tag |= 3;
#else
		leaf.m_flags = 3;
		leaf.m_objectsCount = objects.size();
		leaf.m_pObjects = new uint32_t[leaf.m_objectsCount];
		memcpy(leaf.m_pObjects, &objects[0], leaf.m_objectsCount * sizeof(uint32_t));
#endif
	}

	void setLeaf(const PrimitivesList& primsList, uint32_t primStart, uint32_t primCount, const BoundaryBox& bbox)
	{
		m_boundaryBox = bbox;

#if USE_BVHTEMP_TAGGED_POINTER
		leaf.m_objectsCount = primCount;
		leaf.m_pObjects = new uint32_t[primCount];

		unsigned int endIndex = primStart + primCount;
		unsigned int targetIndex = 0;
		for (unsigned int i = primStart; i < endIndex; i++)
		{
			leaf.m_pObjects[targetIndex++] = primsList.primitives[i];
		}
		m_tag |= 3;
#else
		leaf.m_flags = 3;
		leaf.m_objectsCount = primCount;
		leaf.m_pObjects = new uint32_t[primCount];

		unsigned int endIndex = primStart + primCount;
		unsigned int targetIndex = 0;
		for (unsigned int i = primStart; i < endIndex; i++)
		{
			leaf.m_pObjects[targetIndex++] = primsList.primitives[i];
		}
#endif
	}

	void setEmptyStaticLeaf()
	{
#if USE_BVHTEMP_TAGGED_POINTER
		leaf.m_objectsCount = 0;
		leaf.m_pObjects = NULL;
		m_tag = 3;
#else
		leaf.m_flags = 3;
		leaf.m_objectsCount = 0;
		leaf.m_pObjects = NULL;
#endif
	}

	bool isLeaf() const
	{
#if USE_BVHTEMP_TAGGED_POINTER
		unsigned int tag = m_tag & kFlagsMask;
		return tag == 3;
#else
		return (inner.m_flags & 3) == 3;
#endif
	}

	uint32_t getAxis() const
	{
#if USE_BVHTEMP_TAGGED_POINTER
		return m_tag & kFlagsMask;
#else
		return inner.m_flags & 3;
#endif
	}

	BVHTempNode* getLeftChild() const
	{
#if USE_BVHTEMP_TAGGED_POINTER
		return reinterpret_cast<BVHTempNode*>(m_tag & kPointerMask);
#else
		return inner.m_pLeftChild;
#endif
	}

	BVHTempNode* getRightChild() const
	{
#if USE_BVHTEMP_TAGGED_POINTER
		return inner.m_pRightChild;
#else
		return inner.m_pRightChild;
#endif
	}

	BVHTempNode* getChild(unsigned int index) const
	{
#if USE_BVHTEMP_TAGGED_POINTER
		return (index == 0) ? getLeftChild() : inner.m_pRightChild;
#else
		return (index == 0) ? inner.m_pLeftChild : inner.m_pRightChild;
#endif
	}

	void swapChildren()
	{
#if USE_BVHTEMP_TAGGED_POINTER
		BVHTempNode* oldLeft = getLeftChild();
		BVHTempNode* oldRight = inner.m_pRightChild;

		setLeftChild(oldRight);
		inner.m_pRightChild = oldLeft;
#else
		std::swap(inner.m_pLeftChild, inner.m_pRightChild);
#endif
	}

	const uint32_t* getObjects(uint32_t& count) const
	{
#if USE_BVHTEMP_TAGGED_POINTER
		count = leaf.m_objectsCount;
		return getObjectsPtr();
#else
		count = leaf.m_objectsCount;
		return leaf.m_pObjects;
#endif
	}

	uint32_t getObjectsCount() const
	{
#if USE_BVHTEMP_TAGGED_POINTER
		return leaf.m_objectsCount;
#else
		return leaf.m_objectsCount;
#endif
	}

public:
	BoundaryBox			m_boundaryBox;

protected:
	static const uintptr_t kFlagsMask = 3;
	static const uintptr_t kPointerMask = ~kFlagsMask;

#if USE_BVHTEMP_TAGGED_POINTER
	union
	{
		//
		uintptr_t				m_tag;

		struct
		{
			BVHTempNode*		m_pLeftChild; // should be accessed through tag
			BVHTempNode*		m_pRightChild;
		} inner;

		struct
		{
			uint32_t*			m_pObjects; // should be accessed through tag
			uint32_t			m_objectsCount;
		} leaf;
	};
#else
	union
	{
		struct
		{
			unsigned char		m_flags;
			BVHTempNode*		m_pLeftChild;
			BVHTempNode*		m_pRightChild;
		} inner;

		struct
		{
			unsigned char		m_flags;
			uint32_t			m_objectsCount;
			uint32_t*			m_pObjects;
		} leaf;
	};
#endif
};

struct BVHBuildThreadState
{
	BVHBuildThreadState(Partitioner* pPartitioner, bool ownPartitioner, bool mallocTrimOnDelete) :
		m_newNodeAllocator(true, mallocTrimOnDelete, 32768 * 4), m_pPartitioner(pPartitioner), m_ownPartitioner(ownPartitioner)
	{
	}

	~BVHBuildThreadState()
	{
		if (m_ownPartitioner && m_pPartitioner)
		{
			delete m_pPartitioner;
		}
	}

	FixedSlabAllocator		m_newNodeAllocator;

	Partitioner*			m_pPartitioner;

	bool					m_ownPartitioner;
};

struct BVHBuildContext
{
	BVHBuildContext(const AccelStructureConfig& accelConfig, bool useSlabAlloc = false)
		: accelConfig(accelConfig), m_pRootNode(NULL), m_pEmptyLeafTemp(NULL),
			m_nextFreeNodeIndex(0), m_nextFreeObjectArrayIndex(0), m_nextFreeObjectIndex(0), m_triPacketIndex(0),
			m_useSlabAllocator(useSlabAlloc), m_pFirstBuildState(NULL)
	{
		m_pRootNode = new BVHTempNode();

		m_pEmptyLeafTemp = new BVHTempNode();
		m_pEmptyLeafTemp->setEmptyStaticLeaf();
	}

	~BVHBuildContext()
	{
		// m_pNode is deleted elsewhere...

		if (m_pEmptyLeafTemp)
		{
			delete m_pEmptyLeafTemp;
			m_pEmptyLeafTemp = NULL;
		}

		if (m_useSlabAllocator)
		{
			std::vector<BVHBuildThreadState*>::iterator itThread = m_aBuildThreadState.begin();
			for (; itThread != m_aBuildThreadState.end(); ++itThread)
			{
				delete *itThread;
			}

			// purposefully delete this one last, so it's possible to selectively do a mallocTrim()
			// on just it, after the others have already been freed
			if (m_pFirstBuildState)
			{
				delete m_pFirstBuildState;
				m_pFirstBuildState = NULL;
			}

			// now delete the first node which currently isn't allocated from a slab allocator - TODO: should probably do that...
			if (m_useSlabAllocator && m_pRootNode)
			{
				delete m_pRootNode;
				m_pRootNode = NULL;
			}
		}
	}

	BVHBuildThreadState* createFirstBuildState(Partitioner* pParentPartitioner, bool mallocTrim)
	{
		// we don't own this partitioner
		BVHBuildThreadState* pNewBuildState = new BVHBuildThreadState(pParentPartitioner, false, mallocTrim);

		m_pFirstBuildState = pNewBuildState;

		return pNewBuildState;
	}

	BVHBuildThreadState* getNewBuildState(Partitioner* pParentPartitioner, bool mallocTrim)
	{
		BVHBuildThreadState* pNewBuildState = new BVHBuildThreadState(pParentPartitioner, true, mallocTrim);

		m_buildThreadStateLock.lock();
		m_aBuildThreadState.push_back(pNewBuildState);
		m_buildThreadStateLock.unlock();

		return pNewBuildState;
	}

	const AccelStructureConfig&	accelConfig;

	BVHTempNode*			m_pRootNode;

	// single empty leaf node to reduce memory consumption during tree construction
	BVHTempNode*			m_pEmptyLeafTemp;

	// counters for final nodes
	unsigned int			m_nextFreeNodeIndex;

	// counters for final objects lists
	unsigned int			m_nextFreeObjectArrayIndex;

	unsigned int			m_nextFreeObjectIndex;

	unsigned int			m_triPacketIndex;

	bool					m_useSlabAllocator;
	// first and main build state item, purposefully separated
	BVHBuildThreadState*	m_pFirstBuildState;

	Mutex					m_buildThreadStateLock;
	std::vector<BVHBuildThreadState*> m_aBuildThreadState;
};



} // namespace Imagine

#endif // BVH_COMMON_H
