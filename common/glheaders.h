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

/* #include "CL/cl.h" */
/* #include "CL/clext.h" */
/* //Extra CL/GL include */
/* #include "CL/cl_gl.h" */

//#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#endif

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/glew.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glext.h>
//#define glGetProcAddress(n) wglGetProcAddress(n)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
//#include <OpenGL/glu.h>
#include <GLUT/glut.h>
//#define glGetProcAddress(n) aglGetProcAddress(n)
#endif



//print gl info
#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(const char *file, int line);
void print_matrix(GLdouble* mat);
void print_frustum(GLint* vp, GLdouble* mt, GLdouble* pt);
void getGlVersion( int *major, int *minor );



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
void init_shader_ext();




#ifdef __HELP__

#define init_proc(t, n) if (!(n = (t) glGetProcAddress(#n))) return 0;


PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC            glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC           glCompileShaderARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB = NULL;
PFNGLATTACHOBJECTARBPROC            glAttachObjectARB = NULL;
PFNGLGETINFOLOGARBPROC              glGetInfoLogARB = NULL;
PFNGLLINKPROGRAMARBPROC             glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC        glUseProgramObjectARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB = NULL;
PFNGLUNIFORM1FARBPROC               glUniform1f = NULL;

void SetupGLSLProcs()
{
    init_proc(PFNGLCREATEPROGRAMOBJECTARBPROC, glCreateProgramObjectARB);
    init_proc(PFNGLCREATESHADEROBJECTARBPROC, glCreateShaderObjectARB);
}
#endif



#endif /*HEADERS_H_*/
