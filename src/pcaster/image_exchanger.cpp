#include <iostream>

#include <assert.h>

#include "image_exchanger.h"


//-----------------------------------------------------------------------------
//constructor and destructor
//-----------------------------------------------------------------------------    
Image_Exchanger::Image_Exchanger(int myrank, 
                                 int runsize):
    m_rank(myrank),
    m_runsize(runsize),
    m_total_send(0),
    m_total_recv(0),
    m_sbuf_offset(0),
    m_rbuf_offset(0)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_scounts = new int[m_runsize];
    m_sdispls = new int[m_runsize];
    m_rcounts = new int[m_runsize];
    m_rdispls = new int[m_runsize];
}


Image_Exchanger::~Image_Exchanger()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#ifdef _DEBUG
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif    

}



std::ostringstream& operator<<(std::ostringstream& s, Image_Exchanger& pe)
{

    s << pe.m_rank << ": sendcounts[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_scounts[i] << ",";
    s << "], recvcounts[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_rcounts[i] << ",";
    s << "], sdispls[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_sdispls[i] << ",";
    s << "], rdispls[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_rdispls[i] << ",";
    s << "]\n";

    s << ", total send=" << pe.m_total_send
      << ", total recv=" << pe.m_total_recv << "\n";

    s << "sbuf offset=" << pe.m_sbuf_offset
      << ", rbuf offset=" << pe.m_rbuf_offset;
    
    return s;

}



//-----------------------------------------------------------------------------
//comupute send and recv counts, and set send and recv buf offsets
//-----------------------------------------------------------------------------
void Image_Exchanger::arrange_buffer(ImageFragment_Tile* ift, int bufoffset)
{


#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    memset(m_scounts, 0, m_runsize*sizeof(unsigned int));
    memset(m_rcounts, 0, m_runsize*sizeof(unsigned int));
    ift->count_fragments(m_scounts, m_rcounts);

    //based on count arrays, compute s/r displs array
    memset(m_sdispls, 0, m_runsize*sizeof(unsigned int));
    memset(m_rdispls, 0, m_runsize*sizeof(unsigned int));
    for(int n = 1; n<m_runsize; n++)
    {
        m_sdispls[n] = m_sdispls[n-1] + m_scounts[n-1];
        m_rdispls[n] = m_rdispls[n-1] + m_rcounts[n-1];
    }

    m_total_send = m_sdispls[m_runsize-1] + m_scounts[m_runsize-1];
    m_total_recv = m_rdispls[m_runsize-1] + m_rcounts[m_runsize-1];

    m_sbuf_offset = bufoffset;
    m_rbuf_offset = bufoffset + m_total_send;    

    ift->address_fragment_packoffset(m_sbuf_offset, m_sdispls);
}




//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Image_Exchanger::exchange_fragment_images(unsigned int* databuf,
                                               int nviewer,
                                               ImageFragment_Tile* ift)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
 
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


    unsigned int* sendbuf = databuf + m_sbuf_offset;
    unsigned int* recvbuf = databuf + m_rbuf_offset;


    if(nviewer == 1)
    {
        MPI_Gatherv((int*)sendbuf, m_scounts[0], MPI_INT,
                    (int*)recvbuf, m_rcounts, m_rdispls, MPI_INT,
                    0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Alltoallv( (int*)sendbuf, m_scounts, m_sdispls, MPI_INT,
                       (int*)recvbuf, m_rcounts, m_rdispls, MPI_INT, 
                       MPI_COMM_WORLD);
    }

    ift->address_fragments(m_rbuf_offset, m_rdispls);
}





//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Image_Exchanger::sync_fragment_info(OverLap_FootPrint* ofp, 
                                         ImageFragment_Tile* ift,
                                         int nviewer)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


    std::vector<int> infobuf;
    int count = ofp->save_overlap_info(infobuf);

#ifdef _DEBUG6
    fprintf(stderr, "%d: %s: olcount=%d, olbuffer size=%ld\n", 
            m_rank, __func__, count, infobuf.size());
#endif    

    int c = infobuf.size();

//     fprintf(stderr, "%d: nviewer=%d, gather MPI_INT %d\n", 
//             m_rank, nviewer, c);


    memset(m_rcounts, 0, m_runsize*sizeof(unsigned int));

    if(nviewer == 1) 
    {
        MPI_Gather(&c, 1, MPI_INT, 
               m_rcounts, 1, MPI_INT, 
               0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Allgather(&c, 1, MPI_INT, 
                      m_rcounts, 1, MPI_INT, 
                      MPI_COMM_WORLD);
    }


    // vector throws a length_error if resized above max_size
    //terminate called after throwing an instance of 'std::length_error'
    //what():  vector::_M_fill_insert

    std::vector<int> ainfobuf(1, 0);
    memset(m_rdispls, 0, m_runsize*sizeof(unsigned int));



    if( (nviewer == 1 && m_rank==0) || (nviewer > 1) )
    {
        int total = 0;
        for(int i=0; i<m_runsize; i++) total += m_rcounts[i];

//         fprintf(stderr, "std::vector max size=%ld, resize to %d\n", 
//                 ainfobuf.max_size(), total);
        assert(total > 0);

        ainfobuf.resize(total, 0);
    }



    for(int i=0; i<m_runsize-1; i++) 
        m_rdispls[i+1] = m_rdispls[i] + m_rcounts[i];

    //to make &infobuf[0] a legal call
    if(c == 0) infobuf.resize(1);

    if(nviewer == 1) 
    {
        MPI_Gatherv(&infobuf[0], c, MPI_INT,
                    &ainfobuf[0], m_rcounts, m_rdispls, 
                    MPI_INT,
                    0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Allgatherv(&infobuf[0], c, MPI_INT,
                       &ainfobuf[0], (int*)m_rcounts, (int*)m_rdispls, 
                       MPI_INT,
                       MPI_COMM_WORLD);
    }

    //fprintf(stderr, "MPI_SUCCESS on sync frag info\n");

    //only viewer need to have all fragments and count for recv
    //non-viewer only need count send for its own fragments
    if(m_rank < nviewer)
    {
        ift->retrieve_fragments(ainfobuf);
    }
    else if(c > 0)
    {
        ift->retrieve_fragments(infobuf);
    }

}
