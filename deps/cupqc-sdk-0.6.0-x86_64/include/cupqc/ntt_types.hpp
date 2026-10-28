// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_NTT_TYPES_HPP
#define CUPQC_NTT_TYPES_HPP

#include "commondx/detail/stl/type_traits.hpp"
#include "common/extended_types.hpp"

namespace cupqc {

    /* Keep together the modular constants derived from a runtime prime p */
    template<typename Precision>
    struct nttConst {
        static_assert(COMMONDX_STL_NAMESPACE::is_unsigned<Precision>::value, "nttConst requires an unsigned precision.");
        static_assert(sizeof(Precision) == 2 || sizeof(Precision) == 4 || sizeof(Precision) == 8, "nttConst supports 16-, 32- and 64-bit precisions.");

        Precision p;
        Precision n_dash;
        Precision r2;

        __host__ __device__ nttConst(const Precision in_p = Precision(1));
    };

    /* Builds the constants for a given odd prime p. Callable from the host and device code */
    template<typename Precision>
    __host__ __device__ nttConst<Precision>::nttConst(const Precision in_p): p(in_p) {
        // Compute n' = -p^{-1} mod R via Newton's method: x_{i+1} = x_i * (2 - p * x_i)
        // Each iteration doubles the number of correct bits; 6 iterations covers up to 64-bit types.
        Precision p_inv = static_cast<Precision>(1);
        for (int i = 0; i < 6; i++) {
            p_inv *= static_cast<Precision>(2) - in_p * p_inv;
        }
        this->n_dash = static_cast<Precision>(-p_inv); // -p^{-1} mod R

        // Compute R^2 mod p
        constexpr Precision p_bits = sizeof(Precision) * 8;
        constexpr cupqc_common::double_width_t<Precision> R = cupqc_common::double_width_t<Precision>(1);
        const Precision r_mod_p = Precision((R << p_bits) % in_p); // R mod p
        this->r2 = Precision((static_cast<cupqc_common::double_width_t<Precision>>(r_mod_p) * r_mod_p) % in_p); // R^2 mod p
    }

} // namespace cupqc

#endif // CUPQC_NTT_TYPES_HPP
