#ifndef BUFFER_OBJECT_H_
#define BUFFER_OBJECT_H_

#include "glheaders.h"
#include "texture.h"
#include "fbo.h"

class buffer_object
{
private:
	GLuint m_buf;
	GLenum m_T;
	
	GLuint m_datasize;
	
	GLenum m_DMode;
	GLenum m_MMode;
	
public:
	buffer_object();
	~buffer_object();
	
	
	void init(GLuint target, GLuint num, GLuint mode, GLuint stride);
	void bind(GLuint target);
	void unbind(GLuint target);
        void clear();
	void* map(GLuint mode);
	void unmap();
	void send_data(char* data, int offset, int size);
	
	void fb2pbo(int w, int h);
	void fbo2pbo(fbo_unit* f, int id, int w, int h);
	
/* 	void vbo_vertex_array(int); */
/* 	void vbo_color_array(int); */
/* 	void vbo_normal_array(); */
/* 	void vbo_attrib_array(int index, int components); */
};

#endif /*BUFFER_OBJECT_H_*/
