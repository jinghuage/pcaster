
/******************************************************************************
 * read data from hdf5 and print out information
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <hdf5.h>
//#include <silo.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "read_h5file.h"

#define _DEBUG 0

int read_h5_attrs(hid_t input_file, admbase_chunk* md);
int* read_attr(hid_t);
void* read_dataset(hid_t, char*);
void* pre_process_data_1(void*, admbase_chunk*);
void* pre_process_data_2(void*, admbase_chunk*);

admbase_metadata* init_admbase(bool small)
{
    admbase_metadata *metadata = new admbase_metadata; 

    if(small)
    {
        metadata->global_size[0] = 200;
        metadata->global_size[1] = 200;
        metadata->global_size[2] = 200;
        metadata->numofchunks = 8;
        metadata->numofck[0] = 2;
        metadata->numofck[1] = 2;
        metadata->numofck[2] = 2;
    }
    else
    {
        metadata->global_size[0] = 800;
        metadata->global_size[1] = 800;
        metadata->global_size[2] = 800;
        metadata->numofchunks = 512;
        metadata->numofck[0] = 8;
        metadata->numofck[1] = 8;
        metadata->numofck[2] = 8;
    }
    metadata->time = 0.0;
    metadata->origin[0] = -2.0;
    metadata->origin[1] = -2.0;
    metadata->origin[2] = -2.0;
    metadata->min_ext[0] = -2.0;
    metadata->min_ext[1] = -2.0;
    metadata->min_ext[2] = -2.0;
    metadata->max_ext[0] = 396.0;
    metadata->max_ext[1] = 396.0;
    metadata->max_ext[2] = 396.0;
    metadata->delta[0] = 2.0;
    metadata->delta[1] = 2.0;
    metadata->delta[2] = 2.0;


    return metadata;
}


void query_minmax(void* buffer, admbase_chunk* md)
{
    double* databuffer = (double*)buffer;
    double min, max;
    int p2, i;
    
    p2 = md->dims[0] * md->dims[1] * md->dims[2];
    //printf("total num of points: %d\n", p2);

    min = max = databuffer[0];
    for(i=1; i<p2; i++)
    {
        //if(databuffer[i] > 3.0) databuffer[i] = 0.0;
        if(databuffer[i] > max) max = databuffer[i];
        else if(databuffer[i] < min) min = databuffer[i];
    }
    printf("---data min max (%f, %f)---\n", min, max);
    md->min = min;
    md->max = max;

}


void* pre_process_data_1(void* buffer, admbase_chunk* md)
{
    printf("----transform data from double to byte----\n");
    double* databuffer = (double*)buffer;
    double min, max;
    int p2, i;
    
    p2 = md->dims[0] * md->dims[1] * md->dims[2];
    printf("total num of points: %d\n", p2);

    
    //the actuall min max can be retrieved by the query_minmax() func
    //now we randomly define a min and max to make up some TF
//     double min1, min2, max1, max2;
//     min2 = 0.008;
//     max2 = 0.009;
//     min1 = 0.0;
//     max1 = 0.0001;
    min = 1.002;
    max = 1.1;

    //printf("allocate %d bytes\n", p2);
    unsigned char* bytedata = (unsigned char*)malloc(p2);
    assert(bytedata);

    //let's also do some histogram statistics
    unsigned int histogram[256];
    for(i=0; i<256; i++) histogram[i] = 0;
    
//     double dspan1 = max1 - min1;
//     double dspan2 = max2 - min2;
    double dspan = max - min;
    unsigned int id;

    for(i=0; i<p2; i++)
    {        
        double t = databuffer[i];
//         if(t < min2 && t > max1) id = 127;
//         else if(t > max2) id = 255;
//         else if(t < min1) id = 0;
//         else if(t <= max1)
//         {
//             t -= min1;
//             //normalizer the data to byte
//             id = (unsigned int)( t / dspan1 * 125 ) + 1;
//             assert(id>=1 && id<=126);
//         }
//         else if(t >= min2)
//         {
//             t -= min2;
//             id = (unsigned int)( t / dspan2 * 126 ) + 128;
//             assert(id>=128 && id<=254);
//         }
        if(t < min) id = 0;
        else if(t > max) id = 255;
        else
        {
            t -= min;
            id = (unsigned int) (t / dspan * 253) + 1;
            assert(id>=1 && id<=254);
        }
        histogram[id]++;
        bytedata[i] = id;
    }
    printf("histogram: ");
    md->histogram = new unsigned int[256];
    for(i=0; i<256; i++)
    {
        printf("%d, ", histogram[i]);
        md->histogram[i] = histogram[i];
    }
    printf("\n");
    free(buffer);

    return bytedata;
}


void* pre_process_data_2(void* buffer, admbase_chunk* md)
{
    printf("----transform data from double to float----\n");
    double* databuffer = (double*)buffer;
    int p2, i;
    
    p2 = md->dims[0] * md->dims[1] * md->dims[2];
    printf("total num of points: %d\n", p2);

    //printf("allocate %d bytes\n", p2);
    float* floatdata = (float*)malloc(p2 * sizeof(float));
    assert(floatdata);

    for(i=0; i<p2; i++)
    {        
        double t = databuffer[i];
        floatdata[i] = (float) t;
    }
    free(buffer);

    return floatdata;
}


void* pre_process_data_3(void* buffer, admbase_chunk* md)
{
    printf("----transform data from double to byte----\n");

    query_minmax(buffer, md);

    double* databuffer = (double*)buffer;
    int p2, i, j;
    
    p2 = md->dims[0] * md->dims[1] * md->dims[2];
    printf("total num of points: %d\n", p2);

    //printf("allocate %d bytes\n", p2);
    unsigned char* bytedata = (unsigned char*)malloc(p2);
    assert(bytedata);

//hardcode potential isosurface values after a iso-extraction session.
    int n = 12;
    double isovalue[12] = {1.002, 1.003, 1.004, 1.005, 1.006, 1.007,
                           1.01,   1.02,   1.03,   1.04,   1.05,   1.06};

    double dspan, ct, t;
    double min, max;
    unsigned int id;
    int ispan = 255/(n+1);
    

    for(i=0; i<p2; i++)
    {        
        t = databuffer[i];

        for(j=0; j<n; j++)
        {
            if(j == 0){min = md->min; max = isovalue[0];}
            else if(j == n-1){min = isovalue[n-1]; max = md->max;}
            else {min = isovalue[j-1]; max = isovalue[j];}

            if(t>=min && t<max)
            {
                dspan = max - min;
                ct = t - min;

                id = (unsigned int) (ct/dspan * (ispan-1));
                id += ispan * j;
                continue;
            }
        }
        bytedata[i] = id;
    }
    free(buffer);

    return bytedata;
}


void* pre_process_data_4(void* buffer, admbase_chunk* md, 
                         double min, double max)
{
    double* databuffer = (double*)buffer;

    int p2 = md->dims[0] * md->dims[1] * md->dims[2];
    unsigned char* bytedata = (unsigned char*)malloc(p2);
    assert(bytedata);

    for(int i=0; i<p2; i++)
    {        
        double t = (databuffer[i] - min) / (max - min);
        if( t<0.) t = 0.;
        else if(t>1.0) t = 1.0;
        bytedata[i] = (unsigned char)(t*255);
    }
    free(buffer);

    return bytedata;
}



int read_h5file(int id, admbase_chunk* md,  bool rfd)
{
    char filename[256];

    //sprintf(filename, "/home/jinghuage/data/gxx.file_%d.h5", id);
    //sprintf(filename, "/home/jinghuage/data/gxx_3d/gxx.file_%d.h5", id);
    //sprintf(filename, "/scratch/jinghua/data/gxx.file_%d.h5", id);
    sprintf(filename, "/scratch/jinghua/data/gxx_3d/gxx.file_%d.h5", id);
    //sprintf(filename, "/Users/jinghua/Downloads/gxx.file_%d.h5", id);
    //sprintf(filename, "/Users/jinghua/Downloads/gxx.file_%d.h5", id);

    if(_DEBUG) printf("read h5 file %s\n", filename);

    hid_t input_file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    
    if (input_file < 0) {
        printf("Error opening input file #%d %s\n", id, filename);
        return -1;
    }

    read_h5_attrs(input_file, md);

    char datasetname[128] = "ADMBASE::gxx timelevel 0 at iteration 0/chunk0";
    void *buf = read_dataset(input_file, datasetname);
    //void* buf = read_dataset(grp_i, "chunk0");


// //have a look at the data
//     query_minmax(buf, md);
//     free(buf)

    //resample the data: original data is double precision
    void* newdata;
    if(rfd)
    {
        //resample to float
        newdata = pre_process_data_2(buf, md);
    }
    else
    {
        //resample to byte
        //newdata = pre_process_data_3(buf, md);
        newdata = pre_process_data_4(buf, md, 1.001, 1.005);
    }
    md->data = newdata;
   
    H5Fclose(input_file);
    return 0;
}


int read_h5_attrs(hid_t input_file, admbase_chunk* md)
{


    hid_t grp_i = H5Gopen(input_file, "ADMBASE::gxx timelevel 0 at iteration 0");
    if (grp_i < 0) {
        printf(" Group does not exist\n");
        printf("do h5ls -r filename to inquire group\n");
        H5Fclose(input_file);
        return -1;
    }

    int nattr;
    hid_t attr;

//----------------------------------------------------------------------------
//read attributes of parent group
//----------------------------------------------------------------------------
//     nattr = H5Aget_num_attrs(grp_i);
//     printf("%d attributes\n", nattr);

//     for(int i=0; i<nattr; i++)
//     {
//         attr=H5Aopen_idx(grp_i, i);
//         printf("attribute %d: ", i);
//         read_attr(attr);
//         H5Aclose(attr);
//     }

    //example file: 10
    //read metadata
    ///ADMBASE::gxx timelevel 0 at iteration 0
    //global_size: 800, 800, 800
    //time: 0.0
    //origin: -2.0, -2.0, -2.0
    //min_ext: -2.0, -2.0, -2.0
    //max_ext: 1596.0, 1596.0, 1596.0
    //delta: 2.0, 2.0, 2.0

    hid_t dataset = H5Dopen(grp_i, "chunk0");

    if (dataset < 0) 
    {       
        printf("Error opening dataset\n");
        H5Fclose(input_file);        
        return -1;
    }
    hid_t data_type;
    hsize_t dims[3], maxdims[3];


    data_type = H5Dget_type(dataset);
    assert (data_type >= 0);
    hid_t dataspace = H5Dget_space(dataset);
    assert(dataspace >=0);

    //printf("%ld\n", sizeof(hsize_t));
    int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
    //unsigned long *datadims = (unsigned long*)dims;
    
    md->dimensions = dimensions;
    md->dims[0] = (int) dims[0];
    md->dims[1] = (int) dims[1];
    md->dims[2] = (int) dims[2];
    //printf("dimensions=%d: Dims[0] = %d; Dims[1] = %d; Dims[2]=%d\n", 
    //       dimensions, md->dims[0], md->dims[1], md->dims[2]);

    nattr = H5Aget_num_attrs(dataset);
    //printf("%d attributes\n", nattr);
    
    int* chunk_origin;
    for(int i=0; i<nattr; i++)
    {
        attr=H5Aopen_idx(dataset, i);
        //printf("attribute %d: ", i);
        chunk_origin = read_attr(attr);
        H5Aclose(attr);
    }

    md->chunk_origin[0] = *(chunk_origin + 0);
    md->chunk_origin[1] = *(chunk_origin + 1);
    md->chunk_origin[2] = *(chunk_origin + 2);
    free(chunk_origin);
    
    //read metadata
    ///ADMBASE::gxx timelevel 0 at iteration 0/chunk0
    //NO. of dimensions: 3
    //Dimension size: 102x104x104
    //Data type: 64-bit floating point
    //chunk_origin:198,98,0

    H5Sclose(dataspace);
    H5Dclose(dataset);
    H5Tclose(data_type);

    return 0;
}

// This reads an already opened attribute 
// The attribute is not closed
int* read_attr(hid_t attr) 
{
    int* ret = NULL;
    hid_t type = H5Aget_type(attr);
    hid_t spc = H5Aget_space(attr);
    H5T_class_t cls = H5Tget_class(type);
    size_t sz = H5Tget_size(type);            // storage size, arrays handled in the 'space'
    hssize_t pts = H5Sget_simple_extent_npoints(spc);       // number of points > 1 if an array of floats or integers
 

    int i, *ia;
    float f,*fa;
    double d, *da;
    char *s;
 
   switch (cls) 
    {
    case H5T_INTEGER:
        if(pts==1) 
        {
            H5Aread(attr,H5T_NATIVE_INT,&i);
            printf("%d\n", i);
        }
        else 
        {
            ia=(int *)malloc(pts*sizeof(int));
            H5Aread(attr,H5T_NATIVE_INT,ia);
            for (i=0; i<pts; i++) printf("%d, ", ia[i]);
            printf("\n");
            //free(ia);
            //hack to return some data
            ret = ia;
        }
        break;
    case H5T_FLOAT:
        if (sz==4) 
        {
            if (pts==1) 
            {
                H5Aread(attr,H5T_NATIVE_FLOAT,&f);
                printf("%f\n", f);
            }
            else 
            {
                fa=(float *)malloc(pts*sizeof(float));
                H5Aread(attr,H5T_NATIVE_FLOAT,fa);
                for (i=0; i<pts; i++) printf("%f, ", fa[i]);
                printf("\n");
                free(fa);
            }
        }
        else if (sz==8) 
        {
            if (pts ==1)
            {
                H5Aread(attr,H5T_NATIVE_DOUBLE,&d);
                printf("%f\n", d);
            }
            else 
            {
                da=(double*)malloc(pts*sizeof(double));
                H5Aread(attr, H5T_NATIVE_DOUBLE, da);
                for(i=0; i<pts; i++) printf("%f, ", da[i]);
                printf("\n");
                free(da);
            }
        }
        break;
    case H5T_STRING:            
        s=(char *)malloc(sz+1);
        H5Aread(attr,type,s);
//      H5Aread(attr,H5T_NATIVE_CHAR,s);
        printf("%s\n", s);
        free(s);
        break;
    default:
        fprintf(stderr, "Unhandled HDF5 metadata %d\n", cls);
        //ret = -1;
    }
 
    H5Sclose(spc);
    H5Tclose(type);
 
    return ret;
}
 


void* read_dataset(hid_t input_file, char *datasetname)
{
    if(_DEBUG) printf("read dataset %s\n", datasetname);

//    hid_t dataset = H5Dopen(input_file, "/Data-Set-2");
//    hid_t dataset = H5Dopen(input_file, "/HDF4_DIMGROUP/fakeDim2");

    hid_t dataset = H5Dopen(input_file, datasetname);

    if (dataset < 0) {
        H5Fclose(input_file);
        printf("Error opening dataset %s on input file\n", datasetname);
        return NULL;
    }


    hid_t data_type;
    hsize_t dims[3], maxdims[3];
    hid_t memspace;
    void *buffer;

    data_type = H5Dget_type(dataset);
    assert (data_type >= 0);
    hid_t dataspace = H5Dget_space(dataset);
    assert(dataspace >=0);

    int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
//    assert (dimensions == 1);
     
    memspace = H5Screate_simple(dimensions, dims, NULL); 
    assert (memspace >= 0);
    herr_t err = H5Sselect_all(memspace);
    assert(err >=0);
     

    int p2 = H5Sget_select_npoints(memspace);
//    printf("%d\n", p2);
    
    long data_size = H5Tget_size(data_type) * p2;
    //printf("Allocating space for %ld bytes\n", data_size);
    buffer = malloc(data_size);
    assert(buffer);

//     hid_t plist = H5Pcreate(H5P_DATASET_XFER);
//     err = H5Pset_buffer(plist, p2 * sizeof (double), NULL, NULL);
//     assert(err >=0 );

    err = H5Dread(dataset, data_type, memspace, H5S_ALL, H5P_DEFAULT, buffer );
    
    assert(err >= 0);

    H5Sclose(dataspace);
    H5Dclose(dataset);
    

    H5Tclose(data_type);
    H5Sclose(memspace);

    return buffer;
}

