// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_OPERATORS_BIGINT_ON_ERROR_HPP
#define CUPQC_OPERATORS_BIGINT_ON_ERROR_HPP

#include "commondx/detail/expressions.hpp"
#include "commondx/traits/detail/get_operator_fd.hpp"
#include "commondx/traits/detail/is_operator_fd.hpp"

#include "operator_type.hpp"

#include <cstdio>

namespace cupqc {

    enum class bigint_error {
        /// Operation completed successfully.
        success = 0,
        divide_by_zero,
        quotient_overflow,
        barrett_divide_by_zero,
        barrett_input_invariant_violated, // todo: add a comment about what this means. "reduce_barrett(): Input invariant not met! den must be greater than the high limbs of num.\n");
        inv_mod_not_invertible, ///< The value and modulus are not coprime.
        inv_mod_invalid_modulus, ///< The modular-inverse modulus is zero or one.
        inv_mod_zero_input, ///< The modular-inverse input is zero.
        inv_mod_even_modulus ///< The selected modular-inverse implementation supports only odd moduli.

    };

    struct OnErrorNone : public commondx::detail::operator_expression {
        __device__ __forceinline__ static void handle(bigint_error) { }
    };

    struct OnErrorTrap : public commondx::detail::operator_expression {
        __device__ __forceinline__ static void handle(bigint_error err) {
            (void)err;
            __trap();
        }
    };

    struct OnErrorPrintTrap : public commondx::detail::operator_expression {
        __device__ __forceinline__ static void handle(bigint_error err) {
            const unsigned int bx = blockIdx.x;
            const unsigned int by = blockIdx.y;
            const unsigned int bz = blockIdx.z;
            const unsigned int tx = threadIdx.x;
            const unsigned int ty = threadIdx.y;
            const unsigned int tz = threadIdx.z;
            switch (err) {
            case bigint_error::divide_by_zero:
                printf(
                    "cupqc bigint error: divide by zero (block %u,%u,%u thread %u,%u,%u)\n",
                    bx,
                    by,
                    bz,
                    tx,
                    ty,
                    tz);
                break;
            // todo bpinzone deduplicate this code.
            case bigint_error::quotient_overflow:
                printf(
                    "cupqc bigint error: quotient overflow (block %u,%u,%u thread %u,%u,%u)\n",
                    bx, by, bz, tx, ty, tz);
                break;
            case bigint_error::barrett_divide_by_zero:
                printf(
                    "cupqc bigint error: barrett divide by zero (block %u,%u,%u thread %u,%u,%u)\n",
                    bx,
                    by,
                    bz,
                    tx,
                    ty,
                    tz);
                break;
            case bigint_error::barrett_input_invariant_violated:
                printf(
                    "cupqc bigint error: barrett input invariant violated (block %u,%u,%u thread %u,%u,%u)\n",
                    bx,
                    by,
                    bz,
                    tx,
                    ty,
                    tz);
                break;
            case bigint_error::inv_mod_not_invertible:
                printf(
                    "cupqc bigint error: modular inverse does not exist (block %u,%u,%u thread %u,%u,%u)\n",
                    bx, by, bz, tx, ty, tz);
                break;
            case bigint_error::inv_mod_invalid_modulus:
                printf(
                    "cupqc bigint error: modular inverse modulus must be greater than one (block %u,%u,%u thread %u,%u,%u)\n",
                    bx, by, bz, tx, ty, tz);
                break;
            case bigint_error::inv_mod_zero_input:
                printf(
                    "cupqc bigint error: modular inverse input is zero (block %u,%u,%u thread %u,%u,%u)\n",
                    bx, by, bz, tx, ty, tz);
                break;
            case bigint_error::inv_mod_even_modulus:
                printf(
                    "cupqc bigint error: modular inverse does not support even moduli (block %u,%u,%u thread %u,%u,%u)\n",
                    bx, by, bz, tx, ty, tz);
                break;
            default:
                printf(
                    "cupqc bigint error: unknown error %d (block %u,%u,%u thread %u,%u,%u)\n",
                    static_cast<int>(err),
                    bx,
                    by,
                    bz,
                    tx,
                    ty,
                    tz);
                break;
            }
            __trap();
        }
    };

    using default_bigint_error_policy = OnErrorNone;

} // namespace cupqc

namespace commondx::detail {
    template<>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::on_error, cupqc::OnErrorNone>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<>
    struct get_operator_type<cupqc::OnErrorNone> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::on_error;
    };

    template<>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::on_error, cupqc::OnErrorTrap>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<>
    struct get_operator_type<cupqc::OnErrorTrap> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::on_error;
    };

    template<>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::on_error, cupqc::OnErrorPrintTrap>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<>
    struct get_operator_type<cupqc::OnErrorPrintTrap> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::on_error;
    };
} // namespace commondx::detail

#endif // CUPQC_OPERATORS_BIGINT_ON_ERROR_HPP
