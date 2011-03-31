#include <assert.h>
#include <string.h>


#include <iostream>
#include <algorithm>
#include <bitset>
//#include <sstream>

#include <texture.h>

#include "segment_polygon.h"



//-----------------------------------------------------------------------------
//application constructor and destructor
//-----------------------------------------------------------------------------
Segment_Polygon::Segment_Polygon():
    m_root(0),
    m_numPixels(0),
    m_databuf_offset(-1)
{
    for(int i=0; i<4; i++) m_bbox[i] = -1;
}


Segment_Polygon::~Segment_Polygon()
{
}

//-----------------------------------------------------------------------------
//print structure -- virtual function
//-----------------------------------------------------------------------------=
void Segment_Polygon::print()
{
    std::cout << "numPixels: " << m_numPixels << ", "
              << "root: " << m_root << ", "
              << "segments:(" << m_segments.size()/3 << "), "
              << "bufoffset: " << m_databuf_offset;
}



//void Segment_Polygon::print(std::ostringstream& ol_stream)
std::ostringstream& operator<< (std::ostringstream& ol_stream, 
                                const Segment_Polygon& ol)
{
    ol_stream << "numPixels: " << ol.m_numPixels << ", "
              << "root: " << ol.m_root << ", "
              << "segments:(" << ol.m_segments.size()/3 << "), "
              << "bufoffset: " << ol.m_databuf_offset;

    return ol_stream;
}



