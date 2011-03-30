/*****************************************************************************


    Copyright 2009,2010,2011 Jinghua Ge
    ------------------------------

    This file is part of Pcaster.

    Pcaster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Pcaster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Pcaster.  If not, see <http://www.gnu.org/licenses/>.


******************************************************************************/

#include <iostream>
#include <assert.h>

#include "socket_data_mover.h"


// A socket-based implemenation of moving data from point A to point B.
// used among two groups of senders/receivers


//-----------------------------------------------------------------------------
// Constructor / Destructor
//
//myrank: rank within my group (server group or client group)
//socket_num: connnections I establish, I send data to and recv data from
//example: multiple (M) server and multiple (N) clients. 
//server will have rank 0->M, socket_num as number of clients (N)
//client will have rank 0->N, socket_num is number of server (M). 
//-----------------------------------------------------------------------------                                  
Socket_Data_Mover::Socket_Data_Mover(int myrank,
                                     int mode,
                                     int socket_num,
                                     std::string& serverAddress,
                                     int port):
    m_rank(myrank),
    m_mode(mode),
    m_sockNum(socket_num),
    m_socketToRank(new int[socket_num]),
    m_msgSockets(new MessengeSocket*[socket_num])
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(m_mode == NET_SERVER) init_server(serverAddress, port);
    else if(m_mode == NET_CLIENT) init_client(serverAddress, port);   

    FD_ZERO(&m_rset);
    m_maxfdpl = 0;
}


Socket_Data_Mover::~Socket_Data_Mover()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    
    if(m_msgSockets)
    {
        for(int i=0; i<m_sockNum; i++) m_msgSockets[i]->Close();
        delete[] m_msgSockets;   m_msgSockets = 0;
        delete[] m_socketToRank; m_socketToRank = 0;
    }
}


//-----------------------------------------------------------------------------
//init the tcp server, listen and accept connections, establish messagesocket
//-----------------------------------------------------------------------------
void Socket_Data_Mover::init_server(std::string& serverAddress, int port)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    //server could have multiple client connections
    assert(m_sockNum >= 1);
    assert(serverAddress.compare("none"));

    TCPUDPSERVER S;

    std::cout << m_rank 
              << ": I am server: address is " << serverAddress 
              << ", expecting " << m_sockNum << " connections.\n";


    if(! serverAddress.compare("localhost"))
        S.Setup(SOCK_STREAM, "localhost", port, m_sockNum);
    else
        S.Setup(SOCK_STREAM, NULL, port, m_sockNum);

    int ret = S.Start(m_msgSockets);
    assert(ret == 0);

    //server get all clients' mpi rank
    for(int i=0; i<m_sockNum; i++)
    {
        int S = m_msgSockets[i]->get_socket();
        //std::cout << "socket: " << S << "\n";

        FD_SET(S, &m_rset);
        if(S > m_maxfdpl) m_maxfdpl = S;

        m_msgSockets[i]->Receive((char*)(m_socketToRank+i), sizeof(int));     
        fprintf(stderr, "client socket %d has rank %d\n", 
                i, m_socketToRank[i]);
    }

}



//-----------------------------------------------------------------------------
//init the tcp socket, connect to server and establish messagesocket
//-----------------------------------------------------------------------------
void Socket_Data_Mover::init_client(std::string& serverAddress, int port)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    //in this state, client only connect to a single server/viewer
    //this could change in later development
    assert(m_sockNum == 1);
    assert(serverAddress.compare("none"));

    TCPUDPCLIENT C;

    std::cout << m_rank 
              << ": I am client: server address is " << serverAddress 
              << ", initializing " << m_sockNum << " connections.\n";

    if( serverAddress.compare("none") && serverAddress.compare("inplace") )
        C.Setup(SOCK_STREAM, serverAddress.c_str(), port);


    int ret = C.Start(m_msgSockets);
    assert(ret == 0);

    int S = m_msgSockets[0]->get_socket();
    //std::cout << "socket: " << S << "\n";

    FD_SET(S, &m_rset);
    if(S > m_maxfdpl) m_maxfdpl = S;

    //Since client only connects to single server, so no need to distinguish
    m_socketToRank[0] = -1; 

    //client sent rank to server
    m_msgSockets[0]->Send((char*)&m_rank, sizeof(int));

}



