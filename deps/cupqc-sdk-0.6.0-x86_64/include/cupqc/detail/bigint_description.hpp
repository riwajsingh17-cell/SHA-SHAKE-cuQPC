// Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_BIGINT_DESCRIPTION_HPP
#define CUPQC_DETAIL_BIGINT_DESCRIPTION_HPP

#include "commondx/traits/detail/get.hpp"
#include "../operators.hpp"
#include "../traits/detail/description_traits.hpp"

#include <cstdint>
#include <type_traits>

namespace cupqc {
    namespace detail {

        // Maintenance requirement: this support matrix must be kept in sync
        // with the explicit instantiations in bigint/src/CMakeLists.txt.
        // Unsupported configurations cannot be instantiated implicitly because
        // the implementation is shipped as device LTO objects.
        template<unsigned int N_LIMBS, unsigned int TPI>
        struct is_bigint_instantiated : std::bool_constant<
            ((TPI == 1 && (N_LIMBS == 1 || N_LIMBS == 2 || N_LIMBS == 4 ||
                           (N_LIMBS >= 8 && N_LIMBS <= 32))) ||
             (TPI == 2 && N_LIMBS >= 4 && N_LIMBS <= 64) ||
             (TPI == 4 && N_LIMBS >= 8 && N_LIMBS <= 128) ||
             (TPI == 8 && (N_LIMBS == 12 || (N_LIMBS >= 16 && N_LIMBS <= 128))) ||
             (TPI == 16 && N_LIMBS >= 32 && N_LIMBS <= 128) ||
             (TPI == 32 && N_LIMBS >= 64 && N_LIMBS <= 128))> {};

        template<unsigned int N_LIMBS, unsigned int TPI>
        inline constexpr bool is_bigint_instantiated_v =
            is_bigint_instantiated<N_LIMBS, TPI>::value;

        template<class... Operators>
        class bigint_operator_wrapper: public commondx::detail::description_expression { };

        template<class... Operators>
        class bigint_description: public commondx::detail::description_expression {

            using description_type = bigint_operator_wrapper<Operators...>;

        protected:

            // BitWidth
            // * Default value: 256
            static constexpr bool has_bit_width        = has_operator<operator_type::bit_width, description_type>::value;
            using default_bigint_bit_width             = BitWidth<256>;
            using this_bigint_bit_width                = get_or_default_t<operator_type::bit_width, description_type, default_bigint_bit_width>;
            static constexpr auto this_bigint_bit_width_v = this_bigint_bit_width::value;

            // SM
            // * Default value: 800
            static constexpr bool has_sm         = has_operator<operator_type::sm, description_type>::value;
            using default_bigint_sm              = SM<800>;
            using this_bigint_sm                 = get_or_default_t<operator_type::sm, description_type, default_bigint_sm>;
            static constexpr auto this_bigint_sm_v = this_bigint_sm::value;

            // TPI
            // * Default value: 1
            static constexpr bool has_tpi         = has_operator<operator_type::tpi, description_type>::value;
            using default_bigint_tpi              = TPI<1>;
            using this_bigint_tpi                 = get_or_default_t<operator_type::tpi, description_type, default_bigint_tpi>;
            static constexpr auto this_bigint_tpi_v = this_bigint_tpi::value;

            // OnError (device error handling for bigint operations)
            // * Default: OnErrorNone (no action beyond existing semantics)
            static constexpr bool has_on_error = has_operator<operator_type::on_error, description_type>::value;
            using default_bigint_on_error        = default_bigint_error_policy;
            using this_bigint_on_error           = get_or_default_t<operator_type::on_error, description_type, default_bigint_on_error>;

            // Number of limbs (derived from bit width)
            static constexpr unsigned int n_limbs = this_bigint_bit_width_v / 32;
            static_assert(this_bigint_bit_width_v % 32 == 0, "BitWidth must be a multiple of 32");
            static_assert(this_bigint_bit_width_v > 0, "BitWidth must be greater than 0");

            static_assert(this_bigint_tpi_v >= 1, "TPI must be greater than or equal to 1");
            static_assert(this_bigint_tpi_v <= 32, "TPI must be less than or equal to 32");
            static_assert((this_bigint_tpi_v & (this_bigint_tpi_v - 1u)) == 0u, "TPI must be a power of two");
            static_assert(n_limbs >= this_bigint_tpi_v, "N_LIMBS must be greater than or equal to TPI");

            // True if description is complete
            static constexpr bool is_complete = has_bit_width;

            /// ---- Constraints

            // We can only have one of each option
            static constexpr bool has_one_bit_width = has_at_most_one_of<operator_type::bit_width, description_type>::value;
            static constexpr bool has_one_sm        = has_at_most_one_of<operator_type::sm, description_type>::value;
            static constexpr bool has_one_tpi       = has_at_most_one_of<operator_type::tpi, description_type>::value;
            static constexpr bool has_one_on_error  = has_at_most_one_of<operator_type::on_error, description_type>::value;

            static_assert(has_one_bit_width, "Can't create bigint with two BitWidth<> expressions");
            static_assert(has_one_sm,        "Can't create bigint with two SM<> expressions");
            static_assert(has_one_tpi,       "Can't create bigint with two TPI<> expressions");
            static_assert(has_one_on_error,  "Can't create bigint with two on-error policy expressions");

            /// ---- End of Constraints

        };

        template<>
        class bigint_description<>: public commondx::detail::description_expression {};

        template<class... Operators>
        using bigint_full_description = bigint_description<Operators...>;

    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_BIGINT_DESCRIPTION_HPP
