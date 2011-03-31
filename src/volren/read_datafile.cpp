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


#include "read_datafile.h"

int get_depth(int datatype)
{
    switch(datatype)
    {
    case 0: return 1;
    case 1: return 2;
    case 2: return 4;
    case 3: return 8;
    default:
        return 0;
    }
}
/*
bool checkNativeEndianness(){
    unsigned short a=0x1234;
    if (*((unsigned char *)&a)==0x12)
    {
        printf("BIG ENDIAN\n");
        return BIG_ENDIAN;
    }
    else
    {
        printf("LITTLE ENDIAN\n");
        return LITTLE_ENDIAN;
    }
}
*/

void change_byte_order(unsigned short* data, int size)
{
    unsigned char* cd = (unsigned char*)data;

    for(int i=0; i<size; i++)
    {
        unsigned char b = cd[2*i+1];
        cd[2*i+1] = cd[2*i];
        cd[2*i] = b;
    }
}


void apply_data_range(void** data, 
                      int size,
                      int datatype,
                      double nmin,
                      double nmax,
                      int outbyte)
{
    unsigned short* sd=0;
    float* fd = 0;
    double* dd = 0;
    unsigned char* D = 0;


    if(outbyte) D = new unsigned char[size];

    if(datatype==1) sd = (unsigned short*)(*data);
    else if(datatype==2) fd = (float*)(*data);
    else if(datatype==3) dd = (double*)(*data);

    double t=0.;
    for(int i=0; i<size; i++)
    {       
        if(datatype == 1) t = double(sd[i])/65535;
        else if(datatype == 2) t = double(fd[i]);
        else if(datatype == 3) t = dd[i];

        if(t < nmin) t = 0.;
        else if(t > nmax) t = 1.;
        else t = (t - nmin) / (nmax - nmin);

        if(outbyte) D[i] = (unsigned char)(t * 255);
        else
        {
            if(datatype == 1) sd[i] = (unsigned short)(t * 65535);
            else if(datatype == 2) fd[i] = t;
            else if(datatype == 3) dd[i] = t;
        }
    }

    if(outbyte){
        delete[] (char*)(*data);
        *data = D;
    }
}

void find_minmax(void* data, size_t size, int datatype, 
                 double* dmin, double* dmax)
{
    unsigned short* sd=0;
    float* fd = 0;
    double* dd = 0;

    if(datatype==1) sd = (unsigned short*)(data);
    else if(datatype==2) fd = (float*)(data);
    else if(datatype==3) dd = (double*)(data);

    //double min, max;

    if(datatype==1)  *dmin = *dmax = sd[0];
    else if(datatype==2)  *dmin = *dmax = fd[0];
    else if(datatype==3)  *dmin = *dmax = dd[0];

    double t=0.;
    for(size_t i=1; i<size; i++)
    {
        if(datatype==1)  t = sd[i];
        else if(datatype==2)  t = fd[i];
        else if(datatype==3)  t = dd[i];

        if(t > *dmax) *dmax = t;
        else if(t < *dmin) *dmin = t;
    }
    std::cout << "\n";
    std::cout << "min=" << *dmin << ", max=" << *dmax << "\n";

    //*dmin = min;
    //*dmax = max;
}


void* load_Raw_Data(const char* fname, int datatype,
                    int fw, int fh, int fd,
                    int w, int h, int d, 
                    int ox, int oy, int oz, 
                    int comp)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);



    assert(w <= fw);
    assert(h <= fh);
    assert(d <= fd);


    size_t b = (size_t)get_depth(datatype);
    size_t c = (size_t)comp;

    size_t itemsize = c * b;
    size_t offset = itemsize * (fw*fh*oz + fw*oy + ox);

#ifdef _DEBUG
    fprintf(stderr, "read data (%d,%d,%d)->(%d,%d,%d), comp=%ld, depth=%ld, itemsize=%ld, offSet=%ld\n",
            ox, oy, oz, w, h, d, 
            c, b, itemsize, offset);
