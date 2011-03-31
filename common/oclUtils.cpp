//utility functions for cl_object

#include "oclUtils.h"
#include <string.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdarg.h>
#include <map>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// size of PGM file header 
const unsigned int PGMHeaderSize = 0x40;
#define HDASHLINE "-----------------------------------------------\n"

cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID)
{
    char chBuffer[1024];
    cl_uint num_platforms; 
    cl_platform_id* clPlatformIDs;
    cl_int ciErrNum;

    // Get OpenCL platform count
    ciErrNum = clGetPlatformIDs (0, NULL, &num_platforms);
    if (ciErrNum != CL_SUCCESS)
    {
        std::cout << " Error" << ciErrNum << " in clGetPlatformIDs Call !!!\n\n";
        return -1000;
    }
    else 
    {
        if(num_platforms == 0)
        {
            std::cout << "No OpenCL platform found!\n\n";
            return -2000;
        }
        else 
        {
            // if there's a platform or more, make space for ID's
            if ((clPlatformIDs = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id))) == NULL)
            {
                std::cout << "Failed to allocate memory for cl_platform ID's!\n\n";
                return -3000;
            }

            // get platform info for each platform and trap the NVIDIA platform if found
            ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
            for(cl_uint i = 0; i < num_platforms; ++i)
            {
                ciErrNum = clGetPlatformInfo (clPlatformIDs[i], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);
                if(ciErrNum == CL_SUCCESS)
                {
                    if(strcmp("NVIDIA", chBuffer) == 0)
                    {
                        *clSelectedPlatformID = clPlatformIDs[i];
                        break;
                    }
                }
            }

            // default to zeroeth platform if NVIDIA not found
            if(clSelectedPlatformID == NULL)
            {
                std::cout << "WARNING: NVIDIA OpenCL platform not found\n" << 
                    "defaulting to first platform!\n\n";
                *clSelectedPlatformID = clPlatformIDs[0];
            }

            free(clPlatformIDs);
        }
    }

    return CL_SUCCESS;
}


void oclPrintDevName(cl_device_id device)
{
    char device_string[1024];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
    std::cout << " Device" << device_string << std::endl;
}


