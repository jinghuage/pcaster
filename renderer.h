#ifndef RENDERER_H_
#define RENDERER_H_

#include "data_loader.h"
#include "glheaders.h"
#include "shader.h"
#include "misc.h"
#include "draw_routines.h"
#include "texture.h"
#include "imgfile.h"
#include "fbo.h"

class volrenderer
{
public:
    volrenderer();
    ~volrenderer();


    void update_raycaster(int, float, float, float);
    void init_data( int );
    void init(int, int, bool, int);
    void update_3D_textures();
    void draw_volume(int ck_id, int);
    void draw_volume_boundingbox(int, float);
    void draw_debug_screens(bool show_chunk, bool show_fbotexture,
                            bool combine, bool show_transferfunction,bool show_preintegration);





private:

    data_loader *m_fetcher;

    int m_wid, m_hei;
    tex_unit *m_whole_volume;
    tex_unit *m_transferFunc;
    tex_unit *m_preInt;
    //tex_unit *isoPrms;

    shader_object *m_tex_shader, *m_rectex_shader, *m_volrender_shader;
    fbo_unit *m_vpos_front, *m_vpos_back;
    fbo_unit *m_vol_fbo;

    float m_volume_size;
    //int gaussian_filter_size = 128;
//tune the volume rendering shader
    int m_ray_advance_steps;
    float m_ray_step_size;
    float m_iso_thres;
    float m_isovalue;

    bool m_float_volume;
    bool m_overlap;
    
    int m_grp_size, m_rank;

    //void draw_volume_boundingbox(int n, float size);
    void draw_volume_bbonly(int ck_id);

    void init_shaders();
    void init_2D_textures();
    void init_3D_textures();

};

#endif
