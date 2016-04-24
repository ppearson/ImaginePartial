/*
 Imagine
 Copyright 2011-2016 Peter Pearson.

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

#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H

#include <string>
#include <vector>
#include <stdint.h>

struct StringToken
{
	StringToken()
	{

	}

	StringToken(size_t st, size_t len) : start(st), length(len)
	{

	}

	size_t start;
	size_t length;
};

void split(const std::string& str, std::vector<std::string>& tokens, const std::string& sep, int startPos = 0);

// std::vector must have sufficient items pre-allocated
unsigned int fastSplit(const std::string& string, std::vector<StringToken>& tokens, const std::string& sep, int startPos = 0);
unsigned int fastSplitNoEmpties(const std::string& string, std::vector<StringToken>& tokens, const std::string& sep, int startPos = 0);

void splitInTwo(const std::string& str, std::string& str1, std::string& str2, const std::string& sep, int startPos = 0);

bool stringCompare(const char* str1, const char* str2, unsigned int length, unsigned int startIndex = 0);

std::string formatSize(size_t amount);
std::string formatNumberThousandsSeparator(size_t value);

std::string formatTimePeriodSeconds(double seconds, bool keepAsSeconds = false);
std::string formatTimePeriod(uint64_t time);


#endif // STRING_HELPERS_H
