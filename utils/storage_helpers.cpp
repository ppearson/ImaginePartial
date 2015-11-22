/*
 Imagine
 Copyright 2011-2012 Peter Pearson.

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

#include "storage_helpers.h"

void storeString(const std::string& string, std::fstream& stream)
{
	int size = string.size();

	if (size <= 256)
	{
		const unsigned char csize = (unsigned char)size;

		stream.write((char*)&csize, sizeof(unsigned char));
		stream.write(string.c_str(), csize);
	}
	else // cap the string at 256
	{
		std::string strLimitedString = string.substr(0, 256);

		const unsigned char csize = (unsigned char)256;

		stream.write((char*)&csize, sizeof(unsigned char));
		stream.write(strLimitedString.c_str(), csize);
	}
}

void loadString(std::string& string, std::fstream& stream)
{
	unsigned char size = 0;
	stream.read((char*)&size, sizeof(unsigned char));

	char* buf = new char[size + 1];
	stream.read(buf, size);
	buf[size] = 0;

	string.assign(buf);

	delete [] buf;
}

void storeFloat(float& fValue, std::fstream& stream)
{
	stream.write((char*)&fValue, sizeof(float));
}

void loadFloat(float& fValue, std::fstream& stream)
{
	stream.read((char*)&fValue, sizeof(float));
}

void storeUInt(unsigned int& uValue, std::fstream& stream)
{
	uint32_t value32 = static_cast<uint32_t>(uValue);
	stream.write((char*)&value32, sizeof(uint32_t));
}

void loadUInt(unsigned int& uValue, std::fstream& stream)
{
	uint32_t value32;
	stream.read((char*)&value32, sizeof(uint32_t));
	uValue = static_cast<unsigned int>(value32);
}

void storeEnum(unsigned int eValue, std::fstream& stream)
{
	unsigned char cValue = static_cast<unsigned char>(eValue);
	stream.write((char*)&cValue, sizeof(unsigned char));
}

unsigned int loadEnum(std::fstream& stream)
{
	unsigned char cValue;
	stream.read((char*)&cValue, sizeof(unsigned char));

	unsigned int eValue = static_cast<unsigned int>(cValue);
	return eValue;
}

void storeUintAsUChar(unsigned int& uValue, std::fstream& stream)
{
	unsigned char temp = uValue;
	stream.write((char*)&temp, sizeof(unsigned char));
}

void loadUIntFromUChar(unsigned int& uValue, std::fstream& stream)
{
	unsigned char temp;
	stream.read((char*)&temp, sizeof(unsigned char));
	uValue = temp;
}

void storeUChar(unsigned char& cValue, std::fstream& stream)
{
	stream.write((char*)&cValue, sizeof(unsigned char));
}

void loadUChar(unsigned char& cValue, std::fstream& stream)
{
	stream.read((char*)&cValue, sizeof(unsigned char));
}

void storeHash(HashValue& value, std::fstream& stream)
{
	stream.write((char*)&value, sizeof(HashValue));
}

void loadHash(HashValue& value, std::fstream& stream)
{
	stream.read((char*)&value, sizeof(HashValue));
}

void storeBool(bool& value, std::fstream& stream)
{
	stream.write((char*)&value, sizeof(bool));
}

void loadBool(bool& value, std::fstream& stream)
{
	stream.read((char*)&value, sizeof(bool));
}