bool Segment_Polygon::compare(const Segment_Polygon* O)
{
    bool ret = true;


    //check num pixel match.
    if(m_numPixels != O->get_numPixels())
    {
        fprintf(stderr, "No match: num pixels\n");
        ret = false;
    }

    //check segments match.
    const std::vector<unsigned int>& seg = O->get_segments();
    if(seg.size() != m_segments.size())
    {
        fprintf(stderr, "No match: segment size\n");
        ret = false;
    }

    for(unsigned int i=0; i<seg.size()/3; i++)
    {
        unsigned int slid = i*3;
        unsigned int s1 = seg[slid];
        unsigned int l1 = seg[slid + 1];
        unsigned int r1 = seg[slid + 2];
        unsigned int np1 = r1 - l1 + 1;

        unsigned int s2 = m_segments[slid];
        unsigned int l2 = m_segments[slid + 1];
        unsigned int r2 = m_segments[slid + 2];
        unsigned int np2 = r2 - l2 + 1;
        if(np1 != np2)
        {
            fprintf(stderr, "No match seg %d: %d:%d(%d-%d), %d:%d(%d-%d)\n", 
                    i, s2, np2, l2, r2, s1, np1, l1, r1);
            ret = false;
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
//save number of segments, and the segment data 
//into an int vector.
//ox, oy: offset from local coord to global coord
//-----------------------------------------------------------------------------
void Segment_Polygon::serialize_structure(int ox, int oy, 
                                  std::vector<int>& olbuf)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    int size_segs = m_segments.size();
    int size_olbuf = olbuf.size();
    int size = size_olbuf + 4 + size_segs;
    olbuf.resize(size, 0);

    int sid = size_olbuf;
    olbuf[sid++] = m_root;
    olbuf[sid++] = m_numPixels;
    olbuf[sid++] = m_databuf_offset;
    olbuf[sid++] = size_segs;

#ifdef _DEBUG7
    fprintf(stderr, "root=%d, npix=%d, bufoffset=%d, sizeseg=%d\n",
            m_root, m_numPixels, m_databuf_offset, size_segs);
#endif


    assert( m_segments.size() % 3 == 0 );

    for(int i=0; i<size_segs/3; i++)
    {
        olbuf[sid++] = m_segments[i*3] + oy;
        olbuf[sid++] = m_segments[i*3+1] + ox;
        olbuf[sid++] = m_segments[i*3+2] + ox;
    }
}



int Segment_Polygon::deserialize_structure(int* olbuffer)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


    int count = 0;

    m_root = olbuffer[0];
    m_numPixels = olbuffer[1];
    m_databuf_offset = olbuffer[2];
    unsigned int size_segs = olbuffer[3];
    count += 4;


#ifdef _DEBUG7
    fprintf(stderr, "root=%d, npix=%d, bufoffset=%d, sizeseg=%d\n",
            m_root, m_numPixels, m_databuf_offset, size_segs);
#endif


    //reset npix to 0, because later segments will be inserted and 
    //npix counted. I send npix value just to get more debug info.
    m_numPixels = 0;


    for(unsigned int i=0; i<size_segs/3; i++)
    {
        int scanline = olbuffer[count++];
        int sl = olbuffer[count++];
        int sr = olbuffer[count++];
        insert_segment(scanline,
                       sl,
                       sr);

    }

    return count;
}



//-----------------------------------------------------------------------------
//Given the initial 2D pixel buffer, pack the pixel data inside overlap
//into the continuous(linear) logic buffer
//-----------------------------------------------------------------------------
void Segment_Polygon::pack_pixel_data(const unsigned int* inbuf, 
                               unsigned int in_wid, 
                               unsigned int in_hei,
                               unsigned int* outbuf)
{
    assert( m_segments.size() % 3 == 0 );

    std::vector<unsigned int>::iterator iit = m_segments.begin();
    int offset = 0;

    while(iit!=m_segments.end())
    {
        unsigned int scanline = *iit++;
        unsigned int left_extent = *iit++;
        unsigned int right_extent = *iit++;

//         std::cout << "segment: (" << scanline << "," 
//                   << left_extent << "," << right_extent << ")\n";

        assert(left_extent >= 0 && left_extent < in_wid);
        assert(right_extent >= 0 && right_extent < in_wid);

        if(scanline < 0 || scanline >= in_hei)
        {
            fprintf(stderr, "scanline error: %d of %d\n", scanline, in_wid);
            exit(1);
        }

        assert(scanline >= 0 && scanline < in_hei);
        assert(left_extent <= right_extent);

        int cpy_bytes = (right_extent - left_extent + 1) * sizeof(unsigned int);
        memcpy(outbuf + offset, 
               inbuf + in_wid*scanline + left_extent, 
               cpy_bytes);
               
        offset += (right_extent - left_extent + 1);
    }
}



//-----------------------------------------------------------------------------
//pack the segments as a list of point positions
//-----------------------------------------------------------------------------
void Segment_Polygon::pack_pixel_position(unsigned short* buf, int ox, int oy)
{
    unsigned short* p = buf;

    std::vector<unsigned int>::iterator iit = m_segments.begin();
    while(iit!=m_segments.end())
    {
        unsigned int scanline = *iit++;
        unsigned int left_extent = *iit++;
        unsigned int right_extent = *iit++;
        for(unsigned int i=left_extent; i<=right_extent; i++)
        {
            *p++ = (unsigned short)i + (unsigned short)ox;
            *p++ = (unsigned short)scanline + (unsigned short)oy;            
        }
    }
}


//-----------------------------------------------------------------------------
//draw lines and draw polygons: seems some pixels different??
//-----------------------------------------------------------------------------
void Segment_Polygon::define_GL_lines()
{
    //std::cout << "segment size: " << m_segments.size() << ", ";

    assert( m_segments.size() % 3 == 0 );

    std::vector<unsigned int>::iterator iit = m_segments.begin();
    while(iit!=m_segments.end())
    {
        unsigned int scanline = *iit++;
        unsigned int left_extent = *iit++;
        unsigned int right_extent = *iit++;
        glVertex2f(left_extent - 0.5, scanline + 0.5);
        glVertex2f(right_extent + 0.5, scanline + 0.5);
    }
}



void Segment_Polygon::insert_segment(int scanline, int sl, int sr)
{
    //fprintf(stderr, "(%d, %d, %d) ", scanline, sl, sr);

    assert(scanline >= 0);
    assert(sl >= 0);
    assert(sr >= 0);
    assert(sl <= sr);

    m_segments.push_back(scanline);
    m_segments.push_back(sl);
    m_segments.push_back(sr);

    if(m_bbox[0] == -1) m_bbox[0] = sl;
    if(m_bbox[1] == -1) m_bbox[1] = sr;
    if(m_bbox[2] == -1) m_bbox[2] = scanline;
    if(m_bbox[3] == -1) m_bbox[3] = scanline;


    if(m_bbox[2] > scanline) m_bbox[2] = scanline;
    if(m_bbox[3] < scanline) m_bbox[3] = scanline;
    if(m_bbox[0] > sl) m_bbox[0] = sl;
    if(m_bbox[1] < sr) m_bbox[1] = sr;

    m_numPixels += (sr - sl + 1);
}


