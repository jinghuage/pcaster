//shader.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "shader.h"


//------------------------
//textfile
//------------------------
char *textFileRead(const char *fn) 
{
	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"r");

		if (fp != NULL) {
      
		      fseek(fp, 0, SEEK_END);
		      count = ftell(fp);
		      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			else fprintf(stderr, "no content in file\n");
			fclose(fp);
		}
		else fprintf(stderr, "file %s can't be opened\n", fn);
	}
	return content;
}


int textFileWrite(const char *fn, const char *s) {

	FILE *fp;
	int status = 0;

	if (fn != NULL) {
		fp = fopen(fn,"w");

		if (fp != NULL) {
			
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}



shader_object::shader_object()
{
	v = f = g = p = 0;
}

shader_object::~shader_object()
{
    if(v) glDeleteShader( v );
    if(f) glDeleteShader( f );
    if(g) glDeleteShader( g );
    if(p) glDeleteProgram( p );
}


void shader_object::init_from_file(const char* vsfile, const char* fsfile)
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    char *vs = textFileRead(vsfile);
    char *fs = textFileRead(fsfile);
	
    if(vs==0 || fs==0){
        fprintf(stderr, "no shader source available\n");
        exit(0);
    }

    const char * vv = vs;
    const char * ff = fs;
	
    setShader(vv, ff);
	
    free(vs);
    free(fs);
}


void shader_object::init_from_file(const char* vsfile, const char* fsfile, 
                                   const char* gsfile, GLuint itype, GLuint otype,
                                   GLint osize)
{
    char *vs = textFileRead(vsfile);
    char *fs = textFileRead(fsfile);
    char *gs = textFileRead(gsfile);
	
    if(vs==0 || fs==0 || gs==0){
        fprintf(stderr, "no shader source available\n");
        exit(0);
    }

    const char * vv = vs;
    const char * ff = fs;
    const char * gg = gs;
	
    setShader(vv, ff, gg, itype, otype, osize);
	
    free(vs);
    free(fs);
    free(gs);
}


void shader_object::init_from_txt(const char* vs, const char* fs)
{
    setShader(vs, fs);
}

void shader_object::setShader(const char *vs, const char *fs) 
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);	
	

    //printf("set vertex shader source\n");
    glShaderSource(v, 1, &vs, NULL);

    //printf("set fragment shader source\n");
    glShaderSource(f, 1, &fs, NULL);
	

    //printf("compile vertex shader\n");
    glCompileShader(v);
    GLint params;
    glGetShaderiv(v, GL_COMPILE_STATUS, &params);
    if(params == GL_FALSE)
    {
	fprintf(stderr, "compile vertex shader error:\n");
	printShaderInfoLog(v);
        exit(10);
    }
	

    //printf("compile fragment shader\n");
    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &params);
    if(params == GL_FALSE)
    {
	fprintf(stderr, "compile fragment shader error:\n");
	printShaderInfoLog(f);
        exit(10);
    }
	
    //printf("create program\n");
    p = glCreateProgram();
	
    //printf("attach shaders to program\n");
    glAttachShader(p, v);
    glAttachShader(p, f);


    //printf("link program\n");
    glLinkProgram(p);
    glGetProgramiv(p, GL_LINK_STATUS, &params);
    if(params == GL_FALSE)
    {
	fprintf(stderr, "link program error:\n");
	printProgramInfoLog(p);
        exit(10);
    }
    //glUseProgram(p);
}



