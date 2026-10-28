// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQCDX_OPERATORS_FUNCTION_HPP
#define CUPQCDX_OPERATORS_FUNCTION_HPP

namespace cupqc {

    enum class function
    {
        // shared
        Keygen,
        // KEM
        Encaps,
        Decaps,
        // DSS
        Sign,
        Verify,
        // Hash
        Hash,
        // Merkle
        Merkle,
    };

    template<function Value>
    struct Function : public commondx::detail::constant_operator_expression<function, Value> {
    };

} // namespace cupqc

// Register operators
namespace commondx::detail {
    template<cupqc::function Value>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::function, cupqc::Function<Value>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<cupqc::function Value>
    struct get_operator_type<cupqc::Function<Value>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::function;
    };
} // namespace commondx::detail

#endif // CUPQCDX_OPERATORS_FUNCTION_HPP
