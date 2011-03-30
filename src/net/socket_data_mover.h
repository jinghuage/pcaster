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

#ifndef SOCKET_DATA_MOVER_H
#define SOCKET_DATA_MOVER_H

#include <string>
#include <vector>

#include <gl_io.h>

#include "tcpudpclient.h"
#include "tcpudpserver.h"
#include "messageSocket.h"

// A socket-based implemenation of moving data from point A to point B.
// used among two groups of senders/receivers


class Socket_Data_Mover : public Data_Mover{
public:
    enum connectionMode{NET_SERVER=0, NET_CLIENT};

    Socket_Data_Mover(int, int, int, std::string&, int);
    virtual ~Socket_Data_Mover();


    void init_client(std::string&, int);
    void init_server(std::string&, int);

    int Select();

    void send_to(int id, void* buf, int size);
    void recv_from(int id, void* buf, int size);

    void send_to_all(void*, int);
    void recv_from_all(void*, int);


    void send_to_all(void*, int*, int*);
    void recv_from_all(void*, int*, int*);

    void send_to(int id, std::vector<int>& infobuf);
    void recv_from_all(std::vector<int>& infobuf);



private:
    int m_rank;
    int m_mode;
    int m_sockNum;
    int* m_socketToRank;
    MessengeSocket** m_msgSockets;

    fd_set m_rset;
    int    m_maxfdpl;

};



#endif
