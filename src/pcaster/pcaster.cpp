#include <mpi.h>
#include <SDL.h>

#include <sstream>

#include "pcaster.h"


//-----------------------------------------------------------------------------
//constructor and destructor
//-----------------------------------------------------------------------------
Pcaster::Pcaster(int w, int h, int rank, int runsize, float global_scale):
    Vol_Render(w, h, rank, runsize, global_scale),
    m_ofp(0),
    m_exchanger(0),
    m_compositor(0),
    m_demo_mode(-1)
{
    init_parallel_settings();
}


Pcaster::~Pcaster()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}



/*
//-----------------------------------------------------------------------------
//init -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::init()
{
    Vol_Render::init();
}
*/



//-----------------------------------------------------------------------------
//assert the pcaster mode -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::assert_application_mode()
{

    if(m_runsize < 2)
    {
        fprintf(stderr, "pcaster runs in parallel mode\n");
        exit(0);
    }
}




//-----------------------------------------------------------------------------
//init the data pool -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::init_data_pool()
{
    //main data buffer
    m_data_pool = new unsigned int[m_wid*m_hei*4];
    memset(m_data_pool, 0, m_wid*m_hei*4*sizeof(int));
}



//-----------------------------------------------------------------------------
//pcaster init parallel settings -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::init_parallel_settings()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_ofp = new OverLap_FootPrint(m_rank, m_runsize, m_wid, m_hei);

    m_exchanger = new Pixel_Exchanger(m_rank, m_runsize);


    std::string shaderpath(pcaster_options::SHADER_PATH);

    m_compositor = new Pixel_Compositor(m_rank, m_runsize,
                                        pcaster_options::cpst_mode,
                                        m_wid,
                                        m_hei,
                                        shaderpath);
}



//-----------------------------------------------------------------------------
//ofp-based parallel rendering process stages
//-----------------------------------------------------------------------------
void Pcaster::construct_ofp()
{
    m_ofp->build(m_runsize,
                 m_kdGrid,
                 m_fculler);

    //arrangement based on ofp
    m_exchanger->arrange_buffer(m_ofp);

#ifdef _DEBUG7
    std::ostringstream s;
    s << *m_exchanger;
    fprintf(stderr, "%s\n", s.str().c_str());
#endif
}



//-----------------------------------------------------------------------------
void Pcaster::render()
{
    m_kdGrid->sort_data_nodes(m_fculler);

    bool show_render = false;
    if(m_demo_mode == 0) show_render = true;

    //draw my volume
    m_volrder->draw_volume(m_tFunc);
    m_volrder->draw_debug_screens(show_render, pcaster_options::show_vpos);

    //m_tFunc->draw_colormap(m_wid, m_hei);
    //m_tFunc->draw_alphamap(m_wid, m_hei);  
    //m_tFunc->draw_preInt_texture(m_wid, m_hei);  
}





//-----------------------------------------------------------------------------
void Pcaster::pack_pixels()
{
    //read back the raw rendering pixels
    FootPrint* fp = m_ofp->get_footprint();
    m_volrder->prepare_readback();
    fp->read_map();
    m_volrder->wrapup_readback();

    //pack pixels asof overlap structure into data_pool
    m_ofp->pack_overlap_pixels(m_data_pool);

    //to see the initial overlaps, optional
    //m_ofp->draw_overlap_pixels(m_data_pool, m_wid, m_hei, -1);

}




void Pcaster::exchange_pixels()
{
    m_exchanger->dataTransfer(m_data_pool);
    

//#ifdef _DEBUG
    int dp = m_demo_mode-1;
    if(dp >=0 && dp < m_runsize)
    {       
        if(m_rank==dp) //process dp draw send overlaps
            m_ofp->draw_overlap_pixels(m_data_pool, m_wid, m_hei, 2);
        else
        {
            //other process draw recved pixels from process dp
            unsigned int rpos = m_exchanger->get_recvbuf_offset(dp);
            m_ofp->draw_recv_pixels(dp, m_data_pool, rpos, m_wid, m_hei);
        }
    }
//#endif
}