void oclPrintDevInfo(cl_device_id device)
{
    char device_string[1024];
    bool nv_device_attibute_query = false;

    // CL_DEVICE_VENDOR
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(device_string), &device_string, NULL);
    std::cout << "CL_DEVICE_VENDOR: \t\t\t" << device_string << std::endl;

    // CL_DEVICE_NAME
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
    std::cout << "  CL_DEVICE_NAME: \t\t\t" << device_string << std::endl;
 
    // CL_DRIVER_VERSION
    clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(device_string), &device_string, NULL);
    std::cout << "  CL_DRIVER_VERSION: \t\t\t" << device_string << "\n"; 

    // CL_DEVICE_INFO
    cl_device_type type;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
    if( type & CL_DEVICE_TYPE_CPU )
        std::cout << "  CL_DEVICE_TYPE:\t\t\t" << "CL_DEVICE_TYPE_CPU" << "\n";
    if( type & CL_DEVICE_TYPE_GPU )
        std::cout << "  CL_DEVICE_TYPE:\t\t\t" << "CL_DEVICE_TYPE_GPU" << "\n";
    if( type & CL_DEVICE_TYPE_ACCELERATOR )
        std::cout << "  CL_DEVICE_TYPE:\t\t\t" << "CL_DEVICE_TYPE_ACCELERATOR\n";
    if( type & CL_DEVICE_TYPE_DEFAULT )
        std::cout << "  CL_DEVICE_TYPE:\t\t\t" << "CL_DEVICE_TYPE_DEFAULT\n";
    
    // CL_DEVICE_MAX_COMPUTE_UNITS
    cl_uint compute_units;
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
    std::cout << "  CL_DEVICE_MAX_COMPUTE_UNITS:\t\t" << compute_units << "\n";

    // CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
    cl_uint max_dims;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(max_dims), &max_dims, NULL);
    std::cout << "  CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:\t" << max_dims << "\n";

    // CL_DEVICE_MAX_WORK_ITEM_SIZES
    size_t workitem_size[3];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
    std::cout << "  CL_DEVICE_MAX_WORK_ITEM_SIZES:\t" << 
        workitem_size[0] <<"\\"<< workitem_size[1]<<"\\" <<workitem_size[2]<<"\n";
    
    // CL_DEVICE_MAX_WORK_GROUP_SIZE
    size_t workgroup_size;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
    std::cout << "  CL_DEVICE_MAX_WORK_GROUP_SIZE:\t" << workgroup_size<<"\n";

    // CL_DEVICE_MAX_CLOCK_FREQUENCY
    cl_uint clock_frequency;
    clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
    std::cout << "  CL_DEVICE_MAX_CLOCK_FREQUENCY:\t" << clock_frequency <<"MHz\n";

    // CL_DEVICE_ADDRESS_BITS
    cl_uint addr_bits;
    clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(addr_bits), &addr_bits, NULL);
    std::cout << "  CL_DEVICE_ADDRESS_BITS:\t\t" << addr_bits <<"\n";

    // CL_DEVICE_IMAGE_SUPPORT
    cl_bool image_support;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, NULL);
    std::cout << "  CL_DEVICE_IMAGE_SUPPORT:\t\t" << image_support <<"\n";

    // CL_DEVICE_MAX_READ_IMAGE_ARGS
    cl_uint max_read_image_args;
    clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(max_read_image_args), &max_read_image_args, NULL);
    std::cout << "  CL_DEVICE_MAX_READ_IMAGE_ARGS:\t" << max_read_image_args <<"\n";

    // CL_DEVICE_MAX_WRITE_IMAGE_ARGS
    cl_uint max_write_image_args;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(max_write_image_args), &max_write_image_args, NULL);
    std::cout << "  CL_DEVICE_MAX_WRITE_IMAGE_ARGS:\t" << max_write_image_args <<"\n";
    
    // CL_DEVICE_IMAGE2D_MAX_WIDTH
    size_t image2d_max_width;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(image2d_max_width), &image2d_max_width, NULL);
    size_t image2d_max_height;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(image2d_max_height), &image2d_max_height, NULL);
    size_t image3d_max_width;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(image3d_max_width), &image3d_max_width, NULL);
    size_t image3d_max_height;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(image3d_max_height), &image3d_max_height, NULL);
    size_t image3d_max_depth;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(image3d_max_depth), &image3d_max_depth, NULL);
    std::cout << "  CL_DEVICE_IMAGE_MAX_WIDTH:\t\t2d:" << image2d_max_width << ":" << image2d_max_height <<
        " 3d:" << image3d_max_height << ":" << image3d_max_width << ":" << image3d_max_depth << "\n";

    // CL_DEVICE_MAX_MEM_ALLOC_SIZE
    cl_ulong max_mem_alloc_size;
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
    std::cout << "  CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t\t" <<
        (unsigned int)(max_mem_alloc_size / (1024 * 1024)) << "MByte\n";

    // CL_DEVICE_GLOBAL_MEM_SIZE
    cl_ulong mem_size;
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
    std::cout << "  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t" << 
        (unsigned int)(mem_size / (1024 * 1024)) << "MByte\n";

    // CL_DEVICE_ERROR_CORRECTION_SUPPORT
    cl_bool error_correction_support;
    clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, 
                    sizeof(error_correction_support), 
                    &error_correction_support, NULL);
    std::cout << "  CL_DEVICE_ERROR_CORRECTION_SUPPORT:\t" << error_correction_support << "\n";

    // CL_DEVICE_LOCAL_MEM_TYPE
    cl_device_local_mem_type local_mem_type;
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, 
                    sizeof(local_mem_type), 
                    &local_mem_type, NULL);
    std::cout << "  CL_DEVICE_LOCAL_MEM_TYPE:\t\t" << local_mem_type << "\n";

    // CL_DEVICE_LOCAL_MEM_SIZE
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
    std::cout << "  CL_DEVICE_LOCAL_MEM_SIZE:\t\t" << 
        (unsigned int)(mem_size / 1024) << "KByte\n";

    // CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, 
                    sizeof(mem_size), &mem_size, NULL);
    std::cout << "  CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:\t" <<
         (unsigned int)(mem_size / 1024) << "KByte\n";

    // CL_DEVICE_QUEUE_PROPERTIES
    cl_command_queue_properties queue_properties;
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(queue_properties), &queue_properties, NULL);
    if( queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE )
        std::cout << "  CL_DEVICE_QUEUE_PROPERTIES:\t\t" << 
            "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE\n";    
    if( queue_properties & CL_QUEUE_PROFILING_ENABLE )
        std::cout << "  CL_DEVICE_QUEUE_PROPERTIES:\t\t" << 
            "CL_QUEUE_PROFILING_ENABLE\n";
    
    // CL_DEVICE_EXTENSIONS: get device extensions, and if any then parse & log the string onto separate lines
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(device_string), &device_string, NULL);
    if (device_string != 0) 
    {
        std::cout << "  CL_DEVICE_EXTENSIONS:\n";
        std::string stdDevString;
        stdDevString = std::string(device_string);
        size_t szOldPos = 0;
        size_t szSpacePos = stdDevString.find(' ', szOldPos); // extensions string is space delimited
        while (szSpacePos != stdDevString.npos && (szSpacePos - szOldPos) > 0)
        {
            if( strcmp("cl_nv_device_attribute_query", stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str()) == 0 )
                nv_device_attibute_query = true;

            std::cout << "\t\t\t\t\t" << 
                stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str() << "\n";
            
            szOldPos = szSpacePos + 1;
            szSpacePos = stdDevString.find(' ', szOldPos); 
        }
    }
    else 
    {
        std::cout << "  CL_DEVICE_EXTENSIONS: None\n";
    }

    if( nv_device_attibute_query ) {
        cl_uint compute_capability_major, compute_capability_minor;
        clGetDeviceInfo(device, CL_NV_DEVICE_COMPUTE_CAPABILITY_MAJOR, sizeof(cl_uint), &compute_capability_major, NULL);
        clGetDeviceInfo(device, CL_NV_DEVICE_COMPUTE_CAPABILITY_MINOR, sizeof(cl_uint), &compute_capability_minor, NULL);

        std::cout << "  CL_NV_DEVICE_COMPUTE_CAPABILITY:\t" << 
            compute_capability_major << "," << 
            compute_capability_minor << "\n";        

        cl_uint regs_per_block;
        clGetDeviceInfo(device, CL_NV_DEVICE_REGISTERS_PER_BLOCK, sizeof(cl_uint), &regs_per_block, NULL);
        std::cout << "  CL_NV_DEVICE_REGISTERS_PER_BLOCK:\t" << 
            regs_per_block << "\n";        

        cl_uint warp_size;
        clGetDeviceInfo(device, CL_NV_DEVICE_WARP_SIZE, sizeof(cl_uint), &warp_size, NULL);
        std::cout << "  CL_NV_DEVICE_WARP_SIZE:\t\t" << warp_size << "\n";        

        cl_bool gpu_overlap;
        clGetDeviceInfo(device, CL_NV_DEVICE_GPU_OVERLAP, sizeof(cl_bool), &gpu_overlap, NULL);
        std::cout << "  CL_NV_DEVICE_GPU_OVERLAP:\t\t" << 
            gpu_overlap << "\n";

        cl_bool exec_timeout;
        clGetDeviceInfo(device, CL_NV_DEVICE_KERNEL_EXEC_TIMEOUT, sizeof(cl_bool), &exec_timeout, NULL);
        std::cout << "  CL_NV_DEVICE_KERNEL_EXEC_TIMEOUT:\t" << 
            exec_timeout << "\n";

        cl_bool integrated_memory;
        clGetDeviceInfo(device, CL_NV_DEVICE_INTEGRATED_MEMORY, sizeof(cl_bool), &integrated_memory, NULL);
        std::cout << "  CL_NV_DEVICE_INTEGRATED_MEMORY:\t" << 
            integrated_memory << "\n";
    }

    // CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR
    cl_uint vec_width_char;
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(vec_width_char), &vec_width_char, NULL);
    cl_uint vec_width_short;
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(vec_width_short), &vec_width_short, NULL);
    cl_uint vec_width_int;
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(vec_width_int), &vec_width_int, NULL);
    cl_uint vec_width_long;
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(vec_width_long), &vec_width_long, NULL);
    cl_uint vec_width_float;
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(vec_width_float), &vec_width_float, NULL);
    cl_uint vec_width_double;
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(vec_width_double), &vec_width_double, NULL);
    std::cout << "  CL_DEVICE_PREFERRED_VECTOR_WIDTH:\tchar:" << vec_width_char <<
        " short:" <<  vec_width_short << 
        " int:" <<  vec_width_int <<
        " long:" <<  vec_width_long << 
        " float:" <<  vec_width_float << 
        " double:" << vec_width_double;

    std::cout << "\n";
}


