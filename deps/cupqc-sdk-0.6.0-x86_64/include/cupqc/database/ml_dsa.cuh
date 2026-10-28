#ifndef CUPQCDX_DATABASE_ML_DSA_CUH
#define CUPQCDX_DATABASE_ML_DSA_CUH

#include <cstdint>
#include "operators.hpp"

namespace cupqc {
    namespace database {

        // Keygen

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Keygen, size_t> global_memory_size() {
            if (SecCat == 2) {
                return 16384;
            } else if (SecCat == 3) {
                return 30720;
            } else {
                return 57344;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Keygen, unsigned> shared_memory_size() {
           if (SecCat == 2) {
                return 8320;
            } else if (SecCat == 3) {
                return 12416;
            } else {
                return 16512;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Keygen, unsigned> entropy_size() {
            return 32;
        }

        template<algorithm Alg, unsigned SecCat, unsigned NT>
        __device__ void keygen(uint8_t* public_key, uint8_t* secret_key,
                                      uint8_t* entropy,
                                      uint8_t* workspace, uint8_t* smem_workspace);

        // Sign

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Sign, size_t> global_memory_size() {
            if (SecCat == 2) {
                return 40960;
            } else if (SecCat == 3) {
                return 65536;
            } else {
                return 104448;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Sign, signed> shared_memory_size() {
            if (SecCat == 2) {
                return 21632;
            } else if (SecCat == 3) {
                return 29824;
            } else {
                return 40064;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Sign, signed> entropy_size() {
            return 32;
        }

        template<algorithm Alg, unsigned SecCat, unsigned NT, bool internal_api=false>
        __device__ void sign(uint8_t* signature,
                             const uint8_t* message, const size_t message_length,
                             const uint8_t* context, const uint8_t context_length,
                             const uint8_t* secret_key,
                             const uint8_t* entropy,
                             uint8_t* workspace, uint8_t* smem_workspace);

        // Verify

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Verify, size_t> global_memory_size() {
            if (SecCat == 2) {
                return 16384;
            } else if (SecCat == 3) {
                return 30720;
            } else {
                return 57344;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Verify, unsigned> shared_memory_size() {
            if (SecCat == 2) {
                return 13408;
            } else if (SecCat == 3) {
                return 18544;
            } else {
                return 24704;
            }
        }

        template<algorithm Alg, unsigned SecCat, function Func>
        __device__ __host__ constexpr
        std::enable_if_t<Alg == algorithm::ML_DSA && Func == function::Verify, unsigned> entropy_size() {
            return 0;
        }

        template<algorithm Alg, unsigned SecCat, unsigned NT, bool internal_api=false>
        __device__ bool verify(const uint8_t* message, const size_t message_length,
                               const uint8_t* context, const uint8_t context_length,
                               const uint8_t* signature,
                               const uint8_t* public_key,
                               uint8_t* workspace, uint8_t* smem_workspace);

    } // namespace database
} // namespace cupqc

#endif // CUPQCDX_DATABASE_ML_DSA_CUH

