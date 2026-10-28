// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_PQC_EXECUTION_HPP
#define CUPQC_DETAIL_PQC_EXECUTION_HPP

#include "pqc_description.hpp"
#include "database.hpp"
#include "hash_types.hpp"
#include <cstdint>
#include <variant>

namespace cupqc {
    namespace detail {

        template<class... Operators>
        class pqc_execution: public pqc_full_description<Operators...>, public commondx::detail::execution_description_expression
        {
            using base_type = pqc_full_description<Operators...>;
            using this_type = pqc_execution<Operators...>;

        protected:

            /// ---- Constraints

            // We need Block operator to be specified exactly once
            static constexpr bool has_one_block = has_at_most_one_of<operator_type::block, this_type>::value;
            static_assert(has_one_block, "Can't create pqc function with two execution operators");
        };

    } // namespace detail
} // namespace cupqc

// Include algorithm-specific execution classes
#include "pk_execution.hpp"
#include "hash_execution.hpp"
#include "ntt_execution.hpp"

namespace cupqc {
    namespace detail {

        template<class... Operators>
        struct make_description {
        private:
            static constexpr bool has_block_operator     = has_operator<operator_type::block, pqc_operator_wrapper<Operators...>>::value;
            static constexpr bool has_thread_operator    = has_operator<operator_type::thread, pqc_operator_wrapper<Operators...>>::value;
            static constexpr bool has_warp_operator      = has_operator<operator_type::warp, pqc_operator_wrapper<Operators...>>::value;
            static constexpr bool has_execution_operator = has_block_operator || has_thread_operator || has_warp_operator;

            // Determine algorithm type for block execution routing
            using temp_description = pqc_full_description<Operators...>;
            static constexpr bool is_pk = has_block_operator &&
                (temp_description::has_algorithm &&
                 (is_kem_algorithm(temp_description::this_pqc_algorithm_v) ||
                  is_dss_algorithm(temp_description::this_pqc_algorithm_v)));
            static constexpr bool is_merkle = has_block_operator &&
                (temp_description::has_algorithm &&
                 is_merkle_algorithm(temp_description::this_pqc_algorithm_v));
            static constexpr bool is_ntt = has_block_operator &&
                (temp_description::has_algorithm &&
                 temp_description::this_pqc_algorithm_v == algorithm::NTT);

            // Choose the appropriate block execution type based on algorithm
            using block_execution_type = COMMONDX_STL_NAMESPACE::conditional_t<is_pk, pqc_block_execution<Operators...>,
                                          COMMONDX_STL_NAMESPACE::conditional_t<is_merkle, pqc_merkle_block_execution<Operators...>,
                                           pqc_ntt_block_execution<Operators...>>>;

            // Choose between block, thread, or warp execution
            using execution_type = COMMONDX_STL_NAMESPACE::conditional_t<has_block_operator, block_execution_type,
                                    COMMONDX_STL_NAMESPACE::conditional_t<has_thread_operator, pqc_thread_execution<Operators...>,
                                     pqc_warp_execution<Operators...>>>;
            using description_type = pqc_full_description<Operators...>;

        public:
            using type = COMMONDX_STL_NAMESPACE::conditional_t<has_execution_operator, execution_type, description_type>;
        };

        template<class... Operators>
        using make_description_t = typename make_description<Operators...>::type;

    } // namespace detail

    template<class Operator1, class Operator2>
    __host__ __device__ __forceinline__ auto operator+(const Operator1&, const Operator2&) //
        -> typename COMMONDX_STL_NAMESPACE::enable_if<commondx::detail::are_operator_expressions<Operator1, Operator2>::value,
                                   detail::make_description_t<Operator1, Operator2>>::type {
        return detail::make_description_t<Operator1, Operator2>();
    }

    template<class... Operators1, class Operator2>
    __host__ __device__ __forceinline__ auto operator+(const detail::pqc_full_description<Operators1...>&,
                                                       const Operator2&) //
        -> typename COMMONDX_STL_NAMESPACE::enable_if<commondx::detail::is_operator_expression<Operator2>::value,
                                   detail::make_description_t<Operators1..., Operator2>>::type {
        return detail::make_description_t<Operators1..., Operator2>();
    }

    template<class Operator1, class... Operators2>
    __host__ __device__ __forceinline__ auto operator+(const Operator1&,
                                                       const detail::pqc_full_description<Operators2...>&) //
        -> typename COMMONDX_STL_NAMESPACE::enable_if<commondx::detail::is_operator_expression<Operator1>::value,
                                   detail::make_description_t<Operator1, Operators2...>>::type {
        return detail::make_description_t<Operator1, Operators2...>();
    }

    template<class... Operators1, class... Operators2>
    __host__ __device__ __forceinline__ auto operator+(const detail::pqc_full_description<Operators1...>&,
                                                       const detail::pqc_full_description<Operators2...>&) //
        -> detail::make_description_t<Operators1..., Operators2...> {
        return detail::make_description_t<Operators1..., Operators2...>();
    }
} // namespace cupqc

#endif // CUPQC_DETAIL_PQC_EXECUTION_HPP

