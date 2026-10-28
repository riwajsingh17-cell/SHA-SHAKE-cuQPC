// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQCDX_OPERATORS_SECURITY_CATEGORY_HPP
#define CUPQCDX_OPERATORS_SECURITY_CATEGORY_HPP

#include "commondx/detail/expressions.hpp"
#include "commondx/traits/detail/is_operator_fd.hpp"
#include "commondx/traits/detail/get_operator_fd.hpp"

namespace cupqc {

    template<unsigned int Category>
    struct SecurityCategory;

    template<>
    struct SecurityCategory<1>: public commondx::detail::constant_operator_expression<unsigned int, 1> {};

    template<>
    struct SecurityCategory<2>: public commondx::detail::constant_operator_expression<unsigned int, 2> {};

    template<>
    struct SecurityCategory<3>: public commondx::detail::constant_operator_expression<unsigned int, 3> {};

    template<>
    struct SecurityCategory<4>: public commondx::detail::constant_operator_expression<unsigned int, 4> {};

    template<>
    struct SecurityCategory<5>: public commondx::detail::constant_operator_expression<unsigned int, 5> {};

} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<unsigned int Category>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::security_category, cupqc::SecurityCategory<Category>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<unsigned int Category>
    struct get_operator_type<cupqc::SecurityCategory<Category>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::security_category;
    };
} // namespace commondx::detail

#endif // CUPQCDX_OPERATORS_SECURITY_CATEGORY_HPP

