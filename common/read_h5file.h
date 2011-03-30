#ifndef READ_H5FILE_H_
#define READ_H5FILE_H_

#include <math.h>
#include <assert.h>

#include "data_structure.h"

int read_h5file(const char* filebase, int id, volume_chunk*, bool rfd =false);


#define MPC_CM 3.0824e+24  // megaparsec to centimeter conversion
#define MSOLAR_G 1.989e+33 // solar mass to grams conversion
#define RHOCRIT 1.8788e-29 // critical density in g/cm^3

template<class T>
void convert_density_log(T& v)
{
    v = log(fabs(v));
}


//bah, redshit = -1.0 is no good
//don't know the real value
template<class T>
void convert_density_projection(T& v)
{
    float omegamatter = 0.268;
    float boxsize = 50.;
    float hubble = 0.704;
    float redshift = 0.008999;

    double DensityUnits, LengthUnits;


    LengthUnits = (((double) boxsize) * MPC_CM / 
                   ((double) hubble) / (1.0 + ((double) redshift)));

    DensityUnits = ((double) omegamatter) * RHOCRIT * 
        pow( (1.0 + ((double) redshift) ), 3.0)
        *((double) hubble) * ( (double) hubble);

    printf("%f,%f\n", LengthUnits, DensityUnits);

    // grid spacing of the projections
    double proj_delta = 1./1024;

    // density conversion
    double DensityConversion = proj_delta * DensityUnits * LengthUnits / 
        MSOLAR_G * MPC_CM * MPC_CM;

    printf("%f * %f = ", DensityConversion, v);

    v *= DensityConversion;

    printf("%f\n", v);

}


template<class T>
void query_minmax(void* buffer, volume_chunk* md)
{
    assert(md->stride == sizeof(T)); 

    T* databuffer = (T*)buffer;
    T min, max;
    int p2, i;
    
    p2 = md->dims[0] * md->dims[1] * md->dims[2];
    printf("total num of points: %d\n", p2);

    printf("first number: %f\n", databuffer[0]);

    convert_density_log(databuffer[0]);
    //convert_density_projection(databuffer[0]);
    min = max = databuffer[0];
    //printf("log(DensityConversin)=%f\n", log(2594607909.220948) );

    printf("---init data min max (%f, %f)---\n", min, max);
    for(i=1; i<p2; i++)
    {
        //printf("%f, ", databuffer[i]); 
        //if(databuffer[i] > 3.0) databuffer[i] = 0.0;

        convert_density_log(databuffer[i]);
        //convert_density_projection(databuffer[i]);
        double nd = databuffer[i];

        if(nd > max) max = nd;
        else if(nd < min) min = nd;
    }
    printf("---final data min max (%f, %f)---\n", min, max);
    md->min = min;
    md->max = max;


/*     for(int i=0; i<p2; i++) */
/*     { */
/*         databuffer[i] /= max; */
/*         printf("%.5f, ", databuffer[i]); */
/*     } */
}


//normalize by min, and max
template <class T>
unsigned char* normalize_to_byte(void* buffer, volume_chunk* md, 
                         double min, double max)
{
    T* databuffer = (T*)buffer;

    int p2 = md->dims[0] * md->dims[1] * md->dims[2];
    unsigned char* bytedata = (unsigned char*)malloc(p2);
    assert(bytedata);


    int n=0;
    for(int i=0; i<p2; i++)
    {        
        double t = 0.;
        T v = databuffer[i];
        if(v >= max) t = 1.0;
        else if(v <= min) t = 0.;
        else
        {
            t = (v - min) / (max - min);
            n++;
        }
        //if( t<0.) t = 0.;
        //else if(t>1.0) t = 1.0;
        bytedata[i] = (unsigned char)(t*255);
        //printf("%d, ", bytedata[i]);
    }
    free(buffer);

    printf("number of points in range: %d\n", n);

    return bytedata;
}


#endif
