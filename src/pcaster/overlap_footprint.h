#ifndef OVERLAP_FOOTPRINT_H
#define OVERLAP_FOOTPRINT_H

#include <string.h>
#include <iostream>
#include <vector>
#include <map>

#include "renderscreen.h"
#include "data_hierarchy.h"
#include "overlap.h"
#include "footprint.h"

//extract overlaps from a footprint and save as a std::map data structure
class OverLap_FootPrint
{
public:
    OverLap_FootPrint( int myrank, int runsize, int w, int h);
    ~OverLap_FootPrint();

    void clear()
    {
        std::vector<OverLap* >::iterator mit;
        for(mit = m_overlaps.begin(); mit != m_overlaps.end(); mit++)
        {
            OverLap* ol = *mit;
            if(ol){ delete ol; }
        }
        m_overlaps.clear();
        m_my_ol.clear();
        m_single_ol.clear();
        m_other_ol.clear();
    }

    void print()
    {
        std::vector<OverLap* >::iterator mit;
        for(mit = m_overlaps.begin(); mit != m_overlaps.end(); mit++)
        {
            OverLap* ol = *mit;
            if(ol) ol->print();
        }
    }

    void build(int nodenum,
               Data_Hierarchy* dataTree,
               CFrustumCull* fculler);

    void build_onscreen(int nodenum,
               Data_Hierarchy* dataTree,
               CFrustumCull* fculler);

    void set_overlaps(std::vector<OverLap*>& ols)
    {
        m_overlaps = ols;
    }


    FootPrint* get_footprint() { return m_fp; }

    void group_overlaps(unsigned int& single_count,
                        unsigned int& my_count,
                        unsigned int& other_count);

    void address_single_overlaps(unsigned int);
    void address_my_overlaps(unsigned int);
    void address_other_overlaps(unsigned int, unsigned int*);
    void address_overlaps(unsigned int* offsets, int nc);

    void countfor_my_overlaps(unsigned int* count_array);
    void countfor_other_overlaps(unsigned int* count_array);
    void count_overlaps(unsigned int* count_array, int nc);

    int save_overlap_info(std::vector<int>& olbuffer);
    void retrieve_overlaps(std::vector<int>& olbuffer);

    void pack_overlap_pixels(unsigned int* logicbuffer);
    void draw_overlap_pixels(unsigned int* pixbuf, int wid, int hei, int);
    void draw_recv_pixels(int recvfrom, 
                          unsigned int* pix_buffer, 
                          unsigned int rpos,
                          int wid, int hei);

    const std::vector<OverLap*>& get_my_overlaps() const { return m_my_ol; }
    const std::vector<OverLap*>& get_all_overlaps() const { return m_overlaps; }

    static const unsigned int Layer_Procs;
   
private:
    int m_rank;
    int m_runsize;

    int m_render_wid;
    int m_render_hei;

    FootPrint* m_fp;
    CRenderScreen* m_ofScreen;


    //all of my overlaps
    std::vector<OverLap*> m_overlaps;

    //overlap sub-groups 
    std::vector<OverLap*> m_single_ol;
    std::vector<OverLap*> m_my_ol;
    std::vector<OverLap*> m_other_ol;


    int construct_overlaps(FootPrint& fp);
    void validate_overlap_segments(int wid, int hei);
};


#endif
