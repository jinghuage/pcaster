#ifndef SHADER_GLSL_H_
#define SHADER_GLSL_H_

#include "glheaders.h"

class shader_object
{
private:
    //GLuint v, f, g, p;
    GLhandleARB v, f, g, p;
	
public:
	shader_object();
	~shader_object();
	void init_from_txt(const char *vs, const char *fs);
	void init_from_file(const char *vsfile, const char *fsfile);
	GLint getUniformLocation(const GLchar *name);
	void bindAttribLocation(int index, const GLchar* name);
	void use();
	void cleanup();

private:
	void setShader(const char *vs, const char *fs);
//	void setShader(const char *vs, const char *fs, const char* gs); 
//	void setup_geometry_shader();
	void printShaderInfoLog(GLhandleARB);
	void printProgramInfoLog(GLhandleARB);
};

#endif /*SHADER_GLSL_H_*/
