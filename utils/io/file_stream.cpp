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

#include "file_stream.h"

namespace Imagine
{

FileStream::FileStream() : Stream(), m_pFile(NULL)
{
}

FileStream::FileStream(const std::string& path, OpenMode mode) : Stream(), m_pFile(NULL), m_filePath(path)
{
	open(path, mode);
}

FileStream::~FileStream()
{
	close();
}

bool FileStream::read(void* ptr, size_t size)
{
	fread(ptr, 1, size, m_pFile);
	return true;
}

bool FileStream::write(const void* ptr, size_t size)
{
	fwrite(ptr, 1, size, m_pFile);
	return true;
}

bool FileStream::canRead()
{
	return m_pFile != NULL;
}

bool FileStream::canWrite()
{
	return m_pFile != NULL;
}

bool FileStream::open(const std::string& filePath, OpenMode mode)
{
	if (filePath.empty())
		return false;

	std::string strOpenMode;

	switch (mode)
	{
		case eRead:
		default:
			strOpenMode = "rb";
			break;
		case eWrite:
			strOpenMode = "wb+";
			break;
	}

	m_pFile = fopen(filePath.c_str(), strOpenMode.c_str());

	return m_pFile != NULL;
}

void FileStream::close()
{
	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

} // namespace Imagine
