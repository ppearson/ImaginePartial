/*
 Imagine
 Copyright 2016 Peter Pearson.

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

#ifndef SMALL_VECTOR_H
#define SMALL_VECTOR_H

#include <stdlib.h>
#include <string.h>

#include "tagged_pointer.h"

// Note: This is currently only designed to be used with base data types, not items which allocate further heap memory themselves

namespace Imagine
{

template<typename T, size_t baseSize = 4>
class SmallVector
{
public:
	SmallVector() : m_extra()
	{

	}

	SmallVector(const SmallVector<T>& rhs)
	{
		unsigned int coreItems = rhs.m_extra.getTag();
		// TODO: use memcpy instead?
		for (unsigned int i = 0; i < coreItems; i++)
		{
			m_coreItems[i] = rhs.m_coreItems[i];
		}
		m_extra.setTag(coreItems);
		const ExtraStorage* pExtra = rhs.m_extra.getPtr();
		if (pExtra)
		{
			ExtraStorage* pClonedExtra = pExtra->clone();
			m_extra.setPointer(pClonedExtra);
		}
	}

	friend class const_iterator;

	~SmallVector()
	{
		ExtraStorage* pExtra = m_extra.getPtr();
		if (pExtra)
		{
			delete pExtra;
		}
	}

	SmallVector<T>& operator=(const SmallVector<T>& rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		unsigned int coreItems = rhs.m_extra.getTag();
		// TODO: use memcpy instead?
		for (unsigned int i = 0; i < coreItems; i++)
		{
			m_coreItems[i] = rhs.m_coreItems[i];
		}
		m_extra.setTag(coreItems);

		// delete any current extra items
		ExtraStorage* pExtra = m_extra.getPtr();
		if (pExtra)
		{
			delete pExtra;
		}

		const ExtraStorage* pRHSExtra = rhs.m_extra.getPtr();
		if (pRHSExtra)
		{
			ExtraStorage* pClonedExtra = pRHSExtra->clone();
			m_extra.setPointer(pClonedExtra);
		}
		else
		{
			m_extra.setPointer(nullptr);
		}

		return *this;
	}

	// currently, if we use this, we have to pay a double hit in terms of heap allocation...
	class ExtraStorage
	{
	public:
		ExtraStorage() : itemPtr(nullptr), size(0)
		{

		}

		~ExtraStorage()
		{
			if (itemPtr)
			{
				delete [] itemPtr;
			}
		}

		ExtraStorage(T* pSingleItem) : itemPtr(pSingleItem), size(1)
		{

		}

		ExtraStorage(T* pMultipleItems, unsigned int numItems) : itemPtr(pMultipleItems), size(numItems)
		{

		}

		ExtraStorage* clone() const
		{
			T* pItemArrayCopy = new T[size];
			memcpy(pItemArrayCopy, itemPtr, size * sizeof(T));
			ExtraStorage* pCloneItem = new ExtraStorage(pItemArrayCopy, size);

			return pCloneItem;
		}

		void addItem(const T& item)
		{
			// this is likely inefficient, but we're working on the assumption that
			// this will rarely get called

			// allocate a new array one item bigger
			T* pLargerItemArray = new T[size + 1];
			memcpy(pLargerItemArray, itemPtr, size * sizeof(T));
			pLargerItemArray[size] = item;

			// delete the old one
			delete [] itemPtr;
			itemPtr = pLargerItemArray;
			size += 1;
		}

		T*			itemPtr;
		uint32_t	size;
	};
/*
	class iterator
	{
	public:
		iterator() : m_pVector(nullptr), m_currentIndex(-1)
		{

		}

		iterator(SmallVector<T, baseSize>* pVector, unsigned int index) : m_pVector(pVector), m_currentIndex(index)
		{

		}

		iterator(const iterator& rhs) : m_pVector(rhs.m_pVector), m_currentIndex(rhs.m_currentIndex)
		{

		}

		iterator& operator=(const iterator& rhs)
		{
			// TODO: check against this first?

			m_pVector = rhs.m_pVector;
			m_currentIndex = rhs.m_currentIndex;

			return *this;
		}

		T& operator*()
		{
			if (m_currentIndex == -1u)
			{
				return m_pVector->m_coreItems[0];
			}
			else if (m_currentIndex < baseSize)
			{
				return m_pVector->m_coreItems[m_currentIndex];
			}
			else
			{
				ExtraStorage* pExtra = m_pVector->m_extra.getPtr();
				unsigned int offset = m_currentIndex - baseSize;
				return pExtra->itemPtr[offset];
			}
		}

		bool operator==(const iterator& rhs)
		{
			// for end() comparisons
			if (m_pVector == rhs.m_pVector && m_currentIndex == rhs.m_currentIndex)
				return true;

			return false;
		}

		bool operator!=(const iterator& rhs)
		{
			return !(*this == rhs);
		}

		void operator++()
		{
			// TODO: could do something more clever here maybe? But do we need to?
			m_currentIndex ++;
		}

	private:
		SmallVector<T, baseSize>*	m_pVector;
		unsigned int				m_currentIndex;
	};
*/
	class const_iterator
	{
	public:
		const_iterator() : m_pVector(nullptr), m_currentIndex(-1)
		{

		}

		const_iterator(const SmallVector<T, baseSize>* pVector, unsigned int index) : m_pVector(pVector), m_currentIndex(index)
		{

		}

		const_iterator(const const_iterator& rhs) : m_pVector(rhs.m_pVector), m_currentIndex(rhs.m_currentIndex)
		{

		}

		const_iterator& operator=(const const_iterator& rhs)
		{
			// TODO: check against this first?

			m_pVector = rhs.m_pVector;
			m_currentIndex = rhs.m_currentIndex;

			return *this;
		}

		const T& operator*() const
		{
			if (m_currentIndex == -1u)
			{
				return m_pVector->m_coreItems[0];
			}
			else if (m_currentIndex < baseSize)
			{
				return m_pVector->m_coreItems[m_currentIndex];
			}
			else
			{
				const ExtraStorage* pExtra = m_pVector->m_extra.getPtr();
				unsigned int offset = m_currentIndex - baseSize;
				return pExtra->itemPtr[offset];
			}
		}

		bool operator==(const const_iterator& rhs)
		{
			// for end() comparisons
			if (m_pVector == rhs.m_pVector && m_currentIndex == rhs.m_currentIndex)
				return true;

			return false;
		}

		bool operator!=(const const_iterator& rhs)
		{
			return !(*this == rhs);
		}

		void operator++()
		{
			// TODO: could do something more compact/clever here maybe?

			if (!m_pVector->isUsingExtra())
			{
				unsigned int coreSize = m_pVector->m_extra.getTag();
				if (m_currentIndex < (baseSize - 1))
				{
					if (m_currentIndex < (coreSize - 1))
					{
						m_currentIndex ++;
					}
					else
					{
						m_currentIndex = -1;
						m_pVector = nullptr;
					}
				}
				else
				{
					m_currentIndex = -1;
					m_pVector = nullptr;
				}
			}
			else
			{
				unsigned int coreSize = m_pVector->m_extra.getTag();
				if (m_currentIndex < (baseSize - 1))
				{
					if (m_currentIndex < (coreSize - 1))
					{
						m_currentIndex ++;
					}
					else
					{
						m_currentIndex = -1u;
						m_pVector = nullptr;
					}
				}
				else
				{
					// go into extra storage
					const ExtraStorage* pExtra = m_pVector->m_extra.getPtr();
					unsigned int nextOffset = (m_currentIndex + 1) - baseSize;

					if (nextOffset >= pExtra->size)
					{
						m_currentIndex = -1u;
						m_pVector = nullptr;
					}
					else
					{
						m_currentIndex ++;
					}
				}
			}
		}

	private:
		const SmallVector<T, baseSize>*	m_pVector;
		unsigned int					m_currentIndex;
	};

	T& operator[](unsigned int index)
	{
		if (index < baseSize)
		{
			return m_coreItems[index];
		}
		else
		{
			// The core assumption here is that they exist - if not, we're in trouble

			ExtraStorage* pExtra = m_extra.getPtr();
			unsigned int localOffset = index - baseSize;
			return pExtra->itemPtr[localOffset];
		}
	}

	const T& operator[](unsigned int index) const
	{
		if (index < baseSize)
		{
			return m_coreItems[index];
		}
		else
		{
			// The core assumption here is that they exist - if not, we're in trouble

			const ExtraStorage* pExtra = m_extra.getPtr();
			unsigned int localOffset = index - baseSize;
			return pExtra->itemPtr[localOffset];
		}
	}
