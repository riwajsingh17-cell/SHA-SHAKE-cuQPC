#ifndef _CUPQC_STATUS_H
#define _CUPQC_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
     * \brief NVIDIA CUPQC API function return status
     */
typedef enum cupqcStatus
{
    CUPQC_STATUS_SUCCESS                  = 0x0,
    CUPQC_STATUS_HANDLE_NOT_INITIALIZED   = 0x1,  ///< Handle not initialized
    CUPQC_STATUS_INVALID_TYPE             = 0x2,  ///< Invalid input CUPQC type
    CUPQC_STATUS_DATA_NULLPTR             = 0x3,  ///< Data ptr is nullptr
    CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY = 0x4,  ///< Failed to allocate GPU memory
    CUPQC_STATUS_INVALID_VALUE            = 0x5,  ///< Invalid intput parameter
    CUPQC_STATUS_VERIFICATION_FAILURE     = 0x6,  ///< Invalid intput parameter
    CUPQC_STATUS_INTERNAL_ERROR           = 0x10, ///< Internal library error
    // CUPQC_STATUS_VERSION_MISMATCH         = 0xA, ///< Header file and linked library version do not match
} cupqcStatus_t;

#ifdef __cplusplus
}
#endif

#endif //_CUPQC_STATUS_H
