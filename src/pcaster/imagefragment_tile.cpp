#include <assert.h>
#include <algorithm>

#include <draw_routines.h>

#include "imagefragment_tile.h"
#include "pcaster_options.h"



//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
ImageFragment_Tile::ImageFragment_Tile(int w, int h, int rank, int runsize):
    m_render_wid(w),
    m_render_hei(h),
    m_rank(rank),
    m_runsize(runsize),
    m_tnode(0),
    m_tile(0),
    m_global_screen(0)
{
    m_tile = new Tile(m_rank, m_runsize);
    m_global_screen = new CRenderScreen(m_render_wid, 
                                        m_render_hei, 
                                        CRenderScreen::SINGLE,
                                        GL_TEXTURE_RECTANGLE_ARB, 
                                        GL_RGBA, 
                                        GL_RGBA, 
                                        GL_UNSIGNED_BYTE,
                                        GL_NEAREST, GL_NEAREST);   

}



ImageFragment_Tile::~ImageFragment_Tile()
{

}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ImageFragment_Tile::retrieve_fragments(std::vector<int>& infobuf)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    int count = 0;
    int size = infobuf.size();

    while(count < size)
    {
        ImageFragment* frag = new ImageFragment;
        int r = frag->deserialize_structure(&infobuf[count]);
        count += r;

        m_frags.push_back(frag);
    }
        
    infobuf.clear();

#ifdef _DEBUG6
    fprintf(stderr, "%d: %s: retrieve total imagefragments num: %ld\n", 
            m_rank, __func__, m_frags.size() );
#endif

}




//-----------------------------------------------------------------------------
//for all the fragments I have hold of, compute the crossing tiles list
//usually viewer have fragments from ALL renderers
//non-viewer will only have fragments of its own
//-----------------------------------------------------------------------------
void ImageFragment_Tile::compute_cross_tiles(Tiled_Display* screen)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


    std::vector<ImageFragment*>::iterator it;

    for(it = m_frags.begin(); it<m_frags.end(); it++)
    {
        ImageFragment* frag = *it;


        screen->cross_tile(frag->get_bbox(), frag->get_cross_tiles());

#ifdef _DEBUG7
        std::ostringstream s;
        s << *frag;
        fprintf(stderr, "%d: frag: %s\n", m_rank, s.str().c_str());
#endif


    }

}





//-----------------------------------------------------------------------------
//for frags I own, count if I need to send to others
//for frags others own, count if I need to receive from others
//-----------------------------------------------------------------------------
void ImageFragment_Tile::count_fragments(int* scounts, int* rcounts)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    memset(scounts, 0, m_runsize*sizeof(int));
    memset(rcounts, 0, m_runsize*sizeof(int));

    std::vector<ImageFragment*>::iterator it;

    for(it = m_frags.begin(); it<m_frags.end(); it++)
    {
        ImageFragment* frag = *it;
        int root = frag->get_root();
        int npix = frag->get_numPixels();
        std::vector<int>& crosstiles = frag->get_cross_tiles();

        if(root == m_rank) //I own the frag, count send
        {
            std::vector<int>::iterator iit;
            for(iit = crosstiles.begin(); iit != crosstiles.end(); iit++)
            {
                int t = *iit;
                if(t != m_rank) scounts[*iit] += npix;                
            }
        }
        else //others own the frag, count recv
        {
            if(std::find(crosstiles.begin(), crosstiles.end(), m_rank)
               != crosstiles.end())
                rcounts[root] += npix;
        }

    }
}



//-----------------------------------------------------------------------------
//fora frags I own, compute pack offset into sendbuf, 
//may be packed to multiple slots when frag cross multiple tiles.
//sorted by process rank, ready to be used in MPI calls
//-----------------------------------------------------------------------------
void ImageFragment_Tile::address_fragment_packoffset(int sbufoffset, 
                                                      int* sdispls)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    std::vector<ImageFragment*>::iterator it;
    int* spos = new int[m_runsize];
    memcpy(spos, sdispls, m_runsize * sizeof(int)); 

    for(it = m_frags.begin(); it<m_frags.end(); it++)
    {
        ImageFragment* frag = *it;
        int root = frag->get_root();
        int npix = frag->get_numPixels();

        if(root == m_rank)
        {
            std::vector<int>& crosstiles = frag->get_cross_tiles();

            std::vector<int>::iterator iit;
            for(iit = crosstiles.begin(); iit != crosstiles.end(); iit++)
            {
                int t = *iit;
                if(t != m_rank) frag->insert_pack_offset(sbufoffset+spos[t]);
                spos[t] += npix;
            }

#ifdef _DEBUG7
            std::ostringstream s;
            s << *frag;
            fprintf(stderr, "%d: frag: %s\n", m_rank, s.str().c_str());
#endif
        }

    }
}