cl_device_id oclGetFirstDev(cl_context cxGPUContext)
{
    size_t szParmDataBytes;
    cl_device_id* cdDevices;

    // get the list of GPU devices associated with context
    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &szParmDataBytes);
    cdDevices = (cl_device_id*) malloc(szParmDataBytes);

    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, szParmDataBytes, cdDevices, NULL);

    cl_device_id first = cdDevices[0];
    free(cdDevices);

    return first;
}


cl_device_id oclGetMaxFlopsDev(cl_context cxGPUContext)
{
    size_t szParmDataBytes;
    cl_device_id* cdDevices;

    // get the list of GPU devices associated with context
    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &szParmDataBytes);
    cdDevices = (cl_device_id*) malloc(szParmDataBytes);
    size_t device_count = szParmDataBytes / sizeof(cl_device_id);

    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, szParmDataBytes, cdDevices, NULL);

    cl_device_id max_flops_device = cdDevices[0];
	int max_flops = 0;
	
	size_t current_device = 0;
	
    // CL_DEVICE_MAX_COMPUTE_UNITS
    cl_uint compute_units;
    clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);

    // CL_DEVICE_MAX_CLOCK_FREQUENCY
    cl_uint clock_frequency;
    clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
    
    max_flops = compute_units * clock_frequency;
    ++current_device;

    while( current_device < device_count )
    {
        // CL_DEVICE_MAX_COMPUTE_UNITS
        cl_uint compute_units;
        clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);

        // CL_DEVICE_MAX_CLOCK_FREQUENCY
        cl_uint clock_frequency;
        clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
		
        int flops = compute_units * clock_frequency;
        if( flops > max_flops )
        {
            max_flops        = flops;
            max_flops_device = cdDevices[current_device];
        }
        ++current_device;
    }

    free(cdDevices);

    return max_flops_device;
}


