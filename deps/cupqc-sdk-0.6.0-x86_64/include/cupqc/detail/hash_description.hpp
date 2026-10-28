// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_HASH_DESCRIPTION_HPP
#define CUPQC_DETAIL_HASH_DESCRIPTION_HPP

#include "pqc_description.hpp"
#include "../hash_types.hpp"

namespace cupqc {
    namespace detail {

        // SHA3 parameters
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_algorithm<Operators...>(algorithm::SHA3)>, Operators...>
                                   : public pqc_description<Operators...> {

        private:
            using base_type = pqc_description<Operators...>;
            using description_type = pqc_operator_wrapper<Operators...>;

            static constexpr auto sec_category = base_type::this_pqc_security_category_v;

            // Override function default to Hash for hash algorithms
            static constexpr bool has_function = has_operator<operator_type::function, description_type>::value;
            using dummy_default_pqc_function = Function<function::Hash>;
            using this_pqc_function = get_or_default_t<operator_type::function, description_type, dummy_default_pqc_function>;

        public:
            static constexpr auto this_pqc_function_v = this_pqc_function::value;

        private:
            static_assert(sec_category == 1 || sec_category == 2 || sec_category == 4 || sec_category == 5, "SHA-3 only supports security categories 1, 2, 4, 5");
            static_assert(this_pqc_function_v == function::Hash, "SHA-3 only supports Hash function");

        public:
            static constexpr uint32_t capacity = sec_category == 1 ? 448 :
                                                 sec_category == 2 ? 512 :
                                                 sec_category == 4 ? 768 :
                                                                     1024;
            static constexpr uint32_t rate = 1600 - capacity;
            static constexpr uint32_t rate_8 = rate / 8;
            static constexpr uint32_t rate_32 = rate / 32;
            static constexpr uint32_t rate_64 = rate / 64;

            // 0000 0110 - For SHA3, input stream is suffixed with bits 01 followed by 10*1 padding
            static constexpr uint8_t pad = uint8_t(0x06);

            static constexpr bool has_digest_size = true;
            static constexpr size_t digest_size = capacity / (2 * 8) ; // capacity is double the digest
        };

        // Poseidon2 parameters
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_algorithm_no_security_category<Operators...>(algorithm::POSEIDON2)>, Operators...>
                                   : public pqc_description<Operators...> {

        private:
            using base_type = pqc_description<Operators...>;
            using description_type = pqc_operator_wrapper<Operators...>;

            // Override function default to Hash for hash algorithms
            static constexpr bool has_function = has_operator<operator_type::function, description_type>::value;
            using dummy_default_pqc_function = Function<function::Hash>;
            using this_pqc_function = get_or_default_t<operator_type::function, description_type, dummy_default_pqc_function>;

        public:
            static constexpr auto this_pqc_function_v = this_pqc_function::value;

        private:
            static_assert(this_pqc_function_v == function::Hash, "Poseidon2 only supports Hash function");

        public:
            static constexpr uint32_t width = pqc_description<Operators...>::this_pqc_width_v;
            static_assert(width == 16 || width == 24, "Poseidon2 only supports widths 16, 24");
            static constexpr uint32_t capacity = pqc_description<Operators...>::this_pqc_capacity_v;
            static_assert(capacity == 8, "Poseidon2 only supports capacity 8, currently");
            static constexpr auto field = pqc_description<Operators...>::this_pqc_field_v;

            //TODO find a better way to give the field type to the Poseidon2Context
            using field_type = std::conditional_t<(field == field::BabyBear),
                               cupqc_common::BabyBearMont,
                               std::conditional_t<(field == field::BabyBearDefault),
                               cupqc_common::BabyBearDefault,
                               std::conditional_t<(field == field::KoalaBear),
                               cupqc_common::KoalaBearMont,
                               cupqc_common::KoalaBearDefault>>>;

            // Technically Poseidon2 does not have a security category, but we can fake it for Merkle
            // We might use this parameter for deciding the size of the hash output for Merkle Trees
            // TODO: find a better way to handle this
            static constexpr auto security_category = 1;
            static constexpr bool has_digest_size = false;
            static constexpr size_t digest_size = 0;
        };

