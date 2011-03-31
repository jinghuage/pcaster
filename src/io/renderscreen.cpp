//offscreen render env class

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <iostream>

//#include <fbo.h>
#include <draw_routines.h>
//#include <buffer_object.h>
//#include <shader.h>


#include "renderscreen.h"



CRenderScreen::CRenderScreen(int wid, int hei, int bufType, 
                             GLuint target, GLuint ifmt,
                             GLuint efmt, GLuint dtype,
                             GLuint minf, GLuint magf,
                             bool depthbuf, bool stencilbuf)
{
    m_viewport[0] = m_viewport[1] = 0;
    m_viewport[2] = wid;
    m_viewport[3] = hei;

    m_draw_buf = 0;
    m_read_buf = 0;

    switch(bufType)
    {
    case SINGLE:
        m_buf_number = 1; break;
    case DOUBLE:
        m_buf_number = 2; m_read_buf = 1; break;
    default:
        m_buf_number = 1; break;
    }

    
    m_renderBuffer[0] = 0;
    m_renderBuffer[1] = 0;

    m_rectex_shader = 0;

    init_render_buffer(target, ifmt, efmt, dtype, 
                       minf, magf, depthbuf, stencilbuf);
    //init_shader();
}


CRenderScreen::~CRenderScreen()
{
    for(int i=0; i<2; i++)
    {
        if(m_renderBuffer[i]) 
        {
            delete m_renderBuffer[i]; 
            m_renderBuffer[i]=0; 
        }
    }
    
    if(m_rectex_shader) 
    { 
        delete m_rectex_shader;
        m_rectex_shader = 0;
    }
}

void CRenderScreen::print()
{
    if(m_buf_number == 1) std::cout << "SINGLE BUFFER\n";
    else std::cout << "DOUBLE BUFFER\n";

    std::cout << "render screen viewport: " 
              << m_viewport[0] << ", "
              << m_viewport[1] << ", "
              << m_viewport[2] << ", "
              << m_viewport[3] << "\n";
}


// void CRenderScreen::checkError()
// {
//     if(printOpenGLError())
//     {
//         std::cout << __FILE__ << ":" <<  __func__ <<"()\n";
//         exit(1);
//     }
// }

void CRenderScreen::init_render_buffer(GLuint target,
                                       GLuint intFormat,
                                       GLuint extFormat,
                                       GLuint dataType,
                                       GLuint minf, GLuint magf,
                                       bool depthbuf, bool stencilbuf)
{

    for(int i=0; i<m_buf_number; i++)
    {
        //std::cout << "init renderBuffer " << i << "\n";
        m_renderBuffer[i] = new fbo_unit;
        m_renderBuffer[i]->setformat(target, intFormat, extFormat, dataType);
        m_renderBuffer[i]->setfilter(minf, magf);
        m_renderBuffer[i]->init(m_viewport[2], 
                                m_viewport[3], 
                                1, 
                                depthbuf, stencilbuf);
    }

    //checkError();
}


void CRenderScreen::init_shader(std::string& path)
{
    std::string RECTEXV = path + "rec_tex.vert";
    std::string RECTEXF = path + "rec_tex.frag";

    //fprintf(stderr, "renderscreen init shader: rectex_shader: rec_tex.vert+frag\n");
    m_rectex_shader = new shader_object;
    m_rectex_shader->init_from_file(RECTEXV.c_str(), RECTEXF.c_str());
    m_rectex_shader->use();
    glUniform1i(m_rectex_shader->getUniformLocation("tex"), 0);
    glUniform2f(m_rectex_shader->getUniformLocation("vp_offset"), 0., 0.);
    glUniform2f(m_rectex_shader->getUniformLocation("tex_offset"), 0., 0.);
    glUniform2f(m_rectex_shader->getUniformLocation("tex_mf"), 1.0, 1.0);
    glUseProgram(0);

    //checkError();
}


void CRenderScreen::bind_render_buffer()
{
    m_renderBuffer[m_draw_buf]->bind();
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
    glViewport(m_viewport[0],
               m_viewport[1],
               m_viewport[2],
               m_viewport[3]);

    //use the application transformation matrix for fbo
}


void CRenderScreen::unbind_render_buffer()
{
    glPopAttrib();
    m_renderBuffer[m_draw_buf]->unbind();
}

