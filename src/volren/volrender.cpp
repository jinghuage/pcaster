/*****************************************************************************


    Copyright 2009,2010,2011 Jinghua Ge
    ------------------------------

    This file is part of Pcaster.

    Pcaster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Pcaster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Pcaster.  If not, see <http://www.gnu.org/licenses/>.


******************************************************************************/


#include <assert.h>
#include <math.h>
#include <string.h>
#include <SDL/SDL.h>

#include <assert.h>
#include <algorithm>
#include <sstream>

#include "volrender.h"

#if defined( __LOCAL_HDF5__) || defined(__REMOTE_HDF5__ )
#include "async_dataloader.h"
#endif

//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
Vol_Render::Vol_Render(int w, int h, int rank, int runsize, float global_scale):
    GL_Application(w, h),
    m_rank(rank),
    m_runsize(runsize),
    m_data_pool(0),
    m_fculler(0),
    m_kdGrid(0),
    m_node(0),
    m_volrder(0),
    m_tFunc(0),
    m_dataloader(0),
    m_loger(0)
{    
    //constructor should not call virtual functions
    //virutal init will be called in virtual function init()

    m_fculler = new CFrustumCull; 
    init_datagrid(global_scale); 
    init_dataloader();
    init_volrender();
    init_transferFunc();
    if(pcaster_options::timing) init_timerLog();   
}


Vol_Render::~Vol_Render()
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif
}


//-----------------------------------------------------------------------------
//init -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::init()
{
    GL_Application::init();

    //assert_application_mode(); 
    init_data_pool();
}



//-----------------------------------------------------------------------------
//assert the application mode -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::assert_application_mode()
{
    if(m_runsize != 1)
    {
        fprintf(stderr, "this program *don't* run in parallel\n");
        exit(0);
    }

    // if(!pcaster_options::viewer.compare("inplace"))
    // {
    //     fprintf(stderr, "this program only supports remote viewing\n");
    //     exit(0);
    // }
}



//-----------------------------------------------------------------------------
//init the data pool -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::init_data_pool()
{

}



//-----------------------------------------------------------------------------
//application init data grid
//-----------------------------------------------------------------------------
void Vol_Render::init_datagrid(float gscale)
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    //get data scale, normalize by maximum value
    float s[3];
    for(int i=0; i<3; i++) s[i] = pcaster_options::data_dims[i] * 
                               pcaster_options::sample_scale[i];

    float MS = std::max(s[2], std::max(s[1], s[0]));
    for(int i=0; i<3; i++)
    {
        s[i] /= MS;
        s[i] *= gscale;
    }



    m_kdGrid = new Data_Hierarchy();
    m_kdGrid->init_dataTree(pcaster_options::num_data_node, s);

    //get my node
    m_node = m_kdGrid->get_node(m_rank);


    std::ostringstream stm;
    stm << *m_node;
    fprintf(stderr, "%d: node: %s\n\n", m_rank, stm.str().c_str());


}


//-----------------------------------------------------------------------------
//application init dataloader
//-----------------------------------------------------------------------------
void Vol_Render::init_dataloader()
{
#ifdef _DEBUG7    
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


#if defined( __LOCAL_HDF5__) || defined(__REMOTE_HDF5__ )

    assert(pcaster_options::data_dist == CDataLoader::DL_REMOTE &&
           pcaster_options::data_loadmode == CDataLoader::DL_RPC);
    
    m_dataloader = new AsyncDataLoader(m_rank, m_runsize);

#else

    assert(pcaster_options::data_dist != CDataLoader::DL_REMOTE &&
           pcaster_options::data_loadmode != CDataLoader::DL_RPC);

    m_dataloader = new CDataLoader(m_rank, m_runsize);

#endif



    m_dataloader->init(                                   
        pcaster_options::DATA_NAME,
        pcaster_options::DATASET_NAME,
        pcaster_options::FILE_TYPE,
        pcaster_options::comp, 
        pcaster_options::DATA_TYPE,
        pcaster_options::data_dist,
        pcaster_options::data_loadmode,
        pcaster_options::border);

    m_dataloader->compute_local_selection(m_node,
                                          pcaster_options::data_dims, 
                                          pcaster_options::chunk_dims,
                                          pcaster_options::timestep);

    if(pcaster_options::data_dist == CDataLoader::DL_SINGLE &&
       pcaster_options::data_loadmode == CDataLoader::DL_BLOCKING)
    {
        m_dataloader->load_data();
        m_dataloader->process_data(pcaster_options::bytevolume,
                                   pcaster_options::dmin,
                                   pcaster_options::dmax);
    }
    else
    {
        m_dataloader->request_data();
    }
}



