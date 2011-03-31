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

#include <mpi.h>
#include <string>
#include <vector>
#include <math.h>
#include <assert.h>


#include "image_streamer.h"

#ifdef _STREAMING
#include "socket_data_mover.h"
#endif

Image_Streamer::Image_Streamer(int rank, int runsize,
                               int imode, int omode,
                               int num_connection,
                               int imgformat, int w, int h):
    GL_IO(rank, runsize, imode, omode),
    m_num_connection(num_connection)
{
    m_output_mode = NET_TCP;
    m_input_mode = NET_TCP;

    m_img_format = imgformat;

    int maxsize = w * h * 4;
    m_img_buffer = new unsigned char[maxsize];

    if(m_img_format == GL_RGB) m_buf_size = w * h * 3;
    else if(m_img_format == GL_RGBA) m_buf_size = maxsize;

    m_producer=0;
    m_receiver=0;

    printf("image streaming buffer size set to : %d\n", m_buf_size);
}


Image_Streamer::~Image_Streamer()
{

}

void Image_Streamer::acquire_input()
{
#ifdef _STREAMING
    if(m_input_mode == NET_TCP)
    {
        m_mover->recv_from_all( m_img_buffer, m_buf_size );
    }
#endif


    m_receiver->write_screen(m_img_format, m_buf_size, m_img_buffer);
}


void Image_Streamer::deliver_output(void* dst)
{
    m_producer->read_screen(m_img_format, &m_buf_size, (void**)&m_img_buffer);


#ifdef _STREAMING
    if(m_output_mode == NET_TCP)
    {
        m_mover->send_to_all( m_img_buffer, m_buf_size );
    }
#endif

    if(m_output_mode & MEM_COPY)
    {
        memcpy( dst, m_img_buffer, m_buf_size );
    }
}


void Image_Streamer::init_data_mover(int mode, 
                                     std::string& server_address, int port)
{
#ifdef _STREAMING

    m_mover = new Socket_Data_Mover(m_rank, 
                                    mode,
                                    m_num_connection,
                                    server_address, port);

#endif
}


void Image_Streamer::collect_output_from_peer(int comm)
{
#ifdef _PARALLEL
    if(m_collect_mode == COLLECT_MPI)
    {
        MPI_Gather(m_img_buffer, m_buf_size, MPI_BYTE, 
                   m_img_buffer, m_buf_size, MPI_BYTE, 
                   0, comm);
    }
#endif
}


void Image_Streamer::sync_input_to_peer(int comm)
{
#ifdef _PARALLEL
    if(m_sync_mode == SYNC_MPI)
    {
        MPI_Bcast(m_img_buffer, m_buf_size, MPI_BYTE, 0, comm);
    }
#endif
}

