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

#ifndef SOCKET_STREAM_H
#define SOCKET_STREAM_H

#include "stream.h"

namespace Imagine
{

class Socket;

class SocketStream : public Stream
{
public:
	SocketStream();
	virtual ~SocketStream();

	virtual bool read(void* ptr, size_t size);
	virtual bool write(const void* ptr, size_t size);

	virtual bool canRead();
	virtual bool canWrite();

	bool connect(const std::string& host, unsigned int port);
	void attach(Socket* socket);
	void close();

protected:
	Socket*		m_pSocket;
	bool		m_ownSocket; // did we create the socket or not
};

} // namespace Imagine

#endif // SOCKET_STREAM_H