//-----------------------------------------------------------------------------
//application init volrender
//-----------------------------------------------------------------------------
void Vol_Render::init_volrender()
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    std::string shaderpath(pcaster_options::SHADER_PATH);

    m_volrder = new Ray_Caster(m_wid, m_hei, 
                               pcaster_options::bytevolume,
                               shaderpath);
    m_volrder->update_data_scale(m_node);
    m_volrder->update_raycaster(pcaster_options::max_steps,
                                pcaster_options::ray_steps,
                                pcaster_options::color_mul);


    m_volrder->init_volume(m_dataloader,
                           pcaster_options::DATA_TYPE,
                           pcaster_options::comp);

    if(pcaster_options::data_dist == CDataLoader::DL_SINGLE &&
       pcaster_options::data_loadmode == CDataLoader::DL_BLOCKING)
    {
        m_volrder->load_volumeData(m_dataloader);
    }
}



//-----------------------------------------------------------------------------
//application init transferfunc
//-----------------------------------------------------------------------------
void Vol_Render::init_transferFunc()
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    m_tFunc = new CTransferFunc();

    if(!pcaster_options::TFfile.empty())
    {
        m_tFunc->init_TF(pcaster_options::TFfile.c_str()); 
    }else{
        m_tFunc->init_colormap(pcaster_options::colorizer_r,
                               pcaster_options::colorizer_g,
                               pcaster_options::colorizer_b);

        m_tFunc->init_alphamap(pcaster_options::peakscale, 
                               pcaster_options::sharpness, 
                               pcaster_options::shift, 
                               pcaster_options::intensity, 
                               pcaster_options::peaks);
    }
      
    m_tFunc->init_TF_texture();
    m_tFunc->init_preInt_texture();


    if(printOpenGLError()) exit(1);

}




//-----------------------------------------------------------------------------
//application init timer log
//-----------------------------------------------------------------------------
void Vol_Render::init_timerLog()
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


    m_loger = new Log_Timer(pcaster_options::log_dir.c_str(),
                            pcaster_options::render_wid, 
                            pcaster_options::render_hei,
                            pcaster_options::cpst_mode,
                            m_runsize, m_rank, 
                            pcaster_options::timing);
}


//-----------------------------------------------------------------------------
//application draw -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::draw()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_kdGrid->sort_data_nodes(m_fculler);

    //draw my volume
    m_volrder->draw_volume(m_tFunc);
}


void Vol_Render::read_screen(GLenum mode, int* size, void** buf)
{
    if(m_ptr == 0) m_ptr = new GLubyte[m_wid * m_hei * 4];
    int sp[4]={0, 0, m_wid, m_hei};

    m_volrder->read_back(sp, m_ptr, mode); 

    *size = 0;
    if(mode == GL_RGB) *size = m_wid * m_hei * 3;
    else if(mode == GL_RGBA) *size = m_wid * m_hei * 4;

    *buf = (void *)m_ptr;

}



