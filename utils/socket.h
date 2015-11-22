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

#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <string>

#ifdef _MSC_VER
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
#ifdef __SUNPRO_CC
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
#else
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <string.h>
#endif
#endif

#ifndef NI_MAXHOST
	#define NI_MAXHOST  1025
	#define NI_MAXSERV    32
	#ifndef NI_NUMERICHOST
	#define NI_NUMERICHOST  2
	#endif
	#ifndef NI_NUMERICSERV
	#define NI_NUMERICSERV  8
	#endif
	#define socklen_t int
#endif

const int MAX_SEND_LENGTH = 1024;
const int MAX_RECV_LENGTH = 4096;

class Socket
{
public:
	Socket();
	Socket(int port);
	Socket(const std::string& host, int port);

	virtual ~Socket();

	bool create();
	bool bind(int port);
	bool listen(int connections) const;
	bool accept(Socket& sock) const;
	bool accept(Socket* sock) const;

	bool connect();
	bool connect(const std::string& host, int port);
	void close();

	bool send(const std::string& data) const;
	int recv(std::string &data) const;

	bool send(const void* ptr, size_t size);
	bool recv(void* ptr, size_t size);

	//! returns actual size received
	ssize_t receiveChunk(void* ptr, size_t targetSize);

	ssize_t sendChunk(void* ptr, size_t targetSize);

	bool isValid() const { return m_sock != -1; }

	std::string getClientHost();
	std::string getClientAddress();
	int getClientPort() const { return m_clientPort; }

	static bool initWinsocks();
	static bool cleanupWinsocks();

protected:
	sockaddr_in		m_addr;
	int				m_sock;

	int				m_port;
	std::string		m_host;

	std::string		m_clientHost;
	std::string		m_clientAddress;
	int				m_clientPort;
};

#endif
