
#include <string.h>
#include "glheaders.h"



//------------------------
//print info
//------------------------

void getGlVersion( int *major, int *minor )
{
    const char* verstr = (const char*)glGetString( GL_VERSION );
    if( (verstr == NULL) || (sscanf( verstr, "%d.%d", major, minor ) != 2) )
    {
        *major = *minor = 0;
        fprintf( stderr, "Invalid GL_VERSION format!!!\n" );
    }
    else
    {
    	fprintf(stderr, "OPENGL%d.%d\n", *major, *minor);
    }
}


int printOglError(const char *file, int line)
{
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        //printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        printf("glError in file %s @ line %d\n", file, line);
        retCode = 1;

        switch (glErr)
        {
        case GL_INVALID_ENUM: printf("Invalid enumerant\n"); break;
        case GL_INVALID_VALUE: printf("Invalid value\n"); break;
        case GL_INVALID_OPERATION: printf("Invalid operation\n"); break;
        case GL_STACK_OVERFLOW: printf("Stack overflow\n"); break;
        case GL_OUT_OF_MEMORY: printf("Out of memory\n"); break;
        case GL_TABLE_TOO_LARGE: printf("Table too large\n"); break;
        }
        glErr = glGetError();
    }
    return retCode;
}



void print_matrix(GLdouble* mat)
{
	fprintf(stderr, "%f %f %f %f\n", mat[0], mat[4], mat[8], mat[12]);
	fprintf(stderr, "%f %f %f %f\n", mat[1], mat[5], mat[9], mat[13]);
	fprintf(stderr, "%f %f %f %f\n", mat[2], mat[6], mat[10], mat[14]);
	fprintf(stderr, "%f %f %f %f\n", mat[3], mat[7], mat[11], mat[15]);
}

void print_frustum(GLint* vp, GLdouble* mt, GLdouble* pt)
{
	printf("viewport:\n");
	printf("%d, %d, %d, %d\n", vp[0], vp[1], vp[2], vp[3]);

	printf("modelview matrix:\n");
	print_matrix(mt);

	printf("projection matrix:\n");
	print_matrix(pt);
}



//---------------------
//check extension	
//---------------------
#ifndef __APPLE__

int check_gl2()
{
    int major, minor;
    getGlVersion(&major, &minor);

    if (major < 2)
    {
	printf("OpenGL 2.0 not supported\n");
	exit(0);
    }
    return 0;
}

int check_shader_glew()
{
//!!please make sure glew is inited before calling this function

//     if (GL_ARB_vertex_shader && 
//         GL_ARB_fragment_shader && 
//         GL_EXT_geometry_shader4)
//     {
// 	//printf("glew: Ready for GLSL\n");
//         return 0;
//     }
//     else {
// 	printf("glew: not GL_ARB_vertex_shader supported \n");
// 	exit(0);
//     }


//     if (glewIsSupported("GL_VERSION_2_1"))
//         printf("Ready for OpenGL 2.1\n");
//     else {
//         printf("OpenGL 2.1 not supported\n");
//         exit(1);
//     }

    if (GLEW_ARB_vertex_shader && 
        GLEW_ARB_fragment_shader && 
        GL_EXT_geometry_shader4)
    {
        return 0;
        //printf("Ready for GLSL - vertex, fragment, and geometry units\n");
    }
    else {
        printf("GLEW: No GLEW_ARB_vertex_shader supported");
        exit(0);
    }

    //return 0;
}

#endif

static int check_ext(const char *needle)
{
    const GLubyte *haystack, *c;

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return 1;
    }

    fprintf(stderr, "Missing OpenGL extension %s\n", needle);
    return 0;
}



int check_shader_ext()
{
    if (check_ext("ARB_shading_language_100") &&
	check_ext("ARB_vertex_shader")   &&
        check_ext("ARB_shader_objects")  &&
        check_ext("ARB_fragment_shader") ) 
    {
        //printf("GL: ready for glsl\n");
    	return 0;
    }
    else
    {
        printf("GL: not ready for glsl\n");
        exit(0);
    }
}

int check_recttex_ext()
{
    return check_ext("ARB_texture_rectangle");
}

int check_fbo_ext()
{
    if( check_ext("EXT_framebuffer_object") )
    {
        return 1;
    }
    return 0;
}

int check_vbo_ext()
{
	if( check_ext("ARB_vertex_buffer_object"))
	{
	    return 1;
	}
	else return 0;
}

int check_pbo_ext()
{
	return check_ext("ARB_pixel_buffer_object");
}

int check_multitex_ext()
{
	if(check_ext("ARB_multitexture"))
	{
	    return 1;
	}
	return 0;
}

int check_cubemap_ext()
{
	return check_ext("ARB_texture_cube_map");
}

int check_texlodbias_ext()
{
	return check_ext("EXT_texture_lod_bias");
}

int check_comptex_ext()
{
	return check_ext("ARB_texture_compression");
}

//MRT: Multiple Render Target
int check_MRT_ext()
{
	return check_ext("ARB_draw_buffers");
}

void init_glew_ext()
{	
#ifndef __APPLE__
    //glewInit();

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(1);
    }

    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    check_shader_glew();
    check_shader_ext();
    check_gl2();

#endif
}
