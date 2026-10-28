// Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef NTT_OPERATORS_DIRECTION_HPP
#define NTT_OPERATORS_DIRECTION_HPP

#include "commondx/detail/expressions.hpp"
#include "commondx/traits/detail/is_operator_fd.hpp"
#include "commondx/traits/detail/get_operator_fd.hpp"

namespace cupqc {

    enum class nttDirection {
        FORWARD,
        INVERSE
    };

    template<nttDirection Dir>
    struct Direction;

    template<>
    struct Direction<nttDirection::FORWARD>: public commondx::detail::constant_operator_expression<nttDirection, nttDirection::FORWARD> {};

    template<>
    struct Direction<nttDirection::INVERSE>: public commondx::detail::constant_operator_expression<nttDirection, nttDirection::INVERSE> {};

} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<cupqc::nttDirection Dir>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::direction, cupqc::Direction<Dir>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<cupqc::nttDirection Dir>
    struct get_operator_type<cupqc::Direction<Dir>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::direction;
    };
} // namespace commondx::detail

#endif // NTT_OPERATORS_DIRECTION_HPP

