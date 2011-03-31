
#include <assert.h>
#include <math.h>
#include <string.h>

#include <string>
#include <iostream>

#include <glheaders.h>
#include <imgfile.h>

#include "read_datafile.h"
#include "ray_caster.h"
#include "nodeDatabrick.h"


Ray_Caster::Ray_Caster(int w, int h, 
                       int bytevol,
                       std::string& shader_path) :
    CRenderScreen(w, h, CRenderScreen::SINGLE, 
                  GL_TEXTURE_RECTANGLE_ARB, 
                  GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST),
    m_byte_volume(bytevol),
    m_swid(w),
    m_shei(h),
    m_volrender_shader(0),
    m_vpos_front(0),
    m_vpos_back(0),
    m_volume(0),
    m_max_steps(2000),
    m_ray_step_size(0.01),
    m_color_mul(1.0),
    m_shaderpath(shader_path)
{
    m_volume_scale[0] = m_volume_scale[1] = m_volume_scale[1] = 1.0;
    m_volume_pos[0] = m_volume_pos[1] = m_volume_pos[1] = 0.0;
    
    init_shaders(shader_path);
    init_ref_renderscreens();
}


Ray_Caster::~Ray_Caster()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}


void Ray_Caster::update_data_scale(NodeDatabrick* D) 
{
#ifdef _DEBUG7
    fprintf(stderr, "\n%s:%s()\n", __FILE__, __func__);
#endif

    const float* nscale;
    const float* npos;
    const float* pscale;

    npos = D->get_pos();
    nscale = D->get_scale();
    pscale = D->get_physical_scale();

    for(int i=0; i<3; i++)
    {
        m_volume_pos[i] = npos[i] * pscale[i];            
        m_volume_scale[i] = nscale[i] * pscale[i];
    }

#ifdef _DEBUG7
    fprintf(stderr, "physical data pos(%7.3f, %7.3f, %7.3f), data scale(%7.3f, %7.3f, %7.3f)\n",
            m_volume_pos[0], m_volume_pos[1], m_volume_pos[2], 
            m_volume_scale[0], m_volume_scale[1], m_volume_scale[2]);
#endif
}




void Ray_Caster::init_ref_renderscreens()
{
//    fprintf(stderr, "%s:%s()\n", __FILE__, __func__);

    m_vpos_front = new CRenderScreen(m_swid, m_shei, CRenderScreen::SINGLE,
                                     GL_TEXTURE_RECTANGLE_ARB, 
                                     GL_RGBA32F_ARB, 
                                     GL_RGBA, 
                                     GL_UNSIGNED_BYTE,
                                     GL_NEAREST, GL_NEAREST);

    m_vpos_back = new CRenderScreen(m_swid, m_shei, CRenderScreen::SINGLE,
                                    GL_TEXTURE_RECTANGLE_ARB, 
                                    GL_RGBA32F_ARB, 
                                    GL_RGBA, 
                                    GL_UNSIGNED_BYTE,
                                    GL_NEAREST, GL_NEAREST);

    if(printOpenGLError()) exit(1);

}



void Ray_Caster::init_shaders(std::string& path)
{
//    fprintf(stderr, "%s:%s()\n", __FILE__, __func__);

    std::string VOLV = path +"volrender.vert";

#ifdef EYESPACE_CLIPPING
    std::string VOLF = path +"volrender_preint_clip.frag";
#else
    std::string VOLF =  path +"volrender_preint.frag";
#endif

    m_volrender_shader = new shader_object;
    m_volrender_shader->init_from_file(VOLV.c_str(), VOLF.c_str());

    m_volrender_shader->use();
    glUniform1i(m_volrender_shader->getUniformLocation("preInt"), 3);
    glUniform1i(m_volrender_shader->getUniformLocation("vpos_front"), 0);
    glUniform1i(m_volrender_shader->getUniformLocation("vpos_back"), 1);
    glUniform1i(m_volrender_shader->getUniformLocation("volume"), 2);

    glUseProgram(0);


    //checkError
    if(printOpenGLError()) exit(1);

}



