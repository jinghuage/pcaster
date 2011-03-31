#include <mpi.h>
#include <assert.h>

#include <glheaders.h>

#include "overlap_footprint.h"
#include "pixel_exchanger.h"

Pixel_Exchanger::Pixel_Exchanger(int myrank, int runsize):
  m_rank(myrank),
  m_runsize(runsize),
  m_keep_size(0),
  m_comp_size(0),
  m_total_send(0),
  m_total_recv(0),
  m_sendcounts(new unsigned int[m_runsize]),
  m_recvcounts(new unsigned int[m_runsize]),
  m_rdispls(new unsigned int[m_runsize]),
  m_sdispls(new unsigned int[m_runsize]),
  m_sbuf_offset(0),
  m_rbuf_offset(0),
  m_cbuf_offset(0),
  m_kbuf_offset(0)
{
}


//-----------------------------------------------------------------------------
//fill in count array and displs array, and section offsets
//inside data buf
//-----------------------------------------------------------------------------
void Pixel_Exchanger::arrange_buffer(OverLap_FootPrint* ofp)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    m_comp_size = m_keep_size = 0;
    m_total_send = 0;

    ofp->group_overlaps(m_keep_size, m_comp_size, m_total_send);

    ofp->countfor_my_overlaps(m_recvcounts);
    ofp->countfor_other_overlaps(m_sendcounts);

    //based on count arrays, compute s/r displs array
    memset(m_sdispls, 0, m_runsize*sizeof(unsigned int));
    memset(m_rdispls, 0, m_runsize*sizeof(unsigned int));
    for(int n = 1; n<m_runsize; n++)
    {
        m_sdispls[n] = m_sdispls[n-1] + m_sendcounts[n-1];
        m_rdispls[n] = m_rdispls[n-1] + m_recvcounts[n-1];
    }


    m_total_recv = m_rdispls[m_runsize-1] + m_recvcounts[m_runsize-1];


    //send buf offset, send to other process
    m_sbuf_offset = 0; //footprint_size;

    //keep buf offset, single contribution
    m_kbuf_offset = m_sbuf_offset + m_total_send;

    //comp buf offset. this is for local recv. compositor work on both 4 and 5 buf segs
    m_cbuf_offset = m_kbuf_offset + m_keep_size;

    //recv buf offset
    m_rbuf_offset = m_cbuf_offset + m_comp_size;


    ofp->address_single_overlaps(m_kbuf_offset);
    ofp->address_my_overlaps(m_cbuf_offset);
    ofp->address_other_overlaps(m_sbuf_offset, m_sdispls);

}





std::ostringstream& operator<<(std::ostringstream& s, Pixel_Exchanger& pe)
{


    s << pe.m_rank << ": sendcounts[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_sendcounts[i] << ",";
    s << "], recvcounts[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_recvcounts[i] << ",";
    s << "], sdispls[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_sdispls[i] << ",";
    s << "], rdispls[";
    for(int i=0; i<pe.m_runsize; i++) s << pe.m_rdispls[i] << ",";
    s << "]\n";

    s << "keep size=" << pe.m_keep_size
      << ", compsize=" << pe.m_comp_size
      << ", total send=" << pe.m_total_send
      << ", total recv=" << pe.m_total_recv << "\n";

    s << "sbuf offset=" << pe.m_sbuf_offset
      << ", rbuf offset=" << pe.m_rbuf_offset
      << ", cbuf offset=" << pe.m_cbuf_offset
      << ", kbuf offset=" << pe.m_kbuf_offset << "\n";
    
    //fprintf(stderr, "%s\n", s.str().c_str());

    return s;

}




//-----------------------------------------------------------------------------
//save the composite overlap info to be used later
//1. int n -- how many overlaps
//2. int np[n] -- num pixels for each overlap
//2. int p1 -- how many processes for overlap 1
//3. int[p1] -- offset for each process, in sorted order
//4. repeat for each n
//-----------------------------------------------------------------------------
void Pixel_Exchanger::save_comp_info(const int* sort_array, 
                                     std::vector<int>& info_array, 
                                     OverLap_FootPrint* ofp)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


    //save number of composite overlaps and their num pixels
    const std::vector<OverLap*>& composite_overlaps = ofp->get_my_overlaps();
    info_array.push_back(composite_overlaps.size());

    std::vector<OverLap*>::const_iterator oit;
    for(oit = composite_overlaps.begin(); oit!= composite_overlaps.end(); oit++)
    {
      info_array.push_back((*oit)->get_numPixels()); 
    }

    //for each composite overlap, save proc number and databuf offset for each proc
    int c_pos = 0;
    unsigned int* r_pos = new unsigned int[m_runsize];
    memcpy(r_pos, m_rdispls, m_runsize*sizeof(int));

    for(oit = composite_overlaps.begin(); oit!= composite_overlaps.end(); oit++)
    {
        int npix = (*oit)->get_numPixels();
        const std::vector<int>& procs = (*oit)->get_processes();
        info_array.push_back(procs.size());

        //----------------------------------------------------------------------
        //compute the sorted offsets for all procs in this overlap
        std::vector<int>::const_iterator iit;
        std::map<int, int> sorted_offsets;
        for(iit = procs.begin(); iit != procs.end(); iit++)
        {
            int pid = *iit;
            int offset = 0;
            if(pid == m_rank)
            { 
                offset = m_cbuf_offset + c_pos; 
                c_pos += npix; 
            }
            else{
                offset = m_rbuf_offset + r_pos[pid]; 
                r_pos[pid] += npix; 
            }
            sorted_offsets[sort_array[pid]] = offset;
        }
        //----------------------------------------------------------------------

        //now save the sorted offsets into info array
        std::map<int, int>::iterator mit;
        for(mit = sorted_offsets.begin(); mit != sorted_offsets.end(); mit++)
        {
            info_array.push_back(mit->second - m_cbuf_offset);
        }
    }



