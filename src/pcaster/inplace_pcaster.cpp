#include <mpi.h>
#include <SDL/SDL.h>

#include <sstream>

#include "inplace_pcaster.h"


//-----------------------------------------------------------------------------
//constructor and destructor
//-----------------------------------------------------------------------------
Inplace_Pcaster::Inplace_Pcaster(int w, int h, int rank, int runsize,
                                 float global_scale):
    Pcaster(w, h, rank, runsize, global_scale),
    m_screen(0),
    m_tnode(0),
    m_img_cger(0),
    m_ift(0),
    m_nviewer(1)
{
    init_screen();
    init_view_settings();

#ifdef SAGE
    rgbBuffer = NULL;
#endif


}


Inplace_Pcaster::~Inplace_Pcaster()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#ifdef SAGE
    sageInf.shutdown();
#endif

}


/*
//-----------------------------------------------------------------------------
//init -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::init()
{
    Vol_Render::init();
}
*/

#ifdef SAGE
void Inplace_Pcaster::init_sage_stuff(std::vector<float>& origin, 
	std::vector<float>& size,
	std::vector<int>& res)
{

    sailConfig cfg;
    cfg.init("sage_options.conf");
    cfg.setAppName("pcaster");
    cfg.rank = m_rank;
    cfg.appID = 0;
    //cfg.winX = 0;
    //cfg.winY = m_rank * (m_hei/m_runsize);
    cfg.resX = res[0];
    cfg.resY = res[1];

    sageRect renderImageMap;
    renderImageMap.left = origin[0];
    renderImageMap.right = origin[0] + size[0];
    renderImageMap.bottom = origin[1];
    renderImageMap.top = origin[1] + size[1];

     printf("rank %d, resX:%d resY%d left %f right %f bottom %f top %f\n", 
	cfg.rank, cfg.resX, cfg.resY,
	renderImageMap.left, renderImageMap.right, 
	renderImageMap.bottom, renderImageMap.top);

    //renderImageMap.left = ((float) m_rank) / ((float) m_runsize);
    //renderImageMap.right = ((float) m_rank + 1.0) / ((float) m_runsize);
    //renderImageMap.bottom = 0.0;
    //renderImageMap.top = 1.0;



    cfg.imageMap = renderImageMap;
    cfg.pixFmt = PIXFMT_888;
    cfg.rowOrd = BOTTOM_TO_TOP;
    cfg.nodeNum = m_nviewer;
    //cfg.master = true;
    if (m_rank == 0)
                cfg.master = true;
        else
                cfg.master = false;

    sageInf.init(cfg);
    printf("sail initialized \n");

    if (rgbBuffer)
           delete [] rgbBuffer;

    rgbBuffer = (GLubyte *)sageInf.getBuffer();
}
#endif


//-----------------------------------------------------------------------------
//assert the pcaster mode -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::assert_application_mode()
{

#ifdef _DEBUG7
    fprintf(stderr, "%d: **** %s:%s() ****\n", m_rank, __FILE__, __func__);
#endif

    Pcaster::assert_application_mode();

//     if(pcaster_options::viewer.compare("inplace"))
//     {
//         fprintf(stderr, "Must be inplace viewing\n");
//         exit(0);
//     }
}



//-----------------------------------------------------------------------------
//init the data pool -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::init_data_pool()
{
    //main data buffer
    m_data_pool = new unsigned int[m_wid*m_hei*4];
    memset(m_data_pool, 0, m_wid*m_hei*4*sizeof(int));
}



//-----------------------------------------------------------------------------
//application init tiled display 
//-----------------------------------------------------------------------------
void Inplace_Pcaster::init_screen()
{
#ifdef _DEBUG7
    fprintf(stderr, "%d: **** %s:%s() ****\n", m_rank, __FILE__, __func__);
#endif

    int tilerow = 1;
    int tilecol = 1;

    if(pcaster_options::view_mode > 0)
    {
        std::istringstream iss(pcaster_options::tile_config);
        iss >> tilerow  >> tilecol;
    }

    m_screen = new Tiled_Display(m_wid, m_hei, tilerow, tilecol);

    m_nviewer = tilerow * tilecol;
    if(m_rank < m_nviewer)
    {
        //m_tnode = m_screen->get_tile(m_rank);

        //depends on how to map a render rank into a viewer rank
        //usually a certain computer node is set as a certain view tile
        //then you need to find out the compute node hostname and 
        //set the correct tile node to it
        //here I just want render rank 0 to m_nviewer to be viewer rank 0 to
        //m_nviewer as well. tiles will be ranked from bottom to top, and 
        //from left to right

        int r = m_rank / tilecol;
        int c = m_rank % tilecol;
        std::vector<float> sc(2, 0.f);
        sc[0] = 1.0/float(tilecol);
        sc[1] = 1.0/float(tilerow);

        std::vector<float> tr(2, 0.f);
        tr[0] = (c+.5) * sc[0] - .5;
        tr[1] = (r+.5) * sc[1] - .5;

        m_tnode = new NodeRectile(1,1);
        m_tnode->scale(sc);
        m_tnode->translate(tr);

        std::ostringstream s;
        s << *m_tnode;
        fprintf(stderr, "%d: tile: %s\n\n", m_rank, s.str().c_str());


#ifdef SAGE
	std::vector<float> sorigin(2, 0.f);
	std::vector<int> sres(2, 0);

	sorigin[0] = c * sc[0];
	sorigin[1] = r * sc[1];

	sres[0] = m_wid / tilecol;
	sres[1] = m_hei / tilerow;

	init_sage_stuff(sorigin, sc, sres);
		
#endif
    }
}



