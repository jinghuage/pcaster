#ifndef READ_RAWFILE_H_
#define READ_RAWFILE_H_

#include <vector>
#include "data_structure.h"
#include "misc.h"

bool load_Header(const char*, volume_metadata*);
bool save_Header(const char*, volume_metadata*);
int  calcMinPow2(unsigned int);

template<class T>
unsigned char* normalize_rawdata(void* data, int W, int H, int D, 
                                         float min, float max)
{
    printf("--------------------------------------------------------\n");
    printf("DEBUG: %s @ %d: normalize_rawdata\n", __FILE__, __LINE__);
    printf("--------------------------------------------------------\n");


    T* databuffer = (T*)data;
    int p2 = W*H*D;
    unsigned char* bytedata = (unsigned char*)malloc(p2);
    assert(bytedata);
    //printf("%x\n", (int)bytedata);
    memset(bytedata, 0, p2);

    for(int i=0; i<p2; i++)
    {        
        T t = (databuffer[i] - min) / (max - min);
        if( t<0.) t = 0.;
        else if(t>1.0) t = 1.0;
        bytedata[i] = (unsigned char)(t*255);
    }

    free(data);
    //!!!made a mistake here: data is local variable!!!
    //data = (void*)bytedata;
    //printf("%x\n", (int)data);
    return bytedata;

}


template<class T>
bool load_Volume_Data(const char* filename, volume_chunk* vol, 
                      bool float_volume)
{

    printf("--------------------------------------------------------\n");
    printf("DEBUG: %s @ %d: load_Volume_Data\n", __FILE__, __LINE__);
    printf("--------------------------------------------------------\n");

    void *wholedata;

    int W = vol->dims[0];
    int H = vol->dims[1];
    int D = vol->dims[2];

    // Reading of requested part of a volume
    int comp = vol->component;
//    assert ( comp == 1 );

    int stride = sizeof(T);
    vol->stride = stride;

    long offset = W*H*vol->origin[2]*comp*stride;

    printf("W=%d, H=%d, D=%d, comp=%d, stride=%d, offSet=%ld\n",
           W, H, D, comp, stride, offset); 

    wholedata = malloc(W*H*D*comp*stride);
    memset(wholedata, 0, W*H*D*comp*stride);

    FILE *fp = fopen(filename, "rb");
    if( fp == NULL )
    {
        printf("Can't open model data file %s\n", filename);
        exit(1);
    }

    fseek(fp, offset, SEEK_SET);
    fread( wholedata, 1, W*H*D*comp*stride, fp);
    fclose(fp);

    if(float_volume || stride==1) vol->data = wholedata;
    else
    {
        float min = -0.01;
        float max = 0.01;
        //transform data type from T to unsigned char
        unsigned char* bytedata = 
            normalize_rawdata<T>(wholedata, W, H, D, min, max);
        vol->data = bytedata;
    }

    return true;
}


//volume data is type T
template<class T>
unsigned char* add_Derivatives( void *volume,
                                  int w, int h, int d  )
{
    float* der = calculateDerivatives<T>(volume, w, h, d);
    unsigned char* bytedata = (unsigned char *)malloc(w*h*d*4);

    T* vol = (T*)volume;
    float gx, gy, gz;
    for(int i=0; i<w*h*d; i++)
    {
        gx = der[3*i+0];
        gy = der[3*i+1];
        gz = der[3*i+2];
        double length = sqrt( (gx*gx+gy*gy+gz*gz) );

        //printf("gx=%d, gy=%d, gz=%d, length=%d\n", gx, gy, gz, length);
        if(length < 0.001) gx = gy = gz = 0;
        else
        {
            gx = ( gx*255/length + 255 )/2; 
            gy = ( gy*255/length + 255 )/2;
            gz = ( gz*255/length + 255 )/2;
        }

        bytedata[i*4   ] = static_cast<unsigned char>( gx );
        bytedata[i*4 +1] = static_cast<unsigned char>( gy );
        bytedata[i*4 +2] = static_cast<unsigned char>( gz );
        bytedata[i*4 +3] = static_cast<unsigned char>( vol[i] * 255 );

    }

    free(volume);
    free(der);
    return bytedata;
}



#endif
