/*
 Imagine
 Copyright 2013 Peter Pearson.

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

#include "accel_settings.h"

#include "utils/io/stream.h"

namespace Imagine
{

static const unsigned int kStatusMask = (3 << 30);
static const unsigned int kGoodPartitioningFlagMask = (1 << 2);
static const unsigned int kCheckAllAxesFlagMask = (1 << 3);
static const unsigned int kClippingFlagMask = (1 << 4);
static const unsigned int kTrianglePacketsFlagMask = (1 << 5);

static const unsigned int kLeafNodeThesholdMask = (255 << 6);
static const unsigned int kMaxDepthMask = (63 << 14);
static const unsigned int kConserveMemoryMask = (1 << 21);

// Mesh KDTree with leaf node threshold = 0 (default), max depth = 0, good partitioning and clipping
unsigned int AccelSettings::kKDTreeMeshBitSetGoodClipping = 20u;

// Compound KDTree with leaf node threshold = 3
unsigned int AccelSettings::kKDTreeCompoundObjectBitSet = 192u;

// Compound KDTree with leaf node threshold = 3 and good partitioning
unsigned int AccelSettings::kKDTreeCompoundObjectBitSetGood = 196u;

// Compound KDTree with leaf node threshold = 3 and good partitioning with clipping
unsigned int AccelSettings::kKDTreeCompoundObjectBitSetGoodClipping = 212u;

// Compound KDTree with leaf node threshold = 3 and good partitioning and all axes
unsigned int AccelSettings::kKDTreeCompoundObjectBitSetGoodAllAxes = 204u;

// Compound KDTree with leaf node threshold = 3 and good partitioning and all axes, clipping
unsigned int AccelSettings::kKDTreeCompoundObjectBitSetGoodAllAxesClipping = 220u;

// Triangle BVH SSE, with Triangle Packets and good clipping, leaf node threshold = 3
unsigned int AccelSettings::kBVHSSEGoodClippingTrianglePackets = 246u;

// BVH SSE, leaf threshold 2, all axis - for scene
unsigned int AccelSettings::kBVHSSEAllAxisLeafThreshold2 = 142u;

AccelSettings::AccelSettings()
{
	m_flags = 0;
}

AccelSettings::AccelSettings(unsigned int flags)
{
	m_flags = flags;
}

AccelSettings::AccelSettings(const AccelSettings& rhs)
{
	// copy, but clear status value
	m_flags = rhs.m_flags;

	m_flags &= (~kStatusMask);
}

AccelSettings& AccelSettings::operator=(const AccelSettings& rhs)
{
	// copy, but clear status value
	m_flags = rhs.m_flags;

	m_flags &= (~kStatusMask);

	return *this;
}

unsigned int AccelSettings::getType() const
{
	return m_flags & 3;
}

void AccelSettings::setType(unsigned int type)
{
	type = type & 3;

	m_flags |= type;
}

bool AccelSettings::hasGoodPartitioning() const
{
	return (m_flags & kGoodPartitioningFlagMask);
}

void AccelSettings::setGoodPartitioning(bool goodPartitioning)
{
	unsigned int localFlag = 1 << 2;

	setFlagForBoolValue(localFlag, goodPartitioning);
}

bool AccelSettings::hasCheckAllAxes() const
{
	return (m_flags & kCheckAllAxesFlagMask);
}

void AccelSettings::setCheckAllAxes(bool checkAllAxes)
{
	unsigned int localFlag = 1 << 3;

	setFlagForBoolValue(localFlag, checkAllAxes);
}

bool AccelSettings::hasClipping() const
{
	return (m_flags & kClippingFlagMask);
}

void AccelSettings::setClipping(bool clipping)
{
	unsigned int localFlag = 1 << 4;

	setFlagForBoolValue(localFlag, clipping);
}

bool AccelSettings::hasTrianglePackets() const
{
	return (m_flags & kTrianglePacketsFlagMask);
}

void AccelSettings::setTrianglePackets(bool triPackets)
{
	unsigned int localFlag = 1 << 5;

	setFlagForBoolValue(localFlag, triPackets);
}

unsigned int AccelSettings::getLeafNodeThreshold() const
{
	unsigned int leafNodeThreshold = m_flags & kLeafNodeThesholdMask;

	leafNodeThreshold = leafNodeThreshold >> 6;

	return leafNodeThreshold;
}

void AccelSettings::setLeafNodeThreshold(unsigned int threshold)
{
	unsigned int localLeafNodeThreshold = threshold;
	localLeafNodeThreshold = localLeafNodeThreshold << 6;

	m_flags &= ~kLeafNodeThesholdMask;
	m_flags |= localLeafNodeThreshold;
}

unsigned int AccelSettings::getMaxDepth() const
{
	unsigned int maxDepth = m_flags & kMaxDepthMask;

	maxDepth = maxDepth >> 14;

	return maxDepth;
}

void AccelSettings::setMaxDepth(unsigned int maxDepth)
{
	unsigned int localMaxDepth = maxDepth;
	localMaxDepth = localMaxDepth << 14;

	m_flags &= ~kMaxDepthMask;
	m_flags |= localMaxDepth;
}

bool AccelSettings::hasConserveMemory() const
{
	return (m_flags & kConserveMemoryMask);
}

void AccelSettings::setConserveMemory(bool conserveMemory)
{
	setFlagForBoolValue(kConserveMemoryMask, conserveMemory);
}

AccelStructureStatus AccelSettings::getStatus() const
{
	unsigned int status = (m_flags & kStatusMask) >> 30;
	return (AccelStructureStatus)status;
}

void AccelSettings::setStatus(AccelStructureStatus status)
{
	unsigned int localStatus = status;
	localStatus = (localStatus << 30);
	// clear mask first
	m_flags &= ~kStatusMask;
	m_flags |= localStatus;
}

void AccelSettings::load(Stream* stream, unsigned int version)
{
	stream->loadUInt(m_flags);
}

void AccelSettings::store(Stream* stream) const
{
	// clear status value first
	unsigned int storeValue = m_flags;
	storeValue &= (~kStatusMask);

	stream->storeUInt(storeValue);
}

bool AccelSettings::isSameAs(const AccelSettings& other, bool ignoreStatus) const
{
	if (!ignoreStatus)
	{
		return m_flags == other.m_flags;
	}
	else
	{
		unsigned int thisStrippedStatus = m_flags;
		thisStrippedStatus &= ~kStatusMask;

//		fprintf(stderr, "BitVal: %u\n", thisStrippedStatus);

		unsigned int otherStrippedStatus = other.m_flags;
		otherStrippedStatus &= ~kStatusMask;

		return thisStrippedStatus == otherStrippedStatus;
	}
}

void AccelSettings::setFlagForBoolValue(unsigned int flagBit, bool value)
{
	if (value)
	{
		m_flags |= flagBit;
	}
	else
	{
		m_flags &= ~flagBit;
	}
}

} // namespace Imagine
