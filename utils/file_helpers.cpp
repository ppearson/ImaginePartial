/*
 Imagine
 Copyright 2012-2015 Peter Pearson.

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

#include "file_helpers.h"

#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "global_context.h"

#ifndef _MSC_VER
#define DIR_SEP '/'
#else
#define DIR_SEP '\\'
#endif

FileHelpers::FileHelpers()
{
}

bool FileHelpers::findPathWithSearchPaths(const std::string& originalPath, const std::vector<std::string>& searchPaths, std::string& foundPath)
{
	if (FileHelpers::doesFileExist(originalPath))
	{
		foundPath = originalPath;
		return true;
	}

	// otherwise, we need to look for the file

	if (searchPaths.empty())
		return false;

	std::string fileName = getFileName(originalPath);
	if (fileName.empty())
		return false;

	std::vector<std::string>::const_iterator it = searchPaths.begin();
	for (; it != searchPaths.end(); ++it)
	{
		std::string searchPathDir = *it;
		// make sure it's got a separator on the end
		if (searchPathDir[searchPathDir.length() - 1] != DIR_SEP)
		{
			searchPathDir += DIR_SEP;
		}

		std::string checkPath = searchPathDir + fileName;
		if (FileHelpers::doesFileExist(checkPath))
		{
			foundPath = checkPath;
			return true;
		}
	}

	return false;
}

std::string FileHelpers::findTexturePath(const std::string& originalPath)
{
	if (!GlobalContext::instance().searchForTextures())
		return originalPath;

	std::string finalPath;
	if (findPathWithSearchPaths(originalPath, GlobalContext::instance().getTextureSearchPaths(), finalPath))
		return finalPath;

	return originalPath;
}

std::string FileHelpers::getFileExtension(const std::string& path)
{
	std::string extension;
	size_t dotPos = path.find_last_of('.');
	extension = path.substr(dotPos + 1);

	std::transform(extension.begin(), extension.end(),	extension.begin(), tolower);

	return extension;
}

std::string FileHelpers::getFileDirectory(const std::string& path)
{
	char seperator = DIR_SEP;

	std::string directory;
	size_t slashPos = path.find_last_of(seperator, path.length());
	if (slashPos != std::string::npos)
		directory = path.substr(0, slashPos + 1);

	return directory;
}

std::string FileHelpers::getFileName(const std::string& path)
{
	char seperator = DIR_SEP;

	std::string directory;
	size_t slashPos = path.find_last_of(seperator, path.length());
	if (slashPos != std::string::npos)
		directory = path.substr(slashPos + 1);

	return directory;
}

std::string FileHelpers::bakeFrameIntoFileSequencePath(const std::string& path, unsigned int frame)
{
	size_t sequenceCharStart = path.find_first_of("#");
	if (sequenceCharStart == std::string::npos)
		return path;

	size_t sequenceCharEnd = path.find_first_not_of("#", sequenceCharStart);
	unsigned int sequenceCharLength = sequenceCharEnd - sequenceCharStart;

	char szFrameFormatter[16];
	memset(szFrameFormatter, 0, 16);
	sprintf(szFrameFormatter, "%%0%dd", sequenceCharLength);

	// generate frame sequence filename
	char szFrame[16];
	memset(szFrame, 0, 16);
	sprintf(szFrame, szFrameFormatter, (unsigned int)frame);

	std::string sFrame(szFrame);

	std::string sequenceFramePath = path;
	sequenceFramePath.replace(sequenceCharStart, sequenceCharLength, sFrame);

	return sequenceFramePath;
}

bool FileHelpers::isTokenFile(const std::string& filePath)
{
	if (filePath.find("<udim") != std::string::npos)
		return true;

	return false;
}

bool FileHelpers::doesFileExist(const std::string& filePath)
{
	FILE* pFile = fopen(filePath.c_str(), "r");
	if (!pFile)
		return false;

	fclose(pFile);
	return true;
}

bool FileHelpers::doesDirectoryExist(const std::string& directoryPath)
{
	struct stat sb;

	if (stat(directoryPath.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
		return true;

	return false;
}