char* oclLoadProgSource(const char* cFilename, 
                        const char* cPreamble,
                        size_t* szFinalLength)
{
    // locals 
    FILE* pFileStream = NULL;
    size_t szSourceLength;

    // open the OpenCL source code file
    #ifdef _WIN32   // Windows version
        if(fopen_s(&pFileStream, cFilename, "rb") != 0) 
        {       
            return NULL;
        }
    #else           // Linux version
        pFileStream = fopen(cFilename, "rb");
        if(pFileStream == 0) 
        {       
            return NULL;
        }
    #endif

    size_t szPreambleLength = strlen(cPreamble);

    // get the length of the source code
    fseek(pFileStream, 0, SEEK_END); 
    szSourceLength = ftell(pFileStream);
    fseek(pFileStream, 0, SEEK_SET); 

    // allocate a buffer for the source code string and read it in
    char* cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1); 
    memcpy(cSourceString, cPreamble, szPreambleLength);
    if (fread((cSourceString) + szPreambleLength, szSourceLength, 1, pFileStream) != 1)
    {
        fclose(pFileStream);
        free(cSourceString);
        return 0;
    }

    // close the file and return the total length of the combined (preamble + source) string
    fclose(pFileStream);
    if(szFinalLength != 0)
    {
        *szFinalLength = szSourceLength + szPreambleLength;
    }
    cSourceString[szSourceLength + szPreambleLength] = '\0';

    return cSourceString;
}

