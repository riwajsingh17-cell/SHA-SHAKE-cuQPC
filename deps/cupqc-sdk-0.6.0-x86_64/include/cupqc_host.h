#ifndef _CUPQC_H
#define _CUPQC_H

#include "cupqc_version.h"
#include "cupqc_status.h"

#include <cuda_runtime.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

/* \cond PRIVATE */
#ifndef CUPQC_API
#    if __GNUC__ >= 4
#        define CUPQC_API __attribute__((visibility("default")))
#    else
#        define CUPQC_API
#    endif
#endif

#ifdef __cplusplus
#    ifndef CUPQC_NOEXCEPT
#        define CUPQC_NOEXCEPT noexcept
#    endif
#else
#    define CUPQC_NOEXCEPT
#endif
/* \endcond */

/**
 * \defgroup NVIDIA CUPQC C API
 *
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/* \cond PRIVATE */
typedef struct cupqc_base_plan* cupqcHandle_t;
/* \endcond */

/**
     * \brief NVIDIA CUPQC Key Encapsulation Mechanism (KEM) and Digital Signature Scheme (DSS) method types.
     */
typedef enum cupqcMethodType
{
    // KEM
    CUPQC_KEM_KYBER_DEFAULT = 102, ///<
    CUPQC_KEM_KYBER_512     = 101, ///<
    CUPQC_KEM_KYBER_768     = 102, ///<
    CUPQC_KEM_KYBER_1024    = 103, ///<

    // DSS
    CUPQC_DSS_FALCON_DEFAULT = 201,    ///<
    CUPQC_DSS_FALCON_512     = 201,    ///<
    CUPQC_DSS_FALCON_1024    = 202,    ///<

    CUPQC_DSS_DILITHIUM_DEFAULT = 302, ///<
    CUPQC_DSS_DILITHIUM_2       = 301, ///<
    CUPQC_DSS_DILITHIUM_3       = 302, ///<
    CUPQC_DSS_DILITHIUM_5       = 303, ///<
} cupqcMethodType_t;

/**
     * \brief Create a CUPQC handle of type \p type and returns it in \p *plan.
     *
     * Legal values for \p type are:
     * - CUPQC_KEM_KYBER
     * - CUPQC_KEM_KYBER_512
     * - CUPQC_KEM_KYBER_768
     * - CUPQC_KEM_KYBER_1024
     * - CUPQC_DSS_FALCON
     * - CUPQC_DSS_FALCON_512
     * - CUPQC_DSS_FALCON_1024
     *
     * \param plan Pointer to CUPQC handle
     * \param type Type of CUPQC KEM or DSS algorithms
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the generator was not intialized \n
     * - CUPQC_STATUS_INVALID_TYPE if the value for \p rng_type is invalid \n
     * - CUPQC_STATUS_SUCCESS if the generator is generated successfully \n
     */
CUPQC_API cupqcStatus_t cupqcCreate(cupqcHandle_t* plan, cupqcMethodType_t type) CUPQC_NOEXCEPT;
/**
     * \brief Destroy an existing CUPQC plan and free all memory associated with its state.
     *
     * \param plan CUPQC handle to destroy
     *
     * \return
     * - CUPQC_STATUS_GENERATOR_NOT_INITIALIZED if the generator is never created \n
     * - CUPQC_STATUS_SUCCESS if generator is destroyed successfully \n
     */
CUPQC_API cupqcStatus_t cupqcDestroy(cupqcHandle_t plan) CUPQC_NOEXCEPT;

/**
     * \brief Return the \p version number of the NVIDIA CUPQC library.
     *
     * \param version CUPQC library version
     *
     * \return
     * - CUPQC_STATUS_SUCCESS if the version number is successfully returned \n
     */
CUPQC_API cupqcStatus_t cupqcGetVersion(int* version) CUPQC_NOEXCEPT;

/**
     * \brief Associates a CUDA stream with a CUPQC plan. All kernel launches
     * made during plan execution are now done through the associated stream,
     * enabling overlap with activity in other streams (e.g. data copying). If
     * no stream is set, all kernels use the default stream.
     *
     * \param plan Pointer to CUPQC handle
     * \param stream CUDA stream to associate with plan
     *
     * \return
     * - CUPQC_STATUS_GENERATOR_NOT_INITIALIZED if the generator is never created \n
     * - CUPQC_STATUS_SUCCESS if the stream is successfully set \n
     */
