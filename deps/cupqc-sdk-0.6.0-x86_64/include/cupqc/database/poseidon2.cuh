#ifndef CUPQCDX_DATABASE_POSEIDON2_CUH
#define CUPQCDX_DATABASE_POSEIDON2_CUH

#include <cstdint>
#include "hash_types.hpp"

namespace cupqc {
    namespace database {

        // Poseidon2 sponge
        template<typename Exec, uint32_t capacity, uint32_t t, typename Field>
        class Poseidon2Context;

        template<uint32_t capacity, uint32_t t, typename Field>
        class Poseidon2Context<cupqc::Thread, capacity, t, Field>
        {
            static constexpr uint32_t rate = (t - capacity);

            uint32_t state[t];
            int      position;
            bool     finalized;

        public:
            __device__ void reset();
            __device__ void update(const uint32_t* buffer, size_t len);
            __device__ void finalize();
            __device__ void digest(uint32_t* buffer, size_t len);
        };
    } // namespace database
} // namespace cupqc
#endif
