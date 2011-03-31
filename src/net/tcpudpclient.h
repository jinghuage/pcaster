#ifndef _TCPUDP_CLIENT_H_
#define _TCPUDP_CLIENT_H_

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#include "messageSocket.h"


class TCPUDPCLIENT
{
private:
	int m_sockType;
	int m_serverPort;
	int m_clientPort;

	//int m_msgSocket;
	char m_serverAddress[32];
	struct sockaddr_in m_server;

public:
	TCPUDPCLIENT();
	~TCPUDPCLIENT();
	void Setup();
	void Setup(int, const char*, int);

	int Start(MessengeSocket** msg);
};

#endif

