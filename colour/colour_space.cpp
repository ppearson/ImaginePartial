/*
 Imagine
 Copyright 2012-2016 Peter Pearson.

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

#include "colour_space.h"

#include "utils/maths/maths.h"

namespace Imagine
{

bool ColourSpace::m_SRGBLutTableInit = false;
bool ColourSpace::m_byteToFloatLutTableInit = false;

float ColourSpace::m_SRGBToLinearLUT[256];
float ColourSpace::m_byteToFloatLUT[256];

ColourSpace::ColourSpace()
{
}

void ColourSpace::convertSRGBToLinearAccurate(Colour3h& colour)
{
	colour.r = convertSRGBToLinearAccurate(colour.r);
	colour.g = convertSRGBToLinearAccurate(colour.g);
	colour.b = convertSRGBToLinearAccurate(colour.b);
}

void ColourSpace::convertSRGBToLinearAccurate(Colour3f& colour)
{
	colour.r = convertSRGBToLinearAccurate(colour.r);
	colour.g = convertSRGBToLinearAccurate(colour.g);
	colour.b = convertSRGBToLinearAccurate(colour.b);
}

void ColourSpace::convertSRGBToLinearAccurate(Colour4f& colour)
{
	colour.r = convertSRGBToLinearAccurate(colour.r);
	colour.g = convertSRGBToLinearAccurate(colour.g);
	colour.b = convertSRGBToLinearAccurate(colour.b);
}

void ColourSpace::convertLinearToSRGBAccurate(Colour3f& colour)
{
	colour.r = convertLinearToSRGBAccurate(colour.r);
	colour.g = convertLinearToSRGBAccurate(colour.g);
	colour.b = convertLinearToSRGBAccurate(colour.b);
}

void ColourSpace::convertLinearToSRGBAccurate(Colour4f& colour)
{
	colour.r = convertLinearToSRGBAccurate(colour.r);
	colour.g = convertLinearToSRGBAccurate(colour.g);
	colour.b = convertLinearToSRGBAccurate(colour.b);
}

void ColourSpace::convertLinearToSRGBFast(Colour3h& colour)
{
	colour.r = convertLinearToSRGBFast(colour.r);
	colour.g = convertLinearToSRGBFast(colour.g);
	colour.b = convertLinearToSRGBFast(colour.b);
}

void ColourSpace::convertLinearToSRGBFast(Colour3f& colour)
{
	colour.r = convertLinearToSRGBFast(colour.r);
	colour.g = convertLinearToSRGBFast(colour.g);
	colour.b = convertLinearToSRGBFast(colour.b);
}

void ColourSpace::convertLinearToSRGBFast(Colour4f& colour)
{
	colour.r = convertLinearToSRGBFast(colour.r);
	colour.g = convertLinearToSRGBFast(colour.g);
	colour.b = convertLinearToSRGBFast(colour.b);
}

Colour3f ColourSpace::convertByteSRGBToLinearLUT(const Colour3b& colour)
{
	return convertSRGBToLinearLUT(colour.r, colour.g, colour.b);
}

// based off: http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
Colour3f ColourSpace::convertByteHSVToLinearRGB(unsigned char H, unsigned char S, unsigned char V)
{
	if (S == 0)
	{
		// TODO: should likely return V (grey)
		return Colour3f(0.0f);
	}

	// do this at full int precision to prevent overflow...
	unsigned int h = H;
	unsigned int s = S;
	unsigned int v = V;

	unsigned char region = h / 43;
	unsigned int remainder = (h - (region * 43)) * 6;

	unsigned char p = (v * (255 - s)) >> 8;
	unsigned char q = (v * (255 - ((s * remainder) >> 8))) >> 8;
	unsigned char t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

	Colour3b rgb;

	switch (region)
	{
		case 0:
			rgb.r = v;
			rgb.g = t;
			rgb.b = p;
			break;
		case 1:
			rgb.r = q;
			rgb.g = v;
			rgb.b = p;
			break;
		case 2:
			rgb.r = p;
			rgb.g = v;
			rgb.b = t;
			break;
		case 3:
			rgb.r = p;
			rgb.g = q;
			rgb.b = v;
			break;
		case 4:
			rgb.r = t;
			rgb.g = p;
			rgb.b = v;
			break;
		default:
			rgb.r = v;
			rgb.g = p;
			rgb.b = q;
			break;
	}

	return convertSRGBToLinearLUT(rgb.r, rgb.g, rgb.b);
}

Colour3f ColourSpace::convertLinearHSVToLinearRGB(float H, float S, float V)
{
	if (S == 0.0f)
	{
		// TODO: should likely return V (grey)
		return Colour3f(0.0f);
	}
	
	H *= 6.0f; // scale for sectors
	
	float region = std::floor(H);
	float factorial = H - region;
	
	float p = V * (1.0f - S);
	float q = V * (1.0f - S * factorial);
	float t = V * (1.0f - S * (1.0f - factorial));
	
	Colour3f rgb;
	
	switch ((int)region)
	{
		case 0:
			rgb.r = V;
			rgb.g = t;
			rgb.b = p;
			break;
		case 1:
			rgb.r = q;
			rgb.g = V;
			rgb.b = p;
			break;
		case 2:
			rgb.r = p;
			rgb.g = V;
			rgb.b = t;
			break;
		case 3:
			rgb.r = p;
			rgb.g = q;
			rgb.b = V;
			break;
		case 4:
			rgb.r = t;
			rgb.g = p;
			rgb.b = V;
			break;
		default:
			rgb.r = V;
			rgb.g = p;
			rgb.b = q;
			break;
	}
	
	return rgb;
}

void ColourSpace::initLUTs()
{
	if (!m_SRGBLutTableInit)
	{
		const float inv255 = (1.0f / 255.0f);
		for (unsigned int i = 0; i < 256; i++)
		{
			float linearValue = convertSRGBToLinearAccurate(float(i) * inv255);
			m_SRGBToLinearLUT[i] = linearValue;
		}
	}

	if (!m_byteToFloatLutTableInit)
	{
		const float inv255 = (1.0f / 255.0f);
		for (unsigned int i = 0; i < 256; i++)
		{
			float floatValue = float(i) * inv255;
			m_byteToFloatLUT[i] = floatValue;
		}
	}
}

} // namespace Imagine
