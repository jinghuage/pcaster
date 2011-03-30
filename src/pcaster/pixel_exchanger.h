#ifndef PIXEL_EXCHANGER_H
#define PIXEL_EXCHANGER_H

#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>

#include "nodeDatabrick.h"
#include "overlap_footprint.h"

//extract overlaps from a footprint and save as a std::map data structure
class Pixel_Exchanger
{
public:
    Pixel_Exchanger(int myrank, int runsize);
    ~Pixel_Exchanger();

    friend std::ostringstream& operator<<(std::ostringstream& s, Pixel_Exchanger& pe);

    void arrange_buffer(OverLap_FootPrint* ofp);
    void save_comp_info(const int* sort_array, 
                        std::vector<int>& info_array, 
                        OverLap_FootPrint* ofp);

    unsigned int get_pre_compsize(){ return m_comp_size + m_total_recv; }
    unsigned int get_compsize() { return m_comp_size; }
    unsigned int get_compbuf_offset(){ return m_cbuf_offset; }
    unsigned int get_keepsize() { return m_keep_size; }
    unsigned int get_keepbuf_offset(){ return m_kbuf_offset; }
    unsigned int get_recvbuf_offset(int i)
    {
        return m_rbuf_offset + m_rdispls[i];
    }
    
    //MPI calls
    void checkMismatch();
    void dataTransfer(unsigned int* netbuf);

private:
    int m_rank;
    int m_runsize;

    unsigned int m_keep_size;
    unsigned int m_comp_size;
    unsigned int m_total_send;
    unsigned int m_total_recv;

    unsigned int* m_sendcounts;
    unsigned int* m_recvcounts;
    unsigned int* m_rdispls;
    unsigned int* m_sdispls;


    unsigned int m_sbuf_offset;
    unsigned int m_rbuf_offset;
    unsigned int m_cbuf_offset;
    unsigned int m_kbuf_offset;



    //only used when measure exchange data count matrix
    //int *sc_matrix;
    //int *rc_matrix;
};


#endif
