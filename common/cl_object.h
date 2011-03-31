#ifndef CL_OBJECT_H
#define CL_OBJECT_H


// these headers are from ../common/, notice the CL directory as well
#include <glheaders.h>


#define MAX_KERNEL_NUM 10

class cl_object
{
public:

    cl_object(GLuint uiCount = 1, 
              GLuint eDeviceType = CL_DEVICE_TYPE_GPU,
              int device_id = -1);
    ~cl_object();

    void cleanup();
    void init_from_file( const char* );
    void create_kernel(const char*);
    void run_kernel(int, int, int);
    cl_mem create_buffer(int size);
    cl_mem create_from_glbuffer(GLuint buf);
    void read_buffer(void* ptr, int size, cl_mem bufcl);

    void acquire_glbuffer(int, cl_mem*  bufcl);
    void release_glbuffer(int, cl_mem* bufcl);

    cl_kernel get_kernel(int id) {return m_ckKernel[id]; }

    void check_error(int, int);
private:
    // OpenCL vars
    cl_int m_numofkernel;
    cl_kernel *m_ckKernel;
    cl_program m_cpProgram;
    cl_command_queue m_cqCommandQue;
    cl_context m_cxGPUContext;
    cl_device_id m_cdDevice;

   
    cl_int m_ciErrNum;
    
    
};


#endif
