#include <assert.h>

#include <draw_routines.h>

#include "compose_viewer.h"
#inlcude "mpi_interactor.h"
#include "socket_interactor.h"


//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
Compose_Viewer::Compose_Viewer(int w, int h, 
                               int gw, int gh, 
                               int nclients):
    Viewer(w, h, gw, gh, nclients),
    m_ift(0)
{

    init_ift();
}



Compose_Viewer::~Compose_Viewer()
{
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}



//-----------------------------------------------------------------------------
//assert the remote mode -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::assert_application_mode()
{
    if(pcaster_options::standalone == 1)
    {
        fprintf(stderr, "standalone mode doesn't need viewer\n");
        exit(0);
    }

    if(!pcaster_options::viewer.compare("none"))
    {
        fprintf(stderr, "No viewer is required\n");
        exit(0);
    }

}



//-----------------------------------------------------------------------------
//init the image fragment tile -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::init_ift()
{
    m_ift = new ImageFragment_Tile(0, 0, gw, gh);

    //single tile viewer
    NodeRectile tile(1, m_gw, m_gh);
    m_ift->set_tile(&tile);
}



//-----------------------------------------------------------------------------
//init the interactor between remote render and viewer -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::init_interactor()
{
    //only init m_streamer if not inplace viewer
    if(pcaster_options::viewer.compare("inplace"))
    {
        m_streamer = new Socket_Interactor(0, 
                                           User_Interactor::NET_SERVER,
                                           m_numClients,
                                           pcaster_options::viewer);
    }
}



//-----------------------------------------------------------------------------
//renderer sync info to viewer -- virtual function
//renderer save the segment_polygon of their overlaps, 
//and send them to viewer. Viewer will transform them into image fragments.
//-----------------------------------------------------------------------------
void Compose_Viewer::sync_render_structure()
{
    std::vector<int> infobuf;
    std::vector<int> ainfobuf;

    if(m_streamer) //master server is also master client
    {
        m_streamer->sync_info(infobuf, ainfobuf); //
    }

    retrieve_structure(ainfobuf);
}



//-----------------------------------------------------------------------------
//for inplace viewer, without interactor -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::retrieve_structure(std::vector<int> ainfobuf)
{
    m_ift->clear();
    m_ift->retrieve_fragments(ainfobuf);
}



//-----------------------------------------------------------------------------
//pcaster send pixels to viewer -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::stream_pixels()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_ift->count_fragments(m_recvcounts);

    memset(m_rdispls, 0, m_runsize*sizeof(unsigned int));
    for(int n = 1; n<m_numClients; n++)
    {
        m_rdispls[n] = m_rdispls[n-1] + m_recvcounts[n-1];
    }

    m_streamer->recv_pixel(m_data_pool, m_recvcounts, m_rdispls);
    m_streamer->send_ack(&m_fid);

    m_fid++;
}



//-----------------------------------------------------------------------------
//draw the received pixels in full resolution -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::draw_global_screen()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_ifp->address_fragments();

    m_global_screen->bind_render_buffer();
    m_ift->draw_fragment_pixels(m_data_pool, m_gw, m_gh);
    m_global_screen_unbind_render_buffer();

}



//-----------------------------------------------------------------------------
//application draw -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::draw()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    Viewer::draw();
}


//-----------------------------------------------------------------------------
//for inplace viewer, without interactor -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::compose_image(unsigned int* databuf)
{
    m_ifp->address_fragments();

    m_global_screen->bind_render_buffer();
    m_ift->draw_fragment_pixels(databuf, m_gw, m_gh);
    m_global_screen_unbind_render_buffer();

    //draw part of offscreen global image onto screen
    int ofs_vp[4] = {0, 0, m_gw, m_gh};
    int s_vp[4] = {0, 0, m_wid, m_hei};
    std::string shaderpath(pcaster_options::SHADER_PATH);


    m_global_screen->draw_to_framebuffer(ofs_vp, 
                                         s_vp, 
                                         shaderpath);

}


//-----------------------------------------------------------------------------
//process events -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::processKeys(int key)
{
    Viewer::processKeys(key);
}

//-----------------------------------------------------------------------------
//process mouse events -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::processMouseMove(int m, int x, int y)
{
    Viewer::processMouseMove(m, x, y);
}


//-----------------------------------------------------------------------------
//process net events -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::processNetEvents()
{
    Viewer::processNetEvents();
}


//-----------------------------------------------------------------------------
//frame update -- virtual function
//-----------------------------------------------------------------------------
void Compose_Viewer::frame_update(float* navi, int num)
{
    Viewer::frame_update(navi, num);
}
