// Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_BIGINT_EXECUTION_HPP
#define CUPQC_DETAIL_BIGINT_EXECUTION_HPP

#include "bigint_description.hpp"
#include "../database/bigint_storage.hpp"
#include <cstdint>

namespace cupqc {
    namespace detail {

        template<class... Operators>
        class bigint_execution: public bigint_full_description<Operators...>, public commondx::detail::execution_description_expression
        {
            using base_type = bigint_full_description<Operators...>;
            using this_type = bigint_execution<Operators...>;

        protected:

            /// ---- Constraints

            // We need an execution operator to be specified exactly once
            static constexpr bool has_thread_operator = has_operator<operator_type::thread, this_type>::value;
            static constexpr bool has_warp_operator   = has_operator<operator_type::warp, this_type>::value;
            static constexpr bool has_one_execution   = (has_thread_operator + has_warp_operator) == 1;
            static_assert(has_one_execution, "Bigint requires exactly one execution operator (Thread or Warp)");
            static_assert(!(has_thread_operator && base_type::has_tpi), "TPI cannot be used with Thread execution operator");
            static_assert(!has_warp_operator || base_type::has_tpi, "TPI must be specified when using Warp execution operator");
            static_assert(
                is_bigint_instantiated_v<base_type::n_limbs,
                                         base_type::this_bigint_tpi_v>,
                "Requested bigint configuration is not instantiated in the cuPQC bigint library");

        public:
            /// ---- Accessors
            static constexpr auto bit_width = base_type::this_bigint_bit_width_v;
            static constexpr auto sm_arch   = base_type::this_bigint_sm_v;
            static constexpr auto tpi       = base_type::this_bigint_tpi_v;
            static constexpr auto num_limbs = base_type::n_limbs;

            /// ---- Type aliases for convenience
            using error_policy = typename base_type::this_bigint_on_error;
            using modulus = database::montgomery_modulus<num_limbs, error_policy, tpi>;
        };

        template<class... Operators>
        class bigint_thread_execution: public bigint_execution<Operators...>
        {
            using this_type = bigint_thread_execution<Operators...>;
            using base_type = bigint_execution<Operators...>;

        public:
            using base_type::num_limbs;
            using typename base_type::error_policy;
            using bigint = database::bigint_impl<num_limbs, 1, error_policy>;
            using bigint_wide = database::wide_bigint_impl<num_limbs, 1, error_policy>;
        };

        template<class... Operators>
        class bigint_warp_execution: public bigint_execution<Operators...>
        {
            using this_type = bigint_warp_execution<Operators...>;
            using base_type = bigint_execution<Operators...>;

        public:
            using base_type::num_limbs, base_type::tpi;
            using typename base_type::error_policy;
            using bigint = database::bigint_impl<num_limbs, tpi, error_policy>;
            using bigint_wide = database::wide_bigint_impl<num_limbs, tpi, error_policy>;
        };

        template<class... Operators>
        struct make_bigint_description {
        private:
            static constexpr bool has_block_operator  = has_operator<operator_type::block, bigint_operator_wrapper<Operators...>>::value;
            static_assert(!has_block_operator, "Block execution is not yet supported for bigint");
            static constexpr bool has_warp_operator = has_operator<operator_type::warp, bigint_operator_wrapper<Operators...>>::value;
            static constexpr bool has_thread_operator = has_operator<operator_type::thread, bigint_operator_wrapper<Operators...>>::value;
            static constexpr bool has_execution_operator = has_thread_operator || has_warp_operator;
            using execution_type = COMMONDX_STL_NAMESPACE::conditional_t<has_warp_operator,
                bigint_warp_execution<Operators...>, bigint_thread_execution<Operators...>>;
            using description_type = bigint_full_description<Operators...>;

        public:
            using type = COMMONDX_STL_NAMESPACE::conditional_t<has_execution_operator, execution_type, description_type>;
        };

        template<class... Operators>
        using make_bigint_description_t = typename make_bigint_description<Operators...>::type;

    } // namespace detail

    // Helper to check if a type is BitWidth
    namespace detail {
        template<typename T>
        struct is_bitwidth : COMMONDX_STL_NAMESPACE::false_type {};

        template<unsigned int Value>
        struct is_bitwidth<BitWidth<Value>> : COMMONDX_STL_NAMESPACE::true_type {};
    }

    // Operator+ overloads for building bigint types (type-level DSL)
    // These are specifically for bigint and use BitWidth as the distinguishing operator

    // BitWidth + any operator (starts a bigint description)
    template<unsigned int Width, class Operator2>
    __host__ __device__ auto operator+(const BitWidth<Width>&, const Operator2&) //
        -> typename COMMONDX_STL_NAMESPACE::enable_if<commondx::detail::is_operator_expression<Operator2>::value,
                                   detail::make_bigint_description_t<BitWidth<Width>, Operator2>>::type {
        return detail::make_bigint_description_t<BitWidth<Width>, Operator2>();
    }

    // any operator + BitWidth (starts a bigint description)
    template<class Operator1, unsigned int Width>
    __host__ __device__ auto operator+(const Operator1&, const BitWidth<Width>&) //
        -> typename COMMONDX_STL_NAMESPACE::enable_if<commondx::detail::is_operator_expression<Operator1>::value,
                                   detail::make_bigint_description_t<Operator1, BitWidth<Width>>>::type {
        return detail::make_bigint_description_t<Operator1, BitWidth<Width>>();
    }

    // bigint_description + any operator
    template<class... Operators1, class Operator2>
    __host__ __device__ auto operator+(const detail::bigint_full_description<Operators1...>&,
                                                       const Operator2&) //
        -> typename COMMONDX_STL_NAMESPACE::enable_if<commondx::detail::is_operator_expression<Operator2>::value,
                                   detail::make_bigint_description_t<Operators1..., Operator2>>::type {
        return detail::make_bigint_description_t<Operators1..., Operator2>();
    }

    // any operator + bigint_description
    template<class Operator1, class... Operators2>
    __host__ __device__ auto operator+(const Operator1&,
                                                       const detail::bigint_full_description<Operators2...>&) //
        -> typename COMMONDX_STL_NAMESPACE::enable_if<commondx::detail::is_operator_expression<Operator1>::value,
                                   detail::make_bigint_description_t<Operator1, Operators2...>>::type {
        return detail::make_bigint_description_t<Operator1, Operators2...>();
    }

    // bigint_description + bigint_description
    template<class... Operators1, class... Operators2>
    __host__ __device__ auto operator+(const detail::bigint_full_description<Operators1...>&,
                                                       const detail::bigint_full_description<Operators2...>&) //
        -> detail::make_bigint_description_t<Operators1..., Operators2...> {
        return detail::make_bigint_description_t<Operators1..., Operators2...>();
    }

} // namespace cupqc

#endif // CUPQC_DETAIL_BIGINT_EXECUTION_HPP
