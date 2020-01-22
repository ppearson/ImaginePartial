/*
 Imagine
 Copyright 2013-2015 Peter Pearson.

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

#ifndef MATRIX3_H
#define MATRIX3_H

// for memcpy
#include <string.h>

#include "core/vector.h"

namespace Imagine
{

class Matrix3
{
public:
	Matrix3()
	{

	}

	#define MAT(row, col) (col * 3) + row

	inline Matrix3(const Matrix3& rhs)
	{
		memcpy(m, rhs.m, sizeof(float) * 9);
	}

	inline Matrix3& operator=(const Matrix3& rhs)
	{
		memcpy(m, rhs.m, sizeof(float) * 9);
		return *this;
	}

	static Matrix3 multiply(const Matrix3& m1, const Matrix3& m2)
	{
		Matrix3 final;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				final.m[MAT(j,i)] = m1.m[MAT(j,0)] * m2.m[MAT(0,i)] + m1.m[MAT(j,1)] * m2.m[MAT(1,i)] + m1.m[MAT(j,2)] * m2.m[MAT(2,i)];
			}
		}

		return final;
	}

	inline Matrix3& operator *= (const Matrix3& rhs)
	{
		Matrix3 result = multiply(*this, rhs);

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
		Matrix3 transposedMatrix;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				transposedMatrix.m[MAT(i,j)] = m[MAT(j,i)];
			}
		}

		*this = transposedMatrix;
	}

	inline Vector transform(const Vector& vector) const
	{
		return Vector(m[MAT(0,0)] * vector.x + m[MAT(0,1)] * vector.y + m[MAT(0,2)] * vector.z,
					  m[MAT(1,0)] * vector.x + m[MAT(1,1)] * vector.y + m[MAT(1,2)] * vector.z,
					  m[MAT(2,0)] * vector.x + m[MAT(2,1)] * vector.y + m[MAT(2,2)] * vector.z);
	}

	float det() const
	{
		float determinant = (m[MAT(0,0)] * (m[MAT(1,1)] * m[MAT(2,2)] - m[MAT(2,1)] * m[MAT(1,2)]) -
							 m[MAT(1,0)] * (m[MAT(0,1)] * m[MAT(2,2)] - m[MAT(2,1)] * m[MAT(0,2)]) +
							 m[MAT(2,0)] * (m[MAT(0,1)] * m[MAT(1,2)] - m[MAT(1,1)] * m[MAT(0,2)]));

		return determinant;
	}

	Matrix3 inverse() const
	{
		// Calculate the determinant
		Matrix3 inverted;

		float determinant = det();

		if (fabsf(determinant) < 0.000001f)
		{
			// return default identity
			return inverted;
		}

		determinant = 1.0f / determinant;

		// find the inverse

		inverted.m[MAT(0,0)] =  determinant * (m[MAT(1,1)] * m[MAT(2,2)] - m[MAT(2,1)] * m[MAT(1,2)]);
		inverted.m[MAT(1,0)] = -determinant * (m[MAT(1,0)] * m[MAT(2,2)] - m[MAT(2,0)] * m[MAT(1,2)]);
		inverted.m[MAT(2,0)] =  determinant * (m[MAT(1,0)] * m[MAT(2,1)] - m[MAT(2,0)] * m[MAT(1,1)]);

		inverted.m[MAT(0,1)] = -determinant * (m[MAT(0,1)] * m[MAT(2,2)] - m[MAT(2,1)] * m[MAT(0,2)]);
		inverted.m[MAT(1,1)] =  determinant * (m[MAT(0,0)] * m[MAT(2,2)] - m[MAT(2,0)] * m[MAT(0,2)]);
		inverted.m[MAT(2,1)] = -determinant * (m[MAT(0,0)] * m[MAT(2,1)] - m[MAT(2,0)] * m[MAT(0,1)]);

		inverted.m[MAT(0,2)] =  determinant * (m[MAT(0,1)] * m[MAT(1,2)] - m[MAT(1,1)] * m[MAT(0,2)]);
		inverted.m[MAT(1,2)] = -determinant * (m[MAT(0,0)] * m[MAT(1,2)] - m[MAT(1,0)] * m[MAT(0,2)]);
		inverted.m[MAT(2,2)] =  determinant * (m[MAT(0,0)] * m[MAT(1,1)] - m[MAT(1,0)] * m[MAT(0,1)]);

		return inverted;
	}



	////


	float		m[9];
};

} // namespace Imagine

#endif // MATRIX3_H

