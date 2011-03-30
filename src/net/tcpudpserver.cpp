
/******************************************************************************\
 * TCPUDP Server combination
\******************************************************************************/
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>

#include "tcpudpserver.h"
#include "vsocket.h"

#define DEBUG 0
#define DATAMTU 1460

TCPUDPSERVER::TCPUDPSERVER()
{
}

TCPUDPSERVER::~TCPUDPSERVER()
{
}

void TCPUDPSERVER::Setup(int socket_type, const char* serverAdd, int port, int cnum)
{
    m_sockType = socket_type;
    if(serverAdd == 0) m_serverAddress = 0;
    else
    {
        m_serverAddress = (char*)malloc(32);
        strcpy(m_serverAddress, serverAdd);
    }
    m_sockPort = port;
	
    if(socket_type == SOCK_STREAM)
	fprintf(stderr, "setup tcp server at port %d for slaves\n", port);
    else if(socket_type == SOCK_DGRAM)
	fprintf(stderr, "setup udp server at port %d for slaves\n", port);

    assert(cnum >= 1);
    m_msgSocketNum = cnum;
    //m_msgSockets = new int[m_msgSocketNum];
    fprintf(stderr, "ready to accept %d connections\n", m_msgSocketNum);
}


void TCPUDPSERVER::Set_mcast_join()
{
    struct ip_mreq imr;
	
    imr.imr_interface.s_addr = m_server.sin_addr.s_addr;
    imr.imr_multiaddr.s_addr = inet_addr("224.0.1.1");
	
    if(setsockopt(m_listenSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(struct ip_mreq)) == -1)
    {
        printf("error, can't join group\n");
    }
    else
    {
        printf("group joined\n");
    }
}


int TCPUDPSERVER::Start(MessengeSocket** msg)
{

    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if (m_sockPort == 0)
    {
        fprintf(stderr,"socket port is 0!\n");
    }
	
    memset(&m_server,0,sizeof(m_server));
	
    if(m_serverAddress == NULL)
    {
        m_server.sin_addr.s_addr = htonl(INADDR_ANY);
        m_server.sin_family = AF_INET;
    }
    else if (isalpha(m_serverAddress[0])) 
    {   
        /* server address is a name */
        struct hostent *hp;
        hp = gethostbyname(m_serverAddress);
        if (hp == NULL )
        {
            printf("can't resolve address!\n");
            exit(1);
        }

        memcpy(&(m_server.sin_addr),hp->h_addr,hp->h_length);
        m_server.sin_family = hp->h_addrtype;
    }
    else 
    { 
        /* Convert nnn.nnn address to a usable one */
        inet_aton(m_serverAddress, &(m_server.sin_addr));
        m_server.sin_family = AF_INET;
    }

    /* 
     * Port MUST be in Network Byte Order
     */
    m_server.sin_port = htons(m_sockPort);

    m_listenSocket = socket(AF_INET, m_sockType,0); // TCP socket
	
    if (m_listenSocket == -1)
    {
        printf("socket() failed!\n");
        exit(1);
    }

    set_reuseaddr(m_listenSocket);

    if (bind(m_listenSocket,(struct sockaddr*)&m_server,sizeof(m_server) ) == -1) 
    {
        printf("bind() failed!\n");
        //return -1;
        exit(1);
    }

    //
    // So far, everything we did was applicable to TCP as well as UDP.
    // However, there are certain steps that do not work when the server is
    // using UDP.
    //

    // We only listen() and accept() on a TCP socket.

    if (m_sockType == SOCK_STREAM)
    {
        if (listen(m_listenSocket,5) == -1)
        {
            printf("listen() failed!\n");
            //return -1;
            exit(1);
        }
	
        printf("TCP server 'Listening' on port %d\n",m_sockPort);

        socklen_t clientlen =(socklen_t) sizeof(m_client);

	for(int i=0; i<m_msgSocketNum; i++)
        {
            int sockfd = accept(m_listenSocket,(struct sockaddr*)&m_client, &clientlen);
            if (sockfd == -1)
            {
                printf("accept() error!\n");
                exit(1);
            }
            char client_ip[80];
            strcpy(client_ip, inet_ntoa(m_client.sin_addr));
            //fprintf(stderr, "client ip %s, port %d\n", client_ip, htons(m_client.sin_port));
				
            printf("tcpudpserver: accept connection at socket %d\n", sockfd);
            msg[i] = new MessengeSocket(SOCK_STREAM, m_server, sockfd);
        }	
        close(m_listenSocket);
    }
    else if (m_sockType == SOCK_DGRAM)
    {
        msg[0] = new MessengeSocket(SOCK_DGRAM, m_server, m_listenSocket);
    }


    return 0;
}


