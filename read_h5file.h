#ifndef READ_H5FILE_H_
#define READ_H5FILE_H_

struct admbase_metadata
{
    int    global_size[3];
    double time;
    double origin[3];
    double min_ext[3];
    double max_ext[3];
    double delta[3];

    int numofchunks;
    int numofck[3];

    float* iso_prms;
};


struct admbase_chunk
{
    int dimensions;
    int dims[3];
    int chunk_origin[3];
    
    float tos[6]; //computed texture offset due to overlapping border
    int cid[3];        //chunk id in coord system
    float g_scale[3], g_pos[3]; //location in global space

    double min, max;
    unsigned int *histogram;

    void* data;
    
};

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

admbase_metadata* init_admbase(bool);
int read_h5file(int, admbase_chunk*, bool);


#endif
