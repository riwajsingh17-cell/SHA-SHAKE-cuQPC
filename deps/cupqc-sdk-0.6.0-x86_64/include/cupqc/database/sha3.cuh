#ifndef CUPQCDX_DATABASE_SHA3_CUH
#define CUPQCDX_DATABASE_SHA3_CUH

#include <cstdint>
#include "operators.hpp"

namespace cupqc {
    namespace database {
        
        // keccak[c] sponge
        template <typename Exec, uint32_t capacity>
        class KeccakContext;

        template <uint32_t capacity>
        class KeccakContext<cupqc::Thread, capacity> {
            static constexpr uint32_t rate_8 = (1600 - capacity) / 8;
            static constexpr uint32_t rate_64 = rate_8 / 8;

            union State {
                uint64_t u64[25];
                uint32_t u32[50];
                uint8_t  u8[200];
            } state;
            int position;
            bool finalized;
        public:
            __device__ void reset();
            __device__ void update(const uint8_t* buffer, size_t len);
            __device__ void finalize(uint8_t pad);
            __device__ void digest(uint8_t* buffer, size_t len);
        };

        template <uint32_t capacity>
        class KeccakContext<cupqc::Warp, capacity> {
            static constexpr uint32_t rate_8 = (1600 - capacity) / 8;
            static constexpr uint32_t rate_64 = rate_8 / 8;
            union State {
                uint64_t u64;
                uint32_t u32[2];
                uint8_t  u8[8];
            } state;
            int position;
            bool finalized;
        public:
            __device__ void reset();
            __device__ void update(const uint8_t* buffer, size_t len);
            __device__ void finalize(uint8_t pad);
            __device__ void digest(uint8_t* buffer, size_t len);
        };

    } // namespace database
} // namespace cupqc

#endif // CUPQCDX_DATABASE_SHA3_CUH