void Ray_Caster::init_volume(CDataLoader* dataLoader,
                             int datatype, int comp)
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    int *vol_dim = dataLoader->get_brickDims();


    //if byte volume is needed but data is not ubyte
    //then dataloader will take care of the data conversion
    if(m_byte_volume == 1) datatype = 0;
    
    int b = get_depth(datatype);

    GLuint internal_format = GenericImage::resolve_internal_format(comp, b);
    GLuint external_format = GenericImage::resolve_external_format(comp);
    GLuint external_type = GenericImage::resolve_external_type(b);


    m_volume = new tex_unit;

    m_volume->setformat(GL_TEXTURE_3D, 
                        internal_format, 
                        external_format, 
                        external_type);

//#ifdef _DEBUG
    fprintf(stderr, "create volume texture: (%d, %d, %d), depth %d, comp %d\n",
            vol_dim[0], vol_dim[1], vol_dim[2], 
            b, comp);    
//#endif
    
    m_volume->setfilter(GL_LINEAR, GL_LINEAR);
    m_volume->create(vol_dim[0], vol_dim[1], vol_dim[2], 0, NULL);
}



void Ray_Caster::load_volumeData(CDataLoader* dataLoader)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int *vg = dataLoader->get_dims();
    int *vo = dataLoader->get_origin();

//#ifdef _DEBUG
    fprintf(stderr, "update 3D subtexture: (%d, %d, %d)->(%d, %d, %d)\n",
            vo[0], vo[1], vo[2], 
            vg[0], vg[1], vg[2]);  
//#endif

    m_volume->update_subimage(vo[0], vo[1], vo[2], 
                              vg[0], vg[1], vg[2], 
                              dataLoader->get_data());

    //fprintf(stderr, "release data\n");  
    dataLoader->release_data();

    if(printOpenGLError()) exit(1);


    //compute the texture offsets to compensate the padding
    int *p = dataLoader->get_brickPadding();
    int *g = dataLoader->get_brickDims();
    int *av_g = dataLoader->get_avDims();
    int *av_p = dataLoader->get_avPadding();
    int *av_o = dataLoader->get_avOrigin();
    
    for(int i=0; i<3; i++)
    {
        int d = g[i] - p[2*i] - p[2*i+1];
        int av_d = av_g[i] - av_p[2*i] - av_p[2*i+1];

        //object space av attributes
        m_av_scale[i] = 1.0 * av_d / d;
        m_av_shift[i] = 1.0 * (av_o[i] + av_p[2*i] - p[2*i]) / d;

        //texture space av attributes
        m_tos[2*i] = 1.0 * (av_o[i] + av_p[2*i]) / g[i];
        m_tos[2*i+1] = 1.0 * (av_o[i] + av_g[i] - av_p[2*i+1]) / g[i];
    }

#ifdef _DEBUG7
    fprintf(stderr, "tos(%.3f, %.3f, %.3f, %.3f, %.3f, %.3f)\n",
            m_tos[0], m_tos[1], m_tos[2], m_tos[3], m_tos[4], m_tos[5]);

    fprintf(stderr, "av scale(%.3f, %.3f, %.3f)\n",
            m_av_scale[0], m_av_scale[1], m_av_scale[2]);
    fprintf(stderr, "av shift(%.3f, %.3f, %.3f)\n\n",
            m_av_shift[0], m_av_shift[1], m_av_shift[2]);
#endif

//     //to test if anything wrong because of tos setting
//     //or if data is not bordered properly.
//     tos[0] = tos[2] = tos[4] = 0.015;
//     tos[1] = tos[3] = tos[5] = 0.985;
    
}


