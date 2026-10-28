#ifndef _CUPQC_COMMON_CHECK_H
#define _CUPQC_COMMON_CHECK_H
#include <iostream>
#include "cupqc_status.h"

#define CUPQC_CHECK(err) do { cupqc_check((err), __FILE__, __LINE__); } while(false)
inline void cupqc_check(cupqcStatus_t error_code, const char *file, int line)
{
    if (error_code != CUPQC_STATUS_SUCCESS)
    {
        fprintf(stderr, "CUPQC Error %d. In file '%s' on line %d\n", error_code, file, line);
        fflush(stderr);
        exit(error_code);
    }
}

#define CUDA_CHECK(err) do { cuda_check((err), __FILE__, __LINE__); } while(false)
inline void cuda_check(cudaError_t error_code, const char *file, int line)
{
    if (error_code != cudaSuccess)
    {
        fprintf(stderr, "CUDA Error %d: %s. In file '%s' on line %d\n", error_code, cudaGetErrorString(error_code), file, line);
        fflush(stderr);
        exit(error_code);
    }
}

#endif 