//----------------------------------------------------------------------------
//draw render buffer content in offscreen viewport ofs_vp 
//to a sepearte onscreen debug viewport s_vp
//----------------------------------------------------------------------------
void CRenderScreen::draw_to_framebuffer(int* ofs_vp, int* s_vp, 
                                        std::string& path,
                                        bool blend)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    if(m_rectex_shader == 0) init_shader(path);
    //glDisable(GL_DEPTH_TEST);


    glActiveTexture(GL_TEXTURE0);    
    m_renderBuffer[m_draw_buf]->get_color_texture(0)->bind();

    push_all_matrix();

    float tex_mf[2] = {1.0, 1.0}; //min/mag filter into tex
    float tex_offset[2] = {0., 0.};
    float vp_offset[2] = {0., 0.};

    if(ofs_vp == 0) ofs_vp = m_viewport;
    if(s_vp == 0) s_vp = m_viewport;

    if(ofs_vp && s_vp)
    {
        tex_mf[0] = 1.0 * ofs_vp[2] / s_vp[2];
        tex_mf[1] = 1.0 * ofs_vp[3] / s_vp[3];
        tex_offset[0] = ofs_vp[0];
        tex_offset[1] = ofs_vp[1];
        vp_offset[0] = s_vp[0];
        vp_offset[1] = s_vp[1];
    }
    toOrtho(s_vp);
    


//     fprintf(stderr, "tex_offset: %f, %f\n", tex_offset[0], tex_offset[1]);
//     fprintf(stderr, "vp_offset: %f, %f\n", vp_offset[0], vp_offset[1]);
//     fprintf(stderr, "tex_mf: %f, %f\n", tex_mf[0], tex_mf[1]);

    if(blend)
    {
        glEnable(GL_BLEND);
        //enable front to back blending. 
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        //enable back to front blending
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_ONE, GL_ONE);
    }


    m_rectex_shader->use();
    //by default bind to texture unit 0
    //glUniform1i(m_rectex_shader->getUniformLocation("tex"), 0);
    glUniform2f(m_rectex_shader->getUniformLocation("tex_offset"), 
                tex_offset[0], tex_offset[1]);
    glUniform2f(m_rectex_shader->getUniformLocation("vp_offset"), 
                vp_offset[0], vp_offset[1]);
    glUniform2f(m_rectex_shader->getUniformLocation("tex_mf"), 
                tex_mf[0], tex_mf[1]);

    //glColor3f(1.0, 1.0, 0.0);    
    draw_quad();

    glUseProgram(0);
    m_renderBuffer[m_draw_buf]->get_color_texture(0)->unbind();


    if(blend)
    glDisable(GL_BLEND);



    pop_all_matrix();
    //glEnable(GL_DEPTH_TEST);
}


void CRenderScreen::swap_buffer()
{
    m_draw_buf ++;
    m_draw_buf %= m_buf_number;

    m_read_buf ++;
    m_read_buf %= m_buf_number;
}


void CRenderScreen::read_back(int* sp, unsigned char* buf, GLenum mode)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_renderBuffer[m_read_buf]->bind();
    m_renderBuffer[m_read_buf]->read_colorbuffer(0);

//     fprintf(stderr, "read back sp(%d, %d, %d, %d)\n", 
//             sp[0], sp[1], sp[2], sp[3]);

    assert(mode==GL_RGB || mode==GL_RGBA);

    glReadPixels(sp[0], sp[1],
                 sp[2], sp[3], 
                 mode, GL_UNSIGNED_BYTE, buf);

//     toOrtho();
//     glRasterPos2i(0, 0);
//     glDrawPixels(m_W, m_H, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)ptr);
//     toPerspective();

    m_renderBuffer[m_read_buf]->unbind();
    //glReadBuffer(GL_NONE);
}


void CRenderScreen::read_back(int* sp, buffer_object* pbo, int offset, bool needbind, GLenum mode)
{
    if(needbind) m_renderBuffer[m_read_buf]->bind();

    pbo->fbo2pbo(m_renderBuffer[m_read_buf], 0, 
                sp[0], sp[1],
                sp[2], sp[3],
                offset);

    if(needbind) m_renderBuffer[m_read_buf]->unbind();
}

// void CRenderScreen::read_back(FootPrint& fp)
// {
//     m_renderBuffer[m_read_buf]->bind();
//     m_renderBuffer[m_read_buf]->read_colorbuffer(0);

//     fp.read_map();

//     m_renderBuffer[m_read_buf]->unbind();
// }
