//vbo and pbo

#include "buffer_object.h"
#include "misc.h"

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
// 	{GL_LUMINANCE,		1},		//5
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
}

buffer_object::~buffer_object()
{
}


// void buffer_object::init(int target, int num, GLuint format, GLuint type, int mode)
// {
// 	glGenBuffers( 1, &buf );
	
// 	T = bind_target[target];
// 	DMode = draw_mode[mode];
	
// 	Format = data_format[format][0];
// 	Type = data_type[type][0];
// 	Stride = data_format[format][1] * data_type[type][1];
	
// 	int datasize = num * Stride;	

// 	GLint nParam_ArrayObjectSize;
	
// 	if(datasize > 0)
// 	{
// 		glBindBuffer( T, buf );
// 		glBufferData( T, datasize, NULL, DMode );
	
// 		glGetBufferParameteriv( T, GL_BUFFER_SIZE_ARB, &nParam_ArrayObjectSize );
// 		if( nParam_ArrayObjectSize <= 0 )
// 		{
// 			fprintf(stderr, "init buffer object failed\n");
// 			exit(1);
// 		}
// 		glBindBuffer( T, 0);
// 	}
// }


void buffer_object::init(GLuint target, GLuint num, GLuint mode, GLuint stride)
{
	glGenBuffers( 1, &m_buf );
	
	m_T = target;
	m_DMode = mode;
	
//	Stride = stride; //data_format[format][1] * data_type[type][1]
	
	m_datasize = num * stride;	

	GLint nParam_ArrayObjectSize;
	
	if(m_datasize > 0)
	{
		glBindBuffer( m_T, m_buf );
		glBufferData( m_T, m_datasize, NULL, m_DMode );
	
		glGetBufferParameteriv( m_T, GL_BUFFER_SIZE_ARB, &nParam_ArrayObjectSize );
		if( nParam_ArrayObjectSize <= 0 )
		{
			fprintf(stderr, "init buffer object failed\n");
			exit(1);
		}
                else printf("init buffer size :%d\n", nParam_ArrayObjectSize);
		glBindBuffer( m_T, 0);
	}
}

void buffer_object::clear()
{
    bind(m_T);
    glBufferData(m_T, m_datasize, NULL, m_DMode);
    unbind(m_T);
}


void buffer_object::bind(GLuint target)
{
	m_T = target;
	glBindBuffer(m_T, m_buf);
}

void buffer_object::unbind(GLuint target)
{
	m_T = target;
	glBindBuffer(m_T, 0);
}


void* buffer_object::map(GLuint mode)
{
    m_MMode = mode;
    glBindBuffer(m_T, m_buf);

    void* tmp = glMapBuffer(m_T, m_MMode);
    printOpenGLError();
	
    glBindBuffer(m_T, 0);
    return tmp;
}

void buffer_object::unmap()
{
    glBindBuffer(m_T, m_buf);
    glUnmapBuffer(m_T);
    glBindBuffer(m_T, 0);
    printOpenGLError();
}


void buffer_object::send_data(char* data, int offset, int size)
{
	glBufferSubData( m_T, offset, size, data);
}


#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void buffer_object::fb2pbo(int w, int h)
{
	//bind(2);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_buf);
	
	// Read the pixel data from framebuffer to pbo
// glReadPixels() should return immediately.
//	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, 0);
//             BUFFER_OFFSET(0));
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

void buffer_object::fbo2pbo(fbo_unit* f, int id, int w, int h)
{
	//Requesting "GL_DEPTH_COMPONENT" as the external format magically triggers OpenGL to read 
	//the depth buffer/attachment instead of the color buffer set by glReadBuffer.
	
	if(id >= 0) f->read_colorbuffer(id);
	//printOpenGLError();
	
        // glReadPixels() should return immediately.
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_buf);
	glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, 
                     BUFFER_OFFSET(0));
	
        if( printOpenGLError() )
	{
// 		if(Format == GL_RGBA)
// 			fprintf(stderr, "Format is RGBA\n");
// 		else if (Format == GL_DEPTH_COMPONENT) fprintf(stderr, "Format is DEPTH\n");
// 		else fprintf(stderr, "Format is not RGBA or DEPTH\n");
		
// 		if(Type == GL_FLOAT) fprintf(stderr, "Type is GL_FLOAT\n");
// 		else fprintf(stderr, "Type is not GL_FLOAT\n");
		
		fprintf(stderr, "readpixel error!(%d, %d)\n", w, h);
		exit(10);
	}
	
	glReadBuffer(GL_NONE);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
}


// void buffer_object::vbo_vertex_array(int components)
// {
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, buf);
// 	glVertexPointer(components, Type, Stride, BUFFER_OFFSET(0));
// 	//glVertexPointer(components, Type, 0, BUFFER_OFFSET(0));
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
// 	printOpenGLError();
// }

// void buffer_object::vbo_color_array(int components)
// {
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, buf);
// 	glColorPointer(components, Type, Stride, BUFFER_OFFSET(0));
// 	//glColorPointer(components, Type, 0, BUFFER_OFFSET(0));
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
// 	printOpenGLError();
// }

// void buffer_object::vbo_normal_array()
// {
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, buf);
// 	glNormalPointer(Type, Stride, BUFFER_OFFSET(0));
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
// 	printOpenGLError();
// }

// void buffer_object::vbo_attrib_array(int index, int components)
// {
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, buf);
// 	glVertexAttribPointer(index, components, Type, GL_FALSE, Stride, BUFFER_OFFSET(0));
// 	glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
// 	printOpenGLError();
// }
