//vbo and pbo
#include <iostream>
#include "buffer_object.h"
#include "misc.h"

#include <assert.h>

//references
// static GLenum bind_target[] = {
// 	GL_ARRAY_BUFFER_ARB,
// 	GL_ELEMENT_ARRAY_BUFFER_ARB,
// 	GL_PIXEL_PACK_BUFFER_ARB,
// 	GL_PIXEL_UNPACK_BUFFER_ARB
// };

// static GLenum draw_mode[] = {
// 	GL_STATIC_DRAW,
// 	GL_STREAM_DRAW,
//      GL_STREAM_READ,
// 	GL_DYNAMIC_DRAW
// };

// static GLenum map_mode[] = {
// 	GL_READ_ONLY,
// 	GL_WRITE_ONLY,
// 	GL_READ_WRITE
// };

// static GLuint data_format[][2] = {
// 	{GL_DEPTH_COMPONENT,	1},		//0
// 	{GL_RED,		1},		//1
// 	{GL_GREEN,		1},		//2
// 	{GL_BLUE,		1},		//3
// 	{GL_ALPHA, 		1},		//4
// 	{GL_LUMINANCE,		1},		//
// 	{GL_LUMINANCE_ALPHA,	2},		//6
// 	{GL_RGB,		3},		//7
// 	{GL_RGBA,		4}		//8
// };

// static GLuint data_type[][2] = {
// 	{GL_BYTE,			1},		//0
// 	{GL_UNSIGNED_BYTE,	        1},		//1
// 	{GL_SHORT,			2},		//2
// 	{GL_FLOAT,			4},		//3
// 	{GL_INT,			4}		//4
// };


buffer_object::buffer_object()
{
    m_buf = 0;
}

buffer_object::~buffer_object()
{
    if(m_buf){ clear(); glDeleteBuffers(1, &m_buf); m_buf = 0; }
}


void buffer_object::init(GLuint target, GLuint num, GLuint mode, 
                         GLuint comp, GLuint type,
                         GLvoid* data)
{
    glGenBuffers( 1, &m_buf );
	
    m_T = target;
    m_DMode = mode;
    m_dataType = type;
    m_component = comp;
	
    int stride = 1;
    switch(type)
    {
    case GL_FLOAT:
    case GL_INT: 
    case GL_UNSIGNED_INT: stride = 4; break;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT: stride = 2; break;
    }

    m_datasize = num * stride * comp;	

    GLint nParam_ArrayObjectSize;
	
    if(m_datasize > 0)
    {
        glBindBuffer( m_T, m_buf );
        if(data==NULL)
        {
            GLubyte* tmp = new GLubyte[m_datasize];
            memset(tmp, 128, m_datasize);

            glBufferData( m_T, m_datasize, tmp, m_DMode );
            delete[] tmp; 
        }
        else glBufferData( m_T, m_datasize, data, m_DMode );
	
        glGetBufferParameteriv( m_T, GL_BUFFER_SIZE_ARB, &nParam_ArrayObjectSize );
        if( nParam_ArrayObjectSize <= 0 )
        {
            fprintf(stderr, "PBO: init buffer object failed\n");
            exit(1);
        }
        else printf("BUFFER OBJECT %d: init buffer size :%d\n", 
                    m_buf, nParam_ArrayObjectSize);
        //assert(nParam_ArrayObjectSize == m_datasize);
        glBindBuffer( m_T, 0);
    }
}


void buffer_object::clear()
{
    bind(m_T);
    glBufferData(m_T, m_datasize, NULL, m_DMode);
    unbind();
}

//if bind to another target, update m_T as well
void buffer_object::bind(GLuint target)
{
    m_T = target;
    glBindBuffer(m_T, m_buf);
}


void buffer_object::bind()
{
    glBindBuffer(m_T, m_buf);
}


void buffer_object::unbind()
{
    glBindBuffer(m_T, 0);
}


void* buffer_object::map(GLuint mode)
{
    m_MMode = mode;
    glBindBuffer(m_T, m_buf);

    void* tmp = glMapBuffer(m_T, m_MMode);
    if( printOpenGLError() )
    {
        std::cout << __FILE__ << ": " << __func__ <<"()\n";
        exit(1);
    }
	
    glBindBuffer(m_T, 0);
    return tmp;
}