#ifdef _DEBUG7
    std::ostringstream s;
    s << m_rank << ": comp info(";
    std::vector<int>::iterator iit;
    for(iit=info_array.begin(); iit<info_array.end(); iit++)
    {
        s << *iit << ",";
    }
    s << ")\n";
    fprintf(stderr, "%s", s.str().c_str());
#endif

}




//-----------------------------------------------------------------------------
//helper function: determine if maxtrix m1 and m2 are of each other's transpose
//-----------------------------------------------------------------------------
static bool is_transpose(int* m1, int* m2, int dim, int& sproc, int& rproc)
{
    bool ret = true;

    int row=0;
    int col=0;

    for(row=0; row<dim; row++)
    {
        for(col=0; col<dim; col++)
        {
            int id1 = row*dim + col;
            int id2 = col*dim + row;
            if(m1[id1] != m2[id2])
            {
                ret = false;
                fprintf(stderr, "mismatch: m1[%d][%d] = %d, m2[%d][%d]=%d\n",
                        row, col, m1[id1], col, row, m2[id2]);
                sproc = row;
                rproc = col;

                break;
            }
        }
        if(ret == false) break;
    }

    return ret;
}

static void make_transpose(int* m1, int* m2, int dim)
{
    int row=0;
    int col=0;

    for(row=0; row<dim; row++)
    {
        for(col=0; col<dim; col++)
        {
            int id1 = row*dim + col;
            int id2 = col*dim + row;
            if(m1[id1] != m2[id2])
            {
                int minval = std::min(m1[id1], m2[id2]);
                m1[id1] = m2[id2] = minval;
            }
        }
    }
}



static void print_matrix(int rank, int size, const char* des, int* m)
{
    std::ostringstream s;

    s << rank << ": " << des << "\n";
    for(int i=0; i < size; i++) 
    {
        for(int j=0; j<size; j++)
            s << m[i * size + j] << "\t";
        s << "\n";
    }
    //s << "]";

    fprintf(stderr, "%s\n", s.str().c_str());

}



//-----------------------------------------------------------------------------
//check mismatch with all parallel peers
//-----------------------------------------------------------------------------
void Pixel_Exchanger::checkMismatch()
{

    //everybody exchange their sendcounts and recvcounts into sendcount
    //and recvcount matrix
    int *sc_matrix = new int[m_runsize * m_runsize];
    int *rc_matrix = new int[m_runsize * m_runsize];

    MPI_Allgather(m_sendcounts, m_runsize, MPI_INT, 
                  sc_matrix, m_runsize, MPI_INT,
                  MPI_COMM_WORLD);
    MPI_Allgather(m_recvcounts, m_runsize, MPI_INT, 
                  rc_matrix, m_runsize, MPI_INT,
                  MPI_COMM_WORLD);

    //everybody confirm sc_matrix is transpose of rc_matrix
    //save the mismatch send/recv process in sproc and rproc;
    int sproc=0;
    int rproc=0;
    bool t = is_transpose(sc_matrix, rc_matrix, m_runsize, sproc, rproc);

    if(!t) 
    {
        fprintf(stderr, "%d: error: send and recv counts don't match!\n", 
                m_rank);

        //rank 0 print
        if(m_rank == 0)
        {
            print_matrix(m_rank, m_runsize, "sc_matrix", sc_matrix);
            print_matrix(m_rank, m_runsize, "rc_matrix", rc_matrix);
        }


        //choose to pick a minimun value matrix to make the two matrix
        //transposable.
        make_transpose(sc_matrix, rc_matrix, m_runsize);

        //update the count array for data transfer
        memcpy(m_sendcounts, 
               sc_matrix + m_rank * m_runsize, 
               m_runsize*sizeof(int));
        memcpy(m_recvcounts, 
               rc_matrix + m_rank * m_runsize,
               m_runsize*sizeof(int));
    }


    delete[] sc_matrix;
    delete[] rc_matrix;

}


void Pixel_Exchanger::dataTransfer(unsigned int* netbuf)
{
 
    //now call MPI_Alltoallv()
    unsigned int* sbuf = netbuf + m_sbuf_offset;
    unsigned int* rbuf = netbuf + m_rbuf_offset;
    int ret = MPI_Alltoallv( sbuf, (int*)m_sendcounts, (int*)m_sdispls, MPI_INT,
                             rbuf, (int*)m_recvcounts, (int*)m_rdispls, MPI_INT, 
                             MPI_COMM_WORLD );

    if(ret != MPI_SUCCESS)
    {
        char error_string[1024];
        int err_string_len, err_class;

        MPI_Error_class(ret, &err_class);
        MPI_Error_string(err_class, error_string, &err_string_len);
        fprintf(stderr, "%s\n", error_string);
        MPI_Error_string(ret, error_string, &err_string_len);
        fprintf(stderr, "%s\n", error_string);

        std::ostringstream info;
        info << *this;
        fprintf(stderr, "%s\n", info.str().c_str());
    }
//     else
//     {
//         fprintf(stderr, "%d:MPI_Alltoallv success\n", m_rank);
//     }
    fflush(stderr);

}

