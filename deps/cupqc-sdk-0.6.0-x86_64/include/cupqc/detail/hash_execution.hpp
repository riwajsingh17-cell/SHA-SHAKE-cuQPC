// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_HASH_EXECUTION_HPP
#define CUPQC_DETAIL_HASH_EXECUTION_HPP

#include "pqc_description.hpp"
#include "hash_description.hpp"
#include "database.hpp"
#include "hash_types.hpp"

namespace cupqc {
    namespace detail {

        template <typename Exec, algorithm Alg, typename T = void>
        struct pqc_hash_context_helper;

        template <typename Exec, typename T>
        struct pqc_hash_context_helper<Exec, algorithm::SHA3, T>
        {
            using type = typename database::KeccakContext<Exec, T::capacity>;
        };
        template <typename Exec, typename T>
        struct pqc_hash_context_helper<Exec, algorithm::SHAKE, T>
        {
            using type = typename database::KeccakContext<Exec, T::capacity>;
        };
        template <typename Exec, typename T>
        struct pqc_hash_context_helper<Exec, algorithm::SHA2_32, T>
        {
            using type = typename database::SHA2Context<Exec, uint32_t, T::digest>;
        };
        template <typename Exec, typename T>
        struct pqc_hash_context_helper<Exec, algorithm::SHA2_64, T>
        {
            using type = typename database::SHA2Context<Exec, uint64_t, T::digest>;
        };

        template <typename Exec, typename T>
        struct pqc_hash_context_helper<Exec, algorithm::POSEIDON2, T>
        {
            using type = typename database::Poseidon2Context<Exec, T::capacity, T::width, typename T::field_type>;
        };


        template<class... Operators>
        class pqc_thread_execution: public pqc_execution<Operators...>
        {

            using this_type = pqc_thread_execution<Operators...>;
            using base_type = pqc_execution<Operators...>;

            /// ---- Traits

            template<class this_t, function func, class T = void>
            using function_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<(this_t::is_complete || (this_t::is_complete_without_security_category && this_t::this_pqc_algorithm_v == algorithm::POSEIDON2))
                                                                              && this_t::this_pqc_function_v == func, T>;

            template<class this_t, function func, class T = void>
            using sha_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<this_t::is_complete  && this_t::this_pqc_algorithm_v != algorithm::POSEIDON2 && this_t::this_pqc_function_v == func, T>;

            template<class this_t, function func, class T = void>
            using poseidon2_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<this_t::is_complete_without_security_category  && this_t::this_pqc_algorithm_v == algorithm::POSEIDON2 && this_t::this_pqc_function_v == func, T>;

            using context_t = typename pqc_hash_context_helper<cupqc::Thread, this_type::this_pqc_algorithm_v, this_type>::type;

            [[no_unique_address]]
            context_t context;

            /// ---- Constraints
            static_assert(is_hash_algorithm(this_type::this_pqc_algorithm_v),
                "Thread execution is only supported for hashing.");

            /// ---- Accessors
        public:
            static constexpr auto SecurityCategory = base_type::this_pqc_security_category_v;
            static constexpr bool has_security_category = base_type::has_security_category;
            /// ---- Execution
        public:

            // hash
            // N.B., have to use a template to use SFINAE
            template<class this_t = this_type>
            inline __device__ auto reset(void)
                -> function_enable_if_t<this_t, function::Hash> {
                    context.reset();
            }

            template<class this_t = this_type>
            inline __device__ auto update(const uint8_t* buffer, size_t len)
                -> sha_enable_if_t<this_t, function::Hash> {
                    context.update(buffer, len);
            }
            template<class this_t = this_type>
            inline __device__ auto update(const uint32_t* buffer, size_t len)
                -> poseidon2_enable_if_t<this_t, function::Hash> {
                    context.update(buffer, len);
            }

            template<class this_t = this_type>
            inline __device__ auto finalize(void)
                -> function_enable_if_t<this_t, function::Hash> {
                    if constexpr (this_type::this_pqc_algorithm_v == algorithm::SHA2_32 || this_type::this_pqc_algorithm_v == algorithm::SHA2_64 || this_type::this_pqc_algorithm_v == algorithm::POSEIDON2) {
                        context.finalize();
                    } else {
                        context.finalize(this_type::pad);
                    }
            }

            template<class this_t = this_type>
            inline __device__ auto digest(uint8_t* buffer, size_t len)
                -> sha_enable_if_t<this_t, function::Hash> {
                    context.digest(buffer, len);
            }
            template<class this_t = this_type>
            inline __device__ auto digest(uint32_t* buffer, size_t len)
                -> poseidon2_enable_if_t<this_t, function::Hash> {
                    context.digest(buffer, len);
            }
        };

        template<class... Operators>
        class pqc_warp_execution: public pqc_execution<Operators...>
        {

            using this_type = pqc_warp_execution<Operators...>;
            using base_type = pqc_execution<Operators...>;

            /// ---- Traits
            template<class this_t, function func, class T = void>
            using function_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<this_t::is_complete && this_t::this_pqc_function_v == func, T>;

            using context_t = typename pqc_hash_context_helper<cupqc::Warp, this_type::this_pqc_algorithm_v, this_type>::type;

            [[no_unique_address]]
            context_t context;