void Ray_Caster::draw_volume_boundingbox(bool wireframe)
{


    glPushMatrix();
    if(!wireframe)
    {
        //drawTosBox(m_volume_scale, m_volume_pos, m_tos);
        float bbscale[3], bbpos[3];
        for(int i=0; i<3; i++){
            bbscale[i] = m_av_scale[i] * m_volume_scale[i];
            //object space pos
            float p = (m_av_shift[i] + m_av_scale[i])/2.0 - 0.5;
            //scale and traslate to world space pos
            p *= m_volume_scale[i];
            bbpos[i] = m_volume_pos[i] + p;
        }
        drawTosBox(bbscale, bbpos, m_tos);
    }                   
    else
    {
        //cull the back faces
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //draw the front faces as lines
        glPolygonMode(GL_FRONT, GL_LINE);
        //glDisable(GL_DEPTH_TEST);
        //float c[4] = {0.2, 0.2, 0.2, 0.2};
        //drawColorBox(m_volume_scale, m_volume_pos, c);
        //glEnable(GL_DEPTH_TEST);
        drawTosBox(m_volume_scale, m_volume_pos, m_tos);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);
    }
    glPopMatrix();

    if(printOpenGLError()) exit(1);

}


//-----------------------------------------------------------------------------
//put some transformation into textue matrix 0 to be used later
//this is used to transform clipping plane coffiecients (object space)
//into ray/3D texture space
//-----------------------------------------------------------------------------
void Ray_Caster::object_to_texture_transform()
{
    float    ss[3];
    ss[0] = (m_tos[1] - m_tos[0]);
    ss[1] = (m_tos[3] - m_tos[2]);
    ss[2] = (m_tos[5] - m_tos[4]);
    //std::cout<<ss[0]<<","<<ss[1]<<","<<ss[2]<<std::endl;

    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();

        glTranslatef(m_tos[0], m_tos[2], m_tos[4]);
        glScalef(ss[0], ss[1], ss[2]);
        glTranslatef(0.5, 0.5, 0.5);
        glScalef(1.0/m_volume_scale[0], 1.0/m_volume_scale[1], 1.0/m_volume_scale[2]);
        glTranslatef(-m_volume_pos[0], -m_volume_pos[1], -m_volume_pos[2]);        
    }

}


void Ray_Caster::retrieve_Tspace_eyepos(float* ep)
{
    GLfloat mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);


    //object space view position
    //-(mv[12], mv[13], mv[14]) rotated by transpose of roation

    ep[0] = -(mv[0] * mv[12] + mv[1] * mv[13] + mv[2] * mv[14]);
    ep[1] = -(mv[4] * mv[12] + mv[5] * mv[13] + mv[6] * mv[14]);
    ep[2] = -(mv[8] * mv[12] + mv[9] * mv[13] + mv[10] * mv[14]);


    //now transform from object space to texture space

    for(int i=0; i<3; i++)
    {
        ep[i] -= m_volume_pos[i];
        ep[i] /= m_volume_scale[i];
        ep[i] += 0.5;
    }


    //printf("texture space eyepos: (%f, %f, %f)\n", ep[0], ep[1], ep[2]);
}


void Ray_Caster::draw_vpos_faces()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);

//------------------------------------------------------------------
//render front faces to fbo texture
//------------------------------------------------------------------
        
    //is gl face culling enabled already?
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    m_vpos_front->bind_render_buffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_volume_boundingbox(false); 

    m_vpos_front->unbind_render_buffer();

//------------------------------------------------------------------
//render back faces to fbo texture
//------------------------------------------------------------------

    glCullFace(GL_FRONT);
    m_vpos_back->bind_render_buffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_volume_boundingbox(false); 

    m_vpos_back->unbind_render_buffer();


    //if back face culling is enabled, when eye enter the dataset, the 
    //bounding box will not be drawn, which means the pixel shader will
    //not be activated to do the ray casting
    //todo: how about cull the front face then?
    glDisable(GL_CULL_FACE);

}


void Ray_Caster::update_raycaster(int sn, int ss, float mul)
{
#ifdef _DEBUG7
    fprintf(stderr, "maxstep=%d, unitstep=%d, colormul=%7.3f\n", 
            sn, ss, mul);
#endif

    m_max_steps = sn;
    m_ray_step_size = 1.0/ss;
    m_color_mul = mul;
}



