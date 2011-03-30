#ifndef IMAGE_FRAGMENT_H_
#define IMAGE_FRAGMENT_H_


#include <vector>
#include <sstream>
#include <iostream>

#include "segment_polygon.h"


class ImageFragment: public Segment_Polygon
{
public:
    ImageFragment();
    virtual ~ImageFragment();

    virtual void print();


    static const std::string myname;


public:
    friend std::ostringstream& operator<< (std::ostringstream&, 
                                           const ImageFragment&);


    void copy_pixel_data(unsigned int* buf)
    {
        std::vector<int>::iterator iit;
        for(iit = m_pack_offsets.begin(); iit != m_pack_offsets.end(); iit++)
        {
            int pos = *iit;
            memcpy(buf+pos, buf+m_databuf_offset, m_numPixels*sizeof(unsigned int));
        }
    }


    void insert_pack_offset(int pos){ m_pack_offsets.push_back(pos); }


    std::vector<int>& get_cross_tiles(){ return m_cross_tiles; }
    std::vector<int>& get_pack_offsets(){ return m_pack_offsets; }

protected:

    //image fragments in tiles etc.

    std::vector<int> m_cross_tiles;
    std::vector<int> m_pack_offsets;

};


#endif
