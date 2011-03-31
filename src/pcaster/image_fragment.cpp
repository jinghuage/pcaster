#include <assert.h>
#include <string.h>


#include <iostream>
#include <algorithm>
#include <bitset>
//#include <sstream>



#include "image_fragment.h"



//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
ImageFragment::ImageFragment():
    Segment_Polygon()
{
}


ImageFragment::~ImageFragment()
{
}

//-----------------------------------------------------------------------------
//print structure -- virtual function
//-----------------------------------------------------------------------------
void ImageFragment::print()
{
    Segment_Polygon::print();

    std::vector<int>::iterator iit;

    std::cout << " cross tiles: ";
    for(iit = m_cross_tiles.begin(); iit != m_cross_tiles.end(); iit++)
    {
        std::cout << *iit << ", ";
    }

    std::cout << " pack offsets: ";
    for(iit = m_pack_offsets.begin(); iit != m_pack_offsets.end(); iit++)
    {
        std::cout << *iit << ", ";
    }
}


//void ImageFragment::print(std::ostringstream& ol_stream)
std::ostringstream& operator<< (std::ostringstream& ol_stream, 
                                const ImageFragment& frag)
{
    ol_stream << (Segment_Polygon&)frag;

    std::vector<int>::const_iterator iit;

    ol_stream << " cross tiles: ";
    for(iit = frag.m_cross_tiles.begin(); iit != frag.m_cross_tiles.end(); iit++)
    {
        ol_stream << *iit << ", ";
    }

    ol_stream << " pack offsets: ";
    for(iit = frag.m_pack_offsets.begin(); iit != frag.m_pack_offsets.end(); iit++)
    {
        ol_stream << *iit << ", ";
    }

    return ol_stream;
}