//another version
int oclLoadProgramSourceFromFile(
    const char *file_name, 
    char **result_string,
    size_t *string_len)
{
    int fd;
    unsigned file_len; 
    struct stat file_status;
    int ret;
	
    *string_len = 0;
    fd = open(file_name, O_RDONLY);
    if (fd == -1) 
    {
        printf("Error opening file %s\n", file_name);
        return -1;
    }
    ret = fstat(fd, &file_status);
    if (ret) 
    {
        printf("Error reading status for file %s\n", file_name);
        return -1;
    }
    file_len = file_status.st_size;
	
    *result_string = new char[file_len+1];
    ret = read(fd, *result_string, file_len);
    if (!ret) 
    {
        printf("Error reading from file %s\n", file_name);
        return -1;
    }
    (*result_string)[file_len] = '\0';
    close(fd);
	
    *string_len = file_len;
    return 0;
}

cl_device_id oclGetDev(cl_context cxGPUContext, unsigned int nr)
{
    size_t szParmDataBytes;
    cl_device_id* cdDevices;

    // get the list of GPU devices associated with context
    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &szParmDataBytes);
    
    if( szParmDataBytes / sizeof(cl_device_id) < nr ) {
      return (cl_device_id)-1;
    }
    
    cdDevices = (cl_device_id*) malloc(szParmDataBytes);

    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, szParmDataBytes, cdDevices, NULL);
    
    cl_device_id device = cdDevices[nr];
    free(cdDevices);

    return device;
}


void oclGetProgBinary( cl_program cpProgram, cl_device_id cdDevice, char** binary, size_t* length)
{
    // Grab the number of devices associated witht the program
    cl_uint num_devices;
    clGetProgramInfo(cpProgram, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, NULL);

    // Grab the device ids
    cl_device_id* devices = (cl_device_id*) malloc(num_devices * sizeof(cl_device_id));
    clGetProgramInfo(cpProgram, CL_PROGRAM_DEVICES, num_devices * sizeof(cl_device_id), devices, 0);

    // Grab the sizes of the binaries
    size_t* binary_sizes = (size_t*)malloc(num_devices * sizeof(size_t));    
    clGetProgramInfo(cpProgram, CL_PROGRAM_BINARY_SIZES, num_devices * sizeof(size_t), binary_sizes, NULL);

    // Now get the binaries
    char** ptx_code = (char**) malloc(num_devices * sizeof(char*));
    for( unsigned int i=0; i<num_devices; ++i) {
        ptx_code[i]= (char*)malloc(binary_sizes[i]);
    }
    clGetProgramInfo(cpProgram, CL_PROGRAM_BINARIES, 0, ptx_code, NULL);

    // Find the index of the device of interest
    unsigned int idx = 0;
    while( idx<num_devices && devices[idx] != cdDevice ) ++idx;
    
    // If it is associated prepare the result
    if( idx < num_devices )
    {
        *binary = ptx_code[idx];
        *length = binary_sizes[idx];
    }

    // Cleanup
    free( devices );
    free( binary_sizes );
    for( unsigned int i=0; i<num_devices; ++i) {
        if( i != idx ) free(ptx_code[i]);
    }
    free( ptx_code );
}


