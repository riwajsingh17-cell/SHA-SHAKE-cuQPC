#ifndef CUPQCDX_DATABASE_SHA2_CUH
#define CUPQCDX_DATABASE_SHA2_CUH

#include <cstdint>
#include "operators.hpp"

namespace cupqc {
    namespace database {
        
        template <typename Exec, typename wordtype, uint32_t digest_bits>
        class SHA2Context;

        template <typename wordtype, uint32_t digest_bits>
        class SHA2Context<cupqc::Thread, wordtype, digest_bits> {    
            static constexpr unsigned int wordsize = sizeof(wordtype);
            static constexpr unsigned int state_8 = 8*wordsize;
            static constexpr unsigned int rate_8 = 16*wordsize;
            static constexpr unsigned int rate_64 = rate_8/8;
            union H {
                wordtype uw[8];
                uint8_t u8[state_8];
            } state;
            union W {
                wordtype uw[16];
                uint8_t u8[rate_8];
                uint64_t u64[rate_64];
            } msgblock;
            uint64_t msglen;
            bool finalized;
        public:
            __device__ void reset();
            __device__ void update(const uint8_t* buffer, size_t len);
            __device__ void finalize();
            __device__ void digest(uint8_t* buffer, size_t len);
        };

        // template <uint32_t capacity>
        // class SHA2Context<cupqc::Warp, capacity> {
        //     union State {
        //         uint64_t u64;
        //         uint32_t u32[2];
        //         uint8_t  u8[8];
        //     } state;
        //     int position;
        //     bool finalized;
        // public:
        //     __device__ void reset();
        //     __device__ void update(const uint8_t* buffer, size_t len);
        //     __device__ void finalize(uint8_t pad);
        //     __device__ void digest(uint8_t* buffer, size_t len);
        // };

    } // namespace database
} // namespace cupqc

#endif // CUPQCDX_DATABASE_SHA2_CUH
