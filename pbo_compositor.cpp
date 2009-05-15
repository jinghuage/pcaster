//pbo_compositor.cpp
#include <string.h>
#include <mpi.h>

#include "misc.h"
#include "draw_routines.h"
#include "pbo_compositor.h"

//assume volume is rendered in fbo texture
//pbo_compositor implement: 
//pack fbo texture async into pbo
//map pbo to memory, use mpi_reduce to addup pixel values 
//update pbo using memory after mpi_reduce
//unpack pbo to framebuffer as a composited view

pbo_compositor::pbo_compositor()
{
}

pbo_compositor::~pbo_compositor()
{
}


void pbo_compositor::init(int n, int w, int h, int rank)
{
    fprintf(stderr, "init pbos\n");
	

    m_pbos = new buffer_object[n];
    m_npbo = n;

    m_W = w;
    m_H = h;

	
    int point_num = w*h;
    for(int i=0; i<n; i++)
    {
        if(rank!=0) m_pbos[i].init(GL_PIXEL_PACK_BUFFER_ARB, 
                       point_num, GL_STREAM_READ, 4);
        else m_pbos[i].init(GL_PIXEL_UNPACK_BUFFER_ARB, 
                       point_num, GL_STREAM_DRAW, 4);
    }


    m_accm_tex = new tex_unit[n];
    for(int i=0; i<n; i++)
    {
        m_accm_tex[i].setformat(GL_TEXTURE_RECTANGLE_ARB, 
                                GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE);
        m_accm_tex[i].create(w, h, 0, NULL);
    }


    m_request = new MPI_Request[n];
    m_status = new MPI_Status[n];

    printf("init shaders for pbo_compositor\n");
    //m_rectex_shader
    m_rectex_shader = new shader_object;
    printf("rec_tex_shader: rec_tex.vert+frag\n");
    m_rectex_shader->init_from_file("rec_tex.vert", "rec_tex.frag");
    m_rectex_shader->use();
    glUniform1i(m_rectex_shader->getUniformLocation("tex"), 0);
    glUniform2f(m_rectex_shader->getUniformLocation("vpxy"), 0, 0);
    glUniform1f(m_rectex_shader->getUniformLocation("p"), 1.0);

    m_composite_shader = new shader_object;
    printf("composite_shader: composite.vert+frag\n");
    //use 2 tex for local np=2 testing.
    //also may be used for parallel compositing
    m_composite_shader->init_from_file("composite.vert", "composite_4tex.frag");
    m_composite_shader->use();
    glUniform1i(m_composite_shader->getUniformLocation("tex0"), 0);
    glUniform1i(m_composite_shader->getUniformLocation("tex1"), 1);
    glUniform1i(m_composite_shader->getUniformLocation("tex2"), 2);
    glUniform1i(m_composite_shader->getUniformLocation("tex3"), 3);
//     glUniform1i(m_composite_shader->getUniformLocation("tex4"), 4);
//     glUniform1i(m_composite_shader->getUniformLocation("tex5"), 5);
//     glUniform1i(m_composite_shader->getUniformLocation("tex6"), 6);
//     glUniform1i(m_composite_shader->getUniformLocation("tex7"), 7);

    glUseProgram(0);

    define_op();
}

void pbo_compositor::read_pixel()
{
    GLubyte* ptr = new GLubyte[m_W*m_H*4];
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, m_W, m_H, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)ptr);

//     toOrtho();
//     glRasterPos2i(0, 0);
//     glDrawPixels(m_W, m_H, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)ptr);
//     toPerspective();

    delete[] ptr;    
}

void pbo_compositor::async_pack_pbo(fbo_unit* f, int pid)
{
// "index" is used to read pixels from framebuffer to a PBO
    //int index = m_queue_index;

//read pixel from fbo to pbo 1
//return immediately
    if(f)
    {
        f->bind();
        //printf("pbo %d pack from fbo colorbuffer\n", index);
        m_pbos[pid].fbo2pbo(f, 0, m_W, m_H);
        f->unbind();
    }
    else m_pbos[pid].fb2pbo(m_W, m_H);

//     m_queue_index++;
//     m_queue_index %= m_queue_num;
}



