#include <assert.h>
#include <math.h>
#include <string.h>
#include <SDL.h>

#include <assert.h>
#include <algorithm>

#include "standalone_render.h"


//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
Standalone_Render::Standalone_Render(int w, int h, int rank, int runsize, 
                                     float global_scale):
    Vol_Render(w, h, rank, runsize, global_scale),
    m_multipass(false),
    m_sortnodes(true)
{

}


Standalone_Render::~Standalone_Render()
{
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}



//-----------------------------------------------------------------------------
//init -- virtual function
//-----------------------------------------------------------------------------
void Standalone_Render::init()
{
    Vol_Render::init();
}


//-----------------------------------------------------------------------------
//assert the standalone mode -- virtual function
//-----------------------------------------------------------------------------
void Standalone_Render::assert_application_mode()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    Vol_Render::assert_application_mode();

    if(pcaster_options::viewer.compare("inplace"))
    {
        fprintf(stderr, "this program only supports inplace viewing\n");
        exit(0);
    }
}



//-----------------------------------------------------------------------------
//application draw -- virtual function
//-----------------------------------------------------------------------------
void Standalone_Render::draw()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(m_sortnodes) m_kdGrid->sort_data_nodes(m_fculler);
    const int* sortnodes = m_kdGrid->get_sortarray();

    //draw my volume
    if(m_multipass)
    {
        for(int nid=0; nid<pcaster_options::num_data_node; nid++)
        {
            if(pcaster_options::data_dist == CDataLoader::DL_SINGLE &&
               pcaster_options::data_loadmode == CDataLoader::DL_BLOCKING)
            {
                //prepare for the data node
                m_node = m_kdGrid->get_node(sortnodes[nid]);
                m_dataloader->compute_local_selection(m_node, 
                                                      pcaster_options::data_dims,
                                                      pcaster_options::chunk_dims,
                                                      pcaster_options::timestep);

                m_volrder->update_data_scale(m_node);

                //load data and update volume
                m_dataloader->load_data();
                m_dataloader->process_data(pcaster_options::bytevolume,
                                           pcaster_options::dmin,
                                           pcaster_options::dmax);
                m_volrder->load_volumeData(m_dataloader);
            }
            else
            {
                fprintf(stderr, "multipass rendering not yet work with \
                                 progressive data loading mode\n");
            }
            //draw this node
            m_volrder->draw_volume(m_tFunc);
            m_volrder->draw_debug_screens(true, pcaster_options::show_vpos,
                                          false);
        }
    }
    else
    {
        //draw my node
        m_volrder->draw_volume(m_tFunc);
        m_volrder->draw_debug_screens(true, pcaster_options::show_vpos, true);
    }



    m_tFunc->draw_colormap(m_wid, m_hei);
    m_tFunc->draw_alphamap(m_wid, m_hei);  

    if(pcaster_options::show_preint)
        m_tFunc->draw_preInt_texture(m_wid, m_hei);  

    if(pcaster_options::modify_tf)
        m_tFunc->draw_transfer_function(m_wid, m_hei, 
                                        pcaster_options::modify_tf,
                                        pcaster_options::tf_channel);

    m_fid++;
}



//-----------------------------------------------------------------------------
//process special events for standalone mode
//-----------------------------------------------------------------------------
void Standalone_Render::process_multipass_mode(int SDL_key)
{
    bool new_dataopt = false;

    switch(SDL_key)
    {
    case SDLK_F1:
        m_rank++; m_rank %= pcaster_options::num_data_node;
        m_node = m_kdGrid->get_node(m_rank);
        m_node->print();
        new_dataopt = true;
        break;
    case SDLK_F2:
        m_multipass = !m_multipass;
        break;
    case SDLK_s:
        m_sortnodes = !m_sortnodes;
        break;
    }


    render_update(false, false, new_dataopt, false);
}


void Standalone_Render::process_debug_mode(int SDL_key)
{
    switch(SDL_key)
    {
    case SDLK_7: 
        pcaster_options::show_vpos = !pcaster_options::show_vpos;
        break;
    case SDLK_8:
        pcaster_options::show_preint = !pcaster_options::show_preint;
        break;
    }

}




//-----------------------------------------------------------------------------
//process key events -- virtual function
//-----------------------------------------------------------------------------
void Standalone_Render::processKeys(int key)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    
    Vol_Render::processKeys(key);

    process_multipass_mode(key);
    process_debug_mode(key);
}

/*
//-----------------------------------------------------------------------------
//process mouse events -- virtual function
//-----------------------------------------------------------------------------
void Standalone_Render::processMouseMove(int m, int x, int y)
{
    Vol_Render::processMouseMove(m, x, y);
}


//-----------------------------------------------------------------------------
//process net events -- virtual function
//-----------------------------------------------------------------------------
void Standalone_Render::processNetEvents()
{
    Vol_Render::processNetEvents();
}


//-----------------------------------------------------------------------------
//frame update -- virtual function
//-----------------------------------------------------------------------------
void Standalone_Render::frame_update()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    Vol_Render::frame_update();
}

*/
