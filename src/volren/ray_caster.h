#ifndef RAY_CASTER_H_
#define RAY_CASTER_H_


#include <shader.h>
#include <pmisc.h>
#include <draw_routines.h>
#include <texture.h>
#include <imgfile.h>
#include <fbo.h>

#include "renderscreen.h"
#include "transferFunc.h"
#include "dataloader.h"
#include "nodeDatabrick.h"


class Ray_Caster : public CRenderScreen
{
public:
    Ray_Caster(int w, int h, 
               int bytevol, 
               std::string& shader_path);
    virtual ~Ray_Caster();

    void update_data_scale(NodeDatabrick* D);
    void update_raycaster(int, int, float);
    //void update_clip_plane(float, float, float, float);

    void init_volume(CDataLoader*,
                     int datatype, int comp);    
    void load_volumeData(CDataLoader* );

    void draw_volume(CTransferFunc* tfunc);
    void draw_debug_screens(bool, bool, bool bbline=true);


private:
    int m_byte_volume;
    int m_swid, m_shei;

    shader_object *m_volrender_shader;
    CRenderScreen *m_vpos_front, *m_vpos_back;
    tex_unit* m_volume;

    int m_max_steps;
    float m_ray_step_size;
    float m_color_mul;

    std::string m_shaderpath;
    //float m_clip_plane[4];
    
    //this is the physical volume scale. 
    float m_volume_scale[3];
    float m_volume_pos[3];
    float m_tos[6];

    //Use with progressive dataloading, available object space
    float m_av_scale[3];
    float m_av_shift[3];

    void init_shaders(std::string& path);
    void init_ref_renderscreens();   

    void draw_vpos_faces();
    void draw_volume_boundingbox(bool);

    void retrieve_Tspace_eyepos(float* ep);
    void object_to_texture_transform();

};


#endif