//-----------------------------------------------------------------------------
//for frags I own, if I need to send them to others, pack them into send buf
//-----------------------------------------------------------------------------
void ImageFragment_Tile::pack_fragment_images(unsigned int* sendbuf)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    std::vector<ImageFragment*>::iterator it;
    for(it = m_frags.begin(); it<m_frags.end(); it++)
    {
        ImageFragment* frag = *it;
        int root = frag->get_root();

        if(root == m_rank)
        {
            frag->copy_pixel_data(sendbuf);
        }
    }
}



//-----------------------------------------------------------------------------
//after image dispatch to tile, now identify all fragments inside tile
//and set correct databuf_offset, get ready to draw them
//-----------------------------------------------------------------------------
void ImageFragment_Tile::address_fragments(int rbufoffset, int* rdispls)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    std::vector<ImageFragment*>::iterator iit;

    int* rpos = new int[m_runsize];
    memcpy(rpos, rdispls, m_runsize * sizeof(int)); 

    for(iit = m_frags.begin(); iit != m_frags.end(); iit++)
    {
        ImageFragment* frag = *iit;
        int root = frag->get_root();
        int npix = frag->get_numPixels();
        std::vector<int>& crosstiles = frag->get_cross_tiles();

        if(std::find(crosstiles.begin(), crosstiles.end(), m_rank) 
           != crosstiles.end())
        {
            if(root != m_rank)
            {
                frag->set_databuf_offset(rbufoffset + rpos[root]);
                rpos[root] += npix;
            }
        }
        else frag->set_databuf_offset(-1); 

#ifdef _DEBUG7
        std::ostringstream s;
        s << *frag;
        fprintf(stderr, "%d: frag: %s\n", m_rank, s.str().c_str());
#endif

    }

}



//-----------------------------------------------------------------------------
//if I am viewer, I draw all fragments with databuf_offset not set as -1
//-----------------------------------------------------------------------------
void ImageFragment_Tile::draw_fragment_images(unsigned int* pix_buffer, 
                                              int wid, int hei)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, wid, hei);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, wid, 0, hei, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int* sp = m_tile->get_screenport(); 

    //std::map<ColorKey, OverLap*>::iterator mit;
    std::vector<ImageFragment* >::iterator mit;
    for(mit=m_frags.begin(); mit!=m_frags.end(); mit++)
    {
        ImageFragment* frag = *mit;
        if(frag->get_databuf_offset() != -1)
        {
            unsigned int size = frag->get_numPixels();
            unsigned short* pos_buffer = new unsigned short[2 * size];


            frag->pack_pixel_position(pos_buffer, -sp[0], -sp[1]);

            glVertexPointer(2, GL_SHORT, 0, pos_buffer);
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, 
                           pix_buffer + frag->get_databuf_offset());


            glDrawArrays(GL_POINTS, 0, size);

            delete[] pos_buffer;
        }
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    //checkError
    if(printOpenGLError()) exit(1);

}


//-----------------------------------------------------------------------------
//draw full-resolution image to offscreen first, and pick where user 
//wants to see and draw onscreen
//for viewers only
//-----------------------------------------------------------------------------
void ImageFragment_Tile::compose_image(unsigned int* pix_buffer)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    int* sp = m_tile->get_screenport();

    m_global_screen->bind_render_buffer();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_fragment_images(pix_buffer, sp[2], sp[3]);

    m_global_screen->unbind_render_buffer();    
}


void ImageFragment_Tile::read_back(int sp[4], unsigned char* ptr, GLenum mode)
{
    m_global_screen->read_back(sp, ptr, mode);
}


void ImageFragment_Tile::draw_to_framebuffer()
{
    int* sp = m_tile->get_screenport();

    //draw part of offscreen global image onto screen tile
    int ofs_vp[4] = {0, 0, sp[2], sp[3]};
    int s_vp[4] = {0, 0, sp[2], sp[3]};

    std::string shaderpath(pcaster_options::SHADER_PATH);

    m_global_screen->draw_to_framebuffer(ofs_vp, 
                                         s_vp, 
                                         shaderpath);

}
