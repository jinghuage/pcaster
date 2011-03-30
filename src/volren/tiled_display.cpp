
#include "glheaders.h"
#include "draw_routines.h"

#include "tiled_display.h"


//-----------------------------------------------------------------------------
//constructor and destructor
//-----------------------------------------------------------------------------
Tiled_Display::Tiled_Display(int w, int h, int row, int col):
    m_wid(w),
    m_hei(h),
    m_row(row),
    m_col(col),
    m_screen(0)
{
    init_screenTree(row, col);
}


Tiled_Display::~Tiled_Display()
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

}


//-----------------------------------------------------------------------------
//init data tree
//-----------------------------------------------------------------------------
void Tiled_Display::init_screenTree(int row, int col)
{
#ifdef _DEBUG7    
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);


    fprintf(stderr, "tilerow=%d, tilecol=%d\n",
            row, col);
#endif

    m_screen = new screenTree(NodeRectile(row, col));
    m_screen->build_tree();
    m_screen->rank_tree(0);


    m_screen->retrieve_nodedata(tiles);

}


static bool overlap(int al, int ar, int bl, int br)
{
    bool ret = false;
    
    ret  = al>=bl && al<=br;
    ret |= ar>=bl && ar<=br;
    ret |= al<=bl && ar>=br;
    ret |= bl<=al && br>=ar;
    
    return ret;
}

//precondition: usually b1(a frag) is much smaller than b2(a screen tile)
static bool cross(int* b1, int* b2)
{
    bool ret = false;

    ret = overlap(b1[0], b1[1], b2[0], b2[1]);
    ret &= overlap(b1[2], b1[3], b2[2], b2[3]);

    return ret;
}


void Tiled_Display::cross_tile(int* bbox, std::vector<int>& tileids)
{

// #ifdef _DEBUG
//     fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

//     fprintf(stderr, "frag bbox(%d, %d, %d, %d)\n", 
//             bbox[0], bbox[1], bbox[2], bbox[3]);
// #endif

    std::vector<NodeRectile*>::iterator nit;

    int tilebbox[4];
    int tid=0;
    for(nit = tiles.begin(); nit != tiles.end(); nit++)
    {
        NodeRectile* tile = *nit;
        const float* npos = tile->get_pos();
        const float* nscale = tile->get_scale();
        tilebbox[0] = m_wid * (0.5 + npos[0] - nscale[0]/2.0);
        tilebbox[2] = m_hei * (0.5 + npos[1] - nscale[1]/2.0);
        tilebbox[1] = tilebbox[0] + m_wid * nscale[0];
        tilebbox[3] = tilebbox[2] + m_hei * nscale[1];

        if(cross(bbox, tilebbox)) tileids.push_back(tid);
        tid++;
    }
}
