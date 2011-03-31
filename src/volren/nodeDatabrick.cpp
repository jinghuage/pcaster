#include <assert.h>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "nodeDatabrick.h"
#include "frustumCull.h"

#include <draw_routines.h>

const int debug_level=0;

NodeDatabrick::NodeDatabrick():
    NodeDimension(3, 0.0),
    m_bricksize(1)
{
    m_physical_scale[0] = 1.0;
    m_physical_scale[1] = 1.0;
    m_physical_scale[2] = 1.0;
}

NodeDatabrick::NodeDatabrick(int bricksize):
    NodeDimension(3, 0.0),
    m_bricksize(bricksize)
{
    m_physical_scale[0] = 1.0;
    m_physical_scale[1] = 1.0;
    m_physical_scale[2] = 1.0;
}

NodeDatabrick::NodeDatabrick(int bricksize, float data_scale[3]):
    NodeDimension(3, 0.0),
    m_bricksize(bricksize)
{
    m_physical_scale[0] = data_scale[0];
    m_physical_scale[1] = data_scale[1];
    m_physical_scale[2] = data_scale[2];
}


NodeDatabrick::~NodeDatabrick()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}


void NodeDatabrick::print()
{

    NodeDimension::print();
    std::cout << " brick size:" << m_bricksize << "\n";
}







std::ostringstream& operator<<(std::ostringstream& s, 
                                NodeDatabrick& nd)
{
    s << (NodeDimension&)nd;
    s << " brick size:" << nd.m_bricksize;
    
    return s;
}





//draw the cutting plane
void NodeDatabrick::draw_cut()
{
    float position = m_pos[m_direction] + (m_ratio-0.5) * m_scale[m_direction];
//     std::cout << "cut plane: direction=" << m_direction
//               << ", position=" << position << "\n";
        
    glColor4f(0.5-m_pos[0], 0.5-m_pos[1], 0.5-m_pos[2], 0.5);
    draw_boxCutPlane(m_direction, 
                     position, 
                     &m_scale.front(),
                     &m_pos.front());

}

void NodeDatabrick::draw()
{
    //std::cout <<"direction=" << m_direction << ",ratio=" << m_ratio << "\n";
    //NodeDimension::draw();

    float color[4]={0.5, 0.5, 0.5, 0.5};
    //float color[4]={0., 0., 1., 0.5};
    //for(int i=0; i<3; i++) 
    //color[0] = 0.35*(0.5 - m_pos[0]);
    //color[1] = 0.5*(0.5 - m_pos[1]);
    //color[2] = 0.35*(0.5 + m_pos[2]);
    drawColorBox(get_scale(), get_pos(), color);

    //drawColorBox(get_scale(), get_pos(), 0);
    
}


bool NodeDatabrick::apply_condition()
{
    return(m_bricksize == 1);
}


bool NodeDatabrick::howto_split()
{
    //determine direction and ratio here
    //pick a direction with max split scale
    //if equal, split priority is z-y-x
    float split_scale[3];
    for(int i=0; i<3; i++) split_scale[i] = m_physical_scale[i] * m_scale[i];

    float s = std::max(split_scale[0], std::max(split_scale[1], split_scale[2]));                       
    int d = 2;
    if(s == split_scale[2]) d = 2;
    else if(s == split_scale[1]) d = 1;
    else if(s == split_scale[0]) d = 0;


    m_direction = d;
    m_ratio = 0.5;

    if(debug_level == 7)
    std::cout <<"howto_split(): direction=" << m_direction << ",ratio=" << m_ratio << "\n";
    
    return true;
}


void NodeDatabrick::split_data(int whichhalf)
{

    kd_split(whichhalf);

    int leap[2];
    leap[1] = m_bricksize/2;
    leap[0] = m_bricksize - leap[1];

    m_bricksize = leap[whichhalf];

}


bool NodeDatabrick::compute_projection_bbox(CFrustumCull* fculler, float* bbox)
{
    bool ret = fculler->get_boundingbox(get_pos(),
                                        get_scale(), 
                                        bbox);

    return ret;
}
