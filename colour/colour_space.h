/*
 Imagine
 Copyright 2012-2019 Peter Pearson.

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

#ifndef COLOUR_SPACE_H
#define COLOUR_SPACE_H

#include <cmath>

#include "colour3f.h"
#include "colour3h.h"
#include "colour3b.h"
#include "colour4f.h"

namespace Imagine
{

class ColourSpace
{
public:
	ColourSpace();

	static void convertSRGBToLinearAccurate(Colour3h& colour);
	static void convertSRGBToLinearAccurate(Colour3f& colour);
	static void convertSRGBToLinearAccurate(Colour4f& colour);

	static void convertLinearToSRGBAccurate(Colour3f& colour);
	static void convertLinearToSRGBAccurate(Colour4f& colour);

	static void convertLinearToSRGBFast(Colour3h& colour);
	static void convertLinearToSRGBFast(Colour3f& colour);
	static void convertLinearToSRGBFast(Colour4f& colour);

	static Colour3f convertByteSRGBToLinearLUT(const Colour3b& colour);

	static Colour3f convertByteHSVToLinearRGB(unsigned char H, unsigned char S, unsigned char V);
	static Colour3f convertLinearHSVToLinearRGB(float H, float S, float V);
	

	////

	inline static float convertSRGBToLinearAccurate(float value)
	{
		float out;
		if (value <= 0.04045f)
			out = value * 0.077399380804954f; // 1.0 / 12.92
		else
			out = powf(((value + 0.055f) * 0.947867298578199f), 2.4f); // powf(((value + 0.055f) / 1.055f), 2.4f)

		return out;
	}

	// there's only 256 values, so we can use a LUT...
	inline static float convertSRGBToLinearLUT(unsigned char value)
	{
		return m_SRGBToLinearLUT[value];
	}

	inline static Colour3f convertSRGBToLinearLUT(unsigned char red, unsigned char green, unsigned char blue)
	{
		float fRed = m_SRGBToLinearLUT[red];
		float fGreen = m_SRGBToLinearLUT[green];
		float fBlue = m_SRGBToLinearLUT[blue];

		return Colour3f(fRed, fGreen, fBlue);
	}

	static float convertLinearToSRGBAccurate(float value)
	{
		if (value <= 0.0031308f)
		{
			return 12.92f * value;
		}
		else
		{
			return 1.055f * powf(value, 0.4166667f) - 0.055f;
		}
	}

	static float convertLinearToSRGBFast(float value)
	{
		if (value <= 0.0031308f)
		{
			return 12.92f * value;
		}
		else
		{
			return 1.055f * fastPow512(value) - 0.055f;
		}
	}

	// fast equiv function for calculating powf(x, 0.4166667f); (5 / 12)
	inline static float fastPow512(float value)
	{
		 float cbrtValue = cbrtf(value);
		 return cbrtValue * sqrtf(sqrtf(cbrtValue));
	}

	inline static float convertByteToLinearFloat(unsigned char value)
	{
		float finalVal = m_byteToFloatLUT[value];
		return finalVal;
	}

	static void initLUTs();


protected:
	static bool			m_SRGBLutTableInit;
	static bool			m_byteToFloatLutTableInit;

	static float		m_SRGBToLinearLUT[256];
	static float		m_byteToFloatLUT[256];
};

} // namespace Imagine

#endif // COLOUR_SPACE_H
