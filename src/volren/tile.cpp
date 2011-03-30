#include <assert.h>

#include <glheaders.h>

#include "tile.h"


Tile::Tile(int rank, int runsize):
    m_rank(rank),
    m_runsize(runsize)
{
    memset(m_sp, 0, 4*sizeof(int));
}



Tile::~Tile()
{

}




void Tile::compute_screenport(NodeRectile* tnode,
                              int render_wid,
                              int render_hei)
{

    const float* npos = tnode->get_pos();
    const float* nscale = tnode->get_scale();

    m_sp[0] = render_wid * (0.5 + npos[0] - nscale[0]/2.0);
    m_sp[1] = render_hei * (0.5 + npos[1] - nscale[1]/2.0);
    m_sp[2] = render_wid * nscale[0];
    m_sp[3] = render_hei * nscale[1];


#ifdef _DEBUG6
    fprintf(stderr, "%d: Tile::%s: screenport(%d,%d,%d,%d)\n", 
            m_rank, __func__, m_sp[0], m_sp[1], m_sp[2], m_sp[3]);
#endif
}


