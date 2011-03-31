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



#include "GL_Application.h"

GL_Application::GL_Application(int w, int h) : 
    m_wid(w), m_hei(h),
    m_fid(0),
    m_img(0),
    m_ptr(0)
{
    init_glew_ext();
}

GL_Application::~GL_Application()
{
}

void GL_Application::assert_application_mode()
{

}


//-----------------------------------------------------------------------------
//init GL
//-----------------------------------------------------------------------------
void GL_Application::init_GL()
{
//     GLint n;
//     glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &n);
//     printf("Max texture units: %d\n", n);

//     glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &n);
//     printf("Max texture image units: %d\n", n);

//     glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &n);
//     printf("Max 3D texture size: %d\n", n);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //when draw polygon but don't show up, check the cull face 
    glDisable(GL_CULL_FACE);

    //glEnable(GL_DEPTH_TEST);                  
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP_NV);

    glDisable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    //glDisable(GL_BLEND);
    
    //glClearColor(0.5, 0.5, 0.5, 0.5);
    glClearColor(0., 0., 0., 0.);

    //glEnable(GL_CLIP_PLANE0); 

    glClearStencil(0x0);
    glDisable(GL_STENCIL_TEST);

    //glEnable(GL_OCCLUSION_TEST);

    glEnable(GL_TEXTURE_2D);
	
}



void GL_Application::init()
{
    init_GL();
}

void GL_Application::draw()
{
}



void GL_Application::save_screen(int vp[4])
{
    
    if(m_img == 0) m_img = new GenericImage;
    if(m_ptr == 0) m_ptr = new GLubyte[m_wid * m_hei * 4];

    glReadBuffer(GL_FRONT);
    glReadPixels(vp[0], vp[1], vp[2], vp[3], GL_RGB, GL_UNSIGNED_BYTE, 
                 (GLvoid*)m_ptr );


    char filename[32];
    sprintf(filename, "frame%d_%03d_o.png", 0, m_fid);

    m_img->init(m_ptr, vp[2], vp[3], 3, 1);
    m_img->save(filename);
}


void GL_Application::read_screen(GLenum mode, int* size, void** buf)
{
    if(m_ptr == 0) m_ptr = new GLubyte[m_wid * m_hei * 4];

    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, m_wid, m_hei, mode, GL_UNSIGNED_BYTE, 
                 (GLvoid*)m_ptr );

    *size = 0;
    if(mode == GL_RGB) *size = m_wid * m_hei * 3;
    else if(mode == GL_RGBA) *size = m_wid * m_hei * 4;

    *buf = (void *)m_ptr;

}

void GL_Application::write_screen(GLenum mode, int size, void* buf)
{
    glRasterPos2i(0, 0);
    glDrawPixels(m_wid, m_hei, mode, GL_UNSIGNED_BYTE, buf);
}



//-----------------------------------------------------------------------------
// frame rate computation
//-----------------------------------------------------------------------------
float GL_Application::compute_fps(float averagetime)
{
    static int frames = 0;
    static double cutime, pretime, inittime;


    if(frames == 0) cutime = pretime = inittime = getTimeInSecs();
    else  cutime = getTimeInSecs();

    if( (cutime - pretime) >= averagetime)
    {
        float f = frames/averagetime;
   
        
        frames = 0;
        pretime = cutime;

        return f;
    }
    frames++;

    return -1.0;
}


//-----------------------------------------------------------------------------
//process events -- virtual functions implements CG_Application
//-----------------------------------------------------------------------------
void GL_Application::processMouseMove(int m, int x, int y)
{

}

void GL_Application::processKeys(int key)
{
    switch(key)
    {
    case 27: //SDLK_ESCAPE:  
        exit(0);
    default: break;
    }
}

void GL_Application::processNetEvents()
{

}

//-----------------------------------------------------------------------------
// frame update
//-----------------------------------------------------------------------------
void GL_Application::frame_update()
{
    m_fid++;
}
