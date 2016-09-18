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

#include "socket_stream.h"

#include "utils/socket.h"

namespace Imagine
{

SocketStream::SocketStream() : Stream(), m_pSocket(NULL), m_ownSocket(true)
{
}

SocketStream::~SocketStream()
{
	// if we own the socket, close it
	if (m_ownSocket)
	{
		close();

		if (m_pSocket)
		{
			delete m_pSocket;
			m_pSocket = NULL;
		}
	}
}

// TODO: these are crap - need to serialise to memory first then send/receive in chunks...
bool SocketStream::read(void* ptr, size_t size)
{
	return m_pSocket->recv(ptr, size);
}

// TODO: these are crap - need to serialise to memory first then send/receive in chunks...
bool SocketStream::write(const void* ptr, size_t size)
{
	return m_pSocket->send(ptr, size);
}

bool SocketStream::canRead()
{
	return m_pSocket && m_pSocket->isValid();
}

bool SocketStream::canWrite()
{
	return m_pSocket && m_pSocket->isValid();
}

bool SocketStream::connect(const std::string& host, unsigned int port)
{
	if (!m_ownSocket)
		return false;

	m_ownSocket = true;

	if (!m_pSocket)
		m_pSocket = new Socket();

	return m_pSocket->connect(host, (int)port);
}

void SocketStream::attach(Socket* socket)
{
	m_ownSocket = false;
	m_pSocket = socket;
}

void SocketStream::close()
{
	if (m_pSocket)
		m_pSocket->close();
}

} // namespace Imagine
