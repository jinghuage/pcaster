#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

//functions used in this class, if started with "ocl", then it's defined in "octUtils.h"
#include "oclUtils.h"
#include "cl_object.h"

#define debug_level  0
#define MaxDeviceCount 2
#define MaxKernelCount 10
#define UseOpenGLContext 1

cl_object::cl_object(GLuint uiCount, GLuint eDeviceType, int device_id):
    m_numofkernel(0),
    m_ckKernel((cl_kernel*)NULL),
    m_cpProgram(0),
    m_cqCommandQue(0),
    m_cxGPUContext(0)

{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    if(UseOpenGLContext)
    {
#ifdef __APPLE__
        printf("Apple: Using active OpenGL context...\n");
    
        CGLContextObj kCGLContext = CGLGetCurrentContext();              
        CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
        
        cl_context_properties akProperties[] = { 
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, 
            (cl_context_properties)kCGLShareGroup, 0 
        };
            
        // Create a context from a CGL share group    
        m_cxGPUContext = clCreateContext(akProperties, 
                                         0, 
                                         0, 
                                         clLogMessagesToStdoutAPPLE, 
                                         0, 
                                         &m_ciErrNum);
#elif defined __linux__

        m_cxGPUContext = clCreateContextFromType(NULL, 
                                                 CL_DEVICE_TYPE_GPU, 
                                                 NULL, 
                                                 NULL, 
                                                 &m_ciErrNum);
#endif
    }
    else
    {
        assert(uiCount < MaxDeviceCount);

        cl_device_id akAvailableDeviceIds[MaxDeviceCount];
        cl_device_type kRequestedDeviceType = (cl_device_type)eDeviceType;
 
        GLuint uiAvailableDeviceCount = 0;
        m_ciErrNum = clGetDeviceIDs(NULL, kRequestedDeviceType, uiCount, 
                                    akAvailableDeviceIds, &uiAvailableDeviceCount);
        check_error(m_ciErrNum, CL_SUCCESS);

        m_cxGPUContext = clCreateContext(0, uiAvailableDeviceCount, 
                                         akAvailableDeviceIds,
                                         NULL, NULL, &m_ciErrNum);

    }

    check_error(m_ciErrNum, CL_SUCCESS);

    // Get and log the device info
    if(device_id >= 0 )
    {
      m_cdDevice = oclGetDev(m_cxGPUContext, device_id);
    } 
    else 
    {
      m_cdDevice = oclGetMaxFlopsDev(m_cxGPUContext);
    }
    oclPrintDevInfo(m_cdDevice);

    // create a command-queue
    m_cqCommandQue = clCreateCommandQueue(m_cxGPUContext, m_cdDevice, 0, &m_ciErrNum);
    check_error(m_ciErrNum, CL_SUCCESS);

    m_ckKernel = new cl_kernel[MaxKernelCount];

}

void cl_object::cleanup()
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    if(m_ckKernel)
    {
        for(int i=0; i<m_numofkernel; i++) clReleaseKernel(m_ckKernel[i]);
    } 
    if(m_cpProgram)     clReleaseProgram(m_cpProgram);
    if(m_cqCommandQue)  clReleaseCommandQueue(m_cqCommandQue);
    if(m_cxGPUContext)  clReleaseContext(m_cxGPUContext);
}


cl_object::~cl_object()
{
}




void cl_object::init_from_file(const char* filename)
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    // Program Setup
    size_t program_length;


    char *cSourceCL = oclLoadProgSource(filename, "", &program_length);
    assert(cSourceCL != NULL);

    // create the program
    m_cpProgram = clCreateProgramWithSource(m_cxGPUContext, 1,
					  (const char **) &cSourceCL, 
                                            &program_length, &m_ciErrNum);
    check_error(m_ciErrNum, CL_SUCCESS);

    // build the program
    m_ciErrNum = clBuildProgram(m_cpProgram, 0, NULL, NULL, NULL, NULL);
    if (m_ciErrNum != CL_SUCCESS)
    {
        oclLogBuildInfo(m_cpProgram, oclGetFirstDev(m_cxGPUContext));
        cleanup();
        exit (1);
    }
}

void cl_object::create_kernel(const char* kernel_name)
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    // create the kernel
    m_ckKernel[m_numofkernel] = clCreateKernel(m_cpProgram, kernel_name, &m_ciErrNum);
    check_error(m_ciErrNum, CL_SUCCESS);
    m_numofkernel++;

    assert(m_numofkernel < MAX_KERNEL_NUM);
}



void cl_object::run_kernel(int id, int w, int h)
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    size_t szGlobalWorkSize[2];
    // Set work size and execute the kernel
    szGlobalWorkSize[0] = w;
    szGlobalWorkSize[1] = h;

    m_ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue, m_ckKernel[id], 2, NULL, 
                                      szGlobalWorkSize, NULL, 0,0,0 );
    check_error(m_ciErrNum, CL_SUCCESS);

    clFinish(m_cqCommandQue);

}


cl_mem cl_object::create_buffer(int size)
{    
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    cl_mem bufcl = clCreateBuffer(m_cxGPUContext, CL_MEM_WRITE_ONLY, 
                                  size, NULL, &m_ciErrNum);
    check_error(m_ciErrNum, CL_SUCCESS);
    return bufcl;
}



cl_mem cl_object::create_from_glbuffer(GLuint buf)
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

     cl_mem bufcl = clCreateFromGLBuffer(m_cxGPUContext, 
                                         CL_MEM_WRITE_ONLY, 
                                         buf, &m_ciErrNum);
     //std::cout << buf << " to " << bufcl << "\n";
     check_error(m_ciErrNum, CL_SUCCESS);

     //shouldn't return a local variable. hopefully cl_mem is fine since it's a pointer
     return bufcl;
}


void cl_object::read_buffer(void* ptr, int size, cl_mem bufcl)
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    m_ciErrNum = clEnqueueReadBuffer(m_cqCommandQue, bufcl, CL_TRUE, 0, 
                                     size, ptr, 0, NULL, NULL);
    check_error(m_ciErrNum, CL_SUCCESS);
}

void cl_object::acquire_glbuffer(int num, cl_mem* bufcl)
{
    m_ciErrNum = clEnqueueAcquireGLObjects(m_cqCommandQue, num, 
                                           bufcl, 0,0,0);
    check_error(m_ciErrNum, CL_SUCCESS);
}


void cl_object::release_glbuffer(int num, cl_mem* bufcl)
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    m_ciErrNum = clEnqueueReleaseGLObjects(m_cqCommandQue, num, 
                                           bufcl, 0,0,0);
    check_error(m_ciErrNum, CL_SUCCESS);
}


void cl_object::check_error(int ret, int reference)
{
    if(debug_level == 1)
        std::cout<<__FILE__<<":"<<__func__<<std::endl;

    oclReportError(ret);

    if(ret != reference)
    {
        cleanup();
        exit (1);
    }
}
