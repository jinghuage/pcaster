
#include <assert.h>
#include <math.h>
#include <string.h>


#include <iostream>
#include <sstream>
#include <string>


#include "nodeDatabrick.h"
#include "dataloader.h"
#include "read_datafile.h"

//-----------------------------------------------------------------------------
//static function, call low-level data read routines
//-----------------------------------------------------------------------------
void read_data_fromfile(void*& data, 
                        int fileType, 
                        int dataType, 
                        int component,
                        int full_dims[4],
                        const char* filename, 
                        const char* datasetName, 
                        int origin[4], 
                        int dims[3])
{   
    switch(fileType)
    {
    case CDataLoader::DL_RAW:
        data = load_Raw_Data(filename, 
                             dataType, 
                             full_dims[0], full_dims[1], full_dims[2],                             dims[0], dims[1], dims[2], 
                             origin[0], origin[1], origin[2], 
                             component);
        break;
    case CDataLoader::DL_RAW_4D:
        data = load_Raw_Data_4D(filename, 
                             dataType, 
                             full_dims[0], full_dims[1], full_dims[2], full_dims[3],
                             dims[0], dims[1], dims[2], 
                             origin[0], origin[1], origin[2], 
                             origin[3],
                             component);
        break;


#ifdef _HDF5
    case CDataLoader::DL_H5:
        data = load_H5_Data(filename, dataType, datasetName,
                            dims[0], dims[1], dims[2],
                            origin[0], origin[1], origin[2],
                            component, false);
        break;
    case CDataLoader::DL_H5_4D:
        data = load_H5_Data_4D(filename, dataType, datasetName,
                               dims[0], dims[1], dims[2],
                               origin[0], origin[1], origin[2],
                               origin[3],
                               component, false);
        break;
#endif
    }

}


//-----------------------------------------------------------------------------
//class CDataLoader
//-----------------------------------------------------------------------------

CDataLoader::CDataLoader(int rank, int runsize):
    m_rank(rank),
    m_runsize(runsize),
    m_full_dataDims(4, 0),
    m_data(0),
    m_data_chunks(0)
{
    memset(m_brickDims, 0, 3*sizeof(int));
    memset(m_brickOrigin, 0, 4*sizeof(int));
    memset(m_brickPadding, 0, 6*sizeof(int));

    memset(m_chunkNum, 0, 3*sizeof(int));
    memset(m_chunkDims, 0, 3*sizeof(int));
    memset(m_chunkOrigin, 0, 4*sizeof(int));

    memset(m_avDims, 0, 3*sizeof(int));
    memset(m_avOrigin, 0, 4*sizeof(int));
    memset(m_avPadding, 0, 6*sizeof(int));

    m_threads = 0;
    m_cid = 0;
}


CDataLoader::~CDataLoader()
{
}


void CDataLoader::init(  std::string& filebaseName,
                         std::string& datasetName,
                         int fileType, 
                         int comp,
                         int dataType,
                         int data_dist,
                         int data_loadmode,
                         int border)
{
    m_filebaseName = filebaseName;
    m_datasetName = datasetName;
    m_fileType = fileType;
    m_component = comp;
    m_dataType = dataType;
    m_dataDist = data_dist;
    m_dataLoadmode = data_loadmode;
    BORDER = border;
}


//-----------------------------------------------------------------------------
//data loader will determine brick dimensions, brick origins
//and init dataDims and dataOrigin. Also count chunks.
//-----------------------------------------------------------------------------
void CDataLoader::compute_local_selection(NodeDatabrick* D,
                                          std::vector<int>& fulldims,
                                          std::vector<int>& chunkdims,
                                          int timestep)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

//    fprintf(stderr, "timestep %d\n", timestep);


    const float* pos = D->get_pos();
    const float* scale = D->get_scale();


    for(int i=0; i<3; i++)
    {
        float le = pos[i] - scale[i]/2.0;
        float re = pos[i] + scale[i]/2.0;
        int lp = 0;
        int rp = 0;
        if(le > -0.5) lp = 1;
        if(re < 0.5) rp = 1;
//         if(lp == 0) rp = 2;
//         if(rp == 0) lp = 2;
        
        m_full_dataDims[i] = fulldims[i];

        //compute the brick dims and origin, as of inside a whole dataset
        m_brickPadding[i*2] = lp*BORDER;
        m_brickPadding[i*2 + 1] = rp*BORDER;
        m_brickDims[i] = m_full_dataDims[i] * scale[i] + (lp+rp) * BORDER;
        m_brickOrigin[i] = m_full_dataDims[i] * (le+0.5) - lp*BORDER;

        //set the chunk dims and num
        if(chunkdims[i])
        {
            m_chunkDims[i] = chunkdims[i];
            m_chunkNum[i] = m_brickDims[i] / m_chunkDims[i];
        }
    }

    m_full_dataDims[3] = fulldims[3];
    m_brickOrigin[3] = timestep;

    //set avOrigin to be brickDims, ready to be updated after data update
    memcpy(m_avOrigin, m_brickDims, 3*sizeof(int));
    memcpy(m_avPadding, m_brickPadding, 6*sizeof(int)); 

