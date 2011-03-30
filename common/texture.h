#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "glheaders.h"


class tex_unit
{
private:
	GLuint m_texture_id;
	GLsizei m_width, m_height, m_depth;
	
	GLuint T;
	GLuint intFormat, extFormat;
	GLuint dataType;
        GLuint minfilter, magfilter;
	
	GLuint m_mipmaps;
	//GLubyte *m_pPixmap;
	//GLubyte *m_pCompressedPixmap;
	
public:
	tex_unit();
	~tex_unit();
	
        GLsizei get_width() {return m_width; }
        GLsizei get_height() {return m_height; }

	void setformat(GLuint, GLuint, GLuint, GLuint);
        void setfilter(GLuint, GLuint);
	void create(int w, int h, int b, GLvoid*); //2d
        void create(int w, int b, GLvoid*); //1d
        void create(int w, int h, int d, int b, GLvoid*); //3d
        void create(int w, int h, int b, GLvoid* p[6]); //cube
        void clear();
        //void switch_data(GLvoid*);
        void update_subimage(int, int, int, int, int, int, GLvoid*);
        void update_subimage(int, int, int, int, GLvoid*);
        void update_subimage(int, int, GLvoid*);
	void bind();
	void unbind();
	GLuint get_tex_id() {return m_texture_id;}
	//GLubyte *getCompressedPixmap(){return m_pCompressedPixmap;}
        void check_error_2d(int ox, int oy, int w, int h);

private:
	void setup_texture(GLvoid*);
	//void setup_compressed_texture(GLvoid*);
	//void compress_texture(int b, GLvoid* p);

};

#endif /*TEXTURE_H_*/
