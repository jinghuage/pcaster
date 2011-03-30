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


#ifndef EVENT_RELAYER_H
#define EVENT_RELAYER_H

#include <string>
#include <vector>

#include "gl_io.h"

class Event_Relayer : public GL_IO{
public:
    Event_Relayer(int rank, int runsize, int imode, int omode);
    Event_Relayer(int rank, int runsize, int imode, int omode, int nc);
    virtual ~Event_Relayer();

    void acquire_input();
    void sync_input_to_peer(int comm);

    void deliver_output(void*);

    void collect_output_from_peer(int comm);

    void init_data_mover(int, std::string&, int);

public:

    virtual void map_event(std::string&);



    void mapto_world_translate(float trl[3]);
    void mapto_world_translate(float, float, float);
    void mapto_world_rotate(float rot[3]);
    void mapto_world_rotate(float, float, float);
    void mapto_world_transform(float objpos[3], float objrot[3]);

    void mapto_camera_translate(float trl[3]);
    void mapto_camera_translate(float, float, float);
    void mapto_camera_rotate(float rot[3]);
    void mapto_camera_rotate(float, float, float);
    void mapto_camera_position(float pleft[3], float pright[3]);

protected:

    int m_num_connection;

    float m_event_buffer[32];

};



#endif