/* the user-defined function  */
// void myProd( Complex *in, Complex *inout, int *len,
//               MPI_Datatype *dptr )
//  {
//      int i;
//      Complex c;

// for (i=0; i< *len; ++i) {
//          c.real = inout->real*in->real -
//                     inout->imag*in->imag;
//          c.imag = inout->real*in->imag +
//                     inout->imag*in->real;
//          *inout = c;
//          in++; inout++;
//      }
//  }

static void myProd(unsigned char *in, unsigned char *inout, int *len, 
                   MPI_Datatype *dptr)
{
    int i, value;
    for(i=0; i< *len; i++)
    {
        value = *in + *inout;
        if(value > 255) *inout = (unsigned char)255;
        else            *inout = (unsigned char)value;
        ++in;
        ++inout;
    }
}

typedef void MPI_User_function( void * invec, void * inoutvec, int * len, MPI_Datatype *datatype);

void pbo_compositor::define_op()
{
    //MPI_Op myOp;
     MPI_Op_create( (MPI_User_function *)myProd, 1, &m_Op );
}


void pbo_compositor::network_datatransfer(int rank)
{
    // map the PBO 2 to process its data by CPU
    //int index = m_queue_index;
    GLubyte  *ptr=0, *aptr=0;

    if(rank==0)
    {
        for(int i=0; i<m_npbo; i++)
        {
            aptr = (GLubyte*) m_pbos[i].map(GL_WRITE_ONLY);
            MPI_Irecv(aptr, m_W*m_H*4, MPI_BYTE, 
                      MPI_ANY_SOURCE, 123, MPI_COMM_WORLD, &m_request[i]);
        }
        for(int i=0; i<m_npbo; i++)
        {
            MPI_Wait(&m_request[i], &m_status[i]);
            m_pbos[i].unmap();
        }
    }
    else
    {
        //printf("pbo %d map to memory\n", index);
        ptr = (GLubyte*) m_pbos[0].map(GL_READ_ONLY);
        MPI_Isend(ptr, m_W*m_H*4, MPI_BYTE, 
                  0, 123, MPI_COMM_WORLD, &m_request[0]);
        MPI_Wait(&m_request[0], &m_status[0]);
        m_pbos[0].unmap();
    }
     
//    MPI_Barrier(MPI_COMM_WORLD);
}


void pbo_compositor::finish_pbo(int rank, int pid)
{
    GLubyte  *ptr=0, *aptr=0;

    if(rank==0)
    {
        aptr = (GLubyte*) m_pbos[pid].map(GL_WRITE_ONLY);
        m_pbos[pid].unmap();
    }
    else
    {
        //printf("pbo %d map to memory\n", index);
        ptr = (GLubyte*) m_pbos[pid].map(GL_READ_ONLY);
        m_pbos[pid].unmap();
    }
     
//    MPI_Barrier(MPI_COMM_WORLD);
}


void pbo_compositor::memcpy_test()
{
    GLubyte  *ptr=0, *aptr=0;


    aptr = (GLubyte*) m_pbos[0].map(GL_WRITE_ONLY);
    ptr = (GLubyte*) m_pbos[1].map(GL_READ_ONLY);

    memcpy(aptr, ptr, m_W*m_H*4);

    m_pbos[0].unmap();
    m_pbos[1].unmap();
     
}


void pbo_compositor::network_datatransfer_start(int rank, int pid)
{
//    int index = m_queue_index;
    GLubyte  *ptr=0, *aptr=0;

    if(rank==0)
    {
        for(int i=0; i<m_npbo; i++)
        {
        aptr = (GLubyte*) m_pbos[i].map(GL_WRITE_ONLY);
        MPI_Irecv(aptr, m_W*m_H*4, MPI_BYTE, 
                  MPI_ANY_SOURCE, 123, MPI_COMM_WORLD, &m_request[i]);
        }
    }
    else
    {
        //printf("pbo %d map to memory\n", index);
        ptr = (GLubyte*) m_pbos[pid].map(GL_READ_ONLY);
        MPI_Isend(ptr, m_W*m_H*4, MPI_BYTE, 
                  0, 123, MPI_COMM_WORLD, &m_request[pid]);
//         MPI_Send(ptr, m_W*m_H*4, MPI_BYTE, 
//                   0, 123, MPI_COMM_WORLD);
    }
     
//    MPI_Barrier(MPI_COMM_WORLD);
}

