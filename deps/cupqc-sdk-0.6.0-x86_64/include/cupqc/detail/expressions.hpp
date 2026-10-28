// Copyright (c) 2025, NVIDIA CORPORATION.  All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_EXPRESSIONS_HPP
#define CUPQC_DETAIL_EXPRESSIONS_HPP

#include <type_traits>

namespace cupqc {
    namespace detail {
        struct expression {};
        struct operator_expression: expression {};

        struct description_expression: expression {};
        struct execution_description_expression: description_expression {};

        template<class ValueType, ValueType Value>
        struct constant_operator_expression:
            public operator_expression,
            public std::integral_constant<ValueType, Value> {};
    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_EXPRESSIONS_HPP
