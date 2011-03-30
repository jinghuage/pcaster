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


#ifndef IMAGE_STREAMER_H
#define IMAGe_STREAMER_H

#include <string>
#include <vector>

#include <gl_io.h>
#include <GL_Application.h>




class Image_Streamer : public GL_IO{
public:

    Image_Streamer(int rank, int runsize, int imode, int omode, int nc,
                   int, int, int);
    virtual ~Image_Streamer();


    void acquire_input(); //for viewer

    void deliver_output(void* dst=0); //for renderer

    void sync_input_to_peer(int comm);
    void collect_output_from_peer(int comm);

    void init_data_mover(int, std::string&, int);

public:

    void set_image_format(GLenum f){ m_img_format = f; }
    void register_image_producer(GL_Application* app)
    { 
        m_producer = app; 
    }
    void register_image_receiver(GL_Application* app)
    {
        m_receiver = app; 
    }


protected:
    int m_num_connection;
    GLenum m_img_format;

    unsigned char* m_img_buffer;
    int m_buf_size;


    GL_Application* m_producer;
    GL_Application* m_receiver;

};



#endif

