//pbo_mpi_test.cpp
//ask each mpi process to render one block,
//stage1: boundingbox will be sufficient
//readback and loading in using pbos (can have a queue)
//use big volume and subimage paging. Tex coords determine drawing

#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "renderer.h"


volrenderer::volrenderer()
{
}

volrenderer::~volrenderer()
{
}


template <class T>
T clip( T val, T min, T max )
{
    return ( val<min ? min : ( val>max ? max : val ) );
}

unsigned char* create_Preintegration_Table( unsigned char* Table )
{
    printf("Calculating preintegration table\n");
    //unsigned char* Table = vol->TF;
    
    double rInt[256]; rInt[0] = 0.;
    double gInt[256]; gInt[0] = 0.;
    double bInt[256]; bInt[0] = 0.;
    double aInt[256]; aInt[0] = 0.;

    for( int i=1; i<256; i++ )
    {
        const double tauc = ( Table[(i-1)*4+3] + Table[i*4+3] ) / 2.;

        rInt[i] = rInt[i-1] + ( Table[(i-1)*4+0] + Table[i*4+0] )/2.*tauc/255.;
        gInt[i] = gInt[i-1] + ( Table[(i-1)*4+1] + Table[i*4+1] )/2.*tauc/255.;
        bInt[i] = bInt[i-1] + ( Table[(i-1)*4+2] + Table[i*4+2] )/2.*tauc/255.;
        aInt[i] = aInt[i-1] + tauc;
    }

    unsigned char *lookupImg = new unsigned char[ 256*256*4]; // Preint Texture

    int lookupindex=0;

    for( int sb=0; sb<256; sb++ )
    {
        for( int sf=0; sf<256; sf++ )
        {
            int smin, smax;
            if( sb<sf ) { smin=sb; smax=sf; }
            else        { smin=sf; smax=sb; }

            int rcol, gcol, bcol, acol;
            if( smax != smin )
            {
                const double factor = 1. / (double)(smax-smin);
                rcol = static_cast<int>( (rInt[smax]-rInt[smin])*factor );
                gcol = static_cast<int>( (gInt[smax]-gInt[smin])*factor );
                bcol = static_cast<int>( (bInt[smax]-bInt[smin])*factor );
                acol = static_cast<int>( 
                        256.*(    exp(-(aInt[smax]-aInt[smin])*factor/255.)));
            } else
            {
                const int    index  = smin*4;
                const double factor = 1./255.;
                rcol = static_cast<int>( Table[index+0]*Table[index+3]*factor );
                gcol = static_cast<int>( Table[index+1]*Table[index+3]*factor );
                bcol = static_cast<int>( Table[index+2]*Table[index+3]*factor );
                acol = static_cast<int>( 256.*(    exp(-Table[index+3]/255.)) );
            }
            lookupImg[lookupindex++] = clip( rcol, 0, 255 );//MIN( rcol, 255 );
            lookupImg[lookupindex++] = clip( gcol, 0, 255 );//MIN( gcol, 255 );
            lookupImg[lookupindex++] = clip( bcol, 0, 255 );//MIN( bcol, 255 );
            lookupImg[lookupindex++] = clip( acol, 0, 255 );//MIN( acol, 255 );
        }
    }

    return   lookupImg;
}

