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

#ifndef ACCEL_SETTINGS_H
#define ACCEL_SETTINGS_H

namespace Imagine
{

class Stream;

enum AccelStructureStatus
{
	eAccelStructureStatusClean		= 0,
	eAccelStructureStatusPicking	= 1,
	eAccelStructureStatusRendering	= 2
};

class AccelSettings
{
public:
	AccelSettings();
	AccelSettings(unsigned int flags);
	AccelSettings(const AccelSettings& rhs);

	AccelSettings& operator=(const AccelSettings& rhs);

	static unsigned int kKDTreeMeshBitSetGoodClipping;

	static unsigned int kKDTreeCompoundObjectBitSet;
	static unsigned int kKDTreeCompoundObjectBitSetGood;
	static unsigned int kKDTreeCompoundObjectBitSetGoodClipping;
	static unsigned int kKDTreeCompoundObjectBitSetGoodAllAxes;
	static unsigned int kKDTreeCompoundObjectBitSetGoodAllAxesClipping;

	static unsigned int kBVHSSEGoodClippingTrianglePackets;
	static unsigned int kBVHSSEAllAxisLeafThreshold2;

	unsigned int getType() const;
	void setType(unsigned int type);

	bool hasGoodPartitioning() const;
	void setGoodPartitioning(bool goodPartitioning);

	bool hasCheckAllAxes() const;
	void setCheckAllAxes(bool checkAllAxes);

	bool hasClipping() const;
	void setClipping(bool clipping);

	bool hasTrianglePackets() const;
	void setTrianglePackets(bool triPackets);

	unsigned int getLeafNodeThreshold() const;
	void setLeafNodeThreshold(unsigned int threshold);

	unsigned int getMaxDepth() const;
	void setMaxDepth(unsigned int maxDepth);

	bool hasConserveMemory() const;
	void setConserveMemory(bool conserveMemory);

	AccelStructureStatus getStatus() const;
	void setStatus(AccelStructureStatus status);

	void load(Stream* stream, unsigned int version);
	void store(Stream* stream) const;

	bool isSameAs(const AccelSettings& other, bool ignoreStatus) const;

protected:
	void setFlagForBoolValue(unsigned int flagBit, bool value);


protected:
	// bits from right:
	// 2 : type (0 - 2)
	// 1 : good partitioning
	// 1 : check all axes
	// 1 : clipping
	// 1 : triangle packets
	// 8 : leaf node threshold (0 - 255)
	// 6 : max depth (0-63)
	// 1 : conserve memory

	// bit of a cheat this, but we've got the bits to spare:
	// left-hand side
	// 2: acceleration structure status

	unsigned int	m_flags;
};

} // namespace Imagine

#endif // ACCEL_SETTINGS_H
