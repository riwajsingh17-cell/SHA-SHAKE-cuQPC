// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_TRAITS_DETAIL_DESCRIPTION_TRAITS_HPP
#define CUPQC_TRAITS_DETAIL_DESCRIPTION_TRAITS_HPP

#include "../../operators/operator_type.hpp"
#include "commondx/traits/detail/description_traits.hpp"

namespace cupqc {
    namespace detail {
        template<operator_type OperatorType, class Description>
        using get_t = commondx::detail::get_t<operator_type, OperatorType, Description>;
        template<operator_type OperatorType, class Description, class Default = void>
        using get_or_default_t = commondx::detail::get_or_default_t<operator_type, OperatorType, Description, Default>;
        template<operator_type OperatorType, class Description>
        using has_operator = commondx::detail::has_operator<operator_type, OperatorType, Description>;
        template<operator_type OperatorType, class Description>
        using has_at_most_one_of = commondx::detail::has_at_most_one_of<operator_type, OperatorType, Description>;
    } // namespace detail
} // namespace cupqc

#endif // CUPQC_TRAITS_DETAIL_DESCRIPTION_TRAITS_HPP

