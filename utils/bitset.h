/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#ifndef BITSET_H
#define BITSET_H

#include <vector>
#include <algorithm>

#include <stdio.h>
#include <string.h>

namespace Imagine
{

// This is arguably pointless in that std::vector<bool> is specialised to do this anyway,
// but we might need to make this sparse in the future...

class Bitset
{
public:
	Bitset()
	{
	}

	void initialise(unsigned int size)
	{
		unsigned int numCharsNeeded = size / 8;
		if (size % 8 > 0)
		{
			numCharsNeeded += 1;
		}

		m_aBits.resize(numCharsNeeded, 0);
	}

	void reset()
	{
		std::fill(m_aBits.begin(), m_aBits.end(), 0);
	}

	void setBit(unsigned int index)
	{
		unsigned int charIndex = index / 8;
		unsigned int charBitOffset = index % 8;

		unsigned int charValue = m_aBits[charIndex];
		charValue |= (1 << charBitOffset);

		m_aBits[charIndex] = charValue;
	}

	bool isSet(unsigned int index) const
	{
		unsigned int charIndex = index / 8;
		unsigned int charBitOffset = index % 8;

		unsigned int charValue = m_aBits[charIndex];

		return charValue & (1 << charBitOffset);
	}


protected:
	std::vector<unsigned char>	m_aBits;
};

} // namespace Imagine

#endif // BITSET_H
