// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_OPERATORS_NUM_MODULI_HPP
#define CUPQC_OPERATORS_NUM_MODULI_HPP

#include "commondx/detail/expressions.hpp"
#include "commondx/traits/detail/is_operator_fd.hpp"
#include "commondx/traits/detail/get_operator_fd.hpp"

namespace cupqc {

    template<unsigned Value>
    struct NumModuli : public commondx::detail::constant_operator_expression<unsigned, Value> {
    };

} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<unsigned Value>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::num_moduli, cupqc::NumModuli<Value>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<unsigned Value>
    struct get_operator_type<cupqc::NumModuli<Value>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::num_moduli;
    };
} // namespace commondx::detail

#endif // CUPQC_OPERATORS_NUM_MODULI_HPP



