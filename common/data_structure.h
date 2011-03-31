#ifndef DATA_STRUCTURE_H_
#define DATA_STRUCTURE_H_ 

//global data information
struct volume_metadata
{
    int dimensions;
    int dims[3];
    int component;

    float time;
    float origin[3];
    float min_ext[3];
    float max_ext[3];
    float delta[3];
    float scale[3];

    int numofchunks;
    int numofck[3];

    int numofgroups;
    int numofgrp[3];

    int TFsize;
    unsigned char *TF;
    unsigned char* preInt;
};


//global data are composed of multiple chunks
struct volume_chunk
{
    int dimensions;
    int component;
    int stride;
    int dims[3];
    int origin[3];
    
    int g_id[3];
    float g_scale[3];
    float g_pos[3];
    double min, max;
    unsigned int *histogram;

    void* data;
    
};

//chunks are organized into groups
//group the the atomic data block renderer draws (per pass or per node)
struct volume_chunk_grp
{
    int dimensions;
    int component;
    int dims[3];
    int origin[3];
    
    float tos[6]; //computed texture offset due to overlapping border
    int g_id[3];        //id in global coord system
    float g_scale[3], g_pos[3]; //location in global space

    int numofchunks;
    volume_chunk** cks;
};

#endif
