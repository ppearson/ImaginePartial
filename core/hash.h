/*
 Imagine
 Copyright 2011-2015 Peter Pearson.

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

#ifndef HASH_H
#define HASH_H

#include <string>

class Point;
class Vector;

typedef unsigned long long HashValue;

class Hash
{
public:
	Hash();

	void addInt(int value);
	void addUInt(unsigned int value);
	void addLongLong(long long value);
	void addFloat(float value);
	void addFloatFast(float value);
	void addUChar(unsigned char value);
	void addString(const std::string& value);

	HashValue getHash() const { return m_hash; }

	void addPoint(const Point& point);
	void addVector(const Vector& vector);

	static HashValue hashPoint(const Point& point);

	static HashValue stringHash(const std::string& string);
	static HashValue stringHashJenkins(const std::string& string);

protected:
	void addData(const unsigned char* buffer, unsigned int length);
	static HashValue hashData(const unsigned char* buffer, unsigned int length);

public:
	HashValue m_hash;
};

#endif // HASH_H
