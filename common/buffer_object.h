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
	GLuint m_dataType;
        GLuint m_component;

	GLenum m_DMode;
	GLenum m_MMode;
	
public:
	buffer_object();
	~buffer_object();
	
        GLuint get_buffer_id() { return m_buf; }
	
	void init(GLuint target, GLuint num, GLuint mode, GLuint comp, 
                  GLuint type, GLvoid *data=NULL);
	void bind(GLuint target);
        void bind();
	void unbind();
        void clear();
	void* map(GLuint mode);
	void unmap();
	void send_data(char* data, int offset, int size);
	
	void fb2pbo(int, int, int w, int h, GLenum mode=GL_RGBA);
	void fbo2pbo(fbo_unit* f, int id, int, int, int w, int h, 
                     int, GLenum mode=GL_RGBA);
	
	void set_vertex_pointer(int, GLuint);
	void set_color_pointer(int, GLuint);
        void set_texcoord_pointer(int, GLuint);
	void set_normal_pointer(GLuint);
        void set_index_pointer(GLuint);
	void set_attrib_pointer(int index, int components, GLuint);
};

#endif /*BUFFER_OBJECT_H_*/
