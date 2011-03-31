/*****************************************************************************


    Copyright 2010, 2011 Jinghua Ge
    ------------------------------

    This file is part of Psplatter.

    Psplatter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Psplatter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Psplatter.  If not, see <http://www.gnu.org/licenses/>.


******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <iostream>


#include <texture.h>
#include <fbo.h>
#include <shader.h>
#include <buffer_object.h>
#include <draw_routines.h>


#include "glrenderbuffer_ofs.h"



GLRenderBuffer_FBO::GLRenderBuffer_FBO(int wid, int hei, int bufType, int fboType)
{
    m_viewport[0] = m_viewport[1] = 0;
    m_viewport[2] = wid;
    m_viewport[3] = hei;


    switch(bufType)
    {
    case SINGLE:
        m_buf_number = 1; break;
    case DOUBLE:
        m_buf_number = 2; break;
    case TRIPLE:
        m_buf_number = 3; break;
    default:
        m_buf_number = 1; break;
    }

    m_fbo_type = fboType;

    m_draw_buf = m_buf_number - 1;

    m_showtex_shader = 0;

    for(int i=0; i<m_buf_number; i++) m_fbo[i] = new fbo_unit;
}





GLRenderBuffer_FBO::~GLRenderBuffer_FBO()
{
    for(int i=0; i<m_buf_number; i++)
    {
        if(m_fbo[i]) 
        {
            delete m_fbo[i]; 
            m_fbo[i]=0; 
        }
    }
    
    if(m_showtex_shader) 
    { 
        delete m_showtex_shader;
        m_showtex_shader = 0;
    }
}

void GLRenderBuffer_FBO::print()
{
    if(m_buf_number == 1) std::cout << "SINGLE BUFFER\n";
    else std::cout << "DOUBLE BUFFER\n";

    std::cout << "render screen viewport: " 
              << m_viewport[0] << ", "
              << m_viewport[1] << ", "
              << m_viewport[2] << ", "
              << m_viewport[3] << "\n";
}


//-----------------------------------------------------------------------------
//set fbo format
//if not called, default will be used. see fbo.cpp for defaults
//-----------------------------------------------------------------------------

void GLRenderBuffer_FBO::set_format(GLuint target, GLuint ifmt,
                               GLuint efmt, GLuint dtype,
                               GLuint minf, GLuint magf)
{
    m_textureTarget = target;
    m_dataType = dtype;
    m_dataFormat = efmt;

    for(int i=0; i<m_buf_number; i++)
    {
        m_fbo[i]->setformat(target, ifmt, efmt, dtype);
        m_fbo[i]->setfilter(minf, magf);
    }
}




void GLRenderBuffer_FBO::init_fbos()
{
    std::cout << "init " << m_buf_number << "fbos\n";

    bool colorbuf = m_fbo_type & 1;
    bool depthbuf = m_fbo_type & 2;
    bool stencilbuf = m_fbo_type & 4;

    //colorbuf num will be either 0 or 1, don't do Multiple render target (MRT)
    int cbufnum = 0;
    if(colorbuf) cbufnum = 1;

    for(int i=0; i<m_buf_number; i++)
    {
        m_fbo[i]->init(m_viewport[2], 
                       m_viewport[3], 
                       cbufnum,
                       depthbuf, 
                       stencilbuf);
    }

}


void GLRenderBuffer_FBO::init_shader(std::string& path)
{
    std::string SHOWTEXV = path + "showtex.vert";
    std::string SHOWTEXF = path + "showtex.frag";

    //fprintf(stderr, "renderscreen init shader: showtex_shader: rec_tex.vert+frag\n");
    m_showtex_shader = new shader_object;
    m_showtex_shader->init_from_file(SHOWTEXV.c_str(), SHOWTEXF.c_str());
    m_showtex_shader->use();
    glUniform1i(m_showtex_shader->getUniformLocation("tex"), 0);
    glUniform2f(m_showtex_shader->getUniformLocation("tex_offset"), 0., 0.);
    glUniform2f(m_showtex_shader->getUniformLocation("tex_mf"), 1.0, 1.0);
    glUseProgram(0);

    //checkError();
}



//-----------------------------------------------------------------------------
// Must bind the draw fbo
//-----------------------------------------------------------------------------
void GLRenderBuffer_FBO::bind_fbo()
{
    m_fbo[m_draw_buf]->bind();

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(m_viewport[0],
               m_viewport[1],
               m_viewport[2],
               m_viewport[3]);

    //use the application transformation matrix for fbo
}


void GLRenderBuffer_FBO::unbind_fbo()
{
    glPopAttrib();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}



//-----------------------------------------------------------------------------
// Must bind the read texture m_draw_buf + readid
// cid=-1: depth
// cid=0..4, color attachments
//-----------------------------------------------------------------------------
void GLRenderBuffer_FBO::bind_texture(int readid, int cid)
{
    int fid = m_draw_buf - 1 - readid + m_buf_number;
    fid %= m_buf_number;

    //std::cout << "bind fbo " << fid << "'s texture\n";

    if(cid == -1) m_fbo[fid]->get_depth_texture()->bind();
    else m_fbo[fid]->get_color_texture(cid)->bind();    
}



void GLRenderBuffer_FBO::unbind_texture()
{
    glBindTexture(m_textureTarget, 0);
}




//----------------------------------------------------------------------------
//draw render buffer content in offscreen viewport ofs_vp 
//to a sepearte onscreen debug viewport s_vp
//----------------------------------------------------------------------------
void GLRenderBuffer_FBO::draw_to_framebuffer(int readid,
                                        int textureid,
                                        int* ofs_vp, int* s_vp, 
                                        std::string& path,
                                        bool blend)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    if(m_showtex_shader == 0) init_shader(path);


    glActiveTexture(GL_TEXTURE0);    
    bind_texture(readid, textureid);

    if(blend)
    {
        glEnable(GL_BLEND);
        //enable front to back blending. 
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        //enable back to front blending
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_ONE, GL_ONE);
    }
    else glDisable(GL_DEPTH_TEST);

    push_all_matrix();
    toOrtho(s_vp);

/*
    float tex_mf[2] = {1.0, 1.0}; //min/mag filter into tex
    float tex_offset[2] = {0., 0.};

    if(ofs_vp == 0) ofs_vp = m_viewport;
    if(s_vp == 0) s_vp = m_viewport;

    if(ofs_vp && s_vp)
    {
        tex_mf[0] = 1.0 * ofs_vp[2] / m_viewport[2];
        tex_mf[1] = 1.0 * ofs_vp[3] / m_viewport[3];
        tex_offset[0] = 1.0 * ofs_vp[0] / m_viewport[2];
        tex_offset[1] = 1.0 * ofs_vp[1] / m_viewport[3];
    }

//     fprintf(stderr, "tex_offset: %f, %f\n", tex_offset[0], tex_offset[1]);
//     fprintf(stderr, "vp_offset: %f, %f\n", vp_offset[0], vp_offset[1]);
//     fprintf(stderr, "tex_mf: %f, %f\n", tex_mf[0], tex_mf[1]);

    m_showtex_shader->use();
    //by default bind to texture unit 0
    //glUniform1i(m_showtex_shader->getUniformLocation("tex"), 0);
    glUniform2f(m_showtex_shader->getUniformLocation("tex_offset"), 
                tex_offset[0], tex_offset[1]);
    glUniform2f(m_showtex_shader->getUniformLocation("tex_mf"), 
                tex_mf[0], tex_mf[1]);

*/

    glColor3f(1.0, 1.0, 1.0);    
    draw_quad();

