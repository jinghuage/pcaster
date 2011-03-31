#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "misc.h"
#include "data_structure.h"
#include <string>

bool load_volume_data(const char* filename, volume_chunk* vol)
{

    printf("--------------------------------------------------------\n");
    printf("DEBUG: %s @ %d: load_Volume_Data\n", __FILE__, __LINE__);
    printf("--------------------------------------------------------\n");


    int W = vol->dims[0];
    int H = vol->dims[1];
    int D = vol->dims[2];

    // Reading of requested part of a volume
    int comp = vol->component;
//    assert ( comp == 1 );

    int stride = sizeof(float);
    vol->stride = stride;

    printf("W=%d, H=%d, D=%d, comp=%d, stride=%d\n",
           W, H, D, comp, stride); 

    float* wholedata = (float*)malloc(W*H*D*comp*stride);
    memset(wholedata, 0, W*H*D*comp*stride);

    FILE *fp = fopen(filename, "r");
    if( fp == NULL )
    {
        printf("Can't open model data file %s\n", filename);
        exit(1);
    }

    int i, j, k, id;
    float value;
    int WH = W*H;
    while(!feof(fp))
    {
        fscanf( fp, "%d %d %d %f\n", &k, &j, &i, &value);
        id = k * WH + j*W + i;
        wholedata[id] = value;
    }
    fclose(fp);

    vol->data = (void*)wholedata;

    return true;
}



bool write_volume_data(const char* filename, volume_chunk* vol)
{

    printf("--------------------------------------------------------\n");
    printf("DEBUG: %s @ %d: write_volume_data\n", __FILE__, __LINE__);
    printf("--------------------------------------------------------\n");


    int W = vol->dims[0];
    int H = vol->dims[1];
    int D = vol->dims[2];

    // Reading of requested part of a volume
    int comp = vol->component;
//    assert ( comp == 1 );

    int stride = vol->stride;

    printf("W=%d, H=%d, D=%d, comp=%d, stride=%d\n",
           W, H, D, comp, stride); 

    FILE *fp = fopen(filename, "wb");
    if( fp == NULL )
    {
        printf("Can't open file %s to write\n", filename);
        exit(1);
    }

    fwrite(vol->data, 1, W*H*D*comp*stride, fp);
    fclose(fp);

    return true;
}


bool write_gad_data(const char* filename, volume_chunk* vol)
{

    printf("--------------------------------------------------------\n");
    printf("DEBUG: %s @ %d: write_gad_data\n", __FILE__, __LINE__);
    printf("--------------------------------------------------------\n");


    int W = vol->dims[0];
    int H = vol->dims[1];
    int D = vol->dims[2];

    // Reading of requested part of a volume
    int comp = vol->component;
    assert ( comp == 1 );

    int stride = vol->stride;

    printf("W=%d, H=%d, D=%d, comp=%d, stride=%d\n",
           W, H, D, comp, stride); 

    float* der = calculateDerivatives<float>(vol->data, W, H, D);

    FILE *fp = fopen(filename, "wb");
    if( fp == NULL )
    {
        printf("Can't open file %s to write\n", filename);
        exit(1);
    }

    fwrite(der, 1, W*H*D*3*sizeof(float), fp);
    fclose(fp);

    return true;
}


#ifdef __APPLE__
std::string DATA_ROOT  =  "/Users/jinghua/workspace_c++/pcaster/pcaster/data/";
std::string SHADER_ROOT  =  "/Users/jinghua/workspace_c++/pcaster/pcaster/projs/pcaster/";
#elif defined(__linux__)
std::string DATA_ROOT  =  "/home/jinghuage/pcaster/data/";
std::string SHADER_ROOT  =  "/home/jinghuage/pcaster/projs/pcaster/";
#endif

int main(int argc, char* argv[])
{
    volume_chunk* vol = new volume_chunk;

    vol->dims[0] = 41;
    vol->dims[1] = 41;
    vol->dims[2] = 41;

    vol->component = 1;

    std::string fortfile = DATA_ROOT + "fort.302_15_27_39";
    load_volume_data(fortfile.c_str(), vol);

    std::string rawfile = DATA_ROOT + "molecule.raw";
    write_volume_data(rawfile.c_str(), vol);

    std::string gadfile = DATA_ROOT + "molecule.gad";
    write_gad_data(gadfile.c_str(), vol);

    return 0;
}
