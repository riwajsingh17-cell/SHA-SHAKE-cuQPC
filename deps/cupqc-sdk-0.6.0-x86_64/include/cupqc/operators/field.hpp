// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUHASH_OPERATORS_PRIME_HPP
#define CUHASH_OPERATORS_PRIME_HPP

#include "commondx/detail/expressions.hpp"
#include "commondx/traits/detail/is_operator_fd.hpp"
#include "commondx/traits/detail/get_operator_fd.hpp"

namespace cupqc {

   enum class field
    {
        BabyBear,
        BabyBearDefault,
        KoalaBear,
        KoalaBearDefault,
    };

    template<field Value>
    struct Field;

    template<>
    struct Field<field::BabyBear>: public commondx::detail::constant_operator_expression<field, field::BabyBear> {
    };

    template<>
    struct Field<field::BabyBearDefault>: public commondx::detail::constant_operator_expression<field, field::BabyBearDefault> {
    };

    template<>
    struct Field<field::KoalaBear>: public commondx::detail::constant_operator_expression<field, field::KoalaBear> {
    };

    template<>
    struct Field<field::KoalaBearDefault>: public commondx::detail::constant_operator_expression<field, field::KoalaBearDefault> {
    };

} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<cupqc::field Value>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::field, cupqc::Field<Value>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<cupqc::field Value>
    struct get_operator_type<cupqc::Field<Value>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::field;
    };
} // namespace commondx::detail

#endif // CUHASH_OPERATORS_WIDTH_HPP

