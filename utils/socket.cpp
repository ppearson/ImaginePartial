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

#include "socket.h"
#include <errno.h>
#include <unistd.h>

const int MAX_SEND_LENGTH = 1024;
const int MAX_RECV_LENGTH = 4096;

Socket::Socket() : m_sock(-1), m_port(-1)
{
	memset(&m_addr, 0, sizeof(m_addr));
}

Socket::Socket(int port) : m_sock(-1), m_port(port)
{
	memset(&m_addr, 0, sizeof(m_addr));
	create();
}

Socket::Socket(const std::string& host, int port) : m_sock(-1), m_port(port), m_host(host)
{
	memset(&m_addr, 0, sizeof(m_addr));
	create();
}

Socket::~Socket()
{
	close();
}

bool Socket::create()
{
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (!isValid())
		return false;

	int on = 1;
	if (::setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
		return false;

	return true;
}

bool Socket::bind(const int port)
{
	if (!isValid())
		return false;

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_addr.sin_port = htons(port);

	int ret = ::bind(m_sock, (struct sockaddr*)&m_addr, sizeof(m_addr));

	if (ret == -1)
		return false;

	return true;
}

bool Socket::listen(const int connections) const
{
	if (!isValid())
		return false;

	int ret = ::listen(m_sock, connections);

	if (ret == -1)
		return false;

	return true;
}

bool Socket::connect()
{
	if (!isValid())
		return false;

	m_addr.sin_family = AF_INET;
#ifndef __SUNPRO_CC
	m_addr.sin_addr.s_addr = inet_addr(m_host.c_str());
#endif
	m_addr.sin_port = htons(m_port);

#ifdef _MSC_VER
	PHOSTENT hostinfo;
	if ((hostinfo = gethostbyname(m_host.c_str())) != NULL)
	{
		memcpy(&(m_addr.sin_addr), hostinfo->h_addr, hostinfo->h_length);
	}
	else
		return false;
#else
#ifdef __SUNPRO_CC
	struct hostent* hp = gethostbyname(m_host.c_str());
	if (!hp)
		return false;
	m_addr.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;
#else
//	status = inet_pton(AF_INET, m_host.c_str(), &m_addr.sin_addr);
	struct hostent* hp = gethostbyname(m_host.c_str());
	if (!hp)
		return false;
	m_addr.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;
#endif
#endif

	int status  = ::connect(m_sock, (sockaddr*)&m_addr, sizeof(m_addr));

	if (status != 0)
		return false;

	return true;
}

bool Socket::connect(const std::string& host, const int port)
{
	m_host = host;
	m_port = port;

	return connect();
}

void Socket::close()
{
	if (isValid())
	{
#ifdef _MSC_VER
		::closesocket(m_sock);
#else
//		::shutdown(m_sock, SHUT_RDWR);
		::close(m_sock);
#endif
		m_sock = -1;
	}
}

bool Socket::accept(Socket& sock) const
{
	if (!isValid())
		return false;

	int addrSize = sizeof(m_addr);
	sock.m_sock = ::accept(m_sock, (sockaddr*)&m_addr, (socklen_t*)&addrSize);

	if (sock.m_sock <= 0)
		return false;

	return true;
}

bool Socket::accept(Socket* sock) const
{
	if (!isValid())
		return false;

	int addrSize = sizeof(m_addr);
	sock->m_sock = ::accept(m_sock, (sockaddr*)&m_addr, (socklen_t*)&addrSize);

	if (sock->m_sock <= 0)
		return false;

	return true;
}

bool Socket::send(const std::string& data) const
{
	if (!isValid())
		return false;

	int bytesSent = ::send(m_sock, data.c_str(), data.size(), 0);

	if (bytesSent == -1)
		return false;

	return true;
}

int Socket::recv(std::string &data) const
{
	if (!isValid())
		return -1;

	int length = 0;

	char buffer[MAX_RECV_LENGTH + 1];

	int ret = 0;

	do
	{
		memset(buffer, 0, sizeof(buffer));

		ret = ::recv(m_sock, buffer, MAX_RECV_LENGTH, 0);

		if (ret > 0)
		{
			buffer[ret] = '\0';

			data.append(buffer);
			length += ret;
		}
	}
	while (ret > MAX_RECV_LENGTH);

	return length;
}

// these are currently only designed to handle small data sizes
bool Socket::send(const void* ptr, size_t size)
{
	size_t sizeSent = ::send(m_sock, ptr, size, 0);
	return sizeSent == size;
}

bool Socket::recv(void* ptr, size_t size)
{
	size_t sizeReceived = ::recv(m_sock, ptr, size, 0);
	return sizeReceived == size;
}

ssize_t Socket::receiveChunk(void* ptr, size_t targetSize)
{
	return ::recv(m_sock, ptr, targetSize, 0);
}

ssize_t Socket::sendChunk(void* ptr, size_t targetSize)
{
	return ::send(m_sock, ptr, targetSize, 0);
}

std::string Socket::getClientHost()
{
#ifndef _MSC_VER
	if (m_clientHost.empty())
	{
		if (!isValid())
			return "";
#ifndef _MSC_VER
		struct sockaddr_storage addr;
#else
		struct sockaddr_in addr;
#endif
		int addrLen = sizeof(addr);

		int ret = getpeername(m_sock, (sockaddr*)&addr, (socklen_t*)&addrLen);

		if (ret != 0)
			return "";

		char clientHost[NI_MAXHOST];
		char clientService[NI_MAXSERV];

		ret = getnameinfo((sockaddr*)&addr, addrLen, clientHost, sizeof(clientHost), clientService, sizeof(clientService), 0);

		m_clientHost = clientHost;
	}
#endif

	return m_clientHost;
}

std::string Socket::getClientAddress()
{
	if (m_clientAddress.empty())
	{
		if (!isValid())
			return "";

#ifndef _MSC_VER
		struct sockaddr_storage addr;
#else
		struct sockaddr_in addr;
#endif
		int addrLen = sizeof(addr);

		int ret = getpeername(m_sock, (sockaddr*)&addr, (socklen_t*)&addrLen);

		if (ret != 0)
			return "";

		char clientAddress[NI_MAXHOST];
		char clientService[NI_MAXSERV];

		memset(clientAddress, 0, NI_MAXHOST);
		memset(clientService, 0, NI_MAXSERV);

#ifndef _MSC_VER
		ret = getnameinfo((sockaddr*)&addr, addrLen, clientAddress, sizeof(clientAddress), clientService, sizeof(clientService),
						  NI_NUMERICHOST|NI_NUMERICSERV);

#else

		hostent* hostinfo;
		if ((hostinfo = gethostbyaddr((char*) &addr.sin_addr, 4, AF_INET)) != NULL)
		{
			char* pAddr = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
			sprintf(clientAddress, "%s", pAddr);
		}
#endif

		m_clientAddress = clientAddress;
//		m_clientPort = atoi(clientService);
	}

	return m_clientAddress;
}

bool Socket::initWinsocks()
{
#ifdef _MSC_VER
	WSADATA wsaData;

	int ret = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (ret != 0)
	{
		return false;
	}
#endif
	return true;
}

bool Socket::cleanupWinsocks()
{
#ifdef _MSC_VER
	WSACleanup();
#endif
	return true;
}