#ifdef _DEBUG7
    fprintf(stderr, "brick dims(%d,%d,%d), brick origin(%d,%d,%d)\n",
            m_brickDims[0], m_brickDims[1], m_brickDims[2],
            m_brickOrigin[0],m_brickOrigin[1],m_brickOrigin[2]);
#endif
}




//-----------------------------------------------------------------------------
//used in application init, load data in full amount, ***NON-PROGRESSIVE***
//-----------------------------------------------------------------------------
void CDataLoader::load_data()
{
    //DL_SINGLE + DL_BLOCKING  = NON-PROGRESSING DATA LOADING
    assert(m_dataDist == DL_SINGLE && m_dataLoadmode == DL_BLOCKING);

    //no chunks. Read in the whole brick
    char filename[128];
    resolve_filename(filename, -1);
    //std::cout << filename <<"\n";

    //read_data_fromfile is static function
    read_data_fromfile(m_data, 
                       m_fileType, 
                       m_dataType, 
                       m_component,
                       &m_full_dataDims[0],
                       filename, 
                       m_datasetName.c_str(), 
                       m_brickOrigin, 
                       m_brickDims);

    memcpy(m_avDims, m_brickDims, 3*sizeof(int));
    memset(m_avOrigin, 0, 4*sizeof(int));
    memcpy(m_avPadding, m_brickPadding, 6*sizeof(int)); 

    //this is wrong. av is relative to the brick(include brick padding)
    //, origin[0, 0, 0], dim equals to brick dim
    //for(int i=0; i<3; i++) m_avOrigin[i] -= m_avPadding[2*i];
}


//-----------------------------------------------------------------------------
//used in application init, init the data reques tfor progressive update later
// -- virtual function
//for data distribution: DL_CHUNK
//-----------------------------------------------------------------------------
void CDataLoader::request_data()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    //DL_CHUNK || DL_REMOTE  = PROGRESSING DATA LOADING
    assert(m_dataDist == DL_CHUNK);
    m_cid = 0;
 
    switch(m_dataLoadmode)
    {
    case DL_BLOCKING: break;
    case DL_THREAD:  start_threads(); break;
    default: m_data=0;
    }
}


//-----------------------------------------------------------------------------
//used in application frame update (idle) to update new data -- virtual function
//get data and update dataDims|Origin progresively until all data is updated
//for data distribution: DL_CHUNK
//and load mode:  DL_BLOCKING or DL_THREAD
//-----------------------------------------------------------------------------
bool CDataLoader::update_data()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(m_cid >= m_chunkNum[0]*m_chunkNum[1]*m_chunkNum[2]) return false;

    assert(m_dataDist == DL_CHUNK);

    switch(m_dataLoadmode)
    {
    case DL_BLOCKING: loaddata_chunk(m_cid); break;
    case DL_THREAD:  wait_thread(m_cid); break;
    default: m_data = 0;
    }
    
    m_cid++;
    return true;
}


//-----------------------------------------------------------------------------
//resolve filename. For DL_SINGLE, chunkid=-1
//chunk id is also thread id
//-----------------------------------------------------------------------------
void CDataLoader::resolve_filename(char* fname, int chunkid)
{
    std::ostringstream filename;

    filename << m_filebaseName;

    if(m_full_dataDims[3] > 0) 
    {
        if(m_fileType == DL_RAW || m_fileType == DL_H5) //3D dataset
        {
            filename <<  "-t" << m_brickOrigin[3];
        }
    }

    //actually chunkid need to be formatted as "%04d"

    if(chunkid >= 0){
        //calculate the chunk position inside the brick
        int chunkID[3];

        chunkID[2] = chunkid / (m_chunkNum[0] * m_chunkNum[1]);
        int oxy  = chunkid % (m_chunkNum[0] * m_chunkNum[1]);
        chunkID[1] = oxy / m_chunkNum[0];
        chunkID[0] = oxy % m_chunkNum[0];

#ifdef _DEBUG
        fprintf(stderr, "chunkID in brick(%d,%d,%d)\n",
                chunkID[0], chunkID[1], chunkID[2]);
#endif

        int fn[3];
        for(int i=0; i<3; i++)
        {
            int chunkLE = m_brickOrigin[i]/m_chunkDims[i];
            chunkID[i] += chunkLE;
            fn[i] = m_full_dataDims[i] / m_chunkDims[i];
        }

        fprintf(stderr, "chunkID in whole dataset(%d,%d,%d)\n",
                chunkID[0], chunkID[1], chunkID[2]);

        int fileid = chunkID[2] * fn[1] * fn[0] + chunkID[1] * fn[0] + chunkID[0];

        //fprintf(stderr, "chunk fileid=%d\n", fileid);

        char fileext[16];
        sprintf(fileext, ".cpu%04d", fileid);
        filename << fileext;
    }
    else
    {
        switch(m_fileType)
        {
        case DL_RAW:
        case DL_RAW_4D:
            filename << ".raw"; break;

#ifdef _HDF5
        case DL_H5: 
        case DL_H5_4D:
            filename << ".h5"; break;
#endif
        default: 
            assert(0); break;
        }    
    }
    //ERROR: return a local variable
    //return filename.str().c_str();


    strcpy(fname, filename.str().c_str());
}