//-----------------------------------------------------------------------------
//select
//-----------------------------------------------------------------------------
int Socket_Data_Mover::Select()
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 2000;

    FD_ZERO(&m_rset);
    int n = select(m_maxfdpl+1, &m_rset, NULL, NULL, &tv);
	
    return n;
}


//-----------------------------------------------------------------------------
//receive from socket id
//-----------------------------------------------------------------------------
void Socket_Data_Mover::recv_from( int id, void* buf, int size)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_msgSockets[id]->Receive((char*)buf, size);

#ifdef _DEBUG
    fprintf(stderr, "%d: Received %d bytes from socket %d\n", 
            m_rank, size, id); 
#endif

}



//-----------------------------------------------------------------------------
//send to socket id
//-----------------------------------------------------------------------------
void Socket_Data_Mover::send_to( int id, void* buf, int size)
{    
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_msgSockets[id]->Send((char*)buf, size);

#ifdef _DEBUG
    fprintf(stderr, "%d: Sent %d bytes to socket %d\n",
            m_rank, size, id);
#endif

}


//-----------------------------------------------------------------------------
//send to all sockets
//-----------------------------------------------------------------------------
void Socket_Data_Mover::send_to_all(void* netbuf,
                                    int* scounts,
                                    int* sdispls)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    for(int i=0; i<m_sockNum; i++)
    {
        int send_size = scounts[i];
        if(send_size > 0)
        {
            m_msgSockets[i]->Send((char*)netbuf + sdispls[i], 
                                  send_size);
#ifdef _DEBUG
            fprintf(stderr, "%d: Sent %d bytes to socket %d\n",
                    m_rank, send_size, i);
#endif
        }
    }
    fflush(stderr);
}



void Socket_Data_Mover::send_to_all(void* netbuf,
                                    int send_size)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    for(int i=0; i<m_sockNum; i++)
    {
        if(send_size > 0)
        {        
            m_msgSockets[i]->Send((char*)netbuf + i * send_size, 
                                  send_size);
#ifdef _DEBUG
            fprintf(stderr, "%d: Sent %d bytes to socket %d\n",
                    m_rank, send_size, i);
#endif
        }
    }
    fflush(stderr);
}


//-----------------------------------------------------------------------------
//recv from all sockets
//-----------------------------------------------------------------------------
void Socket_Data_Mover::recv_from_all(void* netbuf,    
                                      int* recvcounts,
                                      int* rdispls)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    for(int i=0; i<m_sockNum; i++)
    {
        if(recvcounts[i] > 0)
        {
            m_msgSockets[i]->Receive((char*)netbuf + rdispls[i], 
                                     recvcounts[i]);
#ifdef _DEBUG
            fprintf(stderr, "%d: Received %d bytes from socket %d\n", 
                    m_rank, recvcounts[i], i); 
#endif
        }
    }
}


void Socket_Data_Mover::recv_from_all(void* netbuf,         
                                      int recv_size)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    for(int i=0; i<m_sockNum; i++)
    {
        if(recv_size > 0)
        {
            m_msgSockets[i]->Receive((char*)netbuf + i * recv_size, 
                                     recv_size);
#ifdef _DEBUG
            fprintf(stderr, "%d: Received %d bytes from socket %d\n", 
                    m_rank, recv_size, i); 
#endif
        }
    }
}



//-----------------------------------------------------------------------------
//send of socket id
//-----------------------------------------------------------------------------
void Socket_Data_Mover::send_to( int id, std::vector<int>& olbuffer)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int size = olbuffer.size();

    fprintf(stderr, "%d: olBuffer size=%d\n", m_rank, size);
    m_msgSockets[id]->Send((char*)&size, sizeof(int));

    //fprintf(stderr, "%d: send olbuffer\n", m_rank);
    m_msgSockets[id]->Send((char*)&olbuffer[0], size*sizeof(int));

    olbuffer.clear();
}



//-----------------------------------------------------------------------------
//recv from socket id
//-----------------------------------------------------------------------------
void Socket_Data_Mover::recv_from_all(std::vector<int>& olbuffer)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int size=0;
    olbuffer.clear();

    for(int i=0; i<m_sockNum; i++)
    {
        m_msgSockets[i]->Receive((char*)&size, sizeof(int));
        fprintf(stderr, "recv from socket %d: olBuffer size=%d\n", 
                i, size);

        int n = olbuffer.size();
        olbuffer.resize(n+size, 0);

        m_msgSockets[i]->Receive((char*)&olbuffer[n], size*sizeof(int));
    }
}

