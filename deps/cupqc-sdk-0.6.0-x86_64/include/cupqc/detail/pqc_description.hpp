// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_PQC_DESCRIPTION_HPP
#define CUPQC_DETAIL_PQC_DESCRIPTION_HPP

#include "commondx/traits/detail/get.hpp"

#include "../operators.hpp"
#include "../traits/detail/description_traits.hpp"
#include "../hash_types.hpp"

#include <cstdint>

namespace cupqc {
    namespace detail {

        template<class... Operators>
        class pqc_operator_wrapper: public commondx::detail::description_expression { };

        template<class... Operators>
        class pqc_description: public commondx::detail::description_expression {

            using description_type = pqc_operator_wrapper<Operators...>;

            // Friend declarations
			template<class enable, class... Ops>
			friend class pqc_full_description_helper;
            template<class... Ops>
            friend constexpr bool has_algorithm(algorithm alg);
            template<class... Ops>
            friend constexpr bool has_algorithm_no_security_category(algorithm alg);
            template<class... Ops>
            friend constexpr bool has_no_algorithm();
            template<class... Ops>
            friend struct make_description;

        protected:

            // Algorithm
            // * Default value: NONE
            // * Dummy value: ML_KEM
            static constexpr bool has_algorithm        = has_operator<operator_type::algorithm, description_type>::value;
            using dummy_default_pqc_algorithm          = Algorithm<algorithm::ML_KEM>;
            using this_pqc_algorithm                   = get_or_default_t<operator_type::algorithm, description_type, dummy_default_pqc_algorithm>;
            static constexpr auto this_pqc_algorithm_v = this_pqc_algorithm::value;

            // Security Level
            // * Default value: NONE
            // * Dummy value: 128
            static constexpr bool has_security_category        = has_operator<operator_type::security_category, description_type>::value;
            using dummy_default_pqc_security_category          = SecurityCategory<3>;
            using this_pqc_security_category                   = get_or_default_t<operator_type::security_category, description_type, dummy_default_pqc_security_category>;
            static constexpr auto this_pqc_security_category_v = this_pqc_security_category::value;

            // Width
            // * Default value: 16
            static constexpr bool has_width = has_operator<operator_type::width, description_type>::value;
            using dummy_default_pqc_width = Width<16>;
            using this_pqc_width = get_or_default_t<operator_type::width, description_type, dummy_default_pqc_width>;
            static constexpr auto this_pqc_width_v = this_pqc_width::value;

            // Capacity
            // * Default value: 8
            static constexpr bool has_capacity = has_operator<operator_type::capacity, description_type>::value;
            using dummy_default_pqc_capacity = Capacity<8>;
            using this_pqc_capacity = get_or_default_t<operator_type::capacity, description_type, dummy_default_pqc_capacity>;
            static constexpr auto this_pqc_capacity_v = this_pqc_capacity::value;

            // Field
            // * Default value: BabyBearDefault
            static constexpr bool has_field = has_operator<operator_type::field, description_type>::value;
            using dummy_default_pqc_field = Field<field::BabyBearDefault>;
            using this_pqc_field = get_or_default_t<operator_type::field, description_type, dummy_default_pqc_field>;
            static constexpr auto this_pqc_field_v = this_pqc_field::value;

            // Precision
            // * Default value: uint8_t
            static constexpr bool has_precision = has_operator<operator_type::precision, description_type>::value;
            using dummy_default_pqc_precision = Precision<uint8_t>;
            using this_pqc_precision = get_or_default_t<operator_type::precision, description_type, dummy_default_pqc_precision>;
            using this_pqc_precision_t = typename this_pqc_precision::type;

            // Function
            // * Default value: NONE
            // * Dummy value: Keygen
            static constexpr bool has_function        = has_operator<operator_type::function, description_type>::value;
            using dummy_default_pqc_function          = Function<function::Keygen>;
            using this_pqc_function                   = get_or_default_t<operator_type::function, description_type, dummy_default_pqc_function>;
            static constexpr auto this_pqc_function_v = this_pqc_function::value;

