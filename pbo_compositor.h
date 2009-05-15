#ifndef COMPOSITOR_H_
#define COMPOSITOR_H_

#include <mpi.h>

#include "buffer_object.h"
#include "fbo.h"
#include "texture.h"
#include "shader.h"


class pbo_compositor
{
public:
    pbo_compositor();
    ~pbo_compositor();

    void init(int, int, int, int);
    void async_pack_pbo(fbo_unit* f, int pid);
    void finish_pbo(int, int);
    void memcpy_test();
    void read_pixel();
    void local_add();
    void network_reduce(int);
    void network_datatransfer(int rank);
    void network_datatransfer_start(int rank, int pid);
    void network_datatransfer_wait(int rank, int pid);
    void draw_screen(int);
    void draw_screen(fbo_unit*);
    void draw_screen_accm(fbo_unit*);
    void release_all_pbo_tex();
    void release_pbo(int);

private:
    buffer_object *m_pbos;
    tex_unit *m_accm_tex;
    shader_object* m_rectex_shader, *m_composite_shader;

    int m_W, m_H;
    int m_npbo;

    MPI_Op m_Op;
    MPI_Request *m_request;
    MPI_Status *m_status;

    void define_op();

};

#endif
