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

#ifndef STREAM_H
#define STREAM_H

#include <string>
#include <inttypes.h>

namespace Imagine
{

typedef unsigned long long HashValue;

class Stream
{
public:
	Stream();
	virtual ~Stream();

	virtual bool read(void* ptr, size_t size) = 0;
	virtual bool write(const void* ptr, size_t size) = 0;

	virtual bool canRead() = 0;
	virtual bool canWrite() = 0;

	bool isInError() const { return m_inError; }

	// capped to 255 chars...
	void storeString(const std::string& string);
	void loadString(std::string& string);

	void storeFloat(const float& fValue);
	void loadFloat(float& fValue);

	void storeUInt(const unsigned int& uValue);
	void loadUInt(unsigned int& uValue);

	void storeEnum(unsigned int eValue);
	unsigned int loadEnum();
	unsigned char loadEnumChar();

	void storeUIntAsUChar(const unsigned int& uValue);
	void loadUIntFromUChar(unsigned int& uValue);

	void storeUIntAsUShort(const unsigned int& uValue);
	void loadUIntFromUShort(unsigned int& uValue);

	void storeUChar(const unsigned char& cValue);
	void loadUChar(unsigned char& cValue);

	void storeHash(const HashValue& value);
	void loadHash(HashValue& value);

	void storeBool(const bool& value);
	void loadBool(bool& value);

protected:

	bool	m_inError;
};

} // namespace Imagine

#endif // STREAM_H
