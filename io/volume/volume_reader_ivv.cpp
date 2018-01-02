/*
 Imagine
 Copyright 2014-2016 Peter Pearson.

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

#include "volume_reader_ivv.h"

#include <stdio.h>

#include "global_context.h"

#include "volumes/volume_grid_sparse.h"

namespace Imagine
{

#define PACKED_READ 1

VolumeReaderIVV::VolumeReaderIVV()
{
}

bool VolumeReaderIVV::readHeaderAndBBox(const std::string& filePath, unsigned int flags, BoundaryBox& bbox)
{
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile)
	{
		GlobalContext::instance().getLogger().error("Can't open volume file: %s", filePath.c_str());
		return false;
	}
	
	size_t bytesRead = 0;

	unsigned char version = 0;
	bytesRead = fread(&version, sizeof(unsigned char), 1, pFile);
	
	if (bytesRead != sizeof(unsigned char))
	{
		GlobalContext::instance().getLogger().error("Can't read volume file: %s", filePath.c_str());
		return false;
	}
	
	// TODO: decent error handling on reads...

	bool denseGrid = true;
	unsigned int subCellSize = 0;
	bool floatFormat = true;
	if (version > 0)
	{
		unsigned char formatType = 0;
		fread(&formatType, sizeof(unsigned char), 1, pFile);

		if (formatType == 1)
		{
			// half format
			floatFormat = false;
		}

		if (version > 1)
		{
			unsigned char gridType = 0;
			fread(&gridType, sizeof(unsigned char), 1, pFile);

			if (gridType == 1)
			{
				denseGrid = false;

				unsigned short tempCellSize = 0;
				fread(&tempCellSize, sizeof(unsigned short), 1, pFile);
				subCellSize = tempCellSize;
			}
		}
	}

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;

	fread(&x, sizeof(unsigned int), 1, pFile);
	fread(&y, sizeof(unsigned int), 1, pFile);
	fread(&z, sizeof(unsigned int), 1, pFile);

	Point bbMin;
	Point bbMax;

	fread(&bbMin.x, sizeof(float), 1, pFile);
	fread(&bbMin.y, sizeof(float), 1, pFile);
	fread(&bbMin.z, sizeof(float), 1, pFile);

	fread(&bbMax.x, sizeof(float), 1, pFile);
	fread(&bbMax.y, sizeof(float), 1, pFile);
	fread(&bbMax.z, sizeof(float), 1, pFile);

	BoundaryBox localBB;
	localBB.includePoint(bbMin);
	localBB.includePoint(bbMax);

	bbox = localBB;

	fclose(pFile);
	return true;
}

VolumeInstance* VolumeReaderIVV::readVolume(const std::string& filePath, unsigned int flags, BoundaryBox& bbox)
{
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile)
	{
		GlobalContext::instance().getLogger().error("Error opening volume file: %s", filePath.c_str());
		return NULL;
	}

	size_t bytesRead = 0;

	unsigned char version = 0;
	bytesRead = fread(&version, sizeof(unsigned char), 1, pFile);
	
	if (bytesRead != sizeof(unsigned char))
	{
		GlobalContext::instance().getLogger().error("Can't read volume file: %s", filePath.c_str());
		return NULL;
	}
	
	// TODO: decent error handling on reads...

	bool floatFormat = true;
	bool denseGrid = true;
	unsigned int subCellSize = 0;
	if (version > 0)
	{
		unsigned char formatType = 0;
		fread(&formatType, sizeof(unsigned char), 1, pFile);

		if (formatType == 1)
		{
			// half format
			floatFormat = false;
		}

		if (version > 1)
		{
			unsigned char gridType = 0;
			fread(&gridType, sizeof(unsigned char), 1, pFile);

			if (gridType == 1)
			{
				denseGrid = false;

				unsigned short tempCellSize = 0;
				fread(&tempCellSize, sizeof(unsigned short), 1, pFile);
				subCellSize = tempCellSize;
			}
		}
	}

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;

	fread(&x, sizeof(unsigned int), 1, pFile);
	fread(&y, sizeof(unsigned int), 1, pFile);
	fread(&z, sizeof(unsigned int), 1, pFile);

	Point bbMin;
	Point bbMax;

	fread(&bbMin.x, sizeof(float), 1, pFile);
	fread(&bbMin.y, sizeof(float), 1, pFile);
	fread(&bbMin.z, sizeof(float), 1, pFile);

	fread(&bbMax.x, sizeof(float), 1, pFile);
	fread(&bbMax.y, sizeof(float), 1, pFile);
	fread(&bbMax.z, sizeof(float), 1, pFile);

	BoundaryBox localBB;
	localBB.includePoint(bbMin);
	localBB.includePoint(bbMax);
	bbox = localBB;

	VolumeGridDense* pVolGridDense = NULL;
	VolumeGridSparse* pVolGridSparse = NULL;
	VolumeInstance* pVolInstance = NULL;

	if (denseGrid)
	{
		if (floatFormat)
			pVolGridDense = new VolumeGridDense(eGridValueTypeFloat);
		else
			pVolGridDense = new VolumeGridDense(eGridValueTypeHalf);

		pVolInstance = pVolGridDense;
	}
	else
	{
		if (floatFormat)
			pVolGridSparse = new VolumeGridSparse(eGridValueTypeFloat);
		else
			pVolGridSparse = new VolumeGridSparse(eGridValueTypeHalf);

		pVolInstance = pVolGridSparse;
	}

	if (!pVolInstance)
	{
		fclose(pFile);
		GlobalContext::instance().getLogger().error("Could not allocate memory for volume grid for volume file: %s", filePath.c_str());
		return NULL;
	}

	pVolInstance->setLocalBoundaryBox(localBB);

	if (denseGrid)
	{
		pVolGridDense->resizeGrid(x, y, z);

#if PACKED_READ
		// given that the axis order is fixed, we can just read the whole lot in one go
		size_t fullArrayLength = z * y * x;

		if (floatFormat)
		{
			fread(pVolGridDense->getRawFloatArray(), sizeof(float), fullArrayLength, pFile);
		}
		else
		{
			fread(pVolGridDense->getRawHalfArray(), sizeof(half), fullArrayLength, pFile);
		}
#else
		for (unsigned int k = 0; k < z; k++)
		{
			for (unsigned int j = 0; j < y; j++)
			{
				if (floatFormat)
				{
					for (unsigned int i = 0; i < x; i++)
					{
						float voxelValue = 0.0f;
						fread(&voxelValue, sizeof(float), 1, pFile);

						pVolGridDense->setVoxelValueFloat(i, j, k, voxelValue);
					}
				}
				else
				{
					for (unsigned int i = 0; i < x; i++)
					{
						half voxelValue = 0.0f;
						fread(&voxelValue, sizeof(half), 1, pFile);

						pVolGridDense->setVoxelValueHalf(i, j, k, voxelValue);
					}
				}
			}
		}
#endif
	}
	else
	{
		pVolGridSparse->resizeGrid(x, y, z, subCellSize);

		// iterate over each SubCell, reading in

		std::vector<VolumeGridSparse::SparseSubCell*>& aSubCells = pVolGridSparse->getSubCells();
		
		// group the subCells into batches of 8, so we can be efficient and use an unsigned char
		// as a bitset for the state of the next 8 subcells
			
		unsigned int cellsRemaining = aSubCells.size();
		
		// if it's the newer slightly more compact format
		if (version >= 3)
		{
			for (unsigned int cellIndex = 0; cellIndex < aSubCells.size(); )
			{
				unsigned int batchSize = std::min(cellsRemaining, 8u);
				
				unsigned char subCellStateFlags = 0;
				
				// read in the state of the next 8 subcells
				fread(&subCellStateFlags, sizeof(unsigned char), 1, pFile);
				
				for (unsigned int batchIndex = 0; batchIndex < batchSize; batchIndex++)
				{
					bool hasData = subCellStateFlags & (1 << batchIndex);
					
					if (!hasData)
						continue;
					
					VolumeGridSparse::SparseSubCell* pSubCell = aSubCells[cellIndex + batchIndex];
					
					unsigned int cellDataLength = pSubCell->getResXY() * pSubCell->getResZ();
	
					if (floatFormat)
					{
						pSubCell->allocateIfNeeded(false);
						float* pCellFloatData = pSubCell->getRawFloatData();
						fread(pCellFloatData, sizeof(float), cellDataLength, pFile);
					}
					else
					{
						pSubCell->allocateIfNeeded(true);
						half* pCellHalfData = pSubCell->getRawHalfData();
						fread(pCellHalfData, sizeof(half), cellDataLength, pFile);
					}
				}
				
				cellIndex += 8;
				cellsRemaining -= batchSize;
			}
		}
		else
		{
			unsigned char subCellStatus = 0;
	
			std::vector<VolumeGridSparse::SparseSubCell*>::iterator itSubCell = aSubCells.begin();
			for (; itSubCell != aSubCells.end(); ++itSubCell)
			{
				VolumeGridSparse::SparseSubCell* pSubCell = *itSubCell;
	
				fread(&subCellStatus, sizeof(unsigned char), 1, pFile);
	
				if (subCellStatus == 0)
				{
					// it's empty, so just continue without reading data for this subcell
				}
				else
				{
					unsigned int cellDataLength = pSubCell->getResXY() * pSubCell->getResZ();
	
					if (floatFormat)
					{
						pSubCell->allocateIfNeeded(false);
						float* pCellFloatData = pSubCell->getRawFloatData();
						fread(pCellFloatData, sizeof(float), cellDataLength, pFile);
					}
					else
					{
						pSubCell->allocateIfNeeded(true);
						half* pCellHalfData = pSubCell->getRawHalfData();
						fread(pCellHalfData, sizeof(half), cellDataLength, pFile);
					}
				}
			}
		}
	}

	fclose(pFile);

	return pVolInstance;
}


} // namespace Imagine

namespace
{
	Imagine::VolumeReader* createVolumeReaderIVV()
	{
		return new Imagine::VolumeReaderIVV();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerVolumeReader("ivv", createVolumeReaderIVV);
}