//-----------------------------------------------------------------------------
//update m_chunkOrigin by chunkid
//all parameters relative to the local brick.
//-----------------------------------------------------------------------------
void CDataLoader::resolve_chunk_origin(int cid)
{
    int chunkID[3];
    chunkID[2] = cid / (m_chunkNum[0] * m_chunkNum[1]);
    int oxy  = cid % (m_chunkNum[0] * m_chunkNum[1]);
    chunkID[1] = oxy / m_chunkNum[0];
    chunkID[0] = oxy % m_chunkNum[0];

    for(int i=0; i<3; i++){
        m_chunkOrigin[i] = chunkID[i] * m_chunkDims[i];
    }
}


//-----------------------------------------------------------------------------
//update m_avDims, Origin, and Padding
//all parameters relative to the local brick.
//-----------------------------------------------------------------------------
void CDataLoader::resolve_av_attr()
{
    for(int i=0; i<3; i++)
    {
        //update av dims and origin
        int le = m_chunkOrigin[i];
        int re = le + m_chunkDims[i];
        if(re > m_avDims[i]) m_avDims[i] = re;
        if(le < m_avOrigin[i]) m_avOrigin[i] = le;
        

        //update av padding
        le = m_avOrigin[i];
        re = le + m_avDims[i];
        int lp = le > 0 ? 1 : 0;
        int rp = re < m_brickDims[i] ? 1 : 0;
        m_avPadding[2*i] = lp*BORDER;
        m_avPadding[2*i+1] = rp*BORDER;
    }
}
 




//-----------------------------------------------------------------------------
//load a chunk in progressive data loading, DL_BLOCKING + DL_CHUNK
//-----------------------------------------------------------------------------
void CDataLoader::loaddata_chunk(int cid)
{

    char filename[128];
    resolve_filename(filename, cid);
    //std::cout << "chunk: " << cid << ", filename: " << filename <<"\n";

    int fileOrigin[4] = {0,0,0,0};
    read_data_fromfile(m_data, 
                       m_fileType, 
                       m_dataType, 
                       m_component,
                       m_chunkDims,
                       filename, 
                       m_datasetName.c_str(), 
                       fileOrigin,
                       m_chunkDims);

    resolve_chunk_origin(cid);
    resolve_av_attr();
}




//-----------------------------------------------------------------------------
//start threads
//-----------------------------------------------------------------------------
void CDataLoader::start_threads()
{
    int N = m_chunkNum[0] * m_chunkNum[1] * m_chunkNum[2];
    if(m_threads){ delete[] m_threads; m_threads = 0; }
    if(m_threads == 0) m_threads = new boost::thread[N];

    //allocate data pointer for all N chunks;
    //if data too large may not need to allocate all N, maybe smaller number
    if(m_data_chunks == 0) m_data_chunks = (void**)malloc(N * sizeof(void*));

    int fileOrigin[4] = {0,0,0,0};
    for(int i=0; i<N; i++)
    {
        char filename[128];
        resolve_filename(filename, i);

        //each chunk in separate file
        m_threads[i] = boost::thread(boost::bind(&read_data_fromfile,
                                                 m_data_chunks[i],
                                                 m_fileType, 
                                                 m_dataType, 
                                                 m_component,
                                                 m_chunkDims,
                                                 filename, 
                                                 m_datasetName.c_str(), 
                                                 fileOrigin,
                                                 m_chunkDims));
        resolve_chunk_origin(i);
    }
}



//-----------------------------------------------------------------------------
//wait thread cid
//-----------------------------------------------------------------------------
void CDataLoader::wait_thread(int cid)
{
    m_threads[cid].join();
}



//-----------------------------------------------------------------------------
//data post processing
//-----------------------------------------------------------------------------
void CDataLoader::process_data(int outbyte,
                               double dmin, double dmax)
{
//if native data is not ubyte
    if(m_dataType != DL_UBYTE)
    {
        size_t size = m_brickDims[0] * m_brickDims[1] * m_brickDims[2]; 

        //if you want to have the real minmax value retrieved from data
        //find_minmax(m_data, size, &dmin, &dmax);

        //normalize data based on data range, transform to byte if needed
        apply_data_range(&m_data, size, m_dataType, dmin, dmax, outbyte);
    }
}


