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

#ifndef ITEM_TRACKING_H
#define ITEM_TRACKING_H

#include <cmath>

namespace Imagine
{

template<size_t size = 4>
class SmallestNItems
{
public:
	SmallestNItems() : numItems(0)
	{
		indices[0] = 0;
		for (unsigned int i = 0; i < size; i++)
			sizes[i] = 100000.0f;
	}

	// returns true if it's within the set of smallest items
	bool addItem(float itemSize, unsigned int index)
	{
		if (numItems == 0)
		{
			// it's the first one...
			sizes[0] = itemSize;
			indices[0] = index;

			numItems = 1;

			return true;
		}

		// if we're full, first of all see if it's bigger than the biggest item we have
		if (numItems == size && itemSize >= sizes[numItems - 1])
			return false;

		// otherwise, it is, so we need to work out where to put it, and maybe
		// shift the remainder up by one

		int newPos = (numItems == size) ? size - 1 : numItems;
		for (int i = newPos; i >= 0; i--)
		{
			float testValue = sizes[i];

			if (testValue < itemSize)
				break;

			newPos = i;
		}

		if (newPos == (size - 1))
		{
			// just overwrite the last item
			sizes[newPos] = itemSize;
			indices[newPos] = index;

			return true;
		}
		else
		{
			// otherwise, we need to shift stuff down to make room

			int maxShiftEnd = (numItems == size) ? size - 1 : (int)numItems;
			for (int i = maxShiftEnd; i > newPos; i--)
			{
				sizes[i] = sizes[i - 1];
				indices[i] = indices[i - 1];
			}

			sizes[newPos] = itemSize;
			indices[newPos] = index;

			if (numItems < size)
				numItems ++;

			return true;
		}
	}

	unsigned int getSmallestItemIndex() const
	{
		return indices[0];
	}

	// returns count of number of items, and sets pointer refs to point at first item of respective arrays
	unsigned int getFinalItems(const float*& pSizes, const unsigned int*& pIndices) const
	{
		pSizes = &sizes[0];
		pIndices = &indices[0];

		return numItems;
	}

protected:
	float			sizes[size];
	unsigned int	indices[size];

	unsigned int	numItems;
};

} // namespace Imagine

#endif // ITEM_TRACKING_H

