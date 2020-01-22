/*
 Imagine
 Copyright 2017-2020 Peter Pearson.

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

#ifndef DATA_CONVERSION_H
#define DATA_CONVERSION_H

#include <string>
#include <inttypes.h>
#include <algorithm>

namespace Imagine
{

// TODO: these can be templatised, and we should probably make them be specialised
//       "toLittleEndian" type things which are platform dependent...

inline static float reverseFloatBytes(float value)
{
	// use a separate variable in an attempt to reduce aliasing...
	float finalValue;
	
	const unsigned char* pSrc = (const unsigned char*)&value;
	unsigned char* pDst = (unsigned char*)&finalValue;
	
	for (unsigned int i = 0; i < 4; i++)
	{
		pDst[i] = pSrc[3 - i];
	}
	
	return finalValue;
}

inline static unsigned int reverseUIntBytes(unsigned int value)
{
	// use a separate variable in an attempt to reduce aliasing...
	unsigned int finalValue;
	
	const unsigned char* pSrc = (const unsigned char*)&value;
	unsigned char* pDst = (unsigned char*)&finalValue;
	
	for (unsigned int i = 0; i < 4; i++)
	{
		pDst[i] = pSrc[3 - i];
	}
	
	return finalValue;
}

inline static uint16_t reverseUInt16Bytes(uint16_t value)
{
	uint16_t finalValue;
	
	const unsigned char* pSrc = (const unsigned char*)&value;
	unsigned char* pDst = (unsigned char*)&finalValue;
	
	pDst[0] = pSrc[1];
	pDst[1] = pSrc[0];
	
	return finalValue;
}

}

#endif // DATA_CONVERSION_H

