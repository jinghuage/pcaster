#include <assert.h>
#include <string.h>


#include <iostream>
#include <algorithm>
#include <bitset>
//#include <sstream>

#include <texture.h>

#include "overlap.h"



//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
OverLap::OverLap():
    Segment_Polygon(),
    m_singleton(false)
{
}


OverLap::~OverLap()
{
}



//-----------------------------------------------------------------------------
//print structure -- virtual function
//-----------------------------------------------------------------------------
void OverLap::print()
{
    Segment_Polygon::print();
    
    std::cout << " processes: ";

    std::vector<int>::iterator iit;
    for(iit = m_processes.begin(); iit != m_processes.end(); iit++)
    {
        std::cout << *iit << ", ";
    }
}


//void OverLap::print(std::ostringstream& ol_stream)
std::ostringstream& operator<< (std::ostringstream& ol_stream, const OverLap& ol)
{
    ol_stream << (Segment_Polygon&)ol;

    ol_stream << " processes: ";

    std::vector<int>::const_iterator iit;
    for(iit = ol.m_processes.begin(); iit != ol.m_processes.end(); iit++)
    {
        ol_stream << *iit << ", ";
    }


    return ol_stream;
}




//-----------------------------------------------------------------------------
//get processes from colorkey
//-----------------------------------------------------------------------------
void OverLap::get_proc_from_key(const ColorKey& key)
{
    for(unsigned int i=0; i<ColorKey::keyLen; i++)
    {
      unsigned int k = key.get_key(i);
        std::bitset<32> bk(k);
        for(int j=0; j<32; j++) 
            if(bk[j]) m_processes.push_back(j);
    }
}


//-----------------------------------------------------------------------------
//compute root
//-----------------------------------------------------------------------------
inline bool Positive(int i) { return i >= 0; }
void OverLap::compute_root()
{
    m_root = 0;
    int nproc = m_processes.size();
    if(nproc == 1) //no overlap, single contribution
    {
        m_root = m_processes[0];
        m_singleton = true;
        return;
    }


    unsigned int id = 1;
    for(int i=0; i<nproc; i++) id += m_processes[i];
    id %= nproc;

    m_root = m_processes[id];

    //fprintf(stderr, "nproc=%d, root=%d\n", nproc, m_root);

}