void Ray_Caster::draw_volume(CTransferFunc* tfunc)
{

    draw_vpos_faces();

    m_volrender_shader->use();

    //step scale, which will affect the ray step size value.
    //An-isotropic ray casting for non-cube data space
    float step_scale[3];
    for(int i=0; i<3; i++) step_scale[i] = 1.0/m_volume_scale[i];
    float SMIN = std::min(step_scale[2], std::min(step_scale[1], step_scale[0]));
    for(int i=0; i<3; i++) step_scale[i] /= SMIN;

    glUniform3f(m_volrender_shader->getUniformLocation("step_scale"),
                step_scale[0], step_scale[1], step_scale[2]);


    //render quality: change the step size
    glUniform1f(m_volrender_shader->getUniformLocation("stepSize"), 
                m_ray_step_size);

    //other raycasting attributes: max steps
    glUniform1i(m_volrender_shader->getUniformLocation("maxSteps"), 
                m_max_steps);


    //texture space eye position
    float ep[3];
    retrieve_Tspace_eyepos(ep);
    //printf("texture space eyepos: (%f, %f, %f)\n", ep[0], ep[1], ep[2]);
    glUniform3f(m_volrender_shader->getUniformLocation("eyePos"), 
                ep[0], ep[1], ep[2]);


    //other raycasting attributes: color brightness
    glUniform1f(m_volrender_shader->getUniformLocation("multiplier"), 
                m_color_mul);

    //see if shader setup is correct
    if(printOpenGLError())  exit(1);


    //setup offscreen render buffer
    bind_render_buffer();

    //*** to enable the multipass simulation of parallel rendering
    //then clear color to (0, 0, 0, 0)
    //glClearColor(0.2, 0.2, 0.2, 1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //activate the ray casting shader    
    glActiveTexture(GL_TEXTURE0);
    m_vpos_front->get_buffer_texture()->bind();
    glActiveTexture(GL_TEXTURE1);
    m_vpos_back->get_buffer_texture()->bind();
    glActiveTexture(GL_TEXTURE2);
    m_volume->bind();
    glActiveTexture(GL_TEXTURE3);   
    tfunc->get_preInt_texture()->bind();
 
    draw_volume_boundingbox(false); 

    unbind_render_buffer();

    glActiveTexture(GL_TEXTURE0);
    m_vpos_front->get_buffer_texture()->unbind();
    glActiveTexture(GL_TEXTURE1);
    m_vpos_back->get_buffer_texture()->unbind();
    glActiveTexture(GL_TEXTURE2);
    m_volume->unbind();
    glActiveTexture(GL_TEXTURE3);   
    tfunc->get_preInt_texture()->unbind();
 

    glUseProgram(0);

    //CRenderScreen function. useful if using double buffer
    //swap_buffer();
}


void Ray_Caster::draw_debug_screens(bool show_render,
                                    bool show_vpos, 
                                    bool bboxline)
{
    //std::cout << __FILE__ << ":" << __func__ << std::endl;

    //int vp0[4] = {0, 0, m_swid, m_shei};
    //int vp1[4] = {0, 0, m_swid/2, m_shei/2};
    //int vp2[4] = {m_swid/2, 0, m_swid/2, m_shei/2};
    int vp3[4] = {0, m_shei*3/4, m_swid/4, m_shei/4};
    int vp4[4] = {m_swid/4, m_shei*3/4, m_swid/4, m_shei/4};


    if(show_render)
    {
        int render_vp[4];
        glGetIntegerv(GL_VIEWPORT, render_vp);

        draw_to_framebuffer(0, render_vp, m_shaderpath); 
        if(bboxline) draw_volume_boundingbox(true);
    }

    if(show_vpos)
    {
        m_vpos_front->draw_to_framebuffer(0, vp3, m_shaderpath, false);
        m_vpos_back->draw_to_framebuffer(0, vp4, m_shaderpath, false);
    }

}



