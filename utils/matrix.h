/*
 Imagine
 Copyright 2011-2016 Peter Pearson.

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
	Matrix() : m_width(0), m_height(0), m_pData(NULL)
	{

	}

	Matrix(unsigned int width, unsigned int height, bool initialise = true) : m_width(width), m_height(height), m_pData(NULL)
	{
		if (initialise)
		{
			m_pData = new T[width * height];
		}
		else
		{
			m_pData = static_cast<T*>(::operator new(sizeof(T) * width * height));
		}
	}

	~Matrix()
	{
		if (m_pData)
		{
			// TODO: won't match when using operator new - used tagged pointer to tell?
			delete [] m_pData;
		}
	}

	Matrix<T> &operator=(const Matrix<T>& rhs)
	{
		copy(rhs);

		return *this;
	}

	void initialise(unsigned int width, unsigned int height, bool setToZero)
	{
		if (setToZero)
		{
			m_pData = new T[width * height];
		}
		else
		{
			m_pData = static_cast<T*>(::operator new(sizeof(T) * width * height));
		}
	}

	T* rowPtr(unsigned int row)
	{
		return m_pData + (row * m_width);
	}

	T* rawData()
	{
		return m_pData;
	}

	void copy(const Matrix<T>& source)
	{
		memcpy(m_pData, source.m_pData, m_width * m_height * sizeof(T));
	}

	void copy(const Matrix<T>* source)
	{
		memcpy(m_pData, source->m_pData, m_width * m_height * sizeof(T));
	}

	// only usable for ints or float 0.0f
	void setAll(T val)
	{
		memset(m_pData, val, m_width * m_height * sizeof(T));
	}

	void setZero()
	{
		memset(m_pData, 0, m_width * m_height * sizeof(T));
	}

	T& item(unsigned int x, unsigned int y) { return m_pData[x + m_width * y]; }
	const T& item(unsigned int x, unsigned int y) const { return m_pData[x + m_width * y]; }

	T* itemPtr(unsigned int x, unsigned int y) { return &m_pData[x + m_width * y]; }
	const T* itemPtr(unsigned int x, unsigned int y) const { return &m_pData[x + m_width * y]; }

	Row<T>& operator[](unsigned int row)
	{
		return Row<T>(this, row);
	}

protected:
	unsigned int	m_width;
	unsigned int	m_height;
	T*				m_pData;

};

} // namespace Imagine

#endif
