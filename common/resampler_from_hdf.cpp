#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <hdf5.h>

#include <sstream>

//resample a large hdf5 dataset into a small raw dataset
//input and output dimensions
#define INX 1024
#define INY 1024
#define INZ 1024

#define OUTX 256
#define OUTY 256
#define OUTZ 256



template<class T>
int resample( const char* in_filename,  const char* out_filename);

static int timestep_begin = 0;
static int timestep_end = 0;
int timestep;

int main (int argc, char** argv) {
    printf("Usage: %s <output_file_name_base> <input_file_name_base> timestep [timesteprange]\n", 
           argv[0]);

    assert(argc == 4 || argc==5);

    std::string out_filename_base(argv[1]);
    std::string in_filename_base(argv[2]);
    std::ostringstream out_filename;
    std::ostringstream in_filename;
  
    timestep_begin = atoi(argv[3]);
    printf("time step is %d\n", timestep_begin);

    if(argc == 5) timestep_end = atoi(argv[4]);
    printf("time step range is: %d - %d\n", timestep, timestep_end);


    for(timestep = timestep_begin; timestep <= timestep_end; timestep++)
    {
        in_filename.str().clear();
        out_filename.str().clear();
 
        in_filename << in_filename_base << timestep << ".h5";
        out_filename << out_filename_base << "-float-" << OUTX << "-t" << timestep
                 << ".raw";


        resample<float>(in_filename.str().c_str(), 
                        out_filename.str().c_str());
    }
}


int read_float(hid_t dataset, hid_t dataspace, 
               unsigned int step, unsigned int stepsize, 
               void* databuffer)
{
    printf("read float data, step %d, %d slices\n", step, stepsize);

    hsize_t     start[3];       hsize_t     start_out[3];
    hsize_t     stride[3];      hsize_t     stride_out[3];
    hsize_t     count[3];       hsize_t     count_out[3];
    hsize_t     block[3];       hsize_t     block_out[3];

    start[0]  = step * stepsize;  start_out[0] = 0;
    start[1]  = 0;  start_out[1] = 0;
    start[2]  = 0;  start_out[2] = 0;

    stride[0] = 1; stride_out[0] = 1;
    stride[1] = 1; stride_out[1] = 1;
    stride[2] = 1; stride_out[2] = 1;

    count[0]  = stepsize;  count_out[0] = count[0];
    count[1]  = INY;  count_out[1] = count[1];
    count[2]  = INX;  count_out[2] = count[2];

    block[0]  = 1;  block_out[0] = 1;
    block[1]  = 1;  block_out[1] = 1;
    block[2]  = 1;  block_out[2] = 1;

    herr_t      status;

    status      = H5Sselect_hyperslab        (dataspace,
                                              H5S_SELECT_SET,
                                              start, stride,
                                              count, block);

    size_t my_val = count[0] * count[1] * count[2] * sizeof(float);
    //printf("set to read %ld bytes data\n", my_val);

    assert (H5Sget_select_npoints(dataspace) * sizeof(float) == my_val);

    if ( status < 0 )
    {
        H5Eprint(NULL);
        printf("Cannot select data slab");
        //throw IOException("Cannot select data slab");
        return -1;
    }

    hid_t memspace    = H5Screate_simple  (3, count_out, NULL);


    status      = H5Sselect_hyperslab        (memspace,
                                              H5S_SELECT_SET,
                                              start_out , stride_out,
                                              count_out,  block_out);

    if ( status < 0 )
    {
        H5Eprint(NULL);
        printf("Cannot select mem slab");
        //throw IOException("Cannot select mem slab");
        return -1;
    }

    assert (H5Sget_select_npoints(memspace) * sizeof(float)== my_val);

    hid_t plist       = H5Pcreate     (H5P_DATASET_XFER);
    //is this hint really useful?
    status = H5Pset_buffer  (plist, my_val, NULL, NULL);
    if ( status < 0 )
    {
        H5Eprint(NULL);
        printf("Cannot set data buffer");
        //throw IOException("Cannot set buffer");
        return -1;
    }

    //read data into "data" buffer, fortran ordering!
    status      = H5Dread (dataset,
                           H5T_NATIVE_FLOAT, memspace,
                           dataspace, plist, databuffer);
    if ( status < 0 )
    {
        H5Eprint(NULL);
        printf("READ ERROR!");
        //throw IOException("Read error");
        return -1;
    }

    H5Pclose (plist);
//    H5Sclose (dataspace);
    H5Sclose (memspace);
    return 0;
}


