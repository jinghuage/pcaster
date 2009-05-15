#ifndef FBO_H_
#define FBO_H_

#include "glheaders.h"
#include "texture.h"

#define MAX_BUFFER_NUM 4

class fbo_unit
{
private:
	GLuint frame;
	int W, H;
	
	//attachment textures
	tex_unit* colors[MAX_BUFFER_NUM];	
	tex_unit* depth;
	
	GLuint T;
	GLuint intFormat, extFormat;
	GLuint dataType;
	
	int colorbuf_num;

public:
	fbo_unit();
	~fbo_unit();
	void setformat(GLuint, GLuint, GLuint, GLuint);
	void init(int, int, int, bool);
	void printFBOIncomplete(GLenum err);
	void bind();
	void unbind();
        void setup_transformation(float);
        void restore_transformation();
	tex_unit* get_depth_texture(){ return depth;}
	tex_unit* get_color_texture(int id){return colors[id];}
	void read_colorbuffer(int);
};




#endif /*FBO_H_*/
