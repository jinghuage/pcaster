#ifndef TILE_H
#define TILE_H

#include <string.h>
#include <iostream>
#include <vector>

#include "nodeRectile.h"


class Tile
{
public:
    Tile(int, int);
    ~Tile();

    void set_screenport(int sp[4]) 
    {
      memcpy(m_sp, sp, 4*sizeof(int)); 
    }
    int* get_screenport(){ return m_sp; }


    void compute_screenport(NodeRectile* tnode,
                            int render_wid,
                            int render_hei);


private:
    int m_rank;
    int m_runsize;
    int m_sp[4];

};


#endif
