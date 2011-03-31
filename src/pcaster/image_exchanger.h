#ifndef IMAGE_EXCHANGER_H
#define IMAGE_EXCHANGER_H

#include <mpi.h>

#include <string>
#include <vector>


#include "imagefragment_tile.h"
#include "overlap_footprint.h"



class Image_Exchanger{
public:


    Image_Exchanger(int rank, int runsize);
    virtual ~Image_Exchanger();

    friend std::ostringstream& operator<<(std::ostringstream& s, 
                                          Image_Exchanger& pe);


    void sync_fragment_info(OverLap_FootPrint* ofp, 
                            ImageFragment_Tile* ift,
                            int view_mode);
    void arrange_buffer(ImageFragment_Tile* ift, int offset);
    void exchange_fragment_images(unsigned int*, int, ImageFragment_Tile* ift);

private:
    int m_rank;
    int m_runsize;
    int m_total_send;
    int m_total_recv;
    int m_sbuf_offset;
    int m_rbuf_offset;


    int* m_scounts;
    int* m_sdispls;
    int* m_rcounts;
    int* m_rdispls;


};



#endif
