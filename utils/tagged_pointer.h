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

#ifndef TAGGED_POINTER_H
#define TAGGED_POINTER_H

#include <stdint.h>

namespace Imagine
{

template <typename T, int alignment>
class TaggedPointer
{
public:
	inline TaggedPointer()
	{
		setBoth(NULL, 0);
	}

	inline TaggedPointer(T* pointer, unsigned int tag = 0)
	{
		setBoth(pointer, tag);
	}

	inline void setBoth(T* pointer, unsigned int tag = 0)
	{
		m_pPointer = pointer;
		m_tag |= tag & kTagMask;
	}

	inline void setPointer(T* pointer)
	{
		unsigned int tempTag = m_tag & kTagMask;
		m_pPointer = pointer;
		m_tag |= tempTag;
	}

	inline void setTag(unsigned int tag)
	{
		T* pTemp = reinterpret_cast<T*>(m_tag & kPointerMask);
		m_pPointer = pTemp;
		m_tag |= tag & kTagMask;
	}

	inline T* getPtr()
	{
		return reinterpret_cast<T*>(m_tag & kPointerMask);
	}

	inline const T* getPtr() const
	{
		return reinterpret_cast<T*>(m_tag & kPointerMask);
	}

	inline unsigned int getTag() const
	{
		return m_tag & kTagMask;
	}

private:
	static const uintptr_t kTagMask = alignment - 1;
	static const uintptr_t kPointerMask = ~kTagMask;

	union
	{
		T*			m_pPointer;
		uintptr_t	m_tag;
	};
};

} // namespace Imagine

#endif // TAGGED_POINTER_H