CUPQC_API cupqcStatus_t cupqcSetStream(cupqcHandle_t plan, cudaStream_t stream) CUPQC_NOEXCEPT;

/**
     * \brief Sets up the handle to support at least b batches
     *
     * \param plan CUPQC handle
     * \param b The minimum number of batches to support
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY if there is not enough free memory for the number of requested batches
     * - CUPQC_STATUS_SUCCESS if the batch number is set successfully \n
     */
CUPQC_API cupqcStatus_t cupqcSetBatch(cupqcHandle_t plan, const size_t b) CUPQC_NOEXCEPT;


//=============================================
// KEM
//=============================================
/**
     * \brief After a CUPQC KEM handle is created,  the call returns the minimum size required for public key, secret key, ciphertext, and shared secret, per batch
     * 
     * \param plan CUPQC handle
     * \param length_pk Length of the public key, in bytes
     * \param length_sk Length of the secret key, in bytes
     * \param length_ciphertext Length of the ciphertext, in bytes
     * \param length_shared_secret Length of the shared secret, in bytes
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_SUCCESS if the lengths are successfully returned \n
     */
CUPQC_API cupqcStatus_t cupqcKemGetSize(cupqcHandle_t plan,
                                        size_t*       length_pk,
                                        size_t*       length_sk,
                                        size_t*       length_ciphertext,
                                        size_t*       length_shared_secret) CUPQC_NOEXCEPT;

/**
     * \brief Keypair generation algorithm for KEM.
     * 
     * Caller is responsible for allocating sufficient GPU device memory for \p pk and \p sk. 
     * TODO add size info
     *
     * \param plan CUPQC handle
     * \param pk The public key represented as a byte string
     * \param sk The secret key represented as a byte string
     * \param batch The number of batches of public and secret keys generated
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_DATA_NULLPTR if either public or secret key is nullptr \n
     * - CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY if either public or secret key does not allocate sufficient memory\n
     * - CUPQC_STATUS_SUCCESS if the key pair is generated successfully \n
     */
CUPQC_API cupqcStatus_t cupqcKemKeypair(cupqcHandle_t plan, uint8_t* pk, uint8_t* sk, const size_t batch) CUPQC_NOEXCEPT;

/**
     * \brief Encapsulation algorithm for KEM.
     * 
     * Caller is responsible for allocating sufficient GPU device memory for \p ciphertext and \p shared_secret. 
     *
     * \param plan CUPQC handle
     * \param ciphertext The ciphertext encapsulation represented as a byte string  
     * \param shared_secret The shared secret represented as a byte string 
     * \param pk The public key represented as a byte string
     * \param batch The number of batches
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_DATA_NULLPTR if any data pointer is nullptr \n
     * - CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY if either ciphertext or shared_secret does not allocate sufficient memory\n
     * - CUPQC_STATUS_SUCCESS if the operation is done successfully \n
     */
CUPQC_API cupqcStatus_t cupqcKemEncaps(cupqcHandle_t  plan,
                                       uint8_t*       ciphertext,
                                       uint8_t*       shared_secret,
                                       const uint8_t* pk,
                                       const size_t   batch) CUPQC_NOEXCEPT;

/**
     * \brief Decapsulation algorithm for KEM (Key Encapsulation Mechanism)
     * 
     * Caller is responsible for allocating sufficient GPU device memory for \p shared_secret. 
     *
     * \param plan CUPQC handle
     * \param shared_secret The shared secret represented as a byte string 
     * \param ciphertext The ciphertext encapsulation represented as a byte string  
     * \param sk The secret key represented as a byte string 
     * \param batch The number of batches
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_DATA_NULLPTR if any data pointer is nullptr \n
     * - CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY if shared_secret does not allocate sufficient memory\n
     * - CUPQC_STATUS_SUCCESS if if the operation is done successfully \n
     */