void buffer_object::unmap()
{
    glBindBuffer(m_T, m_buf);
    glUnmapBuffer(m_T);
    glBindBuffer(m_T, 0);
    //printOpenGLError();    
    if( printOpenGLError() )
    {
        std::cout << __FILE__ << ": " << __func__ <<"()\n";
        exit(1);
    }

}


void buffer_object::send_data(char* data, int offset, int size)
{
    bind(GL_ARRAY_BUFFER_ARB);
    glBufferSubData( m_T, offset, size, data);
    unbind();
}


#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void buffer_object::fb2pbo(int x, int y, int w, int h, GLenum mode)
{
    glReadBuffer(GL_BACK);
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_buf);
    glReadPixels(x, y, w, h, mode, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));	
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

void buffer_object::fbo2pbo(fbo_unit* f, int id, 
                            int x, int y, int w, int h, int offset,
                            GLenum mode )
{
    //assume fbo is already bound
    //if id >=0, read back color buffer
    //if you want to read depth buffer, need to set id==-1, glReadBuffer 
    //to "GL_DEPTH_COMPONENT".
    if(id == -1)
    {
        if(m_component == 1) assert(mode == GL_DEPTH_COMPONENT);
        else exit(-1);
    }
    //assert(id >= 0);
    if(id >= 0)
    {
        if(m_component == 3) assert(mode == GL_RGB);
        else if(m_component == 4) assert(mode == GL_RGB || mode == GL_RGBA);
        else exit(-1);
    }

    if(id >= 0) f->read_colorbuffer(id);
    if(id == -1) f->read_depthbuffer();
    //printOpenGLError();
	
    // glReadPixels() should return immediately.
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_buf);
   
    
    glReadPixels(x, y, w, h, mode, m_dataType, 
                      BUFFER_OFFSET(offset));
	
    if( printOpenGLError() )
    {
        if(m_dataType == GL_FLOAT) fprintf(stderr, "Type is GL_FLOAT\n");
        else fprintf(stderr, "Type is not GL_FLOAT\n");
		
        fprintf(stderr, "readpixel error!(%d, %d)\n", w, h);
        exit(10);
    }
	
    //glReadBuffer(GL_NONE);
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
}



//-----------------------------------------------------------------------------
//bind buffer and set pointers
//set the buffer binding target explicitly
//because VBO can be bind as PBO somewhere else and 
//the member binding target would be changed.
//-----------------------------------------------------------------------------

void buffer_object::set_vertex_pointer(int components, GLuint Type)
{
    bind(GL_ARRAY_BUFFER_ARB);
    glVertexPointer(components, Type, 0, BUFFER_OFFSET(0));
    //unbind();
    printOpenGLError();
}

void buffer_object::set_texcoord_pointer(int components, GLuint Type)
{
    bind(GL_ARRAY_BUFFER_ARB);
    glTexCoordPointer(components, Type, 0, BUFFER_OFFSET(0));
    //unbind();
    printOpenGLError();
}

void buffer_object::set_color_pointer(int components, GLuint Type)
{
    bind(GL_ARRAY_BUFFER_ARB);
    glColorPointer(components, Type, 0, BUFFER_OFFSET(0));
    //unbind();
    printOpenGLError();
}

void buffer_object::set_normal_pointer(GLuint Type)
{
    bind(GL_ARRAY_BUFFER_ARB);
    glNormalPointer(Type, 0, BUFFER_OFFSET(0));
    //unbind();
    printOpenGLError();
}

/*
//seems not needed 
void buffer_object::set_index_pointer(GLuint Type)
{
    bind(GL_ELEMENT_ARRAY_BUFFER_ARB);
    glIndexPointer(Type, 0, BUFFER_OFFSET(0));
    //unbind();
    printOpenGLError();
}
*/

void buffer_object::set_attrib_pointer(int index, int components, GLuint Type)
{
    bind(GL_ARRAY_BUFFER_ARB);
    glVertexAttribPointer(index, components, Type, GL_FALSE, 0, BUFFER_OFFSET(0));
    //unbind();
    printOpenGLError();
}
