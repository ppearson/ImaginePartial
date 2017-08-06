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

#include "string_helpers.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <stdarg.h>

namespace Imagine
{

void splitString(const std::string& str, std::vector<std::string>& tokens, const std::string& sep, int startPos)
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

unsigned int fastStringSplit(const std::string& string, std::vector<StringToken>& tokens, const std::string& sep, int startPos)
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
	{
		if (*str1)
			str1++;
	}

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
		units = "B"; // just so it makes sense
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

std::string formatTimePeriodSeconds(double seconds, bool keepAsSeconds)
{
	char szTemp[32];
	if (keepAsSeconds)
	{
		sprintf(szTemp, "%0.4f s", seconds);
		return std::string(szTemp);
	}

	if (seconds < 60.0)
	{
		sprintf(szTemp, "00:%02.f m", seconds);
		return std::string(szTemp);
	}

	unsigned int minutes = (unsigned int)(seconds / 60.0);
	seconds -= minutes * 60;

	// we need to cater for seconds now being something like 59.98999,
	// which when printed out with no decimal places becomes 60, which while
	// technically correct, looks wrong.
	// TODO: a better way of doing this? Just round it as an int?
	if (seconds >= 59.5)
	{
		minutes += 1.0f;
		seconds = 0;
	}

	if (minutes < 60)
	{
		sprintf(szTemp, "%02d:%02.f m", minutes, seconds);
		return std::string(szTemp);
	}

	unsigned int hours = (unsigned int)(minutes / 60.0);
	minutes -= hours * 60;

	sprintf(szTemp, "%d:%02d:%02.f h", hours, minutes, seconds);
	return std::string(szTemp);
}

std::string formatTimePeriod(uint64_t time)
{
	double seconds = (double)time / 1000000.0;

	return formatTimePeriodSeconds(seconds);
}

std::string tabulateColumnStrings(const std::vector<std::string>& column0, const std::vector<std::string>& column1)
{
	// the assumption here is the second column is numeric units, so should be right-aligned, such that it's easy
	// to compare numbers

	if (column0.size() != column1.size())
		return "";

	size_t maxLeftWidth = 0;

	std::vector<std::string>::const_iterator itCol0 = column0.begin();
	for (; itCol0 != column0.end(); ++itCol0)
	{
		const std::string& stringItem = *itCol0;
		maxLeftWidth = std::max(stringItem.size(), maxLeftWidth);
	}

	size_t maxRightWidth = 0;
	std::vector<std::string>::const_iterator itCol1 = column1.begin();
	for (; itCol1 != column1.end(); ++itCol1)
	{
		const std::string& stringItem = *itCol1;
		maxRightWidth = std::max(stringItem.size(), maxRightWidth);
	}

	size_t fullWidth = maxLeftWidth + maxRightWidth + 3;

	std::string finalResult;

	char szTemp[2048];

	for (unsigned int i = 0; i < column0.size(); i++)
	{
		const std::string& leftItem = column0[i];
		const std::string& rightItem = column1[i];

		unsigned int paddingSize = (unsigned int)(fullWidth - leftItem.size() - rightItem.size());

		sprintf(szTemp, "%s%*s %s\n", leftItem.c_str(), paddingSize, "", rightItem.c_str());

		finalResult += szTemp;
	}

	return finalResult;
}

// TODO: lots of duplicate code here from above...
std::string tabulateColumnStrings(const std::vector<std::string>& column0, const std::vector<std::string>& column1, const std::vector<std::string>& column2)
{
	// the assumption here is the second and third columns are numeric units, so should be right-aligned, such that it's easy
	// to compare numbers

	if (column0.size() != column1.size() || column1.size() != column2.size())
		return "";

	size_t maxLeftWidth = 0;

	std::vector<std::string>::const_iterator itCol0 = column0.begin();
	for (; itCol0 != column0.end(); ++itCol0)
	{
		const std::string& stringItem = *itCol0;
		maxLeftWidth = std::max(stringItem.size(), maxLeftWidth);
	}

	size_t maxCentreWidth = 0;
	std::vector<std::string>::const_iterator itCol1 = column1.begin();
	for (; itCol1 != column1.end(); ++itCol1)
	{
		const std::string& stringItem = *itCol1;
		maxCentreWidth = std::max(stringItem.size(), maxCentreWidth);
	}

	size_t maxRightWidth = 0;
	std::vector<std::string>::const_iterator itCol2 = column2.begin();
	for (; itCol2 != column2.end(); ++itCol2)
	{
		const std::string& stringItem = *itCol2;
		maxRightWidth = std::max(stringItem.size(), maxRightWidth);
	}

	size_t centreWidth = maxLeftWidth + maxCentreWidth + 3;
	size_t fullWidth = centreWidth + maxRightWidth + 3;

	std::string finalResult;

	char szTemp[2048];

	for (unsigned int i = 0; i < column0.size(); i++)
	{
		const std::string& leftItem = column0[i];
		const std::string& centreItem = column1[i];
		const std::string& rightItem = column2[i];

		unsigned int firstPaddingSize = (unsigned int)(centreWidth - leftItem.size() - centreItem.size());
		unsigned int secondPaddingSize = (unsigned int)(fullWidth - centreWidth - rightItem.size());

		sprintf(szTemp, "%s%*s %s%*s %s\n", leftItem.c_str(), firstPaddingSize, "", centreItem.c_str(), secondPaddingSize, "", rightItem.c_str());

		finalResult += szTemp;
	}

	return finalResult;
}

void TextOutputColumnifier::addStringsCustom(const std::string& string0, const char* format, ...)
{
	char szTemp[2048];
	va_list argPtr;
	va_start(argPtr, format);
	vsprintf(szTemp, format, argPtr);
	va_end(argPtr);

	std::string customString(szTemp);

	m_column0.push_back(string0);
	m_column1.push_back(customString);
}

void TextOutputColumnifier::addStringsCustom2(const std::string& string0, const std::string& string1, const char* format, ...)
{
	char szTemp[2048];
	va_list argPtr;
	va_start(argPtr, format);
	vsprintf(szTemp, format, argPtr);
	va_end(argPtr);

	std::string customString(szTemp);

	m_column0.push_back(string0);
	m_column1.push_back(string1);
	m_column2.push_back(customString);
}

void TextOutputColumnifier::addBlank(unsigned int numColumns)
{
	m_column0.push_back("");
	m_column1.push_back("");
	if (numColumns > 2)
	{
		m_column2.push_back("");
	}
}

const std::string& TextOutputColumnifier::getString()
{
	if (m_column2.empty())
	{
		m_finalString = tabulateColumnStrings(m_column0, m_column1);
	}
	else
	{
		m_finalString = tabulateColumnStrings(m_column0, m_column1, m_column2);
	}

	return m_finalString;
}

} // namespace Imagine
