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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <iostream>
#include <algorithm>

#include <fbo.h>
#include <pmisc.h>
#include <draw_routines.h>
#include <vector_algebra.h>


#include "camera.h"


//-----------------------------------------------------------------------------
//Constructor and Destructor
//-----------------------------------------------------------------------------
Camera::Camera()
{

    memset(m_position, 0, 6*sizeof(float));
    memset(m_screen, 0, 9*sizeof(float));

}


Camera::Camera(float trl[3], float rot[3]) : 
    Transform(trl, rot)
{

    memset(m_position, 0, 6*sizeof(float));
    memset(m_screen, 0, 9*sizeof(float));

}



Camera::~Camera()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

}


//-----------------------------------------------------------------------------
// update from internal code
//-----------------------------------------------------------------------------
void Camera::lefteye_update(float np[3])
{
    m_position[0][0] = (m_position[0][0] + np[0]) / 2.0;
    m_position[0][1] = (m_position[0][1] + np[1]) / 2.0;
    m_position[0][2] = (m_position[0][2] + np[2]) / 2.0;
}


void Camera::righteye_update(float np[3])
{
    m_position[1][0] = (m_position[1][0] + np[0]) / 2.0;
    m_position[1][1] = (m_position[1][1] + np[1]) / 2.0;
    m_position[1][2] = (m_position[1][2] + np[2]) / 2.0;
}


void Camera::print()
{
    printf("left eye: (%.3f, %.3f, %.3f)\n", 
           m_position[0][0], m_position[0][1], m_position[0][2]);

}


// //-----------------------------------------------------------------------------
// // update from  networked interactor
// //-----------------------------------------------------------------------------
// void Camera::update_position(Interactor* steerer)
// {
// }


//-----------------------------------------------------------------------------
// setup projection matrix, determined by eye and screen position
//-----------------------------------------------------------------------------
void Camera::setup_projection(int whicheye)
{
    setupFrustum(m_position[whicheye],
                 m_screen[0],
                 m_screen[1],
                 m_screen[2],
                 0.1, 100.);
               
}