//-----------------------------------------------------------------------------
void Pcaster::composite()
{
    //fprintf(stderr, "%d: compositing process\n", m_rank);

    //after compsite, now see the overlaps, optional
    if(m_demo_mode-m_runsize == 1) //draw keep overlaps
        m_ofp->draw_overlap_pixels(m_data_pool, m_wid, m_hei, 0);
    else if(m_demo_mode-m_runsize == 2) //draw composite overlaps
        m_ofp->draw_overlap_pixels(m_data_pool, m_wid, m_hei, 1);
        

    std::vector<int> ofp_info;
    m_exchanger->save_comp_info(m_kdGrid->get_sortarray(),
                                ofp_info,
                                m_ofp);

    unsigned int compbuf_offset = m_exchanger->get_compbuf_offset();
    unsigned int pre_compsize = m_exchanger->get_pre_compsize();
    unsigned int compsize = m_exchanger->get_compsize();


    //compositor knows to use GPU or CPU. result will be read back to data_pool
    m_compositor->composite(m_data_pool + compbuf_offset, 
                            pre_compsize, compsize, 
                            ofp_info);


    //after compsite, now see the overlaps, optional
    if(m_demo_mode-m_runsize == 3) //draw composite overlaps
        m_ofp->draw_overlap_pixels(m_data_pool, m_wid, m_hei, 1);

}




//-----------------------------------------------------------------------------
//application draw -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::draw()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(pcaster_options::timing)
    {
        m_loger->flush_log();
        if(m_fid > 0) m_loger->log(Log_Timer::LOG_InterFrame);
        else m_loger->start_timer();
    }

    construct_ofp(); //ofp-based compositing schedule computed
    if(pcaster_options::timing){
        glFinish();
        m_loger->log(Log_Timer::LOG_OFP);
    }

    render();
    if(pcaster_options::timing){
        glFinish();
        MPI_Barrier(MPI_COMM_WORLD);
        m_loger->log(Log_Timer::LOG_Render);
    }


    pack_pixels();
    if(pcaster_options::timing){
        MPI_Barrier(MPI_COMM_WORLD);
        m_loger->log(Log_Timer::LOG_Pack);
    }

    exchange_pixels(); 
    if(pcaster_options::timing){
        MPI_Barrier(MPI_COMM_WORLD);
        m_loger->log(Log_Timer::LOG_Exch);
    }

    composite();
    if(pcaster_options::timing){
        MPI_Barrier(MPI_COMM_WORLD);
        m_loger->log(Log_Timer::LOG_Comp);
    }


    m_fid++;
}


void Pcaster::read_screen(GLenum mode, int* size, void** buf)
{
    if(m_ptr == 0) m_ptr = new GLubyte[m_wid * m_hei * 4];
    //int sp[4]={0, 0, m_wid, m_hei};

    //m_volrender->read_back(sp, m_ptr, mode); 

    unsigned int compbuf_offset = m_exchanger->get_compbuf_offset();
    unsigned int compsize = m_exchanger->get_compsize();

    *size = 0;
    if(mode == GL_RGB) *size = compsize * 3;
    else if(mode == GL_RGBA) *size = compsize * 4;

    *buf = (void *)(m_data_pool + compbuf_offset);

}



//-----------------------------------------------------------------------------
//process special events for pcaster mode
//-----------------------------------------------------------------------------
void Pcaster::process_demo_mode(int SDL_key)
{
    switch(SDL_key)
    {
    case SDLK_UP:   
        m_demo_mode++; 
        std::cout << m_demo_mode << "\n"; 
        break;
    case SDLK_DOWN: 
        m_demo_mode--; 
        std::cout << m_demo_mode << "\n"; 
        break;
    }
}




//-----------------------------------------------------------------------------
//process events -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::processKeys(int SDL_key)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    Vol_Render::processKeys(SDL_key);

    process_demo_mode(SDL_key);
}


/*
//-----------------------------------------------------------------------------
//process net events -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::processNetEvents()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    Vol_Render::processNetEvents();
}



//-----------------------------------------------------------------------------
//process mouse events -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::processMouseMove(int m, int x, int y)
{
    Vol_Render::processMouseMove(m, x, y);
}

*/


//-----------------------------------------------------------------------------
//frame update -- virtual function
//-----------------------------------------------------------------------------
void Pcaster::frame_update()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    //Vol_Render::frame_update();

#ifdef _DEBUG
    if(m_rank==0) fprintf(stderr, "=====FRAME %d =====\n", m_fid);
#endif

}
