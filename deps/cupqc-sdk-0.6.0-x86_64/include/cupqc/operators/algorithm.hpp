// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQCDX_OPERATORS_ALGORITHM_HPP
#define CUPQCDX_OPERATORS_ALGORITHM_HPP

#include "commondx/detail/expressions.hpp"
#include "commondx/traits/detail/is_operator_fd.hpp"
#include "commondx/traits/detail/get_operator_fd.hpp"

namespace cupqc {

    enum class algorithm
    {
        // KEM
        ML_KEM,
        // DSS
        ML_DSA,
        // Merkle Tree
        MERKLE,
        // hashing
        SHA3,
        SHAKE,
        SHA2_32,
        SHA2_64,
        POSEIDON2, 
        // NTT
        NTT,
    };

    template<algorithm Value>
    struct Algorithm : public commondx::detail::constant_operator_expression<algorithm, Value> {
    };

    constexpr bool is_kem_algorithm(algorithm alg) {
        return alg == algorithm::ML_KEM;
    }
    constexpr bool is_dss_algorithm(algorithm alg) {
        return alg == algorithm::ML_DSA;
    }
    constexpr bool is_merkle_algorithm(algorithm alg) {
        return alg == algorithm::MERKLE;
    }
    constexpr bool is_hash_algorithm(algorithm alg) {
        return alg == algorithm::SHA3 || alg == algorithm::SHAKE || alg == algorithm::SHA2_32 || alg == algorithm::SHA2_64 || alg == algorithm::POSEIDON2;
    }
    constexpr bool is_ntt_algorithm(algorithm alg) {
        return alg == algorithm::NTT;
    }

} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<cupqc::algorithm Value>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::algorithm, cupqc::Algorithm<Value>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<cupqc::algorithm Value>
    struct get_operator_type<cupqc::Algorithm<Value>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::algorithm;
    };
} // namespace commondx::detail

#endif // CUPQCDX_OPERATORS_ALGORITHM_HPP


