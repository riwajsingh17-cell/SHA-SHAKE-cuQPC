// Copyright (c) 2020, NVIDIA CORPORATION.  All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_OPERATORS_PRECISION_HPP
#define CUPQC_OPERATORS_PRECISION_HPP

#include <cstdint>
#include <type_traits>

#include "../detail/expressions.hpp"

namespace cupqc {
    namespace detail {
        template<class T>
        struct is_supported_int_type:
            std::integral_constant<bool,
                                   std::is_same<uint8_t, typename std::remove_cv<T>::type>::value  ||
                                   std::is_same<uint16_t, typename std::remove_cv<T>::type>::value ||
                                   std::is_same<uint32_t, typename std::remove_cv<T>::type>::value ||
                                   std::is_same<uint64_t, typename std::remove_cv<T>::type>::value> {};
    } // namespace detail

    template<class T = uint8_t>
    struct Precision: public commondx::detail::operator_expression {
        using type = typename std::remove_cv<T>::type;
        static_assert(detail::is_supported_int_type<type>::value, "Precision must be uint8_t, uint16_t, uint32_t, or uint64_t.");
    };
} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<class T>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::precision, cupqc::Precision<T>>:
        COMMONDX_STL_NAMESPACE::true_type {
        };

    template<class T>
    struct get_operator_type<cupqc::Precision<T>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::precision;
    };

} // namespace commondx::detail

#endif // CUPQC_OPERATORS_TYPE_HPP