        // SHAKE parameters
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_algorithm<Operators...>(algorithm::SHAKE)>, Operators...>
                                   : public pqc_description<Operators...> {

        private:
            using base_type = pqc_description<Operators...>;
            using description_type = pqc_operator_wrapper<Operators...>;

            static constexpr auto sec_category = base_type::this_pqc_security_category_v;

            // Override function default to Hash for hash algorithms
            static constexpr bool has_function = has_operator<operator_type::function, description_type>::value;
            using dummy_default_pqc_function = Function<function::Hash>;
            using this_pqc_function = get_or_default_t<operator_type::function, description_type, dummy_default_pqc_function>;

        public:
            static constexpr auto this_pqc_function_v = this_pqc_function::value;

        private:
            static_assert(sec_category == 1 || sec_category == 2, "SHAKE only supports security categories 1, 2");
            static_assert(this_pqc_function_v == function::Hash, "SHAKE only supports Hash function");

        public:
            static constexpr uint32_t capacity = 256 * sec_category;
            static constexpr uint32_t rate = 1600 - capacity;
            static constexpr uint32_t rate_8 = rate / 8;

            // 0001 1111 - For SHAKE, input stream is suffixed with bits 1111 followed by 10*1 padding
            static constexpr uint8_t pad = uint8_t(0x1F);

            static constexpr bool has_digest_size = false;
            // SHAKE does not have a digest size, so we set it to 0
            // This is used to avoid a compile error when using SHAKE with Merkle Trees
            // TODO: find a better way to handle this
            static constexpr size_t digest_size = 0;
        };

        // SHA2_32
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_algorithm<Operators...>(algorithm::SHA2_32)>, Operators...>
                                   : public pqc_description<Operators...> {

        private:
            using base_type = pqc_description<Operators...>;
            using description_type = pqc_operator_wrapper<Operators...>;

            static constexpr auto sec_category = base_type::this_pqc_security_category_v;

            // Override function default to Hash for hash algorithms
            static constexpr bool has_function = has_operator<operator_type::function, description_type>::value;
            using dummy_default_pqc_function = Function<function::Hash>;
            using this_pqc_function = get_or_default_t<operator_type::function, description_type, dummy_default_pqc_function>;

        public:
            static constexpr auto this_pqc_function_v = this_pqc_function::value;

        private:
            static_assert(sec_category == 1 || sec_category == 2, "SHA2_32 only supports security categories 1, 2");
            static_assert(this_pqc_function_v == function::Hash, "SHA2_32 only supports Hash function");

        public:
            static constexpr uint32_t digest = sec_category == 1 ? 224 : 256;

            static constexpr bool has_digest_size = true;
            static constexpr size_t digest_size = digest / 8 ;
        };

        // SHA2_64
        template<class... Operators>
        class pqc_full_description_helper<std::enable_if_t<has_algorithm<Operators...>(algorithm::SHA2_64)>, Operators...>
                                   : public pqc_description<Operators...> {

        private:
            using base_type = pqc_description<Operators...>;
            using description_type = pqc_operator_wrapper<Operators...>;

            static constexpr auto sec_category = base_type::this_pqc_security_category_v;

            // Override function default to Hash for hash algorithms
            static constexpr bool has_function = has_operator<operator_type::function, description_type>::value;
            using dummy_default_pqc_function = Function<function::Hash>;
            using this_pqc_function = get_or_default_t<operator_type::function, description_type, dummy_default_pqc_function>;

        public:
            static constexpr auto this_pqc_function_v = this_pqc_function::value;

        private:
            static_assert(sec_category == 1 || sec_category == 2 || sec_category == 4 || sec_category == 5, "SHA2_64 only supports security categories 1, 2, 4, 5");
            static_assert(this_pqc_function_v == function::Hash, "SHA2_64 only supports Hash function");

        public:
            static constexpr uint32_t digest = sec_category == 1 ? 224 :
                                               sec_category == 2 ? 256 :
                                               sec_category == 4 ? 384 :
                                               512 ;

            static constexpr bool has_digest_size = true;
            static constexpr size_t digest_size = digest / 8 ;
        };

    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_HASH_DESCRIPTION_HPP
