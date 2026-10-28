// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_PK_DESCRIPTION_HPP
#define CUPQC_DETAIL_PK_DESCRIPTION_HPP

#include "pqc_description.hpp"

namespace cupqc {
    namespace detail {

        // Kyber parameters
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_algorithm<Operators...>(algorithm::ML_KEM)>, Operators...>
                                   : public pqc_description<Operators...> {

        private:
            using base_type = pqc_description<Operators...>;
            static constexpr auto sec_category = base_type::this_pqc_security_category_v;
            static constexpr auto func = base_type::this_pqc_function_v;

            static_assert(sec_category == 1 || sec_category == 3 || sec_category == 5, "Kyber only supports security levels 1, 3, 5");
            static_assert(func == function::Keygen || func == function::Encaps || func == function::Decaps,
                         "ML-KEM only supports Keygen, Encaps, and Decaps functions");

        public:
            // TODO can we avoid the duplication with kem_kyber_parameters.hpp, without leaking anything else in that file?

            static constexpr size_t public_key_size = (sec_category == 1) ?  800 : ((sec_category == 3) ? 1184 : 1568);
            static constexpr size_t secret_key_size = (sec_category == 1) ? 1632 : ((sec_category == 3) ? 2400 : 3168);
            static constexpr size_t ciphertext_size = (sec_category == 1) ?  768 : ((sec_category == 3) ? 1088 : 1568);
            static constexpr size_t shared_secret_size = 32;
        };

        // ML-DSA parameters
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_algorithm<Operators...>(algorithm::ML_DSA)>, Operators...>
                                   : public pqc_description<Operators...> {

        private:
            using base_type = pqc_description<Operators...>;
            static constexpr auto sec_category = base_type::this_pqc_security_category_v;
            static constexpr auto func = base_type::this_pqc_function_v;

            static_assert(sec_category == 2 || sec_category == 3 || sec_category == 5, "ML-DSA only supports security levels 2, 3, 5");
            static_assert(func == function::Keygen || func == function::Sign || func == function::Verify,
                         "ML-DSA only supports Keygen, Sign, and Verify functions");

        public:
            static constexpr size_t public_key_size = (sec_category == 2) ? 1312 : ((sec_category == 3) ? 1952 : 2592);
            static constexpr size_t secret_key_size = (sec_category == 2) ? 2560 : ((sec_category == 3) ? 4032 : 4896);
            static constexpr size_t signature_size  = (sec_category == 2) ? 2420 : ((sec_category == 3) ? 3309 : 4627);
        };

    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_PK_DESCRIPTION_HPP
