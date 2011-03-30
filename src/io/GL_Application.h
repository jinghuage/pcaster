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

#ifndef GL_APPLICATION_H
#define GL_APPLICATION_H

#include <glheaders.h>
#include <pmisc.h>
#include <Timer.h>
#include <imgfile.h>
#include <draw_routines.h>

#include "CG_Application.h"

//implement computer graphics application by OpenGL + GLSL
//leave place for a CUDA_Application implementation of CG_Application

class GL_Application : public CG_Application{
public:
    GL_Application(int w, int h);
    virtual ~GL_Application();

    void assert_application_mode();

    void init();
    void draw();
    void frame_update();

    void processKeys(int);
    void processNetEvents();
    void processMouseMove(int m, int x, int y);


    void save_screen(int vp[4]);
    float compute_fps(float averagetime);


    //virtual function to expose the rendered buffer
    //usually for the output_interactor
    virtual void read_screen(GLenum mode, int* size, void** buf);
    virtual void write_screen(GLenum mode, int size, void* buf);

protected:
    int m_wid, m_hei; //render size
    int m_fid;


    GenericImage * m_img;
    GLubyte* m_ptr;

    void init_GL();

};



#endif
