//fbo.cpp

#include "fbo.h"

static GLenum color_buffers[] = { 
    GL_COLOR_ATTACHMENT0_EXT, 
    GL_COLOR_ATTACHMENT1_EXT,
    GL_COLOR_ATTACHMENT2_EXT,
    GL_COLOR_ATTACHMENT3_EXT
};

fbo_unit::fbo_unit()
{
    colorbuf_num = 0;
	
    intFormat = GL_RGBA;
    extFormat = GL_RGBA;
    dataType = GL_UNSIGNED_BYTE;
    T = GL_TEXTURE_2D;
}

fbo_unit::~fbo_unit()
{
}


//for a complete list of avaialbe format name, see texture.cpp
void fbo_unit::setformat(GLuint target, GLuint ifmt, GLuint efmt, GLuint type)
{
    T = target;
	
    intFormat = ifmt;
    extFormat = efmt;
	
    dataType = type;
}

void fbo_unit::init(int w, int h, int bufnum, bool depthbuffer)
{
    W = w;
    H = h;
	
    colorbuf_num = bufnum;
    bool fbo_setup_error = false;
	
    /* Generate frame buffer and render buffer objects. */
    glGenFramebuffersEXT(1, &frame);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);
    
    /* Initialize the color render buffer. */
    for(int i = 0; i < colorbuf_num; i++)
    {
        colors[i] = new tex_unit;
        colors[i]->setformat(T, intFormat, extFormat, dataType);
        colors[i]->create(w, h, 0, NULL);
        if(printOpenGLError()){
            printf("end at fbo_unit::init:create colormap\n");
            exit(1);
        }
			    
        fprintf(stderr, "fbo colorattach%d texid: %d\n", i, colors[i]->get_tex());
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, color_buffers[i], T, colors[i]->get_tex(), 0);
        if(printOpenGLError()){
            printf("end at fbo_unit::init:create depthmap\n");
            exit(1);
        }
    }
    
    if(depthbuffer)
    {
        /* Initialize the depth render buffer. */
        depth = new tex_unit;
        depth->setformat(T, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
        depth->create(w, h, 0, NULL);
        fprintf(stderr, "fbo depthattach texid: %d\n", depth->get_tex());
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, T, depth->get_tex(), 0);
    }
    
//check fbo status
    GLenum fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(fboStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        printFBOIncomplete(fboStatus);
        printf("\tFBO ERROR\n");
        exit(1);
    }
    else
    { //At this point we have a complete FBO
        printf("FBO OK\n");
    }

    
    //wrapup
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void fbo_unit::bind()
{
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT); // | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
    glViewport(0, 0, W, H);
    glScissor(0, 0, W, H);
	
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);
    /*
      for(int i=0; i<colorbuf_num; i++)
      {
      glDrawBuffer(color_buffers[i]);
      glClear(GL_COLOR_BUFFER_BIT);
      }
      printOpenGLError(); 
      glClear(GL_DEPTH_BUFFER_BIT);
    */
	
    glDrawBuffers(colorbuf_num, color_buffers);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //printOpenGLError(); 
}

void fbo_unit::unbind()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDrawBuffer(GL_BACK);
    glPopAttrib();
}

void fbo_unit::read_colorbuffer(int id)
{
    glReadBuffer(color_buffers[id]);
    printOpenGLError();
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


