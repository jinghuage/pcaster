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


#include <assert.h>


#include <assert.h>
#include <algorithm>
#include <sstream>

#include "viewer.h"



//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
Viewer::Viewer(int w, int h, int gw, int gh):
    GL_Application(w, h),
    m_gw(gw),
    m_gh(gh),
    m_global_screen(0)
{
    m_global_screen = new CRenderScreen(gw, 
                                        gh, 
                                        CRenderScreen::SINGLE,
                                        GL_TEXTURE_RECTANGLE_ARB, 
                                        GL_RGB, 
                                        GL_RGB, 
                                        GL_UNSIGNED_BYTE,
                                        GL_NEAREST, GL_NEAREST);   
}



Viewer::~Viewer()
{
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}



//-----------------------------------------------------------------------------
//assert the remote mode -- virtual function
//-----------------------------------------------------------------------------
void Viewer::assert_application_mode()
{

    if(!pcaster_options::viewer.compare("none"))
    {
        fprintf(stderr, "No viewer is required\n");
        exit(0);
    }

    if(!pcaster_options::viewer.compare("inplace"))
    {
        fprintf(stderr, "Use standalone render instead\n");
        exit(0);
    }
}


//-----------------------------------------------------------------------------
//draw the received pixels in full resolution -- virtual function
//-----------------------------------------------------------------------------
void Viewer::write_screen(GLenum mode, int size, void* buffer)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    assert(mode == GL_RGB);
    assert(size == m_gw * m_gh * 3);

    //somehow glDrawPixels() can't draw into fbo color buffer
    //manipulate fbo color buffer texture directly
    tex_unit* T = m_global_screen->get_buffer_texture();
    T->update_subimage(0, 0, m_gw, m_gh, buffer);

}

//-----------------------------------------------------------------------------
//application draw -- virtual function
//-----------------------------------------------------------------------------
void Viewer::draw()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);


    //draw part of offscreen global image onto screen
    int ofs_vp[4] = {0, 0, m_gw, m_gh};
    int s_vp[4] = {0, 0, m_wid, m_hei};
    std::string shaderpath(pcaster_options::SHADER_PATH);


    m_global_screen->draw_to_framebuffer(ofs_vp, 
                                         s_vp, 
                                         shaderpath);

}


//-----------------------------------------------------------------------------
//process events -- virtual function
//-----------------------------------------------------------------------------
void Viewer::processKeys(int key)
{

}

//-----------------------------------------------------------------------------
//process mouse events -- virtual function
//-----------------------------------------------------------------------------
void Viewer::processMouseMove(int m, int x, int y)
{
    
}


//-----------------------------------------------------------------------------
//process net events -- virtual function
//-----------------------------------------------------------------------------
void Viewer::processNetEvents()
{
    
}


//-----------------------------------------------------------------------------
//frame update -- virtual function
//-----------------------------------------------------------------------------
void Viewer::frame_update()
{
    GL_Application::frame_update();

#ifdef _DEBUG
    fprintf(stderr, "=====FRAME %d =====\n", m_fid);
#endif
}
