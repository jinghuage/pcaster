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
#ifdef _PARALLEL
#include <mpi.h>
#endif

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>


#include <string>
#include <vector>


#include "onitcs.h"
#include "event_relayer.h"

#ifdef _STREAMING
#include <socket_data_mover.h>
#endif

Event_Relayer::Event_Relayer(int rank, int runsize, 
                             int imode, int omode):
    GL_IO(rank, runsize, imode, omode),
    m_num_connection(0)
{
    if(m_input_mode & IPC_SHARE_MEMORY) onitcs_init(0, 0, 0);
    memset(m_event_buffer, 0, 32*sizeof(float));
}


Event_Relayer::Event_Relayer(int rank, int runsize, 
                             int imode, int omode,
                             int nc):
    GL_IO(rank, runsize, imode, omode),
    m_num_connection(nc)
{
    if(m_input_mode & IPC_SHARE_MEMORY) onitcs_init(0, 0, 0);
    memset(m_event_buffer, 0, 32*sizeof(float));
}


Event_Relayer::~Event_Relayer()
{
    if(m_input_mode & IPC_SHARE_MEMORY) onitcs_fini();
}


void Event_Relayer::acquire_input()
{
    if(m_input_mode & IPC_SHARE_MEMORY)
    {
        onit_point* points;

        points = onitcs_acquire_points();

        //point[24]: left eye position
        //point[25]: right eye position

        // printf("point[0]: (%f, %f, %f)\n", 
        //        points[0].world_p[0], points[0].world_p[1], points[0].world_p[2]);


        memcpy(m_event_buffer, points[24].world_p, 3*sizeof(float));
        memcpy(m_event_buffer+3, points[25].world_p, 3*sizeof(float));

        onitcs_release_points();

        float* axis;
        axis = onitcs_acquire_axes();

        //axis[0]: xbox controller button #1(topleft), tune left-right
        //axis[1]: xbox controller button #1(topleft), tune up-down
        //axis[4]: xbox controller button #3(downright), tune left-right


        float tz = axis[1] / 60.;
        float ry = axis[0] * 90. / 60.;
        float rx = axis[4] * 90. / 60.;

        if( fabs(axis[1]) > 0.25 ) m_event_buffer[8]  += tz;
        if( fabs(axis[0]) > 0.25 ) m_event_buffer[10] += ry;
        if( fabs(axis[4]) > 0.25 ) m_event_buffer[9]  += rx;

        onitcs_release_axes();

    }

#ifdef _STREAMING
    if(m_input_mode == NET_TCP)
    {
        m_mover->recv_from_all( m_event_buffer, 32 * sizeof(float) );
    }
#endif

}


void Event_Relayer::sync_input_to_peer(int comm)
{
//    fprintf(stderr, "**** %d: %s:%s() ****\n", m_rank, __FILE__, __func__);

     if(m_runsize == 1){
        printf("single process run, no sync needed\n");
        return;
    }

#ifdef _PARALLEL
    if(m_sync_mode == SYNC_MPI)
    {    
//        fprintf(stderr, "%d: sync scene manager info to other MPI processes\n", m_rank);
        MPI_Bcast(m_event_buffer, 32, MPI_FLOAT, 0, (MPI_Comm)comm); //MPI_COMM_WORLD);
    }
#endif
}



void Event_Relayer::init_data_mover(int mode, 
                                    std::string& server_address, int port)
{
#ifdef _STREAMING

    m_mover = new Socket_Data_Mover(m_rank, 
                                    mode,
                                    m_num_connection,
                                    server_address, port);

#endif
}



void Event_Relayer::deliver_output(void* dst)
{
#ifdef _STREAMING
    if(m_output_mode & NET_TCP)
    {
        m_mover->send_to_all( m_event_buffer, 32 * sizeof(float) );
    }
#endif

    // printf("output mode=%d\n", m_output_mode);

    if(m_output_mode & MEM_COPY)
    {
        // printf("eventbuffer[6-8](%.3f, %.3f, %.3f)\n",
        //        m_event_buffer[6], m_event_buffer[7], m_event_buffer[8]);
        // printf("eventbuffer[9-11](%.3f, %.3f, %.3f)\n",
        //        m_event_buffer[9], m_event_buffer[10], m_event_buffer[11]);

        memcpy( dst, m_event_buffer, 32 * sizeof(float) );
    }
}



