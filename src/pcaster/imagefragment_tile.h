#ifndef IMAGEFRAGMENT_TILE_H
#define IMAGEFRAGMENT_TILE_H

#include <string.h>
#include <iostream>
#include <vector>
#include <map>


#include "image_fragment.h"
#include "nodeRectile.h"
#include "tile.h"
#include "renderscreen.h"
#include "tiled_display.h"

//extract overlaps from a footprint and save as a std::map data structure
class ImageFragment_Tile
{
public:
    ImageFragment_Tile(int w, int h, int myrank, int runsize);
    ~ImageFragment_Tile();


    void clear()
    {
        std::vector<ImageFragment* >::iterator mit;
        for(mit = m_frags.begin(); mit != m_frags.end(); mit++)
        {
            ImageFragment* ol = *mit;
            if(ol){ delete ol; }
        }
        m_frags.clear();
    }

    void print()
    {
        std::vector<ImageFragment* >::iterator mit;
        for(mit = m_frags.begin(); mit != m_frags.end(); mit++)
        {
            ImageFragment* ol = *mit;
            if(ol) ol->print();
        }
    }

    void compute_cross_tiles(Tiled_Display* screen);

    void count_fragments(int* scounts, int* rcounts);
    void address_fragments(int rbufoffset, int* rdispls);
    void address_fragment_packoffset(int sbufoffset, 
                            int* sdispls);
    void retrieve_fragments(std::vector<int>& olbuffer);
    void pack_fragment_images(unsigned int* sendbuf);
    void draw_fragment_images(unsigned int* databuf, int wid, int hei);
    void compose_image(unsigned int* pix_buffer);
    void set_tile(NodeRectile* t){
        m_tnode = t; 
        if(m_tnode)
            m_tile->compute_screenport(m_tnode, m_render_wid, m_render_hei); 
    }


    //todo: Maybe should think of ImageFragment_Tile as a subclass of 
    //class CRenderScreen. 
    void read_back(int sp[4], unsigned char* ptr, GLenum mode);
    void draw_to_framebuffer();
   

private:
    int m_render_wid;
    int m_render_hei;
    int m_rank;
    int m_runsize;

    
    NodeRectile* m_tnode;
    Tile* m_tile;
    CRenderScreen* m_global_screen;

    //all of my fragments
    std::vector<ImageFragment*> m_frags;


};


#endif
