//vsocket.cpp, set socket options

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "vsocket.h"

void set_nonblock(int socket_id)
{
    int flags;
	
    flags = fcntl(socket_id, F_GETFL);
    flags |= O_NONBLOCK;
    if(fcntl(socket_id, F_SETFL, flags) < 0)
    {
        fprintf(stderr, "fcntl error executing nonblock.\n");
    }
}

void set_recv_timeout(int socket_id, int ns, int nus)
{
    struct timeval tv_s;
    socklen_t len_tv = sizeof(struct timeval);


    struct timeval tv;
    tv.tv_sec = ns;
    tv.tv_usec = nus;
	
    if( getsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, &tv_s, &len_tv)== -1) 
    {
        printf("get socket option SO_RCVTIMEO failed\n");
    }
    else
    {
        fprintf(stderr, "default option is: %d, %d\n", 
                tv_s.tv_sec, tv_s.tv_usec);
        setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, &tv, len_tv);
    }	
}


void set_reuseaddr(int socket_id)
{
    int sock_reuseAddr;
    socklen_t len_reuseAddr = sizeof(int);
    if( getsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &sock_reuseAddr, &len_reuseAddr)== -1) 
    {
        printf("get socket option failed\n");
    }
    else
    {
        //printf("default option is: sock_reuseAddr=%d\n", sock_reuseAddr);
        if(sock_reuseAddr==0) sock_reuseAddr = 1;
        setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &sock_reuseAddr, len_reuseAddr);
    }	
}

void set_sendbufsize(int socket_id, int size)
{
    int sock_sndbuf;
    socklen_t len_sndbuf = sizeof(int);
	
    if( getsockopt(socket_id, SOL_SOCKET, SO_SNDBUF, &sock_sndbuf, &len_sndbuf)== -1) 
    {
        printf("get socket option failed\n");
    }
    else
    {
        //printf("default option is: sock_sndbuf=%d\n", sock_sndbuf);
        setsockopt(socket_id, SOL_SOCKET, SO_SNDBUF, &size, len_sndbuf);
        //fprintf(stderr, "set send buffer size %d\n", size);
    }	
}

void set_recvbufsize(int socket_id, int size)
{
    int sock_rcvbuf;
    socklen_t len_rcvbuf = sizeof(int);
	
    if( getsockopt(socket_id, SOL_SOCKET, SO_RCVBUF, &sock_rcvbuf, &len_rcvbuf)== -1) 
    {
        printf("get socket option failed\n");
    }
    else
    {
        //printf("default option is: sock_rcvbuf=%d\n", sock_rcvbuf);
        setsockopt(socket_id, SOL_SOCKET, SO_RCVBUF, &size, len_rcvbuf);
        //fprintf(stderr, "set recv buffer size %d\n", size);
    }	
}
