#ifndef SEGMENT_POLYGON_H_
#define SEGMENT_POLYGON_H_


#include <vector>
#include <sstream>
#include <iostream>




class Segment_Polygon
{
public:
    Segment_Polygon();
    virtual ~Segment_Polygon();

    virtual void print();



public:
    //void print(std::ostringstream&);
    friend std::ostringstream& operator<< (std::ostringstream&, const Segment_Polygon&);


    void pack_pixel_data(const unsigned int* inbuf, 
                          unsigned int in_wid, 
                          unsigned int in_hei, 
                          unsigned int* outbuf);
    void pack_pixel_position(unsigned short* buf, int ox, int oy);

    void serialize_structure(int ox, int oy, 
                             std::vector<int>& olbuf);
    int deserialize_structure(int* olbuffer);

    void insert_segment(int scanline, int sl, int sr);

    int get_root() const { return m_root; }
    void set_root(int root) { m_root = root; }

    bool compare(const Segment_Polygon* O);

    int get_numPixels() const { return m_numPixels; }
    void set_numPixels(int np) { m_numPixels = np; }

    int get_databuf_offset() const { return m_databuf_offset; }
    void set_databuf_offset(int d) { m_databuf_offset = d; }

    int* get_bbox(){ return m_bbox; }

    const std::vector<unsigned int>& get_segments() const { return m_segments; }

    void define_GL_lines();    


protected: 
    int m_root;
    int m_numPixels;

    //the relative offset into the continuous memory space where data is stored
    int m_databuf_offset; 
    int m_bbox[4];  //left, right, bottom, up

    std::vector<unsigned int> m_segments;
};


#endif
