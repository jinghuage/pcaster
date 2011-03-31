#ifndef FOOTPRINT_H
#define FOOTPRINT_H

#include <string.h>
#include <iostream>
#include <vector>

#include "nodeDatabrick.h"
#include "frustumCull.h"
#include "colorkey.h"
#include "scanline.h"

class FootPrint
{
public:
    FootPrint();
    FootPrint(int id, int layer);
    ~FootPrint();

    void set_id(int id) { m_id = id; }
    void set_screenport(int sp[4]) 
    {
      memcpy(m_sp, sp, 4*sizeof(int)); 
    }
    int* get_screenport(){ return m_sp; }

    const ScanLine& get_sl(int i) const { return m_sl[i]; }


    void compute_bbox(NodeDatabrick* node,
                      CFrustumCull* culler, float pbr[4]);

    void compute_screenport(float pbr[4],
                            int render_wid,
                            int render_hei);

    void set_stencil(NodeDatabrick* node);
    void draw_node_bboxes(std::vector<NodeDatabrick*>& dnodes, 
                 int start, int end, int LP);

    unsigned int* get_map();
    void read_map();
    void scan_map();


    void clear()
    {
        if(m_map) {delete[] m_map; m_map = 0; }
        if(m_sl){ delete[] m_sl; m_sl=0; } 
        m_sp[0] = m_sp[1] = m_sp[2] = m_sp[3] = 0;
    }

    void print()
    {
        for(int i=0; i<m_sp[3]; i++)
        {
            std::cout << i << ": ";
            m_sl[i].print();
            m_sl[i].print_key();
        }
    }

    FootPrint& operator+=(const FootPrint& rhs);
    FootPrint& operator=(const FootPrint& rhs);

    void set_origin() { 
      m_slon = 0;
      for(int i=0; i<m_sp[3]; i++) m_sl[i].set_origin();
    }

    bool retrieve_seg(unsigned int& s, unsigned int& l, unsigned int& r, ColorKey& ck);

private:
    int m_id;
    int m_layer; //valid in multilayered footprint, also determine the key slot
    unsigned int* m_map;
    ScanLine* m_sl; 
    int m_slon;
    int m_sp[4];

};


#endif
