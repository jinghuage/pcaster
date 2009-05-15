#ifndef HEADERS_H_
#define HEADERS_H_



#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glut.h>
#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/wglew.h>
#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glext.h>
#define glGetProcAddress(n) wglGetProcAddress(n)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif


//print gl info
#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line);
void print_matrix(GLdouble* mat);
void print_frustum(GLint* vp, GLdouble* mt, GLdouble* pt);
void getGlVersion( int *major, int *minor );

//some gl function wrapup
//void unproject(GLdouble* objpoint, GLint* vp, GLdouble* pt, GLdouble* mt, GLint x, GLint y, GLfloat z);
//void drawBox(float size, float cx, float cy, float cz);
void setup_ortho(float left, float right, float bottom, float top, int x, int y, int w, int h);
void release_ortho(int x, int y, int w, int h);

//gl extensions
int check_gl2_glew();
int check_shader_glew();
int check_shader_ext();
int check_fbo_ext();
int check_vbo_ext();
int check_pbo_ext();
int check_cubemap_ext();
int check_texlodbias_ext();
int check_multitex_ext();
int check_recttex_ext();
int check_comptex_ext();
int check_MRT_ext();

void init_glew_ext();


#endif /*HEADERS_H_*/
