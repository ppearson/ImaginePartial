/*
 Imagine
 Copyright 2012-2016 Peter Pearson.

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

#include <cstdio>
#include <sys/stat.h>
#include <cstring>
#include <dirent.h>
#include <unistd.h>

#include "global_context.h"

namespace Imagine
{

#ifndef _MSC_VER
static const char* kDirSep = "/";
#define kDirSepChar '/'
#else
#define kDirSepChar '\\'
static const char* kDirSep = "\\";
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
		const std::string& searchPathDir = *it;

		std::string checkPath = combinePaths(searchPathDir, fileName);
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

	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

	return extension;
}

std::string FileHelpers::getFileDirectory(const std::string& path)
{
	std::string directory;
	size_t slashPos = path.find_last_of(kDirSepChar, path.length());
	if (slashPos != std::string::npos)
		directory = path.substr(0, slashPos + 1);

	return directory;
}

std::string FileHelpers::getFileName(const std::string& path)
{
	std::string fileName;
	size_t slashPos = path.find_last_of(kDirSepChar, path.length());
	if (slashPos != std::string::npos)
		fileName = path.substr(slashPos + 1);
	else
		return path;

	return fileName;
}

std::string FileHelpers::getFileNameAllPlatforms(const std::string& path)
{
	std::string fileName;
	size_t slashPos = path.find_last_of(kDirSepChar, path.length());
	if (slashPos != std::string::npos)
	{
		fileName = path.substr(slashPos + 1);
	}
	else
	{
		char otherSep = (kDirSepChar == '/') ? '\\' : '/';
		slashPos = path.find_last_of(otherSep, path.length());
		if (slashPos != std::string::npos)
		{
			fileName = path.substr(slashPos + 1);
		}
		else
		{
			return path;
		}
	}

	return fileName;
}

std::string FileHelpers::bakeFrameIntoFileSequencePath(const std::string& path, unsigned int frame)
{
	size_t sequenceCharStart = path.find_first_of('#');
	if (sequenceCharStart == std::string::npos)
		return path;

	size_t sequenceCharEnd = path.find_first_not_of('#', sequenceCharStart);
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

std::string FileHelpers::combinePaths(const std::string& path0, const std::string& path1)
{
	std::string final = path0;

	if (strcmp(final.substr(final.size() - 1, 1).c_str(), kDirSep) != 0)
	{
		final += kDirSep;
	}

	final += path1;

	return final;
}

bool FileHelpers::getFilesInDirectory(const std::string& directoryPath, const std::string& extension, std::vector<std::string>& files)
{
	// Note: opendir() is used on purpose here, as scandir() and lsstat() don't reliably support S_ISLNK on symlinks over NFS,
	//       whereas opendir() allows this
	DIR* dir = opendir(directoryPath.c_str());
	if (!dir)
		return false;

	struct dirent* dirEnt = nullptr;
	char tempBuffer[4096];

	while ((dirEnt = readdir(dir)) != nullptr)
	{
		// ignore directories for the moment
		if (dirEnt->d_type == DT_DIR)
			continue;

		// cope with symlinks by working out what they point at
		if (dirEnt->d_type == DT_LNK)
		{
			std::string fullAbsolutePath = combinePaths(directoryPath, dirEnt->d_name);
			if (readlink(fullAbsolutePath.c_str(), tempBuffer, 4096) == -1)
			{
				// something went wrong, so ignore...
				continue;
			}
			else
			{
				// on the assumption that the target of the symlink is not another symlink (if so, this won't work over NFS)
				// check what type it is
				struct stat statState;
				int ret = lstat(tempBuffer, &statState);

				if (ret == -1 || S_ISDIR(statState.st_mode))
				{
					// ignore for the moment...
					continue;
				}
				else
				{
					// TODO: do we need to check this is a full absolute path?
					// TODO: do de-duplication on target, as opposed to original symlink in dir listing

					if (getFileExtension(dirEnt->d_name) == extension)
					{
						files.emplace_back(fullAbsolutePath);
					}
				}
			}
		}
		else
		{
			// it's hopefully a file
			std::string fullAbsolutePath = combinePaths(directoryPath, dirEnt->d_name);
			if (getFileExtension(dirEnt->d_name) == extension)
			{
				files.emplace_back(fullAbsolutePath);
			}
		}
	}

	closedir(dir);

	return !files.empty();
}

} // namespace Imagine
