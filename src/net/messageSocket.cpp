/******************************************************************************\
* tcpudpclient.cpp - Simple TCP/UDP client using Winsock 1.1
\******************************************************************************/
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "messageSocket.h"
#include "vsocket.h"

#define DEFAULT_PROTO SOCK_STREAM // TCP
#define DATAMTU 1460

MessengeSocket::MessengeSocket(int type, struct sockaddr_in& server, int s):
    m_sockType(type),
    m_sockId(s),
    m_server(server)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    //set_nonblock(m_sockId);
    set_sendbufsize(m_sockId, 4000000);
    set_recvbufsize(m_sockId, 4000000);
}



MessengeSocket::~MessengeSocket()
{
}


//call after Select_read
int MessengeSocket::Socket_readable(fd_set* rset)
{
    return FD_ISSET(m_sockId, rset);
}


int MessengeSocket::SimpleSend(char* Buffer, int len)
{
    int retval;

    if(m_sockType == SOCK_STREAM)
    {
        retval = send(m_sockId,Buffer,len, 0);
    }
    else
    {
        retval = sendto(m_sockId, Buffer, len, 0, 
                        (struct sockaddr *)&m_server, sizeof(m_server));        
    }

    if (retval == -1) 
    {
        //SOCKET_ERROR
        printf("send to socket failed!\n");
        //return -1;
        close(m_sockId);
        exit(1);
    }

    printf("Sent %d Data to server\n", retval);
    return retval;
}

int MessengeSocket::SimpleReceive(char*Buffer, int len)
{
    int retval=0;
    int total = 0;

    if(m_sockType == SOCK_STREAM)
    {
        while(len>0)
        {
            retval = recv(m_sockId,Buffer+total,len,0 );
            if (retval == -1) 
            {
                //SOCKET_ERROR
                printf("recv from socket failed!\n");
                close(m_sockId);
                //return -1;
                exit(1);
            }
            if (retval == 0) 
            {
                printf("socket recv %d bytes\n", retval);
                //close(m_sockId);
                return -1;
            }
            printf("MessengeSocket::Receive %d bytes of total %d\n", retval, len);
            len -= retval;
            total += retval;
        }
    }
    else
    {
        socklen_t serverlen = (socklen_t) sizeof(m_server);
        retval = recvfrom(m_sockId, Buffer, len, 0, 
                          (struct sockaddr *)&m_server, &serverlen);
        if (retval == -1) 
        {
            //SOCKET_ERROR
            printf("recv() failed!\n");
            exit(1);
            //return -1;
        }
    }
    //printf("Received total %d bytes data from server\n",total);
    return 0;
}



int MessengeSocket::Writev(int fd, char* buf, int len, int frameid)
{

    ssize_t bytes_written;
    int i, msgcnt;

    msgcnt = len/DATAMTU;
    if(len % DATAMTU) msgcnt++;

    struct iovec iov[2]; 
    int l;
    int header[2];
    header[1] = frameid;
    for(i=0; i<msgcnt; i++)
    {
        //printf("udp: packet %d\n", i);

        header[0] = i;
        if(i == msgcnt-1) l = len % DATAMTU;
        else l = DATAMTU;

        iov[0].iov_base = header;
        iov[0].iov_len = 8;
        iov[1].iov_base = buf + i*DATAMTU;
        iov[1].iov_len = l;
        
        bytes_written = writev(fd, iov, 2);
        if(bytes_written == -1) {printf("writev error\n");  return -1; }
    }

    return i;
}


int MessengeSocket::Send(char* Buffer, int len, int frameid)
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int retval;

    if(m_sockType == SOCK_STREAM)
    {
        retval = send(m_sockId,Buffer,len,0);
    }
    else
    {
        //retval = sendto(m_sockId, Buffer, len, 0, (struct sockaddr *)&m_server, sizeof(m_server));
        retval = Writev(m_sockId, Buffer, len, frameid);
    }

    if (retval == -1) 
    {
        //SOCKET_ERROR
        fprintf(stderr, "send to socket failed!\n");
        //return -1;
        close(m_sockId);
        exit(1);
    }

    //fprintf(stderr, "Sent %d Data to server\n", retval);
    return retval;
}


int MessengeSocket::Readv(int fd, char* buf, int len, int frameid)
{
    int ret = 0;
    ssize_t bytes_read;
    int i, msgcnt;

    msgcnt = len/DATAMTU;
    if(len % DATAMTU) msgcnt++;

    struct iovec iov[2]; 
    int fid;
    int *header = new int[2];
    char* b = new char[DATAMTU];

    iov[0].iov_base = header;
    iov[0].iov_len = 8;
    iov[1].iov_base = b;
    iov[1].iov_len = DATAMTU;

    //struct timeval tv; 
    bool getfirstpacket = false;

    while(1)
    {
//         if(getfirstpacket)
//         {
//             FD_SET(fd, &m_rset);
//             tv.tv_sec = 0; 
//             tv.tv_usec = 20000; 
//             int n = select(m_maxfdpl+1, &m_rset, NULL, NULL, &tv);
//             if(n==0) { 
//                 printf("timeout\n"); 
//                 return ret; 
//             }
//             else if(n == -1) { printf("select error\n"); return -1; }
//         }

        bytes_read = readv(fd, iov, 2);
        if(bytes_read == -1) { printf("readv error\n"); return -1; }

        i = header[0];
        fid = header[1];
        printf("get udp packet (%d, %d)\n", i, fid);
        if(frameid != fid) 
        { 
            //printf("wrong frame\n"); 
            continue; 
        }
 
        memcpy(buf+i*DATAMTU, b, bytes_read-8); 
        getfirstpacket = true;

        ret++;
    }

    return ret;
}


int MessengeSocket::Receive(char* Buffer, int len, int frameid)
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int retval=0;
    int total = 0;

    if(m_sockType == SOCK_STREAM)
    {
        while(len>0)
        {
            retval = recv(m_sockId,Buffer+total,len,0 );
            if (retval == -1) 
            {
                //SOCKET_ERROR
                fprintf(stderr, "recv from socket failed!\n");
                close(m_sockId);
                //return -1;
                exit(1);
            }
            if (retval == 0) 
            {
                fprintf(stderr, "socket recv %d bytes\n", retval);
                close(m_sockId);
                exit(1);
                //return -1;
            }
            //fprintf(stderr, "Receive %d bytes of total %d\n", retval, len);
            len -= retval;
            total += retval;
        }
    }
    else
    {
        //socklen_t serverlen = (socklen_t) sizeof(m_server);
        //retval = recvfrom(m_sockId, Buffer, len, 0, (struct sockaddr *)&m_server, &serverlen);
        retval = Readv(m_sockId, Buffer, len, frameid);
        if (retval == -1) 
        {
            //SOCKET_ERROR
            printf("recv() failed!\n");
            exit(1);
            //return -1;
        }
    }
    //fprintf(stderr, "Received total %d bytes data from server\n",total);
    return 0;
}


void MessengeSocket::Close()
{    
    close(m_sockId);
}