/*
	iterator begin()
	{
		return iterator(this, 0);
	}
*/

	const_iterator begin()
	{
		return const_iterator(this, 0);
	}

	const_iterator begin() const
	{
		return const_iterator(this, 0);
	}
/*
	iterator end()
	{
		return iterator();
	}
*/
	const_iterator end() const
	{
		return const_iterator();
	}

	// TODO: make this really like emplace_back() ...
	void emplace_back(const T& value)
	{
		size_t nextIndex;
		if (!isUsingExtra())
		{
			nextIndex = (size_t)m_extra.getTag();

			if (nextIndex < baseSize)
			{
				// there's space in the core items to use
				m_coreItems[nextIndex] = value;

				m_extra.setTag((unsigned int)nextIndex + 1);
			}
			else
			{
				// we've used all the core items, so we need to go to extra storage on the heap

				// allocate a single item
				T* pSingleItem = new T[1];
				*pSingleItem = value;
				ExtraStorage* pExtraStorage = new ExtraStorage(pSingleItem);

				m_extra.setPointer(pExtraStorage);
			}
		}
		else
		{
			// we're already using extra storage, so we need to increase this...
			ExtraStorage* pExtra = m_extra.getPtr();
			pExtra->addItem(value);
		}
	}

	size_t size() const
	{
		size_t finalSize = (size_t)m_extra.getTag();
		if (!isUsingExtra())
		{
			return finalSize;
		}

		const ExtraStorage* pExtra = m_extra.getPtr();

		finalSize += (size_t)pExtra->size;
		return finalSize;
	}

	size_t capacity() const
	{
		// TODO: do this properly...
		return size();
	}

	void reserve(size_t size)
	{
		// TODO: hopefully this won't make that much difference, but it might be worth it given how
		//       inefficient ExtraStorage::addItem() is, but would complicate things a bit more as
		//       we'd need to track allocated size and used size instead of just the former as currently...
	}

private:
	// TODO: this shows up in profiles... - inline it? Some other way of doing this without returning a pointer?
	bool isUsingExtra() const
	{
		const ExtraStorage* pExtra = m_extra.getPtr();
		return pExtra != nullptr;
	}


private:
	// main core allocated items
	T				m_coreItems[baseSize];

	// this stores a tagged pointer of the optional extra storage,
	// and the size used of the core items
	TaggedPointer<ExtraStorage, 8>	m_extra;
};

} // namespace Imagine

#endif // SMALL_VECTOR_H

