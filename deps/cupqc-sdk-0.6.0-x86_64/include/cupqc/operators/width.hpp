// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUHASH_OPERATORS_WIDTH_HPP
#define CUHASH_OPERATORS_WIDTH_HPP

#include "commondx/detail/expressions.hpp"
#include "commondx/traits/detail/is_operator_fd.hpp"
#include "commondx/traits/detail/get_operator_fd.hpp"

namespace cupqc {

    template<unsigned int width>
    struct Width;

    template<>
    struct Width<16>: public commondx::detail::constant_operator_expression<unsigned int, 16> {};

    template<>
    struct Width<24>: public commondx::detail::constant_operator_expression<unsigned int, 24> {};

} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<unsigned int width>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::width, cupqc::Width<width>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<unsigned int width>
    struct get_operator_type<cupqc::Width<width>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::width;
    };
} // namespace commondx::detail

#endif // CUHASH_OPERATORS_WIDTH_HPP

