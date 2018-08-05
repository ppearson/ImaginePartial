/*
 Imagine
 Copyright 2011-2017 Peter Pearson.

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

#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <string.h>

#include "tagged_pointer.h"

namespace Imagine
{

template<typename T> class Matrix;

template<typename T>
class Row
{
public:
	explicit Row(class Matrix<T>* matrix, unsigned int row) : m_pMatrix(matrix), m_row(row) { }

	inline T& operator[](int col)
//	inline T& Row<T>::operator[](int row)
	{
		return m_pMatrix->item(col, m_row);
	}

protected:
	Matrix<T>*	m_pMatrix;
	unsigned int	m_row;
};

template<typename T>
class Matrix
{
public:
	Matrix() : m_width(0), m_height(0)
	{

	}

	Matrix(unsigned int width, unsigned int height, bool initValues = true) : m_width(width), m_height(height)
	{
		// use a tagged pointer to keep track of how the memory was allocated in order to delete it appropriately later
		if (initValues)
		{
			m_data.setBoth(new T[width * height], 0);
		}
		else
		{
			m_data.setBoth(static_cast<T*>(::operator new(sizeof(T) * width * height)), 1);
		}
	}

	~Matrix()
	{
		if (m_data.getPtr())
		{
			unsigned int tag = m_data.getTag();
			// work out which delete to call based on if it was operator newed when allocated.
			if (tag == 0)
			{
				delete [] m_data.getPtr();
			}
			else
			{
				delete m_data.getPtr();
			}
		}
	}

	Matrix<T>& operator=(const Matrix<T>& rhs)
	{
		copy(rhs);

		return *this;
	}

	void initialise(unsigned int width, unsigned int height, bool initValues)
	{
		// use a tagged pointer to keep track of how the memory was allocated in order to delete it appropriately later
		if (initValues)
		{
			m_data.setBoth(new T[width * height], 0);
		}
		else
		{
			m_data.setBoth(static_cast<T*>(::operator new(sizeof(T) * width * height)), 1);
		}
	}

	T* rowPtr(unsigned int row)
	{
		return m_data.getPtr() + (row * m_width);
	}

	T* rawData()
	{
		return m_data.getPtr();
	}

	// Note: the assumption here is that the dimensions match - in current usages this is the case,
	//       but this is likely to change in the future, so this will need to be made more robust...
	void copy(const Matrix<T>& source)
	{
		memcpy(m_data.getPtr(), source.m_data.getPtr(), m_width * m_height * sizeof(T));
	}

	void copy(const Matrix<T>* source)
	{
		memcpy(m_data.getPtr(), source->m_data.getPtr(), m_width * m_height * sizeof(T));
	}

	// only usable for ints or float 0.0f
	void setAll(T val)
	{
		memset(m_data.getPtr(), val, m_width * m_height * sizeof(T));
	}

	void setZero()
	{
		memset(m_data.getPtr(), 0, m_width * m_height * sizeof(T));
	}

	T& item(unsigned int x, unsigned int y)
	{
		return m_data.getPtr()[x + m_width * y];
	}
	const T& item(unsigned int x, unsigned int y) const
	{
		return m_data.getPtr()[x + m_width * y];
	}

	T* itemPtr(unsigned int x, unsigned int y)
	{
		return &m_data.getPtr()[x + m_width * y];
	}
	const T* itemPtr(unsigned int x, unsigned int y) const
	{
		return &m_data.getPtr()[x + m_width * y];
	}

	Row<T>& operator[](unsigned int row)
	{
		return Row<T>(this, row);
	}

protected:
	// tagged pointer is used to track how we allocated the original memory...
	TaggedPointer<T, 2>		m_data;
	
	unsigned int			m_width;
	unsigned int			m_height;
};

} // namespace Imagine

#endif
