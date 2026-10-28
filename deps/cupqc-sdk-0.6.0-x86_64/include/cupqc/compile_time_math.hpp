// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_COMPILE_TIME_MATH_HPP
#define CUPQC_COMPILE_TIME_MATH_HPP

#include <cstdint>

namespace cupqc {
    namespace tmp {

        // ---------------------------------------------------------------------------
        // Modular binary exponentiation: base^exp mod p
        //
        // Uses 64-bit arithmetic; requires base < p < 2^32 to avoid overflow.
        // result accumulates the answer, starting at 1.
        // ---------------------------------------------------------------------------
        template<uint64_t base, uint64_t exp, uint64_t mod, uint64_t result = 1>
        struct bin_exp {
            static constexpr uint64_t new_result = (exp & 1) ? (result * base) % mod : result;
            static constexpr uint64_t value =
                bin_exp<(base * base) % mod, (exp >> 1), mod, new_result>::value;
        };

        template<uint64_t base, uint64_t mod, uint64_t result>
        struct bin_exp<base, 0, mod, result> {
            static constexpr uint64_t value = result;
        };

        // ---------------------------------------------------------------------------
        // Modular inverse of N mod p via Fermat's little theorem: N^{p-2} mod p
        // Requires p to be prime and N != 0.
        // ---------------------------------------------------------------------------
        template<unsigned int N, unsigned int p>
        struct mod_n_inv {
            static constexpr unsigned int value = static_cast<unsigned int>(
                bin_exp<static_cast<uint64_t>(N) % p,
                        static_cast<uint64_t>(p) - 2,
                        static_cast<uint64_t>(p)>::value
            );
        };

        // ---------------------------------------------------------------------------
        // Newton iteration for computing p^{-1} mod 2^32
        //
        // Each step: x_{i+1} = x * (2 - p * x)  (all arithmetic mod 2^32)
        // Starting from x = 1, 6 iterations cover all 32-bit odd primes.
        // ---------------------------------------------------------------------------
        template<unsigned int p, unsigned int x, int iter>
        struct newton_iter {
            static constexpr unsigned int value =
                newton_iter<p, x * (2u - p * x), iter - 1>::value;
        };

        template<unsigned int p, unsigned int x>
        struct newton_iter<p, x, 0> {
            static constexpr unsigned int value = x;
        };

        // -p^{-1} mod 2^32  (Montgomery parameter for R = 2^32)
        template<unsigned int p>
        struct mod_p_inv {
            static constexpr unsigned int value =
                static_cast<unsigned int>(-(newton_iter<p, 1u, 6>::value));
        };

    } // namespace tmp
} // namespace cupqc

#endif // CUPQC_COMPILE_TIME_MATH_HPP