//-----------------------------------------------------------------------------
//print options according to the handled events
//-----------------------------------------------------------------------------
void Vol_Render::print_options(bool new_tfunc_c, 
                               bool new_tfunc_a,
                               bool new_data,
                               bool new_cast)
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n\n", __FILE__, __func__);
#endif

    if(new_tfunc_c)
    {
        fprintf(stderr, "colorizer(%7.3f, %7.3f, %7.3f)\n",
                pcaster_options::colorizer_r, 
                pcaster_options::colorizer_g, 
                pcaster_options::colorizer_b);
    }

    if(new_tfunc_a)
    {
        fprintf(stderr, "peaks=%d, shift=%7.3f, scale=%7.3f, intensity=%7.3f, sharpness=%7.3f\n",
                pcaster_options::peaks,
                pcaster_options::shift,
                pcaster_options::peakscale,
                pcaster_options::intensity,
                pcaster_options::sharpness);
    }

    if(new_data)
    {
        fprintf(stderr, "timestep=%d\n", pcaster_options::timestep);
    }

    if(new_cast)
    {
        fprintf(stderr, "ray_steps=%d, max_steps=%d\n",
                pcaster_options::ray_steps,
                pcaster_options::max_steps);
    }

}




//-----------------------------------------------------------------------------
//process key events -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::processKeys(int key)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    GL_Application::processKeys(key);

    bool new_tfunc_c = false;
    bool new_tfunc_a = false;
    bool new_dataopt = false;
    bool new_raycastopt = false;


    static bool print = false;

    switch(key)
    {
    case 'h': print = !print; break; 
    case 'o': pcaster_options::peaks += 1; new_tfunc_a=true; break; 
    case 'p': pcaster_options::peaks -= 1; new_tfunc_a=true; break; 
    case 'a': pcaster_options::shift += 0.05; new_tfunc_a=true; break; 
    case 'd': pcaster_options::shift -= 0.05; new_tfunc_a=true; break; 
    case 'w': pcaster_options::peakscale += 0.01; new_tfunc_a=true; break; 
    case 'z': pcaster_options::peakscale -= 0.01; new_tfunc_a=true; break; 
    case 'u': pcaster_options::intensity += 0.01; new_tfunc_a=true; break; 
    case 'i': pcaster_options::intensity -= 0.01; new_tfunc_a=true; break; 
    case 'j': pcaster_options::sharpness += 0.005; new_tfunc_a=true; break; 
    case 'k': pcaster_options::sharpness -= 0.005; new_tfunc_a=true; break; 
    case SDLK_1: pcaster_options::colorizer_r += 0.1; new_tfunc_c=true; break;
    case SDLK_2: pcaster_options::colorizer_r -= 0.1; new_tfunc_c=true; break;
    case SDLK_3: pcaster_options::colorizer_g += 0.1; new_tfunc_c=true; break;
    case SDLK_4: pcaster_options::colorizer_g -= 0.1; new_tfunc_c=true; break;
    case SDLK_5: pcaster_options::colorizer_b += 0.1; new_tfunc_c=true; break;
    case SDLK_6: pcaster_options::colorizer_b -= 0.1; new_tfunc_c=true; break;
    }

    //special keys ctrl+
    if (SDL_GetModState() & KMOD_CTRL)
    {
        switch (key)  
        {
        case SDLK_LEFT:  
            pcaster_options::max_steps--; new_raycastopt=true; break;
        case SDLK_RIGHT: 
            pcaster_options::max_steps++; new_raycastopt=true; break;
        case SDLK_UP:
            pcaster_options::ray_steps += 16; new_raycastopt=true; break;
        case SDLK_DOWN:
            pcaster_options::ray_steps -= 16; new_raycastopt=true; break;            
        case SDLK_m: 
            pcaster_options::modify_tf = 1 - pcaster_options::modify_tf; 
            break;
        }
    }

    //special keys shift+
    if (SDL_GetModState() & KMOD_SHIFT)
    {
        switch (key)  
        {
        case SDLK_UP:
            pcaster_options::timestep++;
            if(pcaster_options::timestep >= pcaster_options::min_timestep && 
               pcaster_options::timestep <= pcaster_options::max_timestep)
                new_dataopt = true;
            else pcaster_options::timestep = pcaster_options::max_timestep;
            break;
        case SDLK_DOWN:
            pcaster_options::timestep--;
            if(pcaster_options::timestep >= pcaster_options::min_timestep &&
               pcaster_options::timestep <= pcaster_options::max_timestep)
                new_dataopt = true;
            else pcaster_options::timestep = pcaster_options::min_timestep;
            break;
        }
    }

    render_update(new_tfunc_c, new_tfunc_a, new_dataopt, new_raycastopt);

    //if(print)
    print_options(new_tfunc_c, new_tfunc_a, new_dataopt, new_raycastopt); 
}