void oclLogPtx(cl_program cpProgram, cl_device_id cdDevice, const char* cPtxFileName)
{
    // Grab the number of devices associated with the program
    cl_uint num_devices;
    clGetProgramInfo(cpProgram, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, NULL);

    // Grab the device ids
    cl_device_id* devices = (cl_device_id*) malloc(num_devices * sizeof(cl_device_id));
    clGetProgramInfo(cpProgram, CL_PROGRAM_DEVICES, num_devices * sizeof(cl_device_id), devices, 0);

    // Grab the sizes of the binaries
    size_t* binary_sizes = (size_t*)malloc(num_devices * sizeof(size_t));    
    clGetProgramInfo(cpProgram, CL_PROGRAM_BINARY_SIZES, num_devices * sizeof(size_t), binary_sizes, NULL);

    // Now get the binaries
    char** ptx_code = (char**)malloc(num_devices * sizeof(char*));
    for( unsigned int i=0; i<num_devices; ++i)
    {
        ptx_code[i] = (char*)malloc(binary_sizes[i]);
    }
    clGetProgramInfo(cpProgram, CL_PROGRAM_BINARIES, 0, ptx_code, NULL);

    // Find the index of the device of interest
    unsigned int idx = 0;
    while((idx < num_devices) && (devices[idx] != cdDevice)) 
    {
        ++idx;
    }
    
    // If the index is associated, log the result
    if(idx < num_devices)
    {
         
        // if a separate filename is supplied, dump ptx there 
        if (NULL != cPtxFileName)
        {
            std::cout << "\nWriting ptx to separate file: " <<  cPtxFileName << "\n\n";
            FILE* pFileStream = NULL;
            #ifdef _WIN32
                fopen_s(&pFileStream, cPtxFileName, "wb");
            #else
                pFileStream = fopen(cPtxFileName, "wb");
            #endif

            fwrite(ptx_code[idx], binary_sizes[idx], 1, pFileStream);
            fclose(pFileStream);        
        }
        else // log to logfile and console if no ptx file specified
        {
            std::cout << HDASHLINE << ptx_code[idx] << HDASHLINE;
        }
    }

    // Cleanup
    free(devices);
    free(binary_sizes);
    for(unsigned int i = 0; i < num_devices; ++i)
    {
        free(ptx_code[i]);
    }
    free( ptx_code );
}


void oclLogBuildInfo(cl_program cpProgram, cl_device_id cdDevice)
{
    // write out the build log and ptx, then exit
    char cBuildLog[4096];
    clGetProgramBuildInfo(cpProgram, cdDevice, CL_PROGRAM_BUILD_LOG, 
                          sizeof(cBuildLog), cBuildLog, NULL );
    if(strlen(cBuildLog) < 1)
        std::cout << "Compute Engine: Empty Build Log! Unkown Error!\n";
    else
        std::cout << HDASHLINE << cBuildLog << HDASHLINE;
}

// -----------------------------------------------------------------------------
// Helper function for De-allocating cl objects
// -----------------------------------------------------------------------------
void oclDeleteMemObjs(cl_mem* cmMemObjs, int iNumObjs)
{
    int i;
    for (i = 0; i < iNumObjs; i++)
    {
        if (cmMemObjs[i])clReleaseMemObject(cmMemObjs[i]);
    }
}  


