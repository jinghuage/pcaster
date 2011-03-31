//fbo.cpp
#include <iostream>
#include "misc.h"
#include "fbo.h"
#include "draw_routines.h"

static GLenum color_buffers[] = { 
    GL_COLOR_ATTACHMENT0_EXT, 
    GL_COLOR_ATTACHMENT1_EXT,
    GL_COLOR_ATTACHMENT2_EXT,
    GL_COLOR_ATTACHMENT3_EXT
};

fbo_unit::fbo_unit()
{
    colorbuf_num = 0;
    depth = 0;
    frame = 0;
	
    intFormat = GL_RGBA;
    extFormat = GL_RGBA;
    dataType = GL_UNSIGNED_BYTE;
    T = GL_TEXTURE_2D;
    minfilter = GL_LINEAR;
    magfilter = GL_LINEAR;
}

fbo_unit::~fbo_unit()
{
    if(frame)
    { 
        for(int i = 0; i < colorbuf_num; i++)
        {
            delete get_color_texture(i);
        }
        if(depth) delete depth;
        glDeleteFramebuffersEXT(1, &frame); frame = 0; 
        //glDeleteRenderbuffersEXT(1, &depth);
    }
}


//for a complete list of avaialbe format name, see texture.cpp
void fbo_unit::setformat(GLuint target, GLuint ifmt, GLuint efmt, GLuint type)
{
    T = target;
	
    intFormat = ifmt;
    extFormat = efmt;
	
    dataType = type;
}

void fbo_unit::setfilter(GLuint minf, GLuint magf)
{
    minfilter = minf;
    magfilter = magf;
}


void fbo_unit::init(int w, int h, int bufnum, 
                    bool depthbuffer, bool depthstencilbuffer,
                    GLvoid* p)
{
//     fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
//     std::cout << "colorbuf_num=" << bufnum << "\n";

    W = w;
    H = h;
	
    colorbuf_num = bufnum;
	


    /* Generate frame buffer and render buffer objects. */
    glGenFramebuffersEXT(1, &frame);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);


    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    
    /* Initialize the color render buffer. */
    for(int i = 0; i < colorbuf_num; i++)
    {
        colors[i] = new tex_unit;
        colors[i]->setformat(T, intFormat, extFormat, dataType);
        colors[i]->setfilter(minfilter, magfilter);
        colors[i]->create(w, h, 0, p);
        if(printOpenGLError()){
            printf("reminder: opengl error may appear before this line, try print gl error at previous gl calls as well\n");
        }
			    

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, color_buffers[i], 
                                  T, colors[i]->get_tex_id(), 0);
        if(printOpenGLError()){
            printf("reminder: opengl error may appear before this line, try print gl error at previous gl calls as well\n"); 
        }
    }
    //fprintf(stderr, "set %d color attachment for fbo\n", colorbuf_num);
    //if(colorbuf_num > 0) glDrawBuffers(colorbuf_num, color_buffers);
    //else glDrawBuffer(GL_NONE);
    
    if(depthbuffer)
    {

        depth = new tex_unit;
        depth->setformat(T, 
                         GL_DEPTH_COMPONENT24, 
                         GL_DEPTH_COMPONENT, 
                         GL_FLOAT);
        depth->create(w, h, 0, (GLvoid*)NULL);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
                                  GL_DEPTH_ATTACHMENT_EXT, 
                                  T, depth->get_tex_id(), 0);
        fprintf(stderr, "set depth attachment for fbo\n");
    }

    if(depthstencilbuffer)
    {

        depthstencil = new tex_unit;
        depthstencil->setformat(T, 
                                GL_DEPTH24_STENCIL8_EXT, 
                                GL_DEPTH_STENCIL_EXT, 
                                GL_UNSIGNED_INT_24_8_EXT);
        depthstencil->create(w, h, 0, (GLvoid*)NULL);


        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_DEPTH_ATTACHMENT_EXT, 
                                  T, depthstencil->get_tex_id(), 0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_STENCIL_ATTACHMENT_EXT, 
                                  T, depthstencil->get_tex_id(), 0);

        fprintf(stderr, "set depthstencil attachment for fbo\n");

    }

    if( printOpenGLError() ) exit(1);


    //check fbo status
    GLenum fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(fboStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        printFBOIncomplete(fboStatus);
        printf("\tFBO ERROR\n");
        exit(1);
    }

    //wrapup
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}



void fbo_unit::bind()
{
    //std::cout << "bind fbo frame:" << frame << std::endl;
   	
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);

//    if(colorbuf_num > 0) glDrawBuffers(colorbuf_num, color_buffers);
//    else glDrawBuffer(GL_NONE);

    if( printOpenGLError() ) exit(1);
}



void fbo_unit::unbind()
{
    //std::cout << "unbind fbo frame:" << frame << std::endl;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    if( printOpenGLError() )    exit(1);

}


void fbo_unit::read_colorbuffer(int id)
{
    glReadBuffer(color_buffers[id]);
}


void fbo_unit::read_depthbuffer()
{
    glReadBuffer(GL_DEPTH_COMPONENT);
}


void fbo_unit::printFBOIncomplete(GLenum err)
{
    switch(err){
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        printf("FBO Unsupported Format"); 
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        printf("FBO Attachment Incomplete");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        printf("FBO Missing Attachment");
        break;
        //case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
        //printf("FBO Duplicate Attachment");
        //break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        printf("FBO Dimensions not the same");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        printf("FBO Formats not the same");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        printf("FBO Missing Draw Buffer");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        printf("FBO Missing Read Buffer");
    default:    printf("UNKNOWN FBO ERROR %x", err);
    }
}


//call this function if your fbo need completely different
// projection and modoelview matrix
//otherwise will share the same matrix stack as the main program
void fbo_unit::setup_transformation(float ratio)
{
    
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Set the correct perspective.
    //float ratio = 0.1*wid/hei;
    glFrustum(-ratio, ratio, -0.1, 0.1, 0.1, 100);
    //glOrtho(-1, 1, -1, 1, -100, 100);
	
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}


void fbo_unit::restore_transformation()
{

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}


