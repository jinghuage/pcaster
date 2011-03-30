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

#ifndef TRANSFORM_H_
#define TRANSFORM_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glheaders.h>

class Transform
{
public:
    Transform()
    {
        memset(m_translation, 0, 3*sizeof(float));
        memset(m_rotation, 0, 3*sizeof(float));
    }

    Transform(float trl[3], float rot[3])
    {
        memcpy(m_translation, trl, 3*sizeof(float));
        memcpy(m_rotation, rot, 3*sizeof(float));
    }
    virtual ~Transform(){};

    void set_translation(float trl[3])
    { 
        memcpy(m_translation, trl, 3*sizeof(float));
    } 

    void set_rotation(float rot[3])
    {
        memcpy(m_rotation, rot, 3*sizeof(float));
    }

    void rotate_x(float r){ m_rotation[0] += r; }
    void rotate_y(float r){ m_rotation[1] += r; }
    void rotate_z(float r){ m_rotation[2] += r; }

    void translate_x(float t){ m_translation[0] += t; }
    void translate_y(float t){ m_translation[1] += t; }
    void translate_z(float t){ m_translation[2] += t; }


    void print()
    {
        fprintf(stderr, "trl(%.3f, %.3f, %.3f), rot(%.3f, %.3f, %.3f)\n", 
                m_translation[0], m_translation[1], m_translation[2],
                m_rotation[0], m_rotation[1], m_rotation[2]);
    }

    void transform()
    {
        glTranslatef(m_translation[0], m_translation[1], m_translation[2]);        

        glRotatef(m_rotation[0], 1.0, 0.0, 0.0);
        glRotatef(m_rotation[1], 0.0, 1.0, 0.0);
        glRotatef(m_rotation[2], 0.0, 0.0, 1.0);
    }


protected:

    float m_translation[3]; //0.0f, 0.1f, -3.8f, //-3.8, -2.3, -1.4
    float m_rotation[3]; //-90.0, 90., 0.0, //-30.234375, -230.625000, 0.,
};


#endif