            /// ---- Constraints
            static_assert(is_hash_algorithm(this_type::this_pqc_algorithm_v),
                "Warp execution is only supported for hashing.");

            /// ---- Accessors
        public:

            /// ---- Execution
        public:

            // hash
            // N.B., have to use a template to use SFINAE
            template<class this_t = this_type>
            inline __device__ auto reset(void)
                -> function_enable_if_t<this_t, function::Hash> {
                    context.reset();
            }
            template<class this_t = this_type>
            inline __device__ auto update(const uint8_t* buffer, size_t len)
                -> function_enable_if_t<this_t, function::Hash> {
                    context.update(buffer, len);
            }
            template<class this_t = this_type>
            inline __device__ auto finalize(void)
                -> function_enable_if_t<this_t, function::Hash> {
                    context.finalize(this_type::pad);
            }
            template<class this_t = this_type>
            inline __device__ auto digest(uint8_t* buffer, size_t len)
                -> function_enable_if_t<this_t, function::Hash> {
                    context.digest(buffer, len);
            }

        };


        template<class... Operators>
        class pqc_merkle_block_execution: public pqc_execution<Operators...>
        {

            using this_type = pqc_merkle_block_execution<Operators...>;
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

            // Size
            // * Default value: 2048
            static constexpr bool has_size    = has_operator<operator_type::size, base_type>::value;
            using default_pqc_size            = Size<2048>;
            using this_pqc_size               = get_or_default_t<operator_type::size, base_type, default_pqc_size>;
            static constexpr auto this_size_v = this_pqc_size::value;

            /// ---- Constraints
            static_assert(is_merkle_algorithm(this_type::this_pqc_algorithm_v),
                            "Merkle block execution only supports Merkle algorithm.");
            static_assert(this_pqc_block_dim::y == 1 && this_pqc_block_dim::z == 1,
                          "Provided block dimension is invalid, y and z dimensions must both be 1.");
            static constexpr bool valid_block_dim = this_pqc_block_dim::flat_size >= 32 && this_pqc_block_dim::flat_size <= 1024;
            static_assert(valid_block_dim,
                          "Provided block dimension is invalid, BlockDim<> must have at least 32 threads, and can't have more than 1024 threads.");

            static_assert(this_pqc_batches_per_block_v > 0, "Providing number of batches per block is invalid.  BatchesPerBlock<0> is unsupported.");

            template<class this_t, function func, class T = void>
            using merkle_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<this_t::is_complete_merkle && this_t::this_pqc_function_v == func, T>;

            /// ---- Accessors
        public:
            static constexpr auto BlockDim = this_pqc_block_dim_v;
            static constexpr auto Size = this_size_v;

            /// ---- Execution
        public:
            // Merkle Tree operations
            template<class this_t = this_type, class Hash>
            inline __device__ auto create_leaf(typename this_type::this_pqc_precision_t* leaf, const typename this_type::this_pqc_precision_t* data, Hash& hash, const size_t inbuf_len)
                -> merkle_enable_if_t<this_t, function::Merkle> {
                return database::create_leaf<Hash, this_type::this_pqc_precision_t>(leaf, data, hash, inbuf_len);
            }

            template<class this_t = this_type, class Hash>
            inline __device__ auto generate_tree(Hash& hash, tree<this_type::this_size_v, Hash, typename this_type::this_pqc_precision_t>& merkle)
                -> merkle_enable_if_t<this_t, function::Merkle> {
                return database::generate_tree<this_type::this_size_v, Hash, this_type::this_pqc_precision_t>(hash, merkle);
            }

            template<class this_t = this_type, class Hash, uint32_t N>
            inline __device__ auto generate_sub_tree(Hash& hash, tree<N, Hash, typename this_type::this_pqc_precision_t>& merkle, const size_t sub_tree_num)
                -> merkle_enable_if_t<this_t, function::Merkle> {
                return database::generate_sub_tree<N, this_type::this_size_v, Hash, this_type::this_pqc_precision_t>(hash, merkle, sub_tree_num);
            }

            template<class this_t = this_type, class Hash>
            inline __device__ auto generate_proof(proof<this_type::this_size_v, Hash, typename this_type::this_pqc_precision_t>& proof,
                                                  const typename this_type::this_pqc_precision_t* proof_leaf, const uint32_t leaf_index,
                                                  const tree<this_type::this_size_v, Hash, typename this_type::this_pqc_precision_t>& merkle)
                -> merkle_enable_if_t<this_t, function::Merkle> {
                return database::generate_proof<this_type::this_size_v, Hash, this_type::this_pqc_precision_t>(proof, proof_leaf, leaf_index, merkle);
            }

            template<class this_t = this_type, class Hash>
            inline __device__ auto verify_proof(const proof<this_type::this_size_v, Hash, typename this_type::this_pqc_precision_t>& proof,
                                                const typename this_type::this_pqc_precision_t* verify_leaf, const uint32_t leaf_index,
                                                const typename this_type::this_pqc_precision_t* root, Hash& hash)
                -> merkle_enable_if_t<this_t, function::Merkle, bool> {
                return database::verify_proof<this_type::this_size_v, Hash, this_type::this_pqc_precision_t>(proof, verify_leaf, leaf_index, root, hash);
            }
        };

    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_HASH_EXECUTION_HPP
