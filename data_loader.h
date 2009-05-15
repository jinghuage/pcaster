#ifndef DATA_LOADER_H_
#define DATA_LOADER_H_


#include "read_rawfile.h"
#include "glheaders.h"


#if defined(__REMOTE_HDF5__) || defined(__LOCAL_HDF5__)
#include "uniformgrids/4DUniformRequest.h"
#include "ClientCreator.h"
#include "uniformgrids/4DUniformRequest.h"
#include <log4cxx/basicconfigurator.h>



#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>


#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <fstream>

#endif

//global data information
struct admbase_metadata
{
    int dimensions;
    int dims[3];
    int origin[3];

    double time;
    double min_ext[3];
    double max_ext[3];
    double delta[3];

    int numofchunks;
    int numofck[3];

    float* iso_prms;
};

//global data are composed of multiple chunks
struct admbase_chunk
{
    int dimensions;
    int dims[3];
    int origin[3];
    
    float tos[6]; //computed texture offset due to overlapping border
    int cid[3];        //chunk id in coord system
    float g_scale[3], g_pos[3]; //location in global space

    double min, max;
    unsigned int *histogram;

    void* data;
    
};

//chunks are organized into groups
//group the the atomic data block renderer draws (per pass or per node)
struct admbase_chunk_grp
{
    int dimensions;
    int dims[3];
    int origin[3];
    
    float tos[6]; //computed texture offset w/ or w/t overlapping border
    float g_scale[3], g_pos[3]; //location in global space
    float g_boundary[6]; 

    int numofchunks_grp;
    
};

class data_loader
{
public:
    data_loader();
    ~data_loader();

    void init_from_rawvol(int, int);
    void init_from_h5vol(int, int,  bool, bool, float);
    void compute_global_attrs(int, float);
    void compute_grp_attrs(float);
    void init_TF_manual();
    void init_TF_auto();

    float* get_global_pos() { return m_cks_grp->g_pos; }
    float* get_global_scale() { return m_cks_grp->g_scale; }
    float* get_tex_offset() { return m_cks_grp->tos; }
    float* get_boundary() { return m_cks_grp->g_boundary; }

    unsigned char* get_TF() {return m_vol->TF; }
    int   get_TFsize() {return m_vol->TFsize; }

    int* get_global_size() { return m_metadata->dims;}
    int get_chunk_num() { return m_metadata->numofchunks; }

    int* get_grp_size() { return m_cks_grp->dims; }
    int get_grpchunk_num() {return m_cks_grp->numofchunks_grp; }
    int* get_grp_origin() { return m_cks_grp->origin; }

    int* get_chunk_origin(int n) { return m_cks[n]->origin; }
    int* get_chunk_dims(int n) { return m_cks[n]->dims; }
    void* get_chunk_data(int n) { return m_cks[n]->data; }

private:
    volModel* m_vol;
    admbase_metadata *m_metadata;
    admbase_chunk_grp *m_cks_grp;
    admbase_chunk **m_cks;

};

#endif