#endif

    size_t size = w * h * d * itemsize;
    void* wholedata = malloc(size);
    if(wholedata == NULL)
    {
        printf("allocate data failed\n");
        exit(1);
    }
    memset(wholedata, 0, size);


    FILE *fp = fopen(fname, "rb");
    if( fp == NULL )
    {
        fprintf(stderr, "Can't open data file %s\n", fname);
        exit(1);
    }

    //read data line by line
    char* linedata = (char*)wholedata;
    for(int i=0; i<d; i++)
    {        
        int o = 0;
        for(int j=0; j<h; j++)
        {
            //std::cout << i << "," << j << "\n";
            fseek(fp, offset+o, SEEK_SET);
            size_t ret = fread(linedata, itemsize, w, fp);
            assert(ret == w);

            o += fw * itemsize;
            linedata += w * itemsize;
        }
        offset += fw * fh * itemsize;
    }

    fclose(fp);

    return wholedata;
}


void* load_Raw_Data_4D(const char* filename, int datatype,
                    int fw, int fh, int fd, int ft,
                    int w, int h, int d, 
                    int ox, int oy, int oz, 
                    int timestep, int comp)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    assert(w <= fw);
    assert(h <= fh);
    assert(d <= fd);
    assert(timestep <= ft);

    size_t b = (size_t)get_depth(datatype);
    size_t c = (size_t)comp;

    size_t itemsize = c * b;
    size_t offset = itemsize * (timestep*fw*fh*fd + fw*fh*oz + fw*oy + ox);

#ifdef _DEBUG
    fprintf(stderr, "(%d,%d,%d)->(%d,%d,%d), comp=%ld, depth=%ld, offSet=%ld\n",
            ox, oy, oz, w, h, d, c, b, offset);
#endif

    size_t size = w * h * d * itemsize;
    void* wholedata = malloc(size);
    if(wholedata == NULL)
    {
        printf("allocate data failed\n");
        exit(1);
    }
    memset(wholedata, 0, size);


    FILE *fp = fopen(filename, "rb");
    if( fp == NULL )
    {
        fprintf(stderr, "Can't open data file %s\n", filename);
        exit(1);
    }

    //read data line by line
    char* linedata = (char*)wholedata;
    for(int i=0; i<d; i++)
    {        
        int o = 0;
        for(int j=0; j<h; j++)
        {
            fseek(fp, offset+o, SEEK_SET);
            size_t ret = fread(linedata, itemsize, w, fp);
            //assert(ret == w);
            o += fw * itemsize;
            linedata += w * itemsize;
        }
        offset += fw * fh * itemsize;
    }

    fclose(fp);

    return wholedata;
}


#ifdef _HDF5
void* load_H5_Data(const char* filename, int datatype,
                   const char* datasetname, 
                   int W, int H, int D, 
                   int ox, int oy, int oz, 
                   int comp,
                   bool fit)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

//only handle h5 data with one component for now
    assert(comp == 1);


    hid_t input_file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (input_file < 0) {
        fprintf(stderr, "%s:%s(): Error opening input file %s\n", 
                __FILE__, __func__, filename);
        exit(1);
    }

    hid_t dataset = H5Dopen(input_file, datasetname);
    if (dataset < 0) {
        H5Eprint(NULL);
        H5Fclose(input_file);
        fprintf(stderr, "Error opening dataset on file %s\n",  filename);
        //throw IOException(string("Error opening dataset"));
        exit(1);
    }

    hid_t dataspace = H5Dget_space(dataset);
    assert(dataspace >=0);
    hsize_t maxdims[3];
    hsize_t dims[3];
    int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
    assert (dimensions == 3);

#ifdef _DEBUG
    fprintf(stderr, "data dimension(%d, %d, %d)\n",
            (int)dims[0], (int)dims[1], (int)dims[2]);

    fprintf(stderr, "read from (%d, %d, %d), size (%d, %d, %d)\n", 
            ox, oy, oz,
            D, H, W);
    if(fit) fprintf(stderr, "read data to fit\n");
