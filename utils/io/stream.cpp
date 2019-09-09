/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#include "stream.h"

namespace Imagine
{

Stream::Stream() : m_inError(false)
{
}

Stream::~Stream()
{
}

void Stream::storeString(const std::string& string)
{
	size_t size = string.size();

	if (size <= 255)
	{
		const unsigned char csize = (unsigned char)size;

		write((char*) &csize, sizeof(unsigned char));
		write(string.c_str(), csize);
	}
	else // cap the string at 255
	{
		std::string strLimitedString = string.substr(0, 255);

		const unsigned char csize = (unsigned char)255;

		write((char*) &csize, sizeof(unsigned char));
		write(strLimitedString.c_str(), csize);
	}
}

void Stream::loadString(std::string& string)
{
	unsigned char size = 0;
	read((char*) &size, sizeof(unsigned char));
/*
	char* buf = new char[size + 1];
	read(buf, size);
	buf[size] = 0;

	string.assign(buf);

	delete [] buf;
*/
	string.resize(size);
	read(&string[0], size);
}

void Stream::storeFloat(const float& fValue)
{
	write((const char*) &fValue, sizeof(float));
}

void Stream::loadFloat(float& fValue)
{
	read((char*) &fValue, sizeof(float));
}

void Stream::storeUInt(const unsigned int& uValue)
{
	uint32_t value32 = static_cast<uint32_t>(uValue);
	write((const char*) &value32, sizeof(uint32_t));
}

void Stream::loadUInt(unsigned int& uValue)
{
	uint32_t value32;
	read((char*) &value32, sizeof(uint32_t));
	uValue = static_cast<unsigned int>(value32);
}

void Stream::storeInt(const int& iValue)
{
	int32_t value32 = static_cast<int32_t>(iValue);
	write((const char*) &value32, sizeof(int32_t));
}

void Stream::loadInt(int& iValue)
{
	int32_t value32;
	read((char*) &value32, sizeof(int32_t));
	iValue = static_cast<unsigned int>(value32);
}

void Stream::storeEnum(unsigned int eValue)
{
	unsigned char cValue = static_cast<unsigned char>(eValue);
	write((char*) &cValue, sizeof(unsigned char));
}

unsigned int Stream::loadEnum()
{
	unsigned char cValue;
	read((char*) &cValue, sizeof(unsigned char));

	unsigned int eValue = static_cast<unsigned int>(cValue);
	return eValue;
}

unsigned char Stream::loadEnumChar()
{
	unsigned char cValue;
	read((char*) &cValue, sizeof(unsigned char));

	return cValue;
}

void Stream::storeUIntAsUChar(const unsigned int& uValue)
{
	unsigned char temp = uValue;
	write((const char*) &temp, sizeof(unsigned char));
}

void Stream::loadUIntFromUChar(unsigned int& uValue)
{
	unsigned char temp;
	read((char*) &temp, sizeof(unsigned char));
	uValue = temp;
}

void Stream::storeUIntAsUShort(const unsigned int& uValue)
{
	unsigned short temp = uValue;
	write((const char*) &temp, sizeof(unsigned short));
}

void Stream::loadUIntFromUShort(unsigned int& uValue)
{
	unsigned short temp;
	read((char*) &temp, sizeof(unsigned short));
	uValue = temp;
}

void Stream::storeUChar(const unsigned char& cValue)
{
	write((const char*) &cValue, sizeof(unsigned char));
}

void Stream::loadUChar(unsigned char& cValue)
{
	read((char*) &cValue, sizeof(unsigned char));
}

void Stream::storeHash(const HashValue& value)
{
	write((const char*) &value, sizeof(HashValue));
}

void Stream::loadHash(HashValue& value)
{
	read((char*) &value, sizeof(HashValue));
}

void Stream::storeBool(const bool& value)
{
	write((const char*) &value, sizeof(bool));
}

void Stream::loadBool(bool& value)
{
	read((char*) &value, sizeof(bool));
}

} // namespace Imagine