/*
    glUseProgram(0);
*/

    unbind_texture();


    if(blend) glDisable(GL_BLEND);
    else glEnable(GL_DEPTH_TEST);


    pop_all_matrix();

}

//-----------------------------------------------------------------------------
//readbuf0 = (drawbuf -1 -0)%bufnum
//readbuf1 = (drawbuf -1 -1)%bufnum
//rotation: drawbuf = (drawbuf +1)%bufnum
//after rotation, readbuf 0 is the most updated buffer
//-----------------------------------------------------------------------------
void GLRenderBuffer_FBO::rotate_buffer()
{
    m_draw_buf ++;
    m_draw_buf %= m_buf_number;
}


//-----------------------------------------------------------------------------
//read back fbo texture cid to system memory buffer 
//-----------------------------------------------------------------------------
void GLRenderBuffer_FBO::read_back(int readid, int cid, int* sp, unsigned char* buf)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int fid = (m_draw_buf - 1 - readid + m_buf_number) % m_buf_number;
    m_fbo[fid]->bind();

    if(cid == -1) m_fbo[fid]->read_depthbuffer();
    else m_fbo[fid]->read_colorbuffer(cid);

//     fprintf(stderr, "read back sp(%d, %d, %d, %d)\n", 
//             sp[0], sp[1], sp[2], sp[3]);

    glReadPixels(sp[0], sp[1],
                 sp[2], sp[3], 
                 m_dataFormat, m_dataType, buf);

//     toOrtho();
//     glRasterPos2i(0, 0);
//     glDrawPixels(m_W, m_H, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)ptr);
//     toPerspective();

    m_fbo[fid]->unbind();
    //glReadBuffer(GL_NONE);
}


//-----------------------------------------------------------------------------
//read back fbo texture cid to GPU memeory buffer object (pbo)
//pbo can be used to bind as vbo and draw
//-----------------------------------------------------------------------------
void GLRenderBuffer_FBO::read_back(int readid, int cid, 
                              int* sp, 
                              buffer_object* pbo, 
                              int offset, 
                              bool needbind)
{
    int fid = (m_draw_buf - 1 - readid + m_buf_number) % m_buf_number;
    if(needbind) m_fbo[fid]->bind();

    pbo->fbo2pbo(m_fbo[fid], cid, 
                sp[0], sp[1],
                sp[2], sp[3],
                offset);

    if(needbind) m_fbo[fid]->unbind();
}



//-----------------------------------------------------------------------------
// update the read fbo's texture subimage directly from a pbo/vbo
// usually used to init_simulation_states the read fbos in multi-buffer rotation
//-----------------------------------------------------------------------------
void GLRenderBuffer_FBO::init_texture_image(int readid,
                                       int cid, 
                                       int* sp, 
                                       buffer_object* pbo)
{
    pbo->bind(GL_PIXEL_UNPACK_BUFFER_ARB);
    if( printOpenGLError() ) exit(1);

    tex_unit* pos;

    int fid = m_draw_buf - 1 - readid + m_buf_number;
    fid %= m_buf_number;

    if(cid == -1) pos = m_fbo[fid]->get_depth_texture();
    else pos = m_fbo[fid]->get_color_texture(cid);
    if( printOpenGLError() ) exit(1);

    pos->update_subimage(sp[0], sp[1], sp[2], sp[3], 0);

    pbo->unbind();
}