#endif

    hsize_t     start[3];       hsize_t     start_out[3];
    hsize_t     stride[3];      hsize_t     stride_out[3];
    hsize_t     count[3];       hsize_t     count_out[3];
    hsize_t     block[3];       hsize_t     block_out[3];

    start[0]  = oz;  start_out[0] = 0;
    start[1]  = oy;  start_out[1] = 0;
    start[2]  = ox;  start_out[2] = 0;

    stride[0] = fit? dims[0]/D : 1; stride_out[0] = 1;
    stride[1] = fit? dims[1]/H : 1; stride_out[1] = 1;
    stride[2] = fit? dims[2]/W : 1; stride_out[2] = 1;

    count[0]  = D;  count_out[0] = count[0];
    count[1]  = H;  count_out[1] = count[1];
    count[2]  = W;  count_out[2] = count[2];

    block[0]  = 1;  block_out[0] = 1;
    block[1]  = 1;  block_out[1] = 1;
    block[2]  = 1;  block_out[2] = 1;


    herr_t status;

    status = H5Sselect_hyperslab(dataspace,
                                 H5S_SELECT_SET,
                                 start, stride,
                                 count, block);
    int b = get_depth(datatype);
    size_t my_val = count[0] * count[1] * count[2] * b;
    //printf("set to read %ld bytes data\n", my_val);

    assert(H5Sget_select_npoints(dataspace) * b == my_val);

    //allocate the output buffer
    void *wholedata = malloc(my_val);
    if(wholedata == NULL) 
    {
        fprintf(stderr, "can't allocate output buffer\n");
        exit(1);
    }
    

    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "Cannot select data slab\n");
        //throw IOException("Cannot select data slab");
        exit(1);
    }

    hid_t memspace = H5Screate_simple(3, count_out, NULL);


    status = H5Sselect_hyperslab(memspace,
                                 H5S_SELECT_SET,
                                 start_out , stride_out,
                                 count_out,  block_out);

    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "Cannot select mem slab\n");
        //throw IOException("Cannot select mem slab");
        exit(1);
    }

    assert(H5Sget_select_npoints(memspace) * b == my_val);

    hid_t plist = H5Pcreate(H5P_DATASET_XFER);
    //is this hint really useful?
    status = H5Pset_buffer(plist, my_val, NULL, NULL);
    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "Cannot set data buffer\n");
        //throw IOException("Cannot set buffer");
        exit(1);
    }

    //read data into "data" buffer, fortran ordering!
    hid_t mem_type;
    switch(datatype)
    {
    case 0: mem_type = H5T_NATIVE_CHAR; break;
    case 1: mem_type = H5T_NATIVE_SHORT; break;
    case 2: mem_type = H5T_NATIVE_FLOAT; break; //H5T_IEEE_F32LE
    case 3: mem_type = H5T_NATIVE_DOUBLE; break;
    default: mem_type = H5T_NATIVE_CHAR;
    }

    status = H5Dread (dataset,
                      mem_type, memspace,
                      dataspace, plist, wholedata);
    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "READ ERROR!\n");
        //throw IOException("Read error");
        exit(1);
    }

    H5Pclose (plist);
    H5Sclose (memspace);
    H5Sclose( dataspace );
    H5Dclose( dataset );
    H5Fclose( input_file );


    return wholedata;

}


