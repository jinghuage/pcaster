#ifndef DATALOADER_H_
#define DATALOADER_H_

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <boost/thread.hpp>



class NodeDatabrick;

class CDataLoader
{
public:
    enum dataType{DL_UBYTE=0, DL_USHORT, DL_FLOAT, DL_DOUBLE};
    enum fileType{DL_RAW=0, DL_H5, DL_RAW_4D, DL_H5_4D};
    enum dataDist{DL_SINGLE=0, DL_CHUNK, DL_REMOTE};
    enum loadMode{DL_BLOCKING=0, DL_THREAD, DL_RPC};

    CDataLoader(int rank, int runsize);
    virtual ~CDataLoader();

    void init(  std::string& filebaseName,
                std::string& datasetName,
                int fileType, 
                int comp,
                int dataType,
                int file_dist,
                int data_loadmode,
                int border);   

    //forward functions
    //ray caster call to init volume texture
    int* get_brickDims() { return m_brickDims; }
    int* get_brickPadding() { return m_brickPadding; }

    //ray caster call to compute progressive rendering attributes
    int* get_avDims() { return m_avDims; }
    int* get_avOrigin() { return m_avOrigin; }
    int* get_avPadding() {return m_avPadding; }

    //ray caster call to update volume texture
    int* get_dims() { 
        return m_chunkDims[0] > 0 ? m_chunkDims : m_avDims;         
    } 
    int* get_origin() { 
        return m_chunkDims[0] > 0 ? m_chunkOrigin : m_avOrigin;
    }
    virtual void* get_data() { return m_data; }


    //compute the brick dims and origin, as of inside a whole dataset
    void compute_local_selection(NodeDatabrick* D,
                                 std::vector<int>& fulldims,
                                 std::vector<int>& chunkdims,
                                 int timestep);


    //Non-progressive (single and blocking) data loading
    void load_data();

    //Progressive data loading
    virtual void request_data();
    virtual bool update_data();

    //post process
    void process_data(int outbyte,
                      double dmin, double dmax);

    void release_data(){ if(m_data) { free(m_data); m_data = 0; } }



protected:
    int m_rank, m_runsize;

    //the FULL data dims
    std::vector<int> m_full_dataDims;

    //data attributes
    std::string m_filebaseName;
    std::string m_datasetName;
    int m_fileType;
    int m_component;
    int m_dataType;
    int m_dataDist;
    int m_dataLoadmode;

    //special attribute
    int BORDER;


    //the local (brick) dims and origin
    //origin relative to whole dataset
    //dims includes padding. Compute before data loading. 
    int m_brickDims[3];  
    int m_brickOrigin[4];
    int m_brickPadding[6];

    //chunck dims and origin, and chunk num inside local brick
    //origin[012] relative to local brick, Update after data update
    //dims includes padding
    int m_chunkNum[3];
    int m_chunkDims[3];
    int m_chunkOrigin[4];

    //when loading progressively by chunks, the currently available data dims
    //which is less than brick but bigger then chunk
    //origin[012] relative to local brick, origin[3] is timestep
    //dims includes padding. Update after data update
    int m_avDims[3];
    int m_avOrigin[4]; //when load from back to front, origin[012] is (0,0,0)
    int m_avPadding[6];

    //the current available chunk data id
    int m_cid;

private:
    //thread also use chunkdims and chunkNum
    boost::thread* m_threads;

    //the data
    void* m_data;
    void** m_data_chunks; //if multiple chunks reading requests. 

    void start_threads();
    void wait_thread(int cid);

protected:
    //local functions
    void resolve_filename(char*, int chunkid);
    void resolve_chunk_origin(int cid);
    void resolve_av_attr();
    void loaddata_chunk(int cid);


};


#endif