void shader_object::setShader(const char *vs, const char *fs, const char *gs,
                              GLuint itype, GLuint otype, GLint osize) 
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);	
	
    if(gs) g = glCreateShader(GL_GEOMETRY_SHADER_EXT);

    glShaderSource(v, 1, &vs, NULL);
    glShaderSource(f, 1, &fs, NULL);
	
    if(g)
    {
        glShaderSource(g, 1, &gs, NULL);
    }


    //printf("compile vertex shader\n");
    glCompileShader(v);
    GLint params;
    glGetShaderiv(v, GL_COMPILE_STATUS, &params);
    if(params == GL_FALSE)
    {
	fprintf(stderr, "compile vertex shader error:\n");
	printShaderInfoLog(v);
        exit(10);
    }

    //printf("compile fragment shader\n");
    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &params);
    if(params == GL_FALSE)
    {
	fprintf(stderr, "compile fragment shader error:\n");
	printShaderInfoLog(f);
        exit(10);
    }
	
    if(g)
    {
        //printf("compile geometry shader\n");
        glCompileShader(g);
        glGetShaderiv(g, GL_COMPILE_STATUS, &params);
        if(params == GL_FALSE)
        {
            fprintf(stderr, "compile geometry shader error:\n");
            printShaderInfoLog(g);
            exit(10);
        }
    }

    //printf("create program\n");
    p = glCreateProgram();
	
    //printf("attach shaders to program\n");
    glAttachShader(p, v);
    glAttachShader(p, f);
    if(g) 
    {
        glAttachShader(p, g);
        setup_geometry_shader(itype, otype, osize);
    }


    //printf("link program\n");
    glLinkProgram(p);
    glGetProgramiv(p, GL_LINK_STATUS, &params);
    if(params == GL_FALSE)
    {
	fprintf(stderr, "link program error:\n");
	printProgramInfoLog(p);
        exit(10);
    }

    if( printOpenGLError() )    exit(1);	
    //glUseProgram(p);
}



void shader_object::printShaderInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

void shader_object::printProgramInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}


void shader_object::use()
{
    glUseProgram(p);
}


GLint shader_object::getUniformLocation(const GLchar* name)
{
    GLint loc;
    loc = glGetUniformLocation(p, name);

    if (loc == -1)
        printf("No such uniform named \"%s\", also maybe this uniform is not used in the shader\n", name);

    //printOpenGLError();  // Check for OpenGL errors
    
    return loc;  
}

void shader_object::bindAttribLocation(int index, const GLchar* name)
{
    glBindAttribLocation(p, index, name);
}

//exampe geometry shader: 
/*
  #version 120 
  #extension GL_EXT_geometry_shader4 : enable

  void main(void)
  {
  mat4x4 bezierBasis=mat4x4( 1, -3, 3, -1, 0, 3, -6, 3 , 0, 0, 3, -3 , 0, 0, 0, 1);
  for(int i=0; i<64; i++) 
  {
  float t = i / (64.0-1.0);
  vec4 tvec= vec4(1, t, t*t, t*t*t);
  vec4 b =tvec*bezierBasis;
  vec4 p = gl_PositionIn[0]*b.x+ gl_PositionIn[1]*b.y+ gl_PositionIn[2]*b.z+ gl_PositionIn[3]*b.w;
  gl_Position =p;
  EmitVertex();
  }
  EndPrimitive();
  }
*/

//-----------------------------------------------------------------------------
//supported geometry input type: 
//GL_POINTS, GL_LINES, GL_TRIANGLES,
//GL_LINES_ADJACENCY_EXT, GL_TRIANGLES_ADJACENCY_EXT
//
//supported geometry output type: 
//GL_POINTS, GL_LINE_STRIP, GL_TRIANGLE_STRIP, 
//
//after shader create, before program link
//-----------------------------------------------------------------------------

void shader_object::setup_geometry_shader(GLuint itype, 
                                          GLuint otype, 
                                          GLint osize)
{	
    //Get max number of geometry shader output vertices
    GLint n;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &n);
    //std::cout<<"Max GS output vertices:"<< n <<"\n";
    if(osize > n) osize = n;


    glProgramParameteriEXT(p, GL_GEOMETRY_INPUT_TYPE_EXT , itype );
    glProgramParameteriEXT(p, GL_GEOMETRY_OUTPUT_TYPE_EXT , otype);
    glProgramParameteriEXT(p, GL_GEOMETRY_VERTICES_OUT_EXT, osize);

    if( printOpenGLError() )    exit(1); 
}

