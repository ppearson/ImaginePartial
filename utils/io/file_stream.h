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

#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include "stream.h"

#include <stdio.h>

namespace Imagine
{

class FileStream : public Stream
{
public:

	enum OpenMode
	{
		eRead,
		eWrite
	};

	FileStream();
	FileStream(const std::string& path, OpenMode mode);
	virtual ~FileStream();

	virtual bool read(void* ptr, size_t size);
	virtual bool write(const void* ptr, size_t size);

	virtual bool canRead();
	virtual bool canWrite();

	bool open(const std::string& filePath, OpenMode mode);
	void close();

protected:
	FILE*			m_pFile;
	std::string		m_filePath;
};

} // namespace Imagine

#endif // FILE_STREAM_H
