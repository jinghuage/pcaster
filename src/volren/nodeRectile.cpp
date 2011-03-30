#include <assert.h>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "nodeRectile.h"

#include <draw_routines.h>


//-----------------------------------------------------------------------------
//constructor and destructor
//-----------------------------------------------------------------------------
NodeRectile::NodeRectile(int row, int col):
    NodeDimension(2, 0.0)
{
    m_screen_dims[0] = col;
    m_screen_dims[1] = row;
}



NodeRectile::~NodeRectile()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}


void NodeRectile::print()
{
    NodeDimension::print();
}


std::ostringstream& operator<< (std::ostringstream& stm, 
                                const NodeRectile& nd)
{
    stm << (NodeDimension&)nd;

    return stm;
}


//draw the cutting plane/line
void NodeRectile::draw_cut()
{
    NodeDimension::draw_cut();
}

//draw the tile as quad
void NodeRectile::draw()
{

}


bool NodeRectile::apply_condition()
{
    return(m_screen_dims[0] == 1 && m_screen_dims[1] == 1);
}


bool NodeRectile::howto_split()
{

    int d = m_screen_dims[0] > m_screen_dims[1] ? 0 : 1;

    m_direction = d;
    m_ratio = float(m_screen_dims[d] / 2) / float(m_screen_dims[d]);

#ifdef _DEBUG7
    std::cout <<"howto_split(): direction=" << m_direction << ",ratio=" << m_ratio << "\n";
#endif

    return true;
}


void NodeRectile::split_data(int whichhalf)
{

    kd_split(whichhalf);

    int leap[2];
    leap[1] = m_screen_dims[m_direction] / 2;
    leap[0] = m_screen_dims[m_direction] - leap[1];

    m_screen_dims[m_direction] = leap[whichhalf];

}
