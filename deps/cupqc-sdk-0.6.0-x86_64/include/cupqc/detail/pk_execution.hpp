// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_PK_EXECUTION_HPP
#define CUPQC_DETAIL_PK_EXECUTION_HPP

#include "pqc_description.hpp"
#include "pk_description.hpp"
#include "database.hpp"

namespace cupqc {
    namespace detail {

        template<class... Operators>
        class pqc_block_execution: public pqc_execution<Operators...>
        {

            using this_type = pqc_block_execution<Operators...>;
            using base_type = pqc_execution<Operators...>;

            /// ---- Traits

            // Block Dimension
            // * Default value: selected by implementation
            static constexpr bool has_block_dim        = has_operator<operator_type::block_dim, base_type>::value;
            using default_pqc_block_dim                = BlockDim<128>;
            using this_pqc_block_dim                   = get_or_default_t<operator_type::block_dim, base_type, default_pqc_block_dim>;
            static constexpr auto this_pqc_block_dim_v = this_pqc_block_dim::value;

            // Batches per Block
            // * Default value: 1
            static constexpr bool has_batches_per_block        = has_operator<operator_type::batches_per_block, base_type>::value;
            using default_pqc_batches_per_block                = BatchesPerBlock<1>;
            using this_pqc_batches_per_block                   = get_or_default_t<operator_type::batches_per_block, base_type, default_pqc_batches_per_block>;
            static constexpr auto this_pqc_batches_per_block_v = this_pqc_batches_per_block::value;

            /// ---- Constraints
            static_assert(is_kem_algorithm(this_type::this_pqc_algorithm_v) || is_dss_algorithm(this_type::this_pqc_algorithm_v),
                            "PK block execution only supports KEM and DSS algorithms.");
            static_assert(this_pqc_block_dim::y == 1 && this_pqc_block_dim::z == 1,
                          "Provided block dimension is invalid, y and z dimensions must both be 1.");
            static constexpr bool valid_block_dim = this_pqc_block_dim::flat_size >= 32 && this_pqc_block_dim::flat_size <= 1024;
            static_assert(valid_block_dim,
                          "Provided block dimension is invalid, BlockDim<> must have at least 32 threads, and can't have more than 1024 threads.");

            static_assert(this_pqc_batches_per_block_v > 0, "Providing number of batches per block is invalid.  BatchesPerBlock<0> is unsupported.");

            template<class this_t, function func, class T = void>
            using function_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<this_t::is_complete && this_t::this_pqc_function_v == func, T>;

            /// ---- Accessors
        public:
            static constexpr auto BlockDim = this_pqc_block_dim_v;
            static constexpr auto SecurityCategory = base_type::this_pqc_security_category_v;

            /// ---- Execution
        public:
            static constexpr size_t workspace_size = database::global_memory_size<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_function_v>();
            static constexpr size_t shared_memory_size = database::shared_memory_size<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_function_v>();
            static constexpr size_t entropy_size = database::entropy_size<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_function_v>();

            // keygen
            // N.B., have to use a template to use SFINAE
            template<class this_t = this_type>
            inline __device__ auto execute(uint8_t* public_key, uint8_t* secret_key,
                                           uint8_t* entropy,
                                           uint8_t* workspace, uint8_t* smem_workspace)
                    -> function_enable_if_t<this_t, function::Keygen> {
                database::keygen<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_block_dim_v.x>(public_key, secret_key, entropy, workspace, smem_workspace);
            }

            // encaps
            // N.B., have to use a template to use SFINAE
            template<class this_t = this_type>
            inline __device__ auto execute(uint8_t* cipher_text, uint8_t* shared_secret, const uint8_t* public_key,
                                           uint8_t* entropy,
                                           uint8_t* workspace, uint8_t* smem_workspace)
                    -> function_enable_if_t<this_t, function::Encaps> {
                database::encaps<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_block_dim_v.x>(cipher_text, shared_secret, public_key, entropy, workspace, smem_workspace);
            }

            // decaps
            // N.B., have to use a template to use SFINAE
            template<class this_t = this_type>
            inline __device__ auto execute(uint8_t* shared_secret, const uint8_t* ciphertext, const uint8_t* secret_key,
                                           uint8_t* workspace, uint8_t* smem_workspace)
                    -> function_enable_if_t<this_t, function::Decaps> {
                database::decaps<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_block_dim_v.x>(shared_secret, ciphertext, secret_key, workspace, smem_workspace);
            }

            // sign
            // N.B., have to use a template to use SFINAE
            template<class this_t = this_type>
            inline __device__ auto execute(uint8_t* signature, const uint8_t* message, const size_t message_length, const uint8_t* secret_key,
                                           uint8_t* entropy, uint8_t* workspace, uint8_t* smem_workspace)
                    -> function_enable_if_t<this_t, function::Sign> {

                constexpr uint8_t* context = nullptr;
                constexpr uint8_t context_length = 0;
                this->execute(signature, message, message_length, context, context_length, secret_key, entropy, workspace, smem_workspace);
            }
            template<class this_t = this_type>
            inline __device__ auto execute(uint8_t* signature, const uint8_t* message, const size_t message_length, const uint8_t* context, const uint8_t context_length, const uint8_t* secret_key,
                                           uint8_t* entropy, uint8_t* workspace, uint8_t* smem_workspace)
                    -> function_enable_if_t<this_t, function::Sign> {
                database::sign<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_block_dim_v.x>(signature, message, message_length, context, context_length, secret_key, entropy, workspace, smem_workspace);
            }

            // verify
            // N.B., have to use a template to use SFINAE
            template<class this_t = this_type>
            inline __device__ auto execute(const uint8_t* message, const size_t message_length, const uint8_t* signature, const uint8_t* public_key,
                                           uint8_t* workspace, uint8_t* smem_workspace)
                    -> function_enable_if_t<this_t, function::Verify, bool> {
                constexpr uint8_t* context = nullptr;
                constexpr uint8_t context_length = 0;
                return this->execute(message, message_length, context, context_length, signature, public_key, workspace, smem_workspace);
            }
            template<class this_t = this_type>
            inline __device__ auto execute(const uint8_t* message, const size_t message_length, const uint8_t* context, const uint8_t context_length, const uint8_t* signature, const uint8_t* public_key,
                                           uint8_t* workspace, uint8_t* smem_workspace)
                    -> function_enable_if_t<this_t, function::Verify, bool> {
                return database::verify<this_type::this_pqc_algorithm_v, this_type::this_pqc_security_category_v, this_type::this_pqc_block_dim_v.x>(message, message_length, context, context_length, signature, public_key, workspace, smem_workspace);
            }
        };

    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_PK_EXECUTION_HPP