//-----------------------------------------------------------------------------
//update according to the handled events
//-----------------------------------------------------------------------------
void Vol_Render::render_update(bool new_tfunc_c, 
                               bool new_tfunc_a,
                               bool new_data,
                               bool new_cast)
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n\n", __FILE__, __func__);
#endif

    //if tfunc configs changed, update the transfer func
    if(new_tfunc_c) m_tFunc->init_colormap(pcaster_options::colorizer_r,
                                         pcaster_options::colorizer_g,
                                         pcaster_options::colorizer_b);

    if(new_tfunc_a) m_tFunc->init_alphamap(pcaster_options::peakscale, 
                                        pcaster_options::sharpness, 
                                        pcaster_options::shift, 
                                        pcaster_options::intensity, 
                                        pcaster_options::peaks);

    if(new_tfunc_c || new_tfunc_a) m_tFunc->update_textures();


    //if data timestep changes, load the new data
    //volume renderer also update its volume from the data
    if(new_data)
    {
        m_dataloader->compute_local_selection(m_node, 
                                              pcaster_options::data_dims,
                                              pcaster_options::chunk_dims,
                                              pcaster_options::timestep);
        
        m_volrder->update_data_scale(m_node);
        if(pcaster_options::data_dist == CDataLoader::DL_SINGLE &&
           pcaster_options::data_loadmode == CDataLoader::DL_BLOCKING)
        {
            m_dataloader->load_data();
            m_dataloader->process_data(pcaster_options::bytevolume,
                                       pcaster_options::dmin,
                                       pcaster_options::dmax);
            m_volrder->load_volumeData(m_dataloader);
        }
        else
        {
            m_dataloader->request_data();
        }
    }


    if(new_cast)
    {
        m_volrder->update_raycaster(pcaster_options::max_steps,
                                    pcaster_options::ray_steps,
                                    pcaster_options::color_mul);

    }
}



//-----------------------------------------------------------------------------
//process mouse events -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::processMouseMove(int m, int x, int y)
{
    if(!pcaster_options::modify_tf) return;

    static int px = -1, py = -1;
    x-=50;
    y-=50;
    
    //moddle mouse button down + motion: edit transfer function

    if(m == 2)
    {
        x = clip(x, 0, 255);
        y = clip(y, 0, 255);

        m_tFunc->update_TF(px, py, x, y, pcaster_options::tf_channel);

        m_tFunc->update_textures();
    }

    px = clip(x, 0, 255);
    py = clip(y, 0, 255);
}


//-----------------------------------------------------------------------------
//process net events -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::processNetEvents()
{

}


//-----------------------------------------------------------------------------
//frame update -- virtual function
//-----------------------------------------------------------------------------
void Vol_Render::frame_update()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    GL_Application::frame_update();

    if(!(pcaster_options::data_dist == CDataLoader::DL_SINGLE &&
         pcaster_options::data_loadmode == CDataLoader::DL_BLOCKING))
    {
        bool ret = m_dataloader->update_data();
        if(ret) m_volrder->load_volumeData(m_dataloader);
    }

#ifdef _DEBUG
    fprintf(stderr, "=====FRAME %d =====\n", m_fid);
#endif

}
