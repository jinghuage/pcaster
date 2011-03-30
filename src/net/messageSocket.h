#ifndef _MESSENGE_SOCKET_H_
#define _MESSENGE_SOCKET_H_

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



class MessengeSocket
{
public:
    MessengeSocket(int, struct sockaddr_in&, int);
	~MessengeSocket();

        int get_socket(){ return m_sockId; }

	int Socket_readable(fd_set*);

	int Send(char* Buffer, int len, int frameid=0);
	int Receive(char* Buffer, int len, int frameid=0);

	int SimpleSend(char*, int);
	int SimpleReceive(char*, int);
	
	int Readv(int fd, char*, int, int);
        int Writev(int fd, char*, int, int);

	void Close();

private:
	int m_sockType;
	int m_sockId;
        struct sockaddr_in m_server;
};

#endif