void pbo_compositor::network_datatransfer_wait(int rank, int pid)
{
    //int index = m_queue_index;
    if(rank==0)
    {
        for(int i=0; i<m_npbo; i++)
        {
        MPI_Wait(&m_request[i], &m_status[i]);
        m_pbos[i].unmap();
        }
    }
    else
    {
        MPI_Wait(&m_request[pid], &m_status[pid]);
        m_pbos[pid].unmap();
    }
     
//    MPI_Barrier(MPI_COMM_WORLD);
}


void pbo_compositor::network_reduce(int rank)
{
// map the PBO 2 to process its data by CPU
    //int index = m_queue_index;
    GLubyte  *ptr=0, *aptr=0;

    //printf("pbo %d map to memory\n", index);
    ptr = (GLubyte*) m_pbos[0].map(GL_READ_ONLY);
    if(rank ==0) aptr = (GLubyte*) m_pbos[1].map(GL_WRITE_ONLY);


//what about the first frame when pbo 2 doesn't have data yet?
//will ptr be NULL? don't think so...
    if(ptr)
    {
        //printf("process\n");
        //MPI_SUM won't work with MPI_BYTE. (only integer and float)
        //MPI_Reduce(ptr, aptr, m_W*m_H*4, MPI_BYTE, m_Op, 0, MPI_COMM_WORLD);
        MPI_Reduce(ptr, aptr, m_W*m_H, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    m_pbos[0].unmap();
    if(rank==0) m_pbos[1].unmap();
    
//    MPI_Barrier(MPI_COMM_WORLD);
}

void pbo_compositor::draw_screen_accm(fbo_unit* f)
{
    int i;
    for(i=0; i<m_npbo; i++)
    {
    //draw accumulation buffer to screen
    m_pbos[i].bind(GL_PIXEL_UNPACK_BUFFER_ARB);
    m_accm_tex[i].update_subimage(0, 0, m_W, m_H, 0);
    m_pbos[i].unbind(GL_PIXEL_UNPACK_BUFFER_ARB);
    }

//     glActiveTexture(GL_TEXTURE0);    
//     m_accm_tex->bind();
//     m_rectex_shader->use();
//     draw_fullscreen_quad();
//     m_accm_tex->unbind();
//     glUseProgram(0);    


    //printf("composite\n");
    glActiveTexture(GL_TEXTURE0);
    f->get_color_texture(0)->bind();
    for(i=1; i<=m_npbo; i++)
    {
        glActiveTexture(GL_TEXTURE0+i);
        m_accm_tex[i-1].bind();
    }
    m_composite_shader->use();
    draw_fullscreen_quad();
    glUseProgram(0);

    for(i=0; i<m_npbo; i++)
    {
        m_accm_tex[i].unbind();
    }
    f->get_color_texture(0)->unbind();
}

void pbo_compositor::release_all_pbo_tex()
{
    for(int i=0; i<m_npbo; i++)
    {
        m_pbos[i].clear();
        m_accm_tex[i].clear();
    }
}

void pbo_compositor::release_pbo(int pid)
{
    m_pbos[pid].clear();
}

void pbo_compositor::draw_screen(int pid)
{
    //draw pbo to screen
    m_pbos[0].bind(GL_PIXEL_UNPACK_BUFFER_ARB);
    m_accm_tex[0].update_subimage(0, 0, m_W, m_H, 0);
    m_pbos[0].unbind(GL_PIXEL_UNPACK_BUFFER_ARB);
    
    glActiveTexture(GL_TEXTURE0);    
    m_accm_tex[0].bind();
    m_rectex_shader->use();
    draw_fullscreen_quad();
    m_accm_tex[0].unbind();
    glUseProgram(0);    
}

void pbo_compositor::draw_screen(fbo_unit* f)
{
    glActiveTexture(GL_TEXTURE0);
    f->get_color_texture(0)->bind();
    m_rectex_shader->use();
    draw_fullscreen_quad();
    f->get_color_texture(0)->unbind();
    glUseProgram(0);    
}




