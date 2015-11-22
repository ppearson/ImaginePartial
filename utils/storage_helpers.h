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

#ifndef STORAGE_HELPERS_H
#define STORAGE_HELPERS_H

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
//#include <stdint.h>
#include <inttypes.h>

typedef unsigned long long HashValue;

void storeString(const std::string& string, std::fstream& stream);
void loadString(std::string& string, std::fstream& stream);

void storeFloat(float& fValue, std::fstream& stream);
void loadFloat(float& fValue, std::fstream& stream);

void storeUInt(unsigned int& uValue, std::fstream& stream);
void loadUInt(unsigned int& uValue, std::fstream& stream);

void storeEnum(unsigned int eValue, std::fstream& stream);
unsigned int loadEnum(std::fstream& stream);

void storeUintAsUChar(unsigned int& uValue, std::fstream& stream);
void loadUIntFromUChar(unsigned int& uValue, std::fstream& stream);

void storeUChar(unsigned char& cValue, std::fstream& stream);
void loadUChar(unsigned char& cValue, std::fstream& stream);

void storeHash(HashValue& value, std::fstream& stream);
void loadHash(HashValue& value, std::fstream& stream);

void storeBool(bool& value, std::fstream& stream);
void loadBool(bool& value, std::fstream& stream);

#endif // STORAGE_HELPERS_H
