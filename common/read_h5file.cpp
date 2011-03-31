
/******************************************************************************
 * read data from hdf5 and print out information
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <hdf5.h>
//#include <silo.h>
#include <string.h>
#include <assert.h>

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "read_h5file.h"

#define _DEBUG 0

int read_h5_attrs(hid_t input_file, volume_chunk* md);
int* read_attr(hid_t);
void* read_dataset(hid_t, char*);
void* pre_process_data_1(void*, volume_chunk*);
void* pre_process_data_2(void*, volume_chunk*);


void query_minmax(void* buffer, volume_chunk* md)
{
//    if(md->stride == 4) 

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


void* pre_process_data_1(void* buffer, volume_chunk* md)
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


void* pre_process_data_2(void* buffer, volume_chunk* md)
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


void* pre_process_data_3(void* buffer, volume_chunk* md)
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


void* pre_process_data_4(void* buffer, volume_chunk* md, 
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





int read_h5_attrs(hid_t input_file, int id, volume_chunk* md)
{

    //char grp_name[128] = "ADMBASE::gxx timelevel 0 at iteration 0";
    char grp_name[128];
    sprintf(grp_name, "Grid0000%04d", id+1);

    hid_t grp_i = H5Gopen(input_file, grp_name);
    printf("Read group %s: ", grp_name);

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

    nattr = H5Aget_num_attrs(grp_i);
    printf("%d attributes\n", nattr);

    for(int i=0; i<nattr; i++)
    {
        attr=H5Aopen_idx(grp_i, i);
        printf("attribute %d: ", i);
        read_attr(attr);
        H5Aclose(attr);
    }

    //example file: 10
    //read metadata
    ///ADMBASE::gxx timelevel 0 at iteration 0
    //global_size: 800, 800, 800
    //time: 0.0
    //origin: -2.0, -2.0, -2.0
    //min_ext: -2.0, -2.0, -2.0
    //max_ext: 1596.0, 1596.0, 1596.0
    //delta: 2.0, 2.0, 2.0

    //hid_t dataset = H5Dopen(grp_i, "chunk0");



    std::string dataset_name[15];
    dataset_name[0] = "Cooling_Time";
    dataset_name[1] = "Dark_Matter_Density";
    dataset_name[2] = "Density";
    dataset_name[3] = "Electron_Density";
    dataset_name[4] = "HII_Density";
    dataset_name[5] = "HI_Density";
    dataset_name[6] = "HeIII_Density";
    dataset_name[7] = "HeII_Density";
    dataset_name[8] = "HeI_Density";
    dataset_name[9] = "Metal_Density";
    dataset_name[10] = "Temperature";
    dataset_name[11] = "TotalEnergy";
    dataset_name[12] = "x-velocity";
    dataset_name[13] = "y-velocity";
    dataset_name[14] = "z-velocity";

    for(int ds_id=0; ds_id<12; ds_id++)
    { 
        hid_t dataset = H5Dopen(grp_i, dataset_name[ds_id].c_str());
        printf("Opening dataset %s\n", dataset_name[ds_id].c_str());

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
        
        md->stride = (int)H5Tget_size(data_type);

        hid_t dataspace = H5Dget_space(dataset);
        assert(dataspace >= 0);

        //printf("%ld\n", sizeof(hsize_t));
        int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
        //unsigned long *datadims = (unsigned long*)dims;
    
        md->dimensions = dimensions;
        md->dims[0] = (int) dims[0];
        md->dims[1] = (int) dims[1];
        md->dims[2] = (int) dims[2];
        printf("dimensions=%d: Dims[0] = %d; Dims[1] = %d; Dims[2]=%d\n", 
              dimensions, md->dims[0], md->dims[1], md->dims[2]);

        nattr = H5Aget_num_attrs(dataset);
        printf("%d attributes\n", nattr);
    
        //int* chunk_origin;
        for(int i=0; i<nattr; i++)
        {
            attr=H5Aopen_idx(dataset, i);
            printf("attribute %d: ", i);
            read_attr(attr);
            H5Aclose(attr);
        }

//         md->origin[0] = *(chunk_origin + 0);
//         md->origin[1] = *(chunk_origin + 1);
//         md->origin[2] = *(chunk_origin + 2);
//         free(chunk_origin);
    
        //read metadata
        ///ADMBASE::gxx timelevel 0 at iteration 0/chunk0
        //NO. of dimensions: 3
        //Dimension size: 102x104x104
        //Data type: 64-bit floating point
        //chunk_origin:198,98,0

        H5Sclose(dataspace);
        H5Dclose(dataset);
        H5Tclose(data_type);
    }

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
 
    printf("%d sets of ", (int)pts);

    int i, *ia;
    float f,*fa;
    double d, *da;
    char *s;
 
    switch (cls) 
    {
    case H5T_INTEGER:
        printf("integer attibutes: ");
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
            free(ia);
            //hack to return some data
            //ret = ia;
        }
        break;
    case H5T_FLOAT:
        if (sz==4) 
        {
            printf("float attributes: ");
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
            printf("double attributes: ");
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
        printf("string attributes: ");
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
    printf("read dataset %s\n", datasetname);

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

    //data_type = H5Dget_type(dataset);
    //assert (data_type >= 0);

    data_type = H5T_NATIVE_FLOAT; //H5T_IEEE_F32BE;


    hid_t dataspace = H5Dget_space(dataset);
    assert(dataspace >= 0);

    int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
    assert (dimensions == 3);
    int Nx = dims[0];
    int Ny = dims[1];
    int Nz = dims[2];
    printf("dims(%d, %d, %d)\n", Nx, Ny, Nz);
    dims[0] = Nz;
    dims[1] = Ny;
    dims[2] = Nx;
     
    memspace = H5Screate_simple(dimensions, dims, NULL); 
    assert (memspace >= 0);

    herr_t err = H5Sselect_all(memspace);
    assert(err >=0);
     
    int p2 = H5Sget_select_npoints(memspace);
    printf("number of points: %d\n", p2);
    
    long data_size = H5Tget_size(data_type) * p2;
    printf("Allocating space for %ld bytes\n", data_size);

    buffer = malloc(data_size);
    assert(buffer);

    memset(buffer, 0, data_size);

//     hid_t plist = H5Pcreate(H5P_DATASET_XFER);
//     err = H5Pset_buffer(plist, p2 * sizeof (double), NULL, NULL);
//     assert(err >=0 );

    err = H5Dread(dataset, data_type, memspace, H5S_ALL, H5P_DEFAULT, buffer );
    
    assert(err >= 0);

    H5Sclose(dataspace);
    H5Dclose(dataset);
    
    H5Tclose(data_type);
    H5Sclose(memspace);


//    float* buf = (float*)buffer;
//    printf("%f\n", buf[0]);

    return buffer;
}



void save_to_file(unsigned char* data, const char* filename, volume_chunk* md)
{
    FILE* fp = fopen(filename, "wb");
    if(fp == NULL) 
    {
        printf("can't open file %s for write\n", filename);
        exit(1);
    }

    int p2 = md->dims[0] * md->dims[1] * md->dims[2];
    int ret = fwrite(data, 1, p2, fp);
    assert(ret == p2);

    fclose(fp);

}

unsigned char* switchToRowOrder(unsigned char* bytedata_co, volume_chunk* md)
{
    int p2 = md->dims[0] * md->dims[1] * md->dims[2];
    unsigned char* bytedata_ro = (unsigned char*)malloc(p2);

    int Nx = md->dims[0];
    int Ny = md->dims[1];
    int Nz = md->dims[2];

    for(int z=0; z<Nz; z++)
    {
        for(int y=0; y<Ny; y++)
        {
            for(int x=0; x<Nx; x++)
            {
                int id1 = z * Nx * Ny + y * Nx + x;
                int id2 = x * Ny * Nz + y * Nz + z;

                bytedata_ro[id2] = bytedata_co[id1];
            }
        }
    } 

    free(bytedata_co);
    return bytedata_ro;
}


int read_h5file(const char* filebase, int id, volume_chunk* md,  bool rfd)
{
    char filename[256];


    sprintf(filename, "%s.cpu%04d", filebase, id);


    printf("read h5 file %s\n", filename);

    hid_t input_file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    
    if (input_file < 0) {
        printf("Error opening input file #%d %s\n", id, filename);
        return -1;
    }

    //read_h5_attrs(input_file, id, md);
    md->dims[0] = 64;
    md->dims[1] = 128;
    md->dims[2] = 128;
    md->stride = 4;

    //char datasetname[128] = "ADMBASE::gxx timelevel 0 at iteration 0/chunk0";
    char datasetname[128];
    sprintf(datasetname, "/Grid0000%04d/Density", id+1);
    void *buf = read_dataset(input_file, datasetname);
    //memset(buf, 0, 4194304);

//have a look at the data
    query_minmax<float>(buf, md);
    //free(buf);
    //local-wise normalization
    unsigned char* bytedata_co = normalize_to_byte<float>(buf, md, -10, 10); //md->min, md->max);
    //unsigned char* bytedata_ro = switchToRowOrder(bytedata_co, md);


    char byte_filename[256];
    sprintf(byte_filename, "%s-byte.cpu%04d", filebase, 1024-id);
    save_to_file(bytedata_co, byte_filename, md);
    free(bytedata_co);

   
    H5Fclose(input_file);
    return 0;
}


int main()
{
    volume_chunk* md = new volume_chunk;

    char filebase[128] = "/data/RD0026/RD0026";
    for(int id=0; id<1024; id++)
    {
        read_h5file(filebase, id, md);
    } 
                
    return 0;
}
