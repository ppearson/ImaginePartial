/*
 Imagine
 Copyright 2015-2016 Peter Pearson.

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

#ifndef ATTRIBUTE_VALUE_H
#define ATTRIBUTE_VALUE_H

#include <stdio.h>
#include <stdint.h> // for int definitions
#include <string.h> // for strcpy
#include <string>
#include <assert.h>

namespace Imagine
{

// Class designed for storing attributes on objects.
// It is designed for speed of lookup as opposed to memory compactness (for now),
// so items are a constant 16 bytes in order to allow fast lookup of Vec3f / Colour3f values
// without a further lookup on the heap causing a cache miss

class AttributeValue
{
public:
	// Note: for the moment, as we're allocating via the union 12 bytes per value anyway, there's no point
	//       specifically allowing for storage of bool, UInt8, Int8, UInt16, Int16 or Float16 (half) format stuff
	//       directly as we don't save any space, so those values will have to be up-cast to bigger types
	//		 to be stored, loosing their true type, but I think that's a reasonable trade-off,
	//       but this might change in the future.
	enum ItemType
	{
		eNone,
		eUInt64,
		eUInt32,
		eInt64,
		eInt32,
		eFloat64, // double
		eFloat32,
		eColour3f,
		ePoint3f,
		eVector3f,
		eVector2f,
		eString,
		eVoidPointer
	};

	AttributeValue() : m_dataCharS(NULL), m_stringLength(0), m_itemType(eNone)
	{

	}

	explicit AttributeValue(uint64_t value) : m_itemType(eUInt64)
	{
		m_dataUInt64 = value;
	}

	explicit AttributeValue(uint32_t value) : m_itemType(eUInt32)
	{
		m_dataUInt32 = value;
	}

	explicit AttributeValue(int64_t value) : m_itemType(eInt64)
	{
		m_dataInt64 = value;
	}

	explicit AttributeValue(int32_t value) : m_itemType(eInt32)
	{
		m_dataInt32 = value;
	}

	explicit AttributeValue(double value) : m_itemType(eFloat64)
	{
		m_dataFloat64 = value;
	}

	explicit AttributeValue(float value) : m_itemType(eFloat32)
	{
		m_dataFloat32 = value;
	}

	explicit AttributeValue(const std::string& stringValue) : m_itemType(eString)
	{
		m_dataCharS = new char[stringValue.size() + 1];
		m_stringLength = stringValue.size();
		strcpy(m_dataCharS, stringValue.c_str());
	}

	explicit AttributeValue(float value0, float value1) : m_itemType(eVector2f)
	{
		m_dataTriple[0] = value0;
		m_dataTriple[1] = value1;
	}

	explicit AttributeValue(float value0, float value1, float value2) : m_itemType(eVector3f)
	{
		m_dataTriple[0] = value0;
		m_dataTriple[1] = value1;
		m_dataTriple[2] = value2;
	}

	explicit AttributeValue(const float* pValues, unsigned int length, ItemType type) : m_itemType(type)
	{
		// assumption here is that length will be 2 or 3 (for the moment)
		assert(length <= 3);

		for (unsigned int i = 0; i < length; i++)
		{
			m_dataTriple[i] = *pValues++;
		}
	}


	AttributeValue(const AttributeValue& rhs) : m_itemType(rhs.m_itemType)
	{
		if (m_itemType != eString || m_itemType != eVoidPointer)
		{
			// if it's not a pointer type, just copy across the raw values
			m_dataTriple[0] = rhs.m_dataTriple[0];
			m_dataTriple[1] = rhs.m_dataTriple[1];
			m_dataTriple[2] = rhs.m_dataTriple[2];
		}
		else
		{
			if (m_itemType == eString)
			{
				m_dataCharS = new char[rhs.m_stringLength + 1];
				m_stringLength = rhs.m_stringLength;
				strcpy(m_dataCharS, rhs.m_dataCharS);
			}
		}
	}

	~AttributeValue()
	{
		if (m_itemType == eString && m_dataCharS)
		{
			delete [] m_dataCharS;
			m_dataCharS = NULL;
		}
		else if (m_itemType == eVoidPointer)
		{
			// TODO: when we actually use this
		}
	}

	AttributeValue& operator=(const AttributeValue& rhs)
	{
		if (&rhs == this)
			return *this;

		m_itemType = rhs.m_itemType;

		if (m_itemType == eString)
		{
			delete [] m_dataCharS;
			m_dataCharS = NULL;

			m_dataCharS = new char[rhs.m_stringLength];
			m_stringLength = rhs.m_stringLength;
			strcpy(m_dataCharS, rhs.m_dataCharS);
		}
		else if (m_itemType == eVoidPointer)
		{

		}
		else
		{
			m_dataTriple[0] = rhs.m_dataTriple[0];
			m_dataTriple[1] = rhs.m_dataTriple[1];
			m_dataTriple[2] = rhs.m_dataTriple[2];
		}

		return *this;
	}

	// we've purposefully made a decision not to bother checking for types here and just returning raw values.
	// It's up to whatever stores these items to verify types and handle defaults if required, but at least at this
	// level it can be assumed that if m_itemType is valid, the value should be valid too...

	uint64_t getUint64Value() const
	{
		return m_dataUInt64;
	}

	uint32_t getUint32Value() const
	{
		return m_dataUInt32;
	}

	int64_t getInt64Value() const
	{
		return m_dataInt64;
	}

	int32_t getInt32Value() const
	{
		return m_dataInt32;
	}

	double getFloat64Value() const
	{
		return m_dataFloat64;
	}

	float getFloat32Value() const
	{
		return m_dataFloat32;
	}

	// I don't really like doing this, but I don't want this class to have dependencies on other data types (Point, Vector, Colour3f) either...
	const float* getCompoundFloatValues() const
	{
		return &m_dataTriple[0];
	}

	const char* getStringValue() const
	{
		return m_dataCharS;
	}

protected:
	// optimise for single items with a union, so we don't have to allocate single items or compound items on the heap (other than strings).
	// Due to the fact we potentially want Vec3fs (Col3f in particular) to be as fast as possible, allow for storing
	// every single non-string/void* type within this union, without requiring a further lookup and cache-miss, at the expense
	// of a bit of space for each item...
	// We might revisit this design in the future..
	union
	{
		uint64_t		m_dataUInt64;
		uint32_t		m_dataUInt32;
		int64_t			m_dataInt64;
		int32_t			m_dataInt32;
		double			m_dataFloat64;
		float			m_dataFloat32;

		struct
		{
			char*		m_dataCharS; // we own this if it is being used as a pointer to data...
			uint32_t	m_stringLength; // don't really need this, but as we have the space in the union...
		};

		struct
		{
			void*		m_dataVoidP; // we own this if it is being used as a pointer to data...
			uint32_t	m_dataLength; // don't really need this, but as we have the space in the union...
		};

		struct
		{
			float		m_dataTriple[3];
		};
	};

	ItemType			m_itemType;
};


} // namespace Imagine

#endif // ATTRIBUTE_VALUE_H

