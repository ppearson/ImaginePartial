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

#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include <vector>
#include <string>

namespace Imagine
{

class FileHelpers
{
public:
	FileHelpers();

	//! tries to find the requested file in the search directories provided if it does not exist in it's original location
	static bool findPathWithSearchPaths(const std::string& originalPath, const std::vector<std::string>& searchPaths, std::string& foundPath);

	//! searches the Texture Map search path (if enabled) for the file if it doesn't exist
	static std::string findTexturePath(const std::string& originalPath);

	static std::string getFileExtension(const std::string& path);
	static std::string getFileDirectory(const std::string& path);
	static std::string getFileName(const std::string& path);

	static std::string bakeFrameIntoFileSequencePath(const std::string& path, unsigned int frame);

	static bool isTokenFile(const std::string& filePath);
	static bool doesFileExist(const std::string& filePath);
	static bool doesDirectoryExist(const std::string& directoryPath);

	static std::string combinePaths(const std::string& path0, const std::string& path1);

	static bool getFilesInDirectory(const std::string& directoryPath, const std::string& extension, std::vector<std::string>& files);
};

} // namespace Imagine

#endif // FILE_HELPERS_H
