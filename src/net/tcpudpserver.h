#ifndef _TCPUDP_SERVER_H_
#define _TCPUDP_SERVER_H_

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


class TCPUDPSERVER
{
private:
	int m_sockType;
	int m_sockPort;
	int m_listenSocket;

        int m_msgSocketNum;
	int* m_msgSockets;
	char* m_serverAddress;
	struct sockaddr_in m_server, m_client;


public:
	TCPUDPSERVER();
	~TCPUDPSERVER();
	void Setup(int, const char*, int, int cnum = 1);
	void Set_mcast_join();
	
	int Start(MessengeSocket** msg);

};

#endif
