/******************************************************************************\
* tcpudpclient.cpp - Simple TCP/UDP client using Winsock 1.1
\******************************************************************************/
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "tcpudpclient.h"
#include "vsocket.h"

#define DEFAULT_PROTO SOCK_STREAM // TCP
#define DATAMTU 1460

TCPUDPCLIENT::TCPUDPCLIENT()
{
}

TCPUDPCLIENT::~TCPUDPCLIENT()
{
}

void TCPUDPCLIENT::Setup()
{
    m_sockType = DEFAULT_PROTO;
    m_serverPort = 5001;
    strcpy(m_serverAddress,"localhost");
}

void TCPUDPCLIENT::Setup(int sock_type, const char* serverAdd, int serverPort)
{
    m_sockType = sock_type;
    m_serverPort = serverPort;
    strcpy(m_serverAddress, serverAdd);
}



int TCPUDPCLIENT::Start(MessengeSocket** msg)
{

    memset(&m_server,0,sizeof(m_server));

    if (isalpha(m_serverAddress[0])) 
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
    m_server.sin_port = htons(m_serverPort);

    int S = socket(AF_INET,m_sockType,0); /* Open a socket */
    if (S < 0 ) 
    {
        printf("error opening socket!\n");
        return -1;
    }
	                 

    while(connect(S,(struct sockaddr*)&m_server,sizeof(m_server)) == -1) //SOCKET_ERROR
    {
        fprintf(stderr, "tcpudpclient connect() failed, try again...\n");
        sleep(1);
    }

    fprintf(stderr, "tcpudpclient socket %d connected to: %s\n", 
            S, m_serverAddress);


    //create the messenge socket for this client connection
    msg[0] = new MessengeSocket(m_sockType, m_server, S);
    return 0;
}


