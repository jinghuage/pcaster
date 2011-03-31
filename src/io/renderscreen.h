#ifndef RENDER_SCREEN_H
#define RENDER_SCREEN_H

#include <glheaders.h>
#include <texture.h>
#include <fbo.h>
#include <shader.h>
#include <buffer_object.h>

#include <string>

class tex_unit;
class fbo_unit;
class buffer_object;
class shader_object;
//class FootPrint;

class CRenderScreen
{
public:

    enum bufferType{SINGLE=0, DOUBLE };


    //CRenderScreen();
    //CRenderScreen(CRenderScreen&); //this is implicit shallow copy
    //CRenderScreen(CRenderScreen*); //de-reference the pointer then pass as shallow copy 
    CRenderScreen(int wid, int hei, int bufType, 
                  GLuint, GLuint, GLuint, GLuint,
                  GLuint minf, GLuint magf, 
                  bool dep=false, bool sten=false);
    virtual ~CRenderScreen();

    //void checkError();

    void bind_render_buffer();
    void unbind_render_buffer();

    void prepare_readback()
    {
        m_renderBuffer[m_read_buf]->bind();
        m_renderBuffer[m_read_buf]->read_colorbuffer(0);
    }

    void wrapup_readback(){ m_renderBuffer[m_read_buf]->unbind(); }

    void swap_buffer();
    tex_unit* get_buffer_texture()
    {
        return m_renderBuffer[m_draw_buf]->get_color_texture(0);
    }


    void read_back(int* sp, unsigned char* buf, GLenum mode=GL_RGBA);
    void read_back(int* in_sp, buffer_object* pbo, int offset, bool needbind=true, GLenum mode=GL_RGB);
    //void read_back(FootPrint& fp);

    void draw_to_framebuffer(int* ofs_vp, int* s_vp, 
                             std::string& path,
                             bool blend=true);
    void print();

protected:
    int m_viewport[4];


private:
    int m_buf_number;
    int m_draw_buf;
    int m_read_buf;
    fbo_unit *m_renderBuffer[2]; //offscreen rendering, double buffer
    shader_object *m_rectex_shader;

    void init_render_buffer(GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, 
                            bool dep=false, bool sten=false);
    void init_shader(std::string&);
};



#endif
