#ifndef CUPQCDX_DATABASE_ML_KEM_CUH
#define CUPQCDX_DATABASE_ML_KEM_CUH

#include <cstdint>
#include "operators.hpp"

namespace cupqc {
    namespace database {

        ///// KeyGen /////
        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Keygen, size_t> global_memory_size() {
           if (SecCat == 1) {
                return 2048;
            } else if (SecCat == 3) {
                return 4608;
            } else {
                return 8192;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Keygen, unsigned> shared_memory_size() {
           if (SecCat == 1) {
                return 2880;
            } else if (SecCat == 3) {
                return 3904;
            } else {
                return 5184;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Keygen, unsigned> entropy_size() {
            return 64;
        }

        template<algorithm Alg, unsigned SecCat, unsigned NT>
        __device__ void keygen(uint8_t* public_key, uint8_t* secret_key,
                                      uint8_t* entropy,
                                      uint8_t* workspace, uint8_t* smem_workspace);

        //// Encaps ////
        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Encaps, size_t> global_memory_size() {
            if (SecCat == 1) {
                return 3584;
            } else if (SecCat == 3) {
                return 6656;
            } else {
                return 10752;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Encaps, unsigned> shared_memory_size() {
            if (SecCat == 1) {
                return 3616;
            } else if (SecCat == 3) {
                return 5152;
            } else {
                return 6688;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Encaps, unsigned> entropy_size() {
            return 32;
        }

        template<algorithm Alg, unsigned SecCat, unsigned NT>
        __device__ void encaps(       uint8_t* ciphertext,
                                            uint8_t* shared_secret,
                                      const uint8_t* public_key,
                                      const uint8_t* entropy,
                                            uint8_t* workspace,
                                            uint8_t* smem_workspace);
        //// Decaps ////

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Decaps, size_t> global_memory_size() {
            if (SecCat == 1) {
                return 4544;
            } else if (SecCat == 3) {
                return 7936;
            } else {
                return 12512;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Decaps, unsigned> shared_memory_size() {
            if (SecCat == 1) {
                return 3680;
            } else if (SecCat == 3) {
                return 5216;
            } else {
                return 6752;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_KEM && Func == function::Decaps, unsigned> entropy_size() {
            return 0;
        }

        template<algorithm Alg, unsigned SecCat, unsigned NT>
        __device__ void decaps(uint8_t* shared_secret,
                                      const uint8_t* ciphertext, const uint8_t* secret_key,
                                      uint8_t* workspace, uint8_t* smem_workspace);
    } // namespace database
} // namespace cupqc

#endif // CUPQCDX_DATABASE_ML_KEM_CUH
