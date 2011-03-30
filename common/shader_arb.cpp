//shader.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "shader_arb.h"


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

	v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	

	glShaderSourceARB(v, 1, &vs, NULL);
	glShaderSourceARB(f, 1, &fs, NULL);

	glCompileShaderARB(v);
	fprintf(stderr, "compile vertex shader\n");
	printShaderInfoLog(v);

	glCompileShaderARB(f);
	fprintf(stderr, "compile fragment shader\n");
	printShaderInfoLog(f);

	p = glCreateProgramObjectARB();
	
	glAttachObjectARB(p, v);
	glAttachObjectARB(p, f);

	glLinkProgramARB(p);
	fprintf(stderr, "link program\n");
	printProgramInfoLog(p);
	
	//glUseProgramObjectARB(p);
}



void shader_object::printShaderInfoLog(GLhandleARB obj)
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

void shader_object::printProgramInfoLog(GLhandleARB obj)
{
    printShaderInfoLog(obj);
}


void shader_object::cleanup( )
{
    glDeleteObjectARB( v );
    glDeleteObjectARB( f );
    glDeleteObjectARB( p );
}

void shader_object::use()
{
	glUseProgramObjectARB(p);
}


GLint shader_object::getUniformLocation(const GLchar* name)
{
	GLint loc;
    loc = glGetUniformLocationARB(p, name);

    if (loc == -1)
        printf("No such uniform named \"%s\"\n", name);

    //printOpenGLError();  // Check for OpenGL errors
    
    return loc;  
}

void shader_object::bindAttribLocation(int index, const GLchar* name)
{
	glBindAttribLocationARB(p, index, name);
}