void Event_Relayer::collect_output_from_peer(int comm)
{
#ifdef _PARALLEL
    if(m_collect_mode == COLLECT_MPI)
    {
        MPI_Gather(m_event_buffer, 32, MPI_FLOAT, 
                   m_event_buffer, 32, MPI_FLOAT,
                   0, comm);
    }
#endif
}


//-----------------------------------------------------------------------------
// map event to a format inside my own event buffer
//-----------------------------------------------------------------------------
void Event_Relayer::map_event(std::string& s)
{
//    if( s.find("world-translate") ) mapto_world_translate( s.blah() );

}



//-----------------------------------------------------------------------------
// specific mapping functions
//-----------------------------------------------------------------------------
void Event_Relayer::mapto_world_translate(float trl[3])
{
    m_event_buffer[6] += trl[0];
    m_event_buffer[7] += trl[1];
    m_event_buffer[8] += trl[2];
}

void Event_Relayer::mapto_world_translate(float tx, float ty, float tz)
{
    m_event_buffer[6] += tx;
    m_event_buffer[7] += ty;
    m_event_buffer[8] += tz;
    // printf("wor_trl(%.3f, %.3f, %.3f)\n", 
    //        m_event_buffer[6], m_event_buffer[7], m_event_buffer[8]);
}

void Event_Relayer::mapto_world_rotate(float rot[3])
{
    m_event_buffer[9]  += rot[0];
    m_event_buffer[10] += rot[1];
    m_event_buffer[11] += rot[2];    
}

void Event_Relayer::mapto_world_rotate(float rx, float ry, float rz)
{
    m_event_buffer[9]  += rx;
    m_event_buffer[10] += ry;
    m_event_buffer[11] += rz;    
    // printf("wor_rot(%.3f, %.3f, %.3f)\n", 
    //        m_event_buffer[9], m_event_buffer[10], m_event_buffer[11]);
}


void Event_Relayer::mapto_camera_translate(float trl[3])
{
    m_event_buffer[0] += trl[0];
    m_event_buffer[1] += trl[1];
    m_event_buffer[2] += trl[2];    
}
void Event_Relayer::mapto_camera_translate(float tx, float ty, float tz)
{
    m_event_buffer[0] += tx;
    m_event_buffer[1] += ty;
    m_event_buffer[2] += tz;    
}


void Event_Relayer::mapto_camera_rotate(float rot[3])
{
    m_event_buffer[12] += rot[0];
    m_event_buffer[13] += rot[1];
    m_event_buffer[14] += rot[2];        
}

void Event_Relayer::mapto_camera_rotate(float rx, float ry, float rz)
{
    m_event_buffer[12] += rx;
    m_event_buffer[13] += ry;
    m_event_buffer[14] += rz;        
}

void Event_Relayer::mapto_camera_position(float pleft[3], float pright[3])
{
    memcpy(m_event_buffer, pleft, 3*sizeof(float));
    memcpy(m_event_buffer+3, pright, 3*sizeof(float));    
}

void Event_Relayer::mapto_world_transform(float objpos[3], float objrot[3])
{
    memcpy(m_event_buffer+6, objpos, 3*sizeof(float));
    memcpy(m_event_buffer+9, objrot, 3*sizeof(float));    
    // printf("wor_trl(%.3f, %.3f, %.3f)\n", 
    //        m_event_buffer[6], m_event_buffer[7], m_event_buffer[8]);
    // printf("wor_rot(%.3f, %.3f, %.3f)\n", 
    //        m_event_buffer[9], m_event_buffer[10], m_event_buffer[11]);
}