//-----------------------------------------------------------------------------
//inplace pcaster init view settings -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::init_view_settings()
{

#ifdef _DEBUG7
    fprintf(stderr, "%d: **** %s:%s() ****\n", m_rank, __FILE__, __func__);
#endif

    m_ift = new ImageFragment_Tile(m_wid, m_hei, m_rank, m_runsize);
    m_ift->set_tile(m_tnode); //if you are not a viewer, then tile=0

    m_img_cger = new Image_Exchanger(m_rank, m_runsize);

}






//-----------------------------------------------------------------------------
//from the ofp, pick the final image fragments, 
//sync among all processes, by Allgatherv
//then ift will group the image fragments by all tiles
//-----------------------------------------------------------------------------
void Inplace_Pcaster::sync_structure()
{

#ifdef _DEBUG7
    fprintf(stderr, "%d: **** %s:%s() ****\n", m_rank, __FILE__, __func__);
#endif

    m_ift->clear();

    m_img_cger->sync_fragment_info(m_ofp, m_ift,
                                   m_nviewer);


    m_ift->compute_cross_tiles(m_screen); 


    //this will compute send and recv count and displs, and set buf offset 
    m_img_cger->arrange_buffer(m_ift, m_wid*m_hei);

#ifdef _DEBUG7
    std::ostringstream s;
    s << *m_img_cger;
    fprintf(stderr, "%s\n\n", s.str().c_str());
#endif
}



//-----------------------------------------------------------------------------
//every process pack image fragments ready to send to others. 
//-----------------------------------------------------------------------------
void Inplace_Pcaster::pack_screen_fragments()
{
#ifdef _DEBUG7
    fprintf(stderr, "%d: **** %s:%s() ****\n", m_rank, __FILE__, __func__);
#endif

    m_ift->pack_fragment_images(m_data_pool);

}



//-----------------------------------------------------------------------------
//renderers dispatch image fragments to each other
//-----------------------------------------------------------------------------
void Inplace_Pcaster::dispatch_fragments()
{
#ifdef _DEBUG7
    fprintf(stderr, "%d: **** %s:%s() ****\n", m_rank, __FILE__, __func__);
#endif

    m_img_cger->exchange_fragment_images(m_data_pool,
                                         m_nviewer,
                                         m_ift);

    //have an initial look of the image fragments, optional
    //m_ift->draw_fragment_images(m_data_pool, m_wid, m_hei);
}





//-----------------------------------------------------------------------------
//application draw -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::draw()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    
    Pcaster::draw();

    sync_structure();
    if(pcaster_options::timing){
        MPI_Barrier(MPI_COMM_WORLD);
        m_loger->log(Log_Timer::LOG_Sync);
    }

    pack_screen_fragments();
    if(pcaster_options::timing){
        m_loger->log(Log_Timer::LOG_ScreenPack);
    }

    dispatch_fragments();
    if(pcaster_options::timing){
        MPI_Barrier(MPI_COMM_WORLD);
        m_loger->log(Log_Timer::LOG_ScreenDispatch);
    }

    if(m_demo_mode == -1 && m_rank < m_nviewer) 
    {
        m_ift->compose_image(m_data_pool);
    }

    if(pcaster_options::timing){
        glFinish();
        m_loger->log(Log_Timer::LOG_View);
    }

}

void Inplace_Pcaster::draw_tile()
{
    m_ift->draw_to_framebuffer();
}

void Inplace_Pcaster::read_screen(GLenum mode, int* size, void** buf)
{
    if(m_ptr == 0) m_ptr = new GLubyte[m_wid * m_hei * 4];
    int sp[4]={0, 0, m_wid, m_hei};

    m_ift->read_back(sp, m_ptr, mode); 

    *size = 0;
    if(mode == GL_RGB) *size = m_wid * m_hei * 3;
    else if(mode == GL_RGBA) *size = m_wid * m_hei * 4;

    *buf = (void *)m_ptr;

}


/*
//-----------------------------------------------------------------------------
//process events -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::processKeys(int SDL_key)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    Pcaster::processKeys(SDL_key);

}



//-----------------------------------------------------------------------------
//process net events -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::processNetEvents()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    Pcaster::processNetEvents();
}



//-----------------------------------------------------------------------------
//process mouse events -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::processMouseMove(int m, int x, int y)
{
    Pcaster::processMouseMove(m, x, y);
}




//-----------------------------------------------------------------------------
//frame update -- virtual function
//-----------------------------------------------------------------------------
void Inplace_Pcaster::frame_update()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    Pcaster::frame_update();

}
*/
