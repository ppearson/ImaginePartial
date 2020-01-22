/*
 Imagine
 Copyright 2013-2019 Peter Pearson.

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

#ifndef MATRIX2_H
#define MATRIX2_H

// for memcpy
#include <string.h>

#include "core/vector2.h"

namespace Imagine
{

class Matrix2
{
public:
	Matrix2()
	{
	}
	
	#define MAT(row, col) (col * 2) + row
	
	Matrix2(float v0, float v1, float v2, float v3)
	{
		m[MAT(0,0)] = v0;
		m[MAT(1,0)] = v1;
		m[MAT(0,1)] = v2;
		m[MAT(1,1)] = v3;
		
//		m[MAT(0,0)] = v0;
//		m[MAT(0,1)] = v1;
//		m[MAT(1,0)] = v2;
//		m[MAT(1,1)] = v3;
	}


	inline Matrix2(const Matrix2& rhs)
	{
		memcpy(m, rhs.m, sizeof(float) * 4);
	}

	inline Matrix2& operator=(const Matrix2& rhs)
	{
		memcpy(m, rhs.m, sizeof(float) * 4);
		return *this;
	}

	static Matrix2 multiply(const Matrix2& m1, const Matrix2& m2)
	{
		Matrix2 final;

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				final.m[MAT(j,i)] = m1.m[MAT(j,0)] * m2.m[MAT(0,i)] + m1.m[MAT(j,1)] * m2.m[MAT(1,i)];
			}
		}

		return final;
	}

	inline Matrix2& operator *= (const Matrix2& rhs)
	{
		Matrix2 result = multiply(*this, rhs);

		*this = result;
		return *this;
	}

	float& at(unsigned int row, unsigned int col)
	{
		return m[MAT(row, col)];
	}

	float at(unsigned int row, unsigned int col) const
	{
		return m[MAT(row, col)];
	}

	void transpose()
	{
		Matrix2 transposedMatrix;
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				transposedMatrix.m[MAT(i,j)] = m[MAT(j,i)];
			}
		}

		*this = transposedMatrix;
	}

	inline Vector2 transform(const Vector2& vector) const
	{
		return Vector2(m[MAT(0,0)] * vector.x + m[MAT(0,1)] * vector.y,
					  m[MAT(1,0)] * vector.x + m[MAT(1,1)] * vector.y);
	}

	float det() const
	{
		float determinant = (m[MAT(0,0)] * (m[MAT(1,1)]) -
							 m[MAT(1,0)] * (m[MAT(0,1)]));

		return determinant;
	}


	////


	float		m[4];
};


} // namespace Imagine

#endif // MATRIX2_H