void* load_H5_Data_4D(const char* filename, int datatype,
                      const char* datasetname, 
                      int W, int H, int D, 
                      int ox, int oy, int oz, 
                      int timestep, 
                      int comp,
                      bool fit)
{
//    fprintf(stderr, "%s:%s()\n", __FILE__, __func__);

//only handle h5 data with one component for now
    assert(comp == 1);

    hid_t input_file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (input_file < 0) {
        fprintf(stderr, "%s:%s(): Error opening input file %s\n", 
                __FILE__, __func__, filename);
        exit(1);
    }

    hid_t dataset = H5Dopen(input_file, datasetname);
    if (dataset < 0) {
        H5Eprint(NULL);
        H5Fclose(input_file);
        fprintf(stderr, "Error opening dataset on file %s\n",  filename);
        //throw IOException(string("Error opening dataset"));
        exit(1);
    }

    hid_t dataspace = H5Dget_space(dataset);
    assert(dataspace >=0);
    hsize_t maxdims[4];
    hsize_t dims[4];
    int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
    assert (dimensions == 4);

#ifdef _DEBUG
    fprintf(stderr, "data dimension(%d, %d, %d, %d)\n",
            (int)dims[0], (int)dims[1], (int)dims[2], (int)dims[3]);

    fprintf(stderr, "origin(%d,%d,%d,%d), count(%d,%d,%d,%d)\n",
            ox, oy, oz, timestep,
            W, H, D, 1);

    if(fit) fprintf(stderr, "read data to fit\n");
#endif

    hsize_t     start[4];       hsize_t     start_out[4];
    hsize_t     stride[4];      hsize_t     stride_out[4];
    hsize_t     count[4];       hsize_t     count_out[4];
    hsize_t     block[4];       hsize_t     block_out[4];

    start[0]  = timestep;  start_out[0] = 0;
    start[1]  = oz;  start_out[1] = 0;
    start[2]  = oy;  start_out[2] = 0;
    start[3]  = ox;  start_out[3] = 0;

    stride[0] = 1; stride_out[0] = 1;
    stride[1] = fit? dims[0]/D : 1; stride_out[1] = 1;
    stride[2] = fit? dims[1]/H : 1; stride_out[2] = 1;
    stride[3] = fit? dims[2]/W : 1; stride_out[3] = 1;

    count[0]  = 1;  count_out[0] = count[0];
    count[1]  = D;  count_out[1] = count[1];
    count[2]  = H;  count_out[2] = count[2];
    count[3]  = W;  count_out[3] = count[3];

    
    block[0]  = 1;  block_out[0] = 1;
    block[1]  = 1;  block_out[1] = 1;
    block[2]  = 1;  block_out[2] = 1;
    block[3]  = 1;  block_out[3] = 1;

    herr_t status;

    status = H5Sselect_hyperslab(dataspace,
                                 H5S_SELECT_SET,
                                 start, stride,
                                 count, block);

    int b = get_depth(datatype);
    size_t my_val = count[0] * count[1] * count[2] * count[3] * b;
    //printf("set to read %ld bytes data\n", my_val);

    assert(H5Sget_select_npoints(dataspace) * b == my_val);

    //allocate the output buffer
    void *wholedata = malloc(my_val);
    if(wholedata == NULL) 
    {
        fprintf(stderr, "can't allocate output buffer\n");
        exit(1);
    }
    

    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "Cannot select data slab\n");
        //throw IOException("Cannot select data slab");
        exit(1);
    }

    hid_t memspace = H5Screate_simple(4, count_out, NULL);


    status = H5Sselect_hyperslab(memspace,
                                 H5S_SELECT_SET,
                                 start_out , stride_out,
                                 count_out,  block_out);

    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "Cannot select mem slab\n");
        //throw IOException("Cannot select mem slab");
        exit(1);
    }

    assert(H5Sget_select_npoints(memspace) * b == my_val);

    hid_t plist = H5Pcreate(H5P_DATASET_XFER);
    //is this hint really useful?
    status = H5Pset_buffer(plist, my_val, NULL, NULL);
    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "Cannot set data buffer\n");
        //throw IOException("Cannot set buffer");
        exit(1);
    }

    //read data into "data" buffer, fortran ordering!
    hid_t mem_type;
    switch(datatype)
    {
    case 0: mem_type = H5T_NATIVE_CHAR; break;
    case 1: mem_type = H5T_NATIVE_SHORT; break;
    case 2: mem_type = H5T_NATIVE_FLOAT; break; //H5T_IEEE_F32LE
    case 3: mem_type = H5T_NATIVE_DOUBLE; break;
    default: mem_type = H5T_NATIVE_CHAR;
    }

    status = H5Dread (dataset,
                      mem_type, memspace,
                      dataspace, plist, wholedata);
    if ( status < 0 )
    {
        H5Eprint(NULL);
        fprintf(stderr, "READ ERROR!\n");
        //throw IOException("Read error");
        exit(1);
    }

    H5Pclose (plist);
    H5Sclose (memspace);
    H5Sclose( dataspace );
    H5Dclose( dataset );
    H5Fclose( input_file );


    return wholedata;

}

#endif