static bool
GetErrorString(int iError, char** acString, size_t *kStringLength)
{
    const char* acErrorString = 0;
    
	switch(iError)
	{
    case(CL_SUCCESS):
        break;
    case(CL_DEVICE_NOT_FOUND):
        acErrorString = "Device not found!";
        break;
    case(CL_DEVICE_NOT_AVAILABLE):
        acErrorString = "Device not available!";
        break;
    case(CL_COMPILER_NOT_AVAILABLE):
        acErrorString = "Device compiler not available!";
        break;
    case(CL_MEM_OBJECT_ALLOCATION_FAILURE):
        acErrorString = "Memory object allocation failure!";
        break;
    case(CL_OUT_OF_RESOURCES):
        acErrorString = "Out of resources!";
        break;
    case(CL_OUT_OF_HOST_MEMORY):
        acErrorString = "Out of host memory!";
        break;
    case(CL_PROFILING_INFO_NOT_AVAILABLE):
        acErrorString = "Profiling information not available!";
        break;
    case(CL_MEM_COPY_OVERLAP):
        acErrorString = "Overlap detected in memory copy operation!";
        break;
    case(CL_IMAGE_FORMAT_MISMATCH):
        acErrorString = "Image format mismatch detected!";
        break;
    case(CL_IMAGE_FORMAT_NOT_SUPPORTED):
        acErrorString = "Image format not supported!";
        break;
    case(CL_INVALID_VALUE):
        acErrorString = "Invalid value!";
        break;
    case(CL_INVALID_DEVICE_TYPE):
        acErrorString = "Invalid device type!";
        break;
    case(CL_INVALID_DEVICE):
        acErrorString = "Invalid device!";
        break;
    case(CL_INVALID_CONTEXT):
        acErrorString = "Invalid context!";
        break;
    case(CL_INVALID_QUEUE_PROPERTIES):
        acErrorString = "Invalid queue properties!";
        break;
    case(CL_INVALID_COMMAND_QUEUE):
        acErrorString = "Invalid command queue!";
        break;
    case(CL_INVALID_HOST_PTR):
        acErrorString = "Invalid host pointer address!";
        break;
    case(CL_INVALID_MEM_OBJECT):
        acErrorString = "Invalid memory object!";
        break;
    case(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR):
        acErrorString = "Invalid image format descriptor!";
        break;
    case(CL_INVALID_IMAGE_SIZE):
        acErrorString = "Invalid image size!";
        break;
    case(CL_INVALID_SAMPLER):
        acErrorString = "Invalid sampler!";
        break;
    case(CL_INVALID_BINARY):
        acErrorString = "Invalid binary!";
        break;
    case(CL_INVALID_BUILD_OPTIONS):
        acErrorString = "Invalid build options!";
        break;
    case(CL_INVALID_PROGRAM):
        acErrorString = "Invalid program object!";
        break;
    case(CL_INVALID_PROGRAM_EXECUTABLE):
        acErrorString = "Invalid program executable!";
        break;
    case(CL_INVALID_KERNEL_NAME):
        acErrorString = "Invalid kernel name!";
        break;
    case(CL_INVALID_KERNEL):
        acErrorString = "Invalid kernel object!";
        break;
    case(CL_INVALID_ARG_INDEX):
        acErrorString = "Invalid index for kernel argument!";
        break;
    case(CL_INVALID_ARG_VALUE):
        acErrorString = "Invalid value for kernel argument!";
        break;
    case(CL_INVALID_ARG_SIZE):
        acErrorString = "Invalid size for kernel argument!";
        break;
    case(CL_INVALID_KERNEL_ARGS):
        acErrorString = "Invalid kernel arguments!";
        break;
    case(CL_INVALID_WORK_DIMENSION):
        acErrorString = "Invalid work dimension!";
        break;
    case(CL_INVALID_WORK_GROUP_SIZE):
        acErrorString = "Invalid work group size!";
        break;
    case(CL_INVALID_GLOBAL_OFFSET):
        acErrorString = "Invalid global offset!";
        break;
    case(CL_INVALID_EVENT_WAIT_LIST):
        acErrorString = "Invalid event wait list!";
        break;
    case(CL_INVALID_EVENT):
        acErrorString = "Invalid event!";
        break;
    case(CL_INVALID_OPERATION):
        acErrorString = "Invalid operation!";
        break;
    case(CL_INVALID_GL_OBJECT):
        acErrorString = "Invalid OpenGL object!";
        break;
    case(CL_INVALID_BUFFER_SIZE):
        acErrorString = "Invalid buffer size!";
        break;
    default:
        acErrorString = "Unknown error!";
        break;
    };
    
    if(!acErrorString)
        return false;
    
    size_t kLength = strlen(acErrorString);
    char * acResult = new char[kLength + 1];
    strcpy(acResult, acErrorString);

    (*acString) = acResult;
    (*kStringLength) = kLength;
    return true;
}

void oclReportError(int iError)
{
    char *acErrorString = 0;
    size_t kLength = 0;
    
    GetErrorString(iError, &acErrorString, &kLength);
    
    if(kLength)
    {
        printf("OpenCL Error[%d]: %s\n", iError, acErrorString);
        delete [] acErrorString;
    }
}

