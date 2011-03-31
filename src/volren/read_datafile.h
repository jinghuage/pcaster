/*****************************************************************************


    Copyright 2009,2010,2011 Jinghua Ge
    ------------------------------

    This file is part of Pcaster.

    Pcaster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Pcaster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Pcaster.  If not, see <http://www.gnu.org/licenses/>.


******************************************************************************/

#ifndef READ_DATAFILE_H_
#define READ_DATAFILE_H_

#include <vector>
#include <pmisc.h>
#include <assert.h>
#include <math.h>

#ifdef _HDF5
#include <hdf5.h>
#endif


void find_minmax(void* data, size_t size, int datatype,
                 double* dmin, double* dmax);

int get_depth(int datatype);

void apply_data_range(void** data, 
                      int size,
                      int datatype,
                      double nmin,
                      double nmax,
                      int outbyte);

void* load_Raw_Data(const char* filename, int datatype,
                    int fw, int fh, int fd,
                    int w, int h, int d, 
                    int ox, int oy, int oz, 
                    int comp);

void* load_Raw_Data_4D(const char* filename, int datatype,
                    int fw, int fh, int fd, int ft,
                    int w, int h, int d, 
                    int ox, int oy, int oz, 
                    int timestep, int comp);


#ifdef _HDF5
void* load_H5_Data(const char* filename, int datatype,
                   const char* datasetname, 
                   int W, int H, int D, 
                   int ox, int oy, int oz, 
                   int comp,
                   bool fit = false);



void* load_H5_Data_4D(const char* filename, int datatype,
                      const char* datasetname, 
                      int W, int H, int D, 
                      int ox, int oy, int oz, 
                      int timestep, 
                      int comp,
                      bool fit = false);
#endif

//volume data is type T
template<class T>
unsigned char* add_Derivatives( T* volume,
                                int w, int h, int d  )
{
    float* der = calculateDerivatives<T>(volume, w, h, d);
    unsigned char* bytedata = (unsigned char *)malloc(w*h*d*4);


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
        bytedata[i*4 +3] = static_cast<unsigned char>( volume[i] * 255 );

    }

    free(volume);
    free(der);
    return bytedata;
}


#endif
