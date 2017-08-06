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

#ifndef GEO_READER_PLY_H
#define GEO_READER_PLY_H

#include "io/geo_reader.h"

#include <fstream>
#include <vector>

namespace Imagine
{

class GeoReaderPly : public GeoReader
{
public:
	GeoReaderPly();
	virtual ~GeoReaderPly()
	{
	}

	enum ePlyType
	{
		eNone,
		eASCII,
		eBinaryBigEndian,
		eBinaryLittleEndian
	};
	
	struct Property
	{
		enum DataType
		{
			eNone,
			eFloat,
			eDouble,
			eChar,
			eUChar,
			eInt,
			eUInt
		};
		
		Property() :	mainDataType(eNone), list(false), listDataType(eNone)
		{
			
		}
		
		static size_t getMemSize(DataType type)
		{
			if (type == eFloat || type == eInt || type == eUInt)
				return 4;
			else if (type == eChar || type == eUChar)
				return 1;
			
			return 0;
		}
		
		std::string		name;
		DataType		mainDataType;
		bool			list;
		DataType		listDataType;
	};
	
	struct Element
	{
		Element() : type(eENone), count(0), xVIndex(-1), yVIndex(-1), zVIndex(-1)
		{
			
		}
		
		enum ElementType
		{
			eENone,
			eEVertex,
			eEFace,
			eEEdge			
		};
		
		ElementType		type;
		unsigned int	count;
		
		// short-cut helpers
		unsigned char	xVIndex;
		unsigned char	yVIndex;
		unsigned char	zVIndex;
		
		std::vector<Property>	properties;
	};

	struct PlyHeader
	{
		PlyHeader() : type(eNone), version(0)
		{
		}

		ePlyType type;
		unsigned int version;
		
		std::vector<Element>	elements;
	};

	virtual bool readFile(const std::string& path, const GeoReaderOptions& options);

	bool readHeader(std::fstream& fileStream, PlyHeader& header) const;
	
	bool readASCIIFile(std::fstream& fileStream, const PlyHeader& header, const GeoReaderOptions& options);
	bool readBinaryFile(std::fstream& fileStream, const std::string& path, const PlyHeader& header, const GeoReaderOptions& options);
};

} // namespace Imagine

#endif // GEO_READER_PLY_H