void volrenderer::init_2D_textures()
{
    //fprintf(stderr, "init 2D textures\n");	
    if(!m_float_volume)
    {
        int TFsize = m_fetcher->get_TFsize();
        unsigned char* TF = m_fetcher->get_TF();
        printf("transfer function: size=%d\n", TFsize);
        
        m_transferFunc = new tex_unit;
        m_transferFunc->setformat(GL_TEXTURE_1D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
        m_transferFunc->create(TFsize, (GLvoid*)TF);
        printf("transferFucn texture: %d\n", TFsize);


        unsigned char* preInt = create_Preintegration_Table(TF);
        m_preInt = new tex_unit;
        m_preInt->setformat(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
        m_preInt->create(256, 256, 0, (GLvoid*)preInt);
        printf("preInt texture: 256, 256\n");

    }
}



void volrenderer::init_3D_textures()
{

    m_whole_volume = new tex_unit;
    if(!m_float_volume)
    {
        m_whole_volume->setformat(GL_TEXTURE_3D, GL_LUMINANCE, GL_LUMINANCE, 
                               GL_UNSIGNED_BYTE);
    }
    else
    {
        printf("----setup float volume!----\n");
        m_whole_volume->setformat(GL_TEXTURE_3D, 
                               GL_LUMINANCE32F_ARB,
                               GL_LUMINANCE,
                               GL_FLOAT);
    }
    int *gsize;
    gsize = m_fetcher->get_grp_size();
    m_whole_volume->create(gsize[0], gsize[1], gsize[2], 0, NULL);
    printf("m_whole_volume texture: (%d, %d, %d)\n", gsize[0], gsize[1], gsize[2]);

    int *o, *d, *grp_o, n;
    n = m_fetcher->get_grpchunk_num();
    grp_o = m_fetcher->get_grp_origin();

    void* data;
    for(int i=0; i<n; i++)
    {
        o = m_fetcher->get_chunk_origin(i);
        d = m_fetcher->get_chunk_dims(i);
        data = m_fetcher->get_chunk_data(i);
        m_whole_volume->update_subimage(o[0]-grp_o[0], 
                                        o[1]-grp_o[1], 
                                        o[2]-grp_o[2], 
                                        d[0], d[1], d[2], data);
        printf("update subimage[%d] texture: (%d, %d, %d)->(%d, %d, %d)\n", 
               i, o[0], o[1], o[2], d[0], d[1], d[2]);
    }
}



void volrenderer::init_shaders()
{
    printf("tex_shader: textureSimple.vert+frag\n");
    m_tex_shader = new shader_object;
    m_tex_shader->init_from_file("textureSimple.vert", "textureSimple.frag");
    m_tex_shader->use();
    glUniform1i(m_tex_shader->getUniformLocation("tex"), 0);

    printf("rectex_shader: rec_tex.vert+frag\n");
    m_rectex_shader = new shader_object;
    m_rectex_shader->init_from_file("rec_tex.vert", "rec_tex.frag");
    m_rectex_shader->use();
    glUniform1i(m_rectex_shader->getUniformLocation("tex"), 0);
    glUniform2f(m_rectex_shader->getUniformLocation("vpxy"), 0, 0);

    m_volrender_shader = new shader_object;
    if(m_float_volume)
    {
        printf("volrender_shader: iso_extraction.vert+frag\n");
        m_volrender_shader->init_from_file("volrender.vert", "iso_extraction.frag");

//         printf("volrender_shader: isosurface.vert+frag\n");
//         m_volrender_shader->init_from_file("volrender.vert", "isosurface.frag");
       
        m_volrender_shader->use();
        glUniform1f(m_volrender_shader->getUniformLocation("isovalue"), m_isovalue);
        glUniform1f(m_volrender_shader->getUniformLocation("isothres"), m_iso_thres);
    }
    else
    {

        printf("volrender_shader: volrender_preint.vert+frag\n");
        m_volrender_shader->init_from_file("volrender_preint.vert", "volrender_preint.frag");
        m_volrender_shader->use();
        glUniform1i(m_volrender_shader->getUniformLocation("preInt"), 3);

    }

    glUniform1i(m_volrender_shader->getUniformLocation("vpos_front"), 0);
    glUniform1i(m_volrender_shader->getUniformLocation("vpos_back"), 1);
    glUniform1i(m_volrender_shader->getUniformLocation("volume"), 2);

    glUniform1f(m_volrender_shader->getUniformLocation("stepSize"), m_ray_step_size);
    glUniform1i(m_volrender_shader->getUniformLocation("maxSteps"), m_ray_advance_steps);


    glUseProgram(0);
}


void volrenderer::update_raycaster(int sn, float ss, float thres, float iso)
{

    m_ray_advance_steps = sn;
    m_ray_step_size = ss;
    m_iso_thres = thres;
    m_isovalue = iso;
}



void volrenderer::init(int w, int h, bool fvol, int grps)
{
    m_wid = w;
    m_hei = h;

    m_grp_size = grps;
    m_float_volume = fvol;
    m_volume_size = 1.0;

    m_fetcher = new data_loader;
    if(!m_float_volume) m_fetcher->init_TF_manual();


    init_shaders();
    init_2D_textures();


    m_vpos_front = new fbo_unit;
    m_vpos_front->setformat(GL_TEXTURE_RECTANGLE_ARB, GL_RGBA32F_ARB, GL_RGBA, GL_UNSIGNED_BYTE);
    m_vpos_front->init(m_wid, m_hei, 1, false);
    printf("vpos_front fbo: %d, %d\n", m_wid, m_hei);

    m_vpos_back = new fbo_unit;
    m_vpos_back->setformat(GL_TEXTURE_RECTANGLE_ARB, GL_RGBA32F_ARB, GL_RGBA, GL_UNSIGNED_BYTE);
    m_vpos_back->init(m_wid, m_hei, 1, false);
    printf("vpos_back fbo: %d, %d\n", m_wid, m_hei);


    m_vol_fbo = new fbo_unit[m_grp_size];
    for(int i=0; i<m_grp_size; i++)
    {
    m_vol_fbo[i].setformat(GL_TEXTURE_RECTANGLE_ARB, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    m_vol_fbo[i].init(m_wid, m_hei, 1, false);
    printf("vol_fbo fbo %d: %d, %d\n", i, m_wid, m_hei);
    }

}


void volrenderer::init_data(int rank)
{

//     m_fetcher->init_from_h5vol( m_grp_size, rank, true, 
//                                 m_float_volume, m_volume_size);

    m_fetcher->init_from_rawvol(m_grp_size, rank);
    init_3D_textures();   

}



void volrenderer::draw_volume_boundingbox(int n, float size)
{
    float *bscale, *bpos, *tos;

    if(n == -1)
    {
        bpos = m_fetcher->get_global_pos();
        bscale = m_fetcher->get_global_scale();
        tos = m_fetcher->get_tex_offset();
    
        glTranslatef(bpos[0], bpos[1], bpos[2]);
        glScalef(bscale[0], bscale[1], bscale[2]);
        drawBox(size, 0.0, 0.0, 0.0, tos);
    }
    else if(n< -1)
    {
    //n from -2
        bpos = m_fetcher->get_global_pos();
        bscale = m_fetcher->get_global_scale();
        
    
        glTranslatef(bpos[0], bpos[1], bpos[2]);
        glScalef(bscale[0], bscale[1], bscale[2]);

        glColor4f(1.0, 1.0, 1.0, 1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawBox(size, 0., 0., 0.);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void volrenderer::draw_volume(int ck_id, int fid)
{
    //set this clear color for fbos used in draw volume
    glClearColor(0.0, 0.0, 0.0, 0.0);
    //glClearColor(0.0, 0.0, ck_id/8.0, 0.0);

//-----------------------------------------------------------------------
//render front faces to fbo texture
//-----------------------------------------------------------------------
        
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    m_vpos_front->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    draw_volume_boundingbox(ck_id, m_volume_size);    
    glPopMatrix();
    
    m_vpos_front->unbind();

//-----------------------------------------------------------------------------
//render back faces to fbo texture
//-----------------------------------------------------------------------------

    glCullFace(GL_FRONT);
    m_vpos_back->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    draw_volume_boundingbox(ck_id, m_volume_size);
    glPopMatrix();
 
    m_vpos_back->unbind();
    //glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

//-----------------------------------------------------------------------------
//draw the volume
//-----------------------------------------------------------------------------

    if(fid >= 0) m_vol_fbo[fid].bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_volrender_shader->use();

//-----------------------------------------------------------------------------
// examine the ray advancing step by step, change the step size if needed

    glUniform1i(m_volrender_shader->getUniformLocation("maxSteps"), m_ray_advance_steps);
    glUniform1f(m_volrender_shader->getUniformLocation("stepSize"), m_ray_step_size);

    if(m_float_volume)
    {
    glUniform1f(m_volrender_shader->getUniformLocation("isovalue"), m_isovalue);
    glUniform1f(m_volrender_shader->getUniformLocation("isothres"), m_iso_thres);
    }
//-----------------------------------------------------------------------------
    

    glActiveTexture(GL_TEXTURE0);
    m_vpos_front->get_color_texture(0)->bind();
    glActiveTexture(GL_TEXTURE1);
    m_vpos_back->get_color_texture(0)->bind();
    glActiveTexture(GL_TEXTURE2);
    m_whole_volume->bind(); 


    if(!m_float_volume)
    {
        glActiveTexture(GL_TEXTURE3);   

        m_preInt->bind();

    }

    glPushMatrix();
    draw_volume_boundingbox(ck_id, m_volume_size);
    glPopMatrix();

    if(fid >= 0) m_vol_fbo[fid].unbind();

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);

}

static void draw_fbo_color_texture(fbo_unit* f, int vp[4], float mag, shader_object* s)
{
        glActiveTexture(GL_TEXTURE0);    
        f->get_color_texture(0)->bind();
        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
        glViewport(vp[0], vp[1], vp[2], vp[3]);
        glScissor(vp[0], vp[1], vp[2], vp[3]);

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -1.0);
        glColor4f(1.0, 1.0, 1.0, 1.0);

        s->use();
        glUniform2f(s->getUniformLocation("vpxy"), vp[0], vp[1]);
        glUniform1f(s->getUniformLocation("p"), mag);
        draw_quad();
        glUseProgram(0);

        
        f->get_color_texture(0)->unbind();

        glPopMatrix();
        glPopAttrib();
}


static void draw_preintegration_texture(tex_unit* f, shader_object* s)
{
        glActiveTexture(GL_TEXTURE0);    
        f->bind();


        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
        glViewport(0, 0, 200, 200);
        glScissor(0, 0, 200, 200);

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -1.0);
        glColor4f(1.0, 1.0, 1.0, 1.0);

        s->use();
        glUniform1i(s->getUniformLocation("m"), true);
        draw_quad();
        glUseProgram(0);

        //m_preInt.unbind();

        glPopMatrix();
        glPopAttrib();


        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
        glViewport(200, 0, 200, 200);
        glScissor(200, 0, 200, 200);

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -1.0);
        glColor4f(1.0, 1.0, 1.0, 1.0);

        s->use();
        glUniform1i(s->getUniformLocation("m"), false);
        draw_quad();
        glUseProgram(0);

        glPopMatrix();
        glPopAttrib();

        f->unbind();
}

static void draw_transfer_function_component(int TFsize, unsigned char* TF, int comp)
{
    int i;
    float deltax = 2.0/TFsize;
    float deltay = 2.0/256;


    if(comp == 0)
    {
        glColor3f(1.0, 0., 0.);
        //draw_quad();

        glBegin(GL_LINE_STRIP);
        for(i = 0; i<TFsize; i++)
        {
            glVertex2f(-1.0 + i*deltax, -1.0+TF[i*4+0]*deltay);
        }
        glEnd();
    }
    else if(comp == 1)
    {

        glColor3f(0.0, 1., 0.);
        glBegin(GL_LINE_STRIP);
        for(i = 0; i<TFsize; i++)
        {
            glVertex2f(-1.0 + i*deltax, -1.0+TF[i*4+1]*deltay);
        }
        glEnd();
    }
    else if(comp == 2)
    {

        glColor3f(0., 0., 1.);
        glBegin(GL_LINE_STRIP);
        for(i = 0; i<TFsize; i++)
        {
            glVertex2f(-1.0 + i*deltax, -1.0+TF[i*4+2]*deltay);
        }
        glEnd();  
    }
    else if(comp == 3)
    {
        glColor3f(1.0, 1., 0.);
        glBegin(GL_LINE_STRIP);
        for(i = 0; i<TFsize; i++)
        {
            glVertex2f(-1.0 + i*deltax, -1.0+TF[i*4+3]*deltay);
        }
        glEnd();
    }
}



static void draw_transfer_function(int TFsize, unsigned char* TF, bool combine)
{

    //glDisable(GL_DEPTH_TEST);

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -1.0);

        for(int i=0; i<4; i++)
        {
            glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
            if(combine)
            {
                glViewport(0, 0, 800, 200);
                glScissor(0, 0, 800, 200);
            }
            else
            {
                glViewport(i*200, 0, 200, 200);
                glScissor(i*200, 0, 200, 200);
            }

            draw_transfer_function_component(TFsize, TF, i);
            glPopAttrib();
        }

        glPopMatrix();

        //glEnable(GL_DEPTH_TEST);
}

void volrenderer::draw_debug_screens(bool show_chunk, bool show_fbotexture,
                                     bool combine, bool show_transferfunction, 
                                     bool show_preintegration)
{
    int vp1[4] = {0, 0, 400, 400};
    //int vp2[4] = {400, 0, 400, 400};
    int vp3[4] = {0, 400, 400, 400};
    int vp4[4] = {400, 400, 400, 400};


//------------------------------------------------------------------------------
//show subvolume fbo texture
//------------------------------------------------------------------------------
    if(show_chunk)
    {
        draw_fbo_color_texture(m_vol_fbo, vp1, 2.0, m_rectex_shader); 
    }

//------------------------------------------------------------------------------
//show front face fbo texture
//------------------------------------------------------------------------------
    if(show_fbotexture)
    {
        draw_fbo_color_texture(m_vpos_front, vp3, 2.0, m_rectex_shader);
        draw_fbo_color_texture(m_vpos_back, vp4, 2.0, m_rectex_shader);
    }

//------------------------------------------------------------------------------
//show transfer function
//------------------------------------------------------------------------------

    if(!m_float_volume && show_transferfunction)
    {
    int TFsize = m_fetcher->get_TFsize();
    unsigned char* TF = m_fetcher->get_TF();
    draw_transfer_function(TFsize, TF, combine);
    }

//------------------------------------------------------------------------------
//show pre integration texture
//------------------------------------------------------------------------------

    if(!m_float_volume && show_preintegration)
    {
        draw_preintegration_texture(m_preInt, m_tex_shader); 
    }
}



