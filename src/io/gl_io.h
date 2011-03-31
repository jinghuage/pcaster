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


#ifndef GL_IO_H
#define GL_IO_H

#include <string>
#include <vector>


//An abstraction interface class of GL_Application IO

//Assume multi-channel ( Multiple to Multiple ) IO

// input category:  accompany renderer: update scene from tracker
// output category:  accompany renderer: stream images to viewer

// input category:   accompany viewer:   recv image 
// output category:  accompany viewer:   event-relayer

class Data_Mover
{
public:
    virtual ~Data_Mover(){};

    virtual void init_server(std::string&, int) = 0;
    virtual void init_client(std::string&, int) = 0;   

    virtual int Select() = 0;

    virtual void send_to(int id, void* buf, int size) = 0;
    virtual void recv_from(int id, void* buf, int size) = 0;

    virtual void send_to_all(void*, int) = 0;
    virtual void recv_from_all(void*, int) = 0;


    virtual void send_to_all(void*, int*, int*) = 0;
    virtual void recv_from_all(void*, int*, int*) = 0;

    virtual void send_to(int id, std::vector<int>& infobuf) = 0;
    virtual void recv_from_all(std::vector<int>& infobuf) = 0;

};



class GL_IO{
public:

    GL_IO(int rank, int runsize, int imode, int omode)
    { 
        m_rank=rank; 
        m_runsize=runsize; 
        m_input_mode = imode;
        m_output_mode = omode;

        m_sync_mode = SYNC_NONE;
        m_collect_mode = COLLECT_NONE;
        m_distribute_mode = DIST_SINGLE;
        m_mover = 0;
    }
    virtual ~GL_IO(){};

    enum IO_Mode{DDB=0, MEM_COPY=1, MOUSE_KEY=2, 
                 IPC_SHARE_MEMORY=4, TUIKIT=8, 
                 NET_TCP=256, 
                 NET_UDP=512};

    enum Sync_Mode{SYNC_NONE=0, SYNC_MPI=1};
    enum Collect_Mode{COLLECT_NONE=0, COLLECT_MPI=1};

    enum Distribute_Mode{DIST_NONE=0, DIST_BCAST=1, DIST_SINGLE=2};

    void set_input_mode(int mode){ m_input_mode = mode; }
    void set_output_mode(int mode){ m_output_mode = mode; }
    void set_sync_mode(int mode){ m_sync_mode = mode; }
    void set_collect_mode(int mode){ m_collect_mode = mode; }
    void set_distribute_mode(int mode){ m_distribute_mode = mode; }


    virtual void acquire_input() = 0;

    virtual void sync_input_to_peer(int comm) = 0;

    virtual void deliver_output(void*) = 0;

    virtual void collect_output_from_peer(int comm) = 0;

    virtual void init_data_mover(int, std::string&, int) = 0;

protected:
    int m_rank;
    int m_runsize;
    int m_input_mode;
    int m_output_mode;
    int m_sync_mode;
    int m_collect_mode;
    int m_distribute_mode;


    Data_Mover* m_mover;

};




#endif
