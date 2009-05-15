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
	cleanup();
}


void shader_object::init_from_file(const char* vsfile, const char* fsfile)
{
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

void shader_object::init_from_txt(const char* vs, const char* fs)
{
	setShader(vs, fs);
}

void shader_object::setShader(const char *vs, const char *fs) 
{

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);	
	

	glShaderSource(v, 1, &vs, NULL);
	glShaderSource(f, 1, &fs, NULL);
	

	glCompileShader(v);
	fprintf(stderr, "compile vertex shader:");
	printShaderInfoLog(v);
	
	glCompileShader(f);
	fprintf(stderr, "compile fragment shader:");
	printShaderInfoLog(f);
	


	p = glCreateProgram();
	
	glAttachShader(p, v);
	glAttachShader(p, f);

	glLinkProgram(p);
	fprintf(stderr, "link program:");
	printProgramInfoLog(p);
	
	//glUseProgram(p);
}

void shader_object::setShader(const char *vs, const char *fs, const char *gs) 
{

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);	
	
	if(gs) g = glCreateShader(GL_GEOMETRY_SHADER_EXT);

	glShaderSource(v, 1, &vs, NULL);
	glShaderSource(f, 1, &fs, NULL);
	
	if(g)
	{
		glShaderSource(g, 1, &gs, NULL);
	}

	glCompileShader(v);
	fprintf(stderr, "compile vertex shader\n");
	printShaderInfoLog(v);
	
	glCompileShader(f);
	fprintf(stderr, "compile fragment shader\n");
	printShaderInfoLog(f);
	
	if(g)
	{
		glCompileShader(g);
		fprintf(stderr, "compile geometry shader\n");
		printShaderInfoLog(g);
	}

	p = glCreateProgram();
	
	glAttachShader(p, v);
	glAttachShader(p, f);
	if(g) {glAttachShader(p, g); 		setup_geometry_shader();}

	glLinkProgram(p);
	fprintf(stderr, "link program\n");
	printProgramInfoLog(p);
	
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


void shader_object::cleanup()
{
    glDeleteShader( v );
    glDeleteShader( f );
    glDeleteProgram( p );
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
        printf("No such uniform named \"%s\"\n", name);

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

//after shader create, before program link
void shader_object::setup_geometry_shader()
{
	
	
//Get max number of geometry shader output vertices
   GLint n;
   glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &n);
   std::cout<<"Max GS output vertices:"<< n <<"\n";
   //glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &n); 
   //std::cout<<"Max GS output components: "<< n <<"\n";
   
   ////Setup Geometry Shader////
   //Set POINTS primitives as INPUT
   glProgramParameteriEXT(p, GL_GEOMETRY_INPUT_TYPE_EXT , GL_POINTS );
   //Set TRIANGLE STRIP as OUTPUT
   glProgramParameteriEXT(p ,GL_GEOMETRY_OUTPUT_TYPE_EXT , GL_TRIANGLE_STRIP);
   //Set maximum number of vertices to be generated by Geometry Shader to 16
   //16 is the maximum number of vertices a marching cube configuration can own
   //This parameter is very important and have an important impact on Shader performances
   //Its value must be chosen closer as possible to real maximum number of vertices
   //glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&temp);
   glProgramParameteriEXT(p ,GL_GEOMETRY_VERTICES_OUT_EXT, 16);
}


/*
//----------------------------------------------------------------------
//function arb version. obsolete, for reference only
//----------------------------------------------------------------------
static void setShaders_arbext(GLhandleARB* p,
                     GLhandleARB* v,
                     GLhandleARB* f,
                     const char *vsfile, const char *fsfile) ; 
static void printInfoLog_arbext(GLhandleARB obj);
static void cleanup_arbext( GLhandleARB p, GLhandleARB v, GLhandleARB f );

static void setShaders_arbext(GLhandleARB* p,
                     GLhandleARB* v,
                     GLhandleARB* f,
                     const char *vsfile, const char *fsfile) 
{

	char *vs,*fs;

	*v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	*f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	

	vs = textFileRead(const_cast<char*>(vsfile));
	fs = textFileRead(const_cast<char*>(fsfile));

	const char * vv = vs;
	const char * ff = fs;

	glShaderSourceARB(*v, 1, &vv,NULL);
	glShaderSourceARB(*f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShaderARB(*v);
	fprintf(stderr, "compile vertex shader\n");
	printInfoLog_arbext(*v);
	glCompileShaderARB(*f);
	fprintf(stderr, "compile fragment shader\n");
	printInfoLog_arbext(*f);

	*p = glCreateProgramObjectARB();
	
	glAttachObjectARB(*p, *v);
	glAttachObjectARB(*p, *f);

	glLinkProgramARB(*p);
	fprintf(stderr, "link program\n");
	printInfoLog_arbext(*p);
	
	//glUseProgramObjectARB(*p);
}



static void printInfoLog_arbext(GLhandleARB obj)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;

    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
					 &infologLength);

    if (infologLength > 0)
    {
	infoLog = (char *)malloc(infologLength);
	glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
	printf("%s\n",infoLog);
	free(infoLog);
    }
}



static void cleanup_arbext( GLhandleARB p, GLhandleARB v, GLhandleARB f )
{
    glDeleteObjectARB( v );
    glDeleteObjectARB( f );
    glDeleteObjectARB( p );
}

*/
