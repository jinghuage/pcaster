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

#ifndef GLRENDERBUFFER_FBO_H
#define GLRENDERBUFFER_FBO_H

#include <string>

#include <glheaders.h>


class tex_unit;
class fbo_unit;
class buffer_object;
class shader_object;


class GLRenderBuffer_FBO
{
public:

    enum bufferType{SINGLE=1, DOUBLE=2, TRIPLE=3};
    enum fboType{COLOR=1, DEPTH=2, STENCIL=4};


    //GLRenderBuffer_FBO();
    //GLRenderBuffer_FBO(GLRenderBuffer_FBO&); //this is implicit shallow copy
    //GLRenderBuffer_FBO(GLRenderBuffer_FBO*); //de-reference the pointer then pass as shallow copy 
    GLRenderBuffer_FBO(int wid, int hei, int bufType, int fboType); 
    virtual ~GLRenderBuffer_FBO();

    void set_format(GLuint, GLuint, GLuint, GLuint,
                    GLuint minf, GLuint magf);

    void init_fbos();

    int get_bufnum(){ return m_buf_number; }

    //bind draw fbo
    void bind_fbo();
    void unbind_fbo();

    //bind read buf m_draw_buf+readid
    void bind_texture(int readid, int cid);
    void unbind_texture();
    

    void rotate_buffer();

    void init_texture_image(int readid,
                            int cid, 
                            int* sp, 
                            buffer_object* pbo);
    void read_back(int readid, int cid, int* sp, unsigned char* buf);
    void read_back(int readid, int cid, int* in_sp, buffer_object* pbo, int offset, bool needbind=true);

    void draw_to_framebuffer(int readid, int textureid, 
                             int* ofs_vp, int* s_vp, 
                             std::string& path,
                             bool blend=true);
    void print();

protected:
    int m_viewport[4];


private:
    int m_buf_number;
    int m_fbo_type;
    int m_draw_buf;
    //int m_read_buf;

    GLuint m_textureTarget;
    GLuint m_dataType;
    GLuint m_dataFormat;
    fbo_unit *m_fbo[2]; //offscreen rendering, double buffer

    shader_object *m_showtex_shader;


    void init_shader(std::string&);
};



#endif
