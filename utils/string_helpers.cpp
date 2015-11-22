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

#include "string_helpers.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

void split(const std::string &str, std::vector<std::string> &tokens, const std::string &sep, int startPos)
{
	int lastPos = str.find_first_not_of(sep, startPos);
	int pos = str.find_first_of(sep, lastPos);

	while (lastPos != -1 || pos != -1)
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(sep, pos);
		pos = str.find_first_of(sep, lastPos);
	}
}

unsigned int fastSplit(const std::string& string, std::vector<StringToken>& tokens, const std::string& sep, int startPos)
{
	int lastPos = string.find_first_not_of(sep, startPos);
	int pos = string.find_first_of(sep, lastPos);

	unsigned int count = 0;

	while (lastPos != -1 || pos != -1)
	{
		assert(tokens.size() > count);

		tokens[count].start = lastPos;
		tokens[count].length = pos - lastPos;
		lastPos = string.find_first_not_of(sep, pos);
		pos = string.find_first_of(sep, lastPos);

		count ++;

//		// need to guard against a trailing separator
//		if (lastPos != -1 || pos != -1)
//			count ++;
	}

	return count;
}

unsigned int fastSplitNoEmpties(const std::string& string, std::vector<StringToken>& tokens, const std::string& sep, int startPos)
{
	int lastPos = startPos;
	int pos = startPos;

	unsigned int count = 0;

	while (true)
	{
		pos = string.find_first_of(sep, lastPos);
		if (pos != -1)
		{
			assert(tokens.size() >= count);

			tokens[count].start = lastPos;
			tokens[count].length = pos - lastPos;

			count ++;
		}
		else
		{
			pos = string.size();

			assert(tokens.size() >= count);

			tokens[count].start = lastPos;
			tokens[count].length = pos - lastPos;

			count ++;

			break;
		}

		lastPos = pos + 1;
	}

	return count;
}

void splitInTwo(const std::string& str, std::string& str1, std::string& str2, const std::string& sep, int startPos)
{
	int pos = str.find_first_of(sep, startPos);

	if (pos != -1)
	{
		str1 = str.substr(startPos, pos - startPos);
		str2 = str.substr(pos + 1);
	}
}

bool stringCompare(const char* str1, const char* str2, unsigned int length, unsigned int startIndex)
{
	while (startIndex--)
		if (*str1)
			str1++;

	while (length--)
	{
		if (!*str1)
			return false;
		if (*str1 != *str2)
			return false;

		str1++;
		str2++;
	}

	return true;
}

std::string formatSize(size_t amount)
{
	char szMemAvailable[16];
	std::string units;
	unsigned int size = 0;
	char szDecimalSize[12];
	if (amount >= 1024 * 1024 * 1024) // GB
	{
		size = amount / (1024 * 1024);
		float fSize = (float)size / 1024.0f;
		sprintf(szDecimalSize, "%.2f", fSize);
		units = "GB";
	}
	else if (amount >= 1024 * 1024) // MB
	{
		size = amount / 1024;
		float fSize = (float)size / 1024.0f;
		sprintf(szDecimalSize, "%.2f", fSize);
		units = "MB";
	}
	else if (amount >= 1024) // KB
	{
		size = amount;
		float fSize = (float)size / 1024.0f;
		sprintf(szDecimalSize, "%.1f", fSize);
		units = "KB";
	}
	else
	{
		sprintf(szDecimalSize, "0");
		units = " ";
	}

	sprintf(szMemAvailable, "%s %s", szDecimalSize, units.c_str());
	std::string final(szMemAvailable);
	return final;
}

std::string formatNumberThousandsSeparator(size_t value)
{
	char szRawNumber[32];
	sprintf(szRawNumber, "%zu", value);

	std::string temp(szRawNumber);

	std::string final;
	int i = temp.size() - 1;
	unsigned int count = 0;
	for (; i >= 0; i--)
	{
		final += temp[i];

		if (count++ == 2 && i != 0)
		{
			final += ",";
			count = 0;
		}
	}

	std::reverse(final.begin(), final.end());

	return final;
}
