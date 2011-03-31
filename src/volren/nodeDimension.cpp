#include <assert.h>
//#include <unistd.h>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "nodeDimension.h"
#include <pmisc.h>
#include <draw_routines.h>

const int debug_level=0;

NodeDimension::NodeDimension():
    m_k(3),
    m_direction(0),
    m_ratio(0.5),
    m_scale_thres(0.1)
{
    init();
}


NodeDimension::NodeDimension(int k, float minscale):
    m_k(k),
    m_direction(0),
    m_ratio(0.5),
    m_scale_thres(minscale)
{
    init();
}


NodeDimension::~NodeDimension()
{

}

void NodeDimension::init()
{
    m_pos.resize(m_k, 0.);
    m_scale.resize(m_k, 1.);
}

void NodeDimension::scale(std::vector<float>& scale)
{

    //item-wise mulltiply of two vector

    for(int i=0; i<m_k; i++)
    {
        m_scale[i] *= scale[i];
    }

}


void NodeDimension::translate(std::vector<float>& tr)
{
     //item-wise add of two vector
 
    for(int i=0; i<m_k; i++)
    {
        m_pos[i] += tr[i];
    }
}


void NodeDimension::kd_split(int whichhalf)
{
    
    assert(m_direction >=0 && m_direction < m_k);
    assert(m_ratio >0.0 && m_ratio < 1.0);


    std::vector<float> s(m_k, 1.0);
    std::vector<float> t(m_k, 0.0);

    //scale and translate in object space
    float sc[2];
    sc[0] = m_ratio;
    sc[1] = 1.0-m_ratio;
    float tr[2];
    tr[0] = -sc[1]/2.;
    tr[1] = sc[0]/2.;


    t[m_direction] = tr[whichhalf] * m_scale[m_direction];
    s[m_direction] = sc[whichhalf];

    translate(t);
    scale(s);
}

bool NodeDimension::howto_split()
{
    m_direction++;
    m_direction %= m_k;
    m_ratio = 0.5;
    return true;
}

void NodeDimension::split_data(int whichhalf)
{

    kd_split(whichhalf);  
}


void NodeDimension::draw_cut()
{
}

void NodeDimension::draw()
{
}

//-----------------------------------------------------------------------------
//check condition, if true, then stop. 
//-----------------------------------------------------------------------------
bool NodeDimension::apply_condition()
{
    float minscale = m_scale[0];
    for(int i=0; i<m_k; i++) 
    {
        if(m_scale[i] < minscale) minscale = m_scale[i];
    }

    if(debug_level==7)
        std::cout << __FILE__ << ":" << __func__ << ": minscale=" << minscale << "\n";

    bool condition = minscale <= m_scale_thres;
    return(condition);
}


void NodeDimension::print()
{
//     std::cout << " directon:" << m_direction << ", ratio:" << m_ratio <<
//         ", scale_thres:" << m_scale_thres << ",";

    std::cout << " pos:";
    for(int i=0; i<m_k; i++)
    {
        std::cout << m_pos[i] << ",";
    }
    std::cout << " scale:";
    for(int i=0; i<m_k; i++)
    {
        std::cout << m_scale[i] << ",";
    }
}


std::ostringstream& operator<< (std::ostringstream& stm, 
                                const NodeDimension& nd)
{
    stm <<  " pos:";
    for(int i=0; i<nd.m_k; i++)
    {
        stm << nd.m_pos[i] << ",";
    }
    stm << " scale:";
    for(int i=0; i<nd.m_k; i++)
    {
        stm << nd.m_scale[i] << ",";
    }

    return stm;
}