CUPQC_API cupqcStatus_t cupqcKemDecaps(cupqcHandle_t  plan,
                                       uint8_t*       shared_secret,
                                       const uint8_t* ciphertext,
                                       const uint8_t* sk,
                                       const size_t   batch) CUPQC_NOEXCEPT;

//=============================================
// DSS
//=============================================
/**
     * \brief After a CUPQC DSS handle is created,  the call returns the minimum size required for public key, secret key per batch
     * 
     * \param plan CUPQC handle
     * \param length_pk Length of the public key, in bytes
     * \param length_sk Length of the secret key, in bytes
     * \param length_sk Max Length of the signature, in bytes
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_SUCCESS if the lengths are successfully returned \n
     */
CUPQC_API cupqcStatus_t cupqcDssGetSize(cupqcHandle_t plan, size_t* length_pk, size_t* length_sk, size_t* max_length_sig)
    CUPQC_NOEXCEPT;

/**
     * \brief Keypair generation algorithm using a DSS (Digital Signature Scheme)
     * 
     * Caller is responsible for allocating sufficient GPU device memory for \p pk and \p sk. 
     * TODO add size info
     *
     * \param plan CUPQC handle
     * \param pk The public key represented as a byte string
     * \param sk The secret key represented as a byte string
     * \param batch The number of batches of public and secret keys generated
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_DATA_NULLPTR if either public or secret key is nullptr \n
     * - CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY if either public or secret key does not allocate sufficient memory\n
     * - CUPQC_STATUS_SUCCESS if if the operation is done successfully \n
     */
CUPQC_API cupqcStatus_t cupqcDssKeypair(cupqcHandle_t plan, uint8_t* pk, uint8_t* sk, const size_t batch) CUPQC_NOEXCEPT;

/**
     * \brief Compute a signature on a provided message with a given private key using a DSS.
     * 
     * Caller is responsible for allocating sufficient GPU device memory for \p sig. 
     *
     * \param plan CUPQC handle
     * \param sig returned Signature represented as a byte string  
     * \param siglen  returned signature length
     * \param msg input message represented as a byte string 
     * \param mlen input message length
     * \param sk input private key
     * \param batch The number of batches
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_DATA_NULLPTR if any data pointer is nullptr \n
     * - CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY if either ciphertext or shared_secret does not allocate sufficient memory\n
     * - CUPQC_STATUS_SUCCESS if if the operation is done successfully \n
     */
CUPQC_API cupqcStatus_t cupqcDssSignature(cupqcHandle_t  plan,
                                          uint8_t*       sig,
                                          size_t*        siglen,
                                          const uint8_t* msg,
                                          const size_t   mlen,
                                          const uint8_t* sk,
                                          const size_t   batch) CUPQC_NOEXCEPT;

/**
     * \brief Verify a signature on a message with a give public key using a DSS.
     * 
     * Caller is responsible for allocating sufficient GPU device memory for \p sig. 
     *
     * \param plan CUPQC handle
     * \param is_valid returned verification passed or failed
     * \param msg input message represented as a byte string 
     * \param mlen input message length
     * \param sig input Signature represented as a byte string  
     * \param siglen signature length
     * \param pk input public key
     * \param batch The number of batches
     *
     * \return
     * - CUPQC_STATUS_HANDLE_NOT_INITIALIZED if the plan was not intialized \n
     * - CUPQC_STATUS_DATA_NULLPTR if any data pointer is nullptr \n
     * - CUPQC_STATUS_DATA_INSUFFICIENT_MEMORY if either ciphertext or shared_secret does not allocate sufficient memory\n
     * - CUPQC_STATUS_VERIFICATION_FAILURE if verification failed
     * - CUPQC_STATUS_SUCCESS if verification succeeded \n
     */
CUPQC_API cupqcStatus_t cupqcDssVerify(cupqcHandle_t  plan,
                                       bool* is_valid,
                                       const uint8_t* msg,
                                       const size_t   msglen,
                                       const uint8_t* sig,
                                       const size_t   siglen,
                                       const uint8_t* pk,
                                       const size_t   batch) CUPQC_NOEXCEPT;


#ifdef __cplusplus
}
#endif

/** @} */

#endif // _CUPQC_H
