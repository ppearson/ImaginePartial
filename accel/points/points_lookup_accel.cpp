/*
 Imagine
 Copyright 2017 Peter Pearson.

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

#include "points_lookup_accel.h"

#include <fstream>
#include <string.h>
#include <stdio.h>

#include "core/matrix4.h"

#include "utils/file_helpers.h"

namespace Imagine
{

// TODO: some of this stuff should be moved out of here at some point

PointsLookupAccel::PointsLookupAccel()
{

}

bool PointsLookupAccel::readPointCloudFile(const std::string& filePath, std::vector<PointColour3f>& points)
{
	std::string extension = FileHelpers::getFileExtension(filePath);
	
	if (extension == "txt" || extension == "xyz")
	{
		std::fstream fileStream(filePath.c_str(), std::ios::in);
		
		char buf[256];
		memset(buf, 0, 256);
	
		points.reserve(100000);
	
		while (fileStream.getline(buf, 256))
		{
			if (buf[0] == 0)
				continue;
	
			PointColour3f newPoint;
	
			unsigned char colR;
			unsigned char colG;
			unsigned char colB;
	
			sscanf(buf, "%f %f %f %hhu %hhu %hhu", &newPoint.position.x, &newPoint.position.y, &newPoint.position.z,
									&colR, &colG, &colB);
	
	
			newPoint.colour.r = (float)colR / 255.0f;
			newPoint.colour.g = (float)colG / 255.0f;
			newPoint.colour.b = (float)colB / 255.0f;
	
			points.push_back(newPoint);
		}
	
		fileStream.close();
/*		
		return true;
		
		FILE* pOutFile = fopen("/tmp/3.ipc", "wb");

		Matrix4 transform;
//		transform.translate(-1.74f, -3.99f, 01.178f);
		transform.rotate(-24.3f, -51.0f, 1.3f, Matrix4::eYXZ);
//		transform.scale(0.1265f, 0.1265f, 0.1265f);
		
		unsigned char version = 2;		
		fwrite(&version, sizeof(version), 1, pOutFile);
		
		unsigned char type = 3;
		fwrite(&type, sizeof(type), 1, pOutFile);
		
		uint32_t numPoints = points.size();		
		fwrite(&numPoints, sizeof(uint32_t), 1, pOutFile);
		
		std::vector<PointColour3f>::const_iterator itPoint = points.begin();
		for (; itPoint != points.end(); ++itPoint)
		{
			const PointColour3f& p = *itPoint;
			
			Point transformedPoint = transform.transform(p.position);
			
			fwrite(&transformedPoint.x, sizeof(float), 3, pOutFile);
			
			unsigned char r = (unsigned char)(p.colour.r * 255.0f);
			unsigned char g = (unsigned char)(p.colour.g * 255.0f);
			unsigned char b = (unsigned char)(p.colour.b * 255.0f);
			
			fwrite(&r, sizeof(unsigned char), 1, pOutFile);
			fwrite(&g, sizeof(unsigned char), 1, pOutFile);			
			fwrite(&b, sizeof(unsigned char), 1, pOutFile);			
		}
		
		fclose(pOutFile);
*/
	}
	else if (extension == "ipc")
	{
		FILE* pInFile = fopen(filePath.c_str(), "rb");
		unsigned char version = 0;
		fread(&version, sizeof(version), 1, pInFile);
		unsigned char type = 0;
		fread(&type, sizeof(type), 1, pInFile);
		
		if (version < 2)
		{
			fclose(pInFile);
			return false;
		}
		
		if (type == 1)
		{
			// just position, so no point opening
			fclose(pInFile);
			return false;
		}
		
		uint32_t numPoints = 0;
		fread(&numPoints, sizeof(uint32_t), 1, pInFile);
		
		points.resize(numPoints);
		
		if (type == 2)
		{
			// position and float colour
			for (unsigned int i = 0; i < numPoints; i++)
			{
				PointColour3f& p = points[i];
				
				fread(&p.position.x, sizeof(float), 3, pInFile);
				
				fread(&p.colour.r, sizeof(float), 3, pInFile);
			}
		}
		else if (type == 3)
		{
			unsigned char r = 0;
			unsigned char g = 0;
			unsigned char b = 0;
			// position and float colour
			for (unsigned int i = 0; i < numPoints; i++)
			{
				PointColour3f& p = points[i];
				
				fread(&p.position.x, sizeof(float), 3, pInFile);
				
				fread(&r, sizeof(unsigned char), 1, pInFile);
				fread(&g, sizeof(unsigned char), 1, pInFile);
				fread(&b, sizeof(unsigned char), 1, pInFile);
				
				p.colour.r = (float)r / 255.0f;
				p.colour.g = (float)g / 255.0f;
				p.colour.b = (float)b / 255.0f;
			}
		}
		
		fclose(pInFile);
	}
	else
	{
		return false;
	}

	return true;
}

} // namespace Imagine
