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

#ifndef CAMERA_H_
#define CAMERA_H_


#include "transform.h"

class Camera : public Transform
{
public:
    Camera();
    Camera(float trl[3], float rot[3]);
    virtual ~Camera();


    enum ID{LEFT=0, RIGHT};


    //set camera use screen corners and focal position

    void set_screen_corners(float bl[3], float br[3], float tl[3])
    {
        memcpy(m_screen[0], bl, 3*sizeof(float));
        memcpy(m_screen[1], br, 3*sizeof(float));
        memcpy(m_screen[2], tl, 3*sizeof(float));
    }
    void set_positions(float lefteye[3], float righteye[3])
    {
        memcpy(m_position[0], lefteye, 3*sizeof(float));
        memcpy(m_position[1], righteye, 3*sizeof(float));
    }

    void lefteye_update(float np[3]);
    void righteye_update(float np[3]);

    void setup_projection(int whicheye);

    void print();

private:

    float m_position[2][3];  // camera focal postion, left and right eye
    float m_screen[3][3]; // must be screen corners BL, BR, TL

};


#endif