            // SM
            // * Default value: NONE
            // * Dummy value: 700
            static constexpr bool has_sm         = has_operator<operator_type::sm, description_type>::value;
            using dummy_default_pqc_sm          = SM<700>;
            using this_pqc_sm                   = get_or_default_t<operator_type::sm, description_type, dummy_default_pqc_sm>;
            static constexpr auto this_pqc_sm_v = this_pqc_sm::value;

            // Merkle Size
            // * Default value: 2048
            static constexpr bool has_size = has_operator<operator_type::size, description_type>::value;
            using dummy_default_pqc_size = Size<2048>;
            using this_pqc_size = get_or_default_t<operator_type::size, description_type, dummy_default_pqc_size>;
            static constexpr auto this_pqc_size_v = this_pqc_size::value;

            // True if description is complete description
            // Not all descriptions require a security category
            static constexpr bool is_complete = has_algorithm && has_security_category && has_function;
            static constexpr bool is_complete_without_security_category = has_algorithm && has_function;
            // True if description is a complete merkle description
            static constexpr bool is_complete_merkle = has_algorithm && has_size && has_precision && has_function;

            /// ---- Constraints

            // We can only have one of each option

            static constexpr bool has_one_algorithm         = has_at_most_one_of<operator_type::algorithm,      description_type>::value;
            static constexpr bool has_one_security_category = has_at_most_one_of<operator_type::security_category, description_type>::value;
            static constexpr bool has_one_function          = has_at_most_one_of<operator_type::function,       description_type>::value;
            static constexpr bool has_one_sm                = has_at_most_one_of<operator_type::sm,             description_type>::value;
            static constexpr bool has_one_size              = has_at_most_one_of<operator_type::size,     description_type>::value;

            static_assert(has_one_algorithm,      "Can't create pqc function with two Algorithm<> expressions");
            static_assert(has_one_security_category, "Can't create pqc function with two SecurityCategory<> expressions");
            static_assert(has_one_function,       "Can't create pqc function with two Function<> expressions");
            static_assert(has_one_sm,             "Can't create pqc function with two SM<> expressions");
            static_assert(has_one_size,           "Can't create pqc function with two Size<> expressions");

            // Function/algorithm compatibility checks are in algorithm-specific description files
            // (pk_description.hpp, hash_description.hpp, etc.)

            /// ---- End of Constraints

        };

        template<>
        class pqc_description<>: public commondx::detail::description_expression {};

        // Wrapper type to handle some of the algorithm specific parameters (e.g., sizes)
        // Making a helper type is needed in order to conditionally create variables

        template<class... Operators>
        inline constexpr bool has_algorithm(algorithm alg) {
            return pqc_description<Operators...>::has_algorithm
                   && pqc_description<Operators...>::this_pqc_algorithm_v == alg
                   && pqc_description<Operators...>::has_security_category;
        }

        template<class... Operators>
        inline constexpr bool has_algorithm_no_security_category(algorithm alg) {
            return pqc_description<Operators...>::has_algorithm
                   && pqc_description<Operators...>::this_pqc_algorithm_v == alg;
        }

        // Exclude Poseidon2 from this check
        template<class... Operators>
        inline constexpr bool has_no_algorithm() {
            return !pqc_description<Operators...>::has_algorithm
                   || (!pqc_description<Operators...>::has_security_category && pqc_description<Operators...>::this_pqc_algorithm_v != algorithm::POSEIDON2);
        }

        template<class enable, class... Operators>
        class pqc_full_description_helper;

        template<class... Operators>
        using pqc_full_description = pqc_full_description_helper<void, Operators...>;

        // Both algorithm and security level are required for parameters
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_no_algorithm<Operators...>()>, Operators...>
                                   : public pqc_description<Operators...> {
        };
    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_PQC_DESCRIPTION_HPP

