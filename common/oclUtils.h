
 
#ifndef OCL_UTILS_H
#define OCL_UTILS_H

// Common headers:  Cross-API utililties and OpenCL header
#include <CL/cl.h>
#include <CL/clext.h>

// reminders for output window and build log
#ifdef _WIN32
    #pragma message ("Note: including shrUtils.h")
    #pragma message ("Note: including cl.h")
#endif


extern "C" cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID);

extern "C" void oclPrintDevInfo(cl_device_id device);

extern "C" void oclPrintDevName(cl_device_id device);

extern "C" cl_device_id oclGetFirstDev(cl_context cxGPUContext);

extern "C" cl_device_id oclGetDev(cl_context cxGPUContext, unsigned int device_idx);

extern "C" cl_device_id oclGetMaxFlopsDev(cl_context cxGPUContext);

extern "C" char* oclLoadProgSource(const char* cFilename, 
                                   const char* cPreamble,
                                   size_t* szFinalLength);

extern "C" int oclLoadProgramSourceFromFile(const char* filename, 
                                            char ** result_string, 
                                            size_t *string_len);

extern "C" void oclGetProgBinary( cl_program cpProgram, cl_device_id cdDevice, char** binary, size_t* length);

extern "C" void oclLogPtx(cl_program cpProgram, cl_device_id cdDevice, const char* cPtxFileName);

extern "C" void oclLogBuildInfo(cl_program cpProgram, cl_device_id cdDevice);

// Helper function for De-allocating cl objects
extern "C" void oclDeleteMemObjs(cl_mem* cmMemObjs, int iNumObjs);

extern "C" void oclReportError(int iError);

#endif