template<class T>
int resample(  const char* in_filename,   const char* out_filename)
{
    hid_t input_file = H5Fopen(in_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (input_file < 0) {
        printf("Error opening input file %s\n", in_filename);
        return -1;
    }

    char datasetname[128];
    sprintf(datasetname, "/Timestep%d", timestep);

    hid_t dataset = H5Dopen(input_file, datasetname);
    if (dataset < 0) {
        H5Eprint(NULL);
        H5Fclose(input_file);
        printf("Error opening dataset on file %s\n",  in_filename);
        //throw IOException(string("Error opening dataset"));
        return -1;
    }
    //hid_t data_type = H5Dget_type(dataset);
    //TODO: replace asserts with errors!
    //assert (data_type == H5T_STD_U8LE);
    hid_t dataspace = H5Dget_space(dataset);
    assert(dataspace >=0);
    hsize_t maxdims[3];
    hsize_t dims[3];
    int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
    assert (dimensions == 3);

    printf("dimension(%d, %d, %d)\n", (int)dims[0], (int)dims[1], (int)dims[2]);
//    H5Sclose(dataspace); //making sure of no leaks



    FILE* raw_output_file = fopen(out_filename, "wb");
    if (raw_output_file == NULL) {
        printf("Error creating output file %s\n", out_filename);
        perror(NULL);
        return -1;
    }
    
    size_t ratioz = INZ/OUTZ;
    size_t ratioy = INY/OUTY;
    size_t ratiox = INX/OUTX;
    printf("sample kernel size(%ld, %ld, %ld)\n", ratiox, ratioy, ratioz);

    size_t kernel_size = ratiox * ratioy * ratioz;
    size_t kernel_index;
    T newvalue, min, max;
    bool start = true;

    size_t input_slice_size = INX * INY * ratioz;
    size_t output_slice_size = OUTX * OUTY;

    T *input_slice_buffer = new T[input_slice_size];
    T *output_slice_buffer = new T[output_slice_size];
    assert(input_slice_buffer);
    assert(output_slice_buffer);

    bool testing_i = false;
    bool testing_k = false;
    double sum = 0.0;
    size_t slice_in = INX * INY;

    for(unsigned int k=0; k<OUTZ; k++)
    //for(unsigned int k=0; k<1; k++)
    {
        printf("k=%d\n", k);
        //read one slice from input data
        //size_t readn = fread(input_slice_buffer, 1, input_slice_size, 
        //                      raw_input_file);
        //assert(readn == input_slice_size * sizeof(T));
        read_float(dataset, dataspace, k, ratioz, (void*)input_slice_buffer);
        

        for(unsigned int j=0; j<OUTY; j++)
        {
            for(unsigned int i=0; i<OUTX; i++)
            {
                sum = 0.0;
                for(unsigned int kk=0; kk<ratioz; kk++)
                    for(unsigned int kj=0; kj<ratioy; kj++)
                        for(unsigned int ki=0; ki<ratiox; ki++)
                        {
                            unsigned int idx = i*ratiox + ki;
                            unsigned int idy = j*ratioy + kj;
                            unsigned int idz = kk;
                            kernel_index = idz * slice_in + idy * INX + idx;
                            sum += input_slice_buffer[kernel_index];
                        }
                newvalue = sum/kernel_size;
                output_slice_buffer[j*OUTX+i] = newvalue;
                if(testing_i) printf("%f,", newvalue);

                //query minmax
                if(start)
                {
                    min = max = newvalue;
                    start = false;
                }
                else
                {
                    if(newvalue > max) max = newvalue;
                    if(newvalue < min) min = newvalue;
                }
                
            }
        }
        size_t written = fwrite(output_slice_buffer, 1, output_slice_size*sizeof(T), 
                                 raw_output_file);
        assert(written == output_slice_size*sizeof(T));

        int kc;
        if( testing_k) kc = getchar();
        if(kc == 's')
        {
            testing_k = false;
        }

        //fseek(raw_input_file, input_slice_size, SEEK_CUR);
        //fseek(raw_output_file, output_slice_size, SEEK_CUR);
    }

    printf("min value: %f, max value: %f\n", min, max);

    //free(input_slice_buffer);
    //free(output_slice_buffer);
    fclose(raw_output_file);

    H5Sclose( dataspace );
    H5Dclose( dataset );
    H5Fclose( input_file );
    return 0;
}
