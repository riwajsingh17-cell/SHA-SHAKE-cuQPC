// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_DETAIL_NTT_EXECUTION_HPP
#define CUPQC_DETAIL_NTT_EXECUTION_HPP

#include <optional>

#include "pqc_description.hpp"
#include "database.hpp"
#include "ntt_types.hpp"

namespace cupqc {
    namespace detail {

        template<class... Operators>
        class pqc_ntt_block_execution: public pqc_execution<Operators...>
        {

            using this_type = pqc_ntt_block_execution<Operators...>;
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

            // Sub Size
            // * Default value: 256
            static constexpr bool has_sub_size = has_operator<operator_type::sub_size, base_type>::value;
            using default_pqc_sub_size = SubSize<256>;
            using this_pqc_sub_size = get_or_default_t<operator_type::sub_size, base_type, default_pqc_sub_size>;
            static constexpr auto this_sub_size_v = this_pqc_sub_size::value;

            // NTT Direction
            // * Default value: FORWARD
            static constexpr bool has_direction    = has_operator<operator_type::direction, base_type>::value;
            using default_direction                = Direction<nttDirection::FORWARD>;
            using this_direction                   = get_or_default_t<operator_type::direction, base_type, default_direction>;
            static constexpr auto this_direction_v = this_direction::value;

            // Modulus
            // * Default value: 1
            static constexpr bool has_modulus = has_operator<operator_type::modulus, base_type>::value;
            using default_modulus = Modulus<1>;
            using this_modulus = get_or_default_t<operator_type::modulus, base_type, default_modulus>;
            static constexpr auto this_modulus_v = this_modulus::value;

            /// ---- Constraints
            static_assert(this_type::this_pqc_algorithm_v == algorithm::NTT,
                            "NTT block execution only supports NTT algorithm.");
            static_assert(this_pqc_block_dim::y == 1 && this_pqc_block_dim::z == 1,
                          "Provided block dimension is invalid, y and z dimensions must both be 1.");
            static constexpr bool valid_block_dim = this_pqc_block_dim::flat_size >= 32 && this_pqc_block_dim::flat_size <= 1024;
            static_assert(valid_block_dim,
                          "Provided block dimension is invalid, BlockDim<> must have at least 32 threads, and can't have more than 1024 threads.");

            static_assert(this_pqc_batches_per_block_v > 0, "Providing number of batches per block is invalid.  BatchesPerBlock<0> is unsupported.");

            static constexpr auto is_general_ntt = base_type::has_algorithm && has_size 
                                                    && base_type::has_precision && has_direction;

            static constexpr auto is_complete_ntt = is_general_ntt && has_modulus;

            static constexpr auto is_general_staged_ntt = is_general_ntt && has_sub_size;

            static constexpr auto is_complete_staged_ntt = is_complete_ntt && has_sub_size;

            template<class this_t, algorithm alg, class T = void>
            using algorithm_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_general_ntt && this_t::this_pqc_algorithm_v == alg, T>;

            template<class this_t, algorithm alg, class T = void>
            using forward_gen_ntt_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_general_ntt && this_t::this_pqc_algorithm_v == alg && this_t::this_direction_v == nttDirection::FORWARD, T>;

            template<class this_t, algorithm alg, class T = void>
            using inverse_gen_ntt_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_general_ntt && this_t::this_pqc_algorithm_v == alg && this_t::this_direction_v == nttDirection::INVERSE, T>;

            template<class this_t, algorithm alg, class T = void>
            using ntt_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_complete_ntt && this_t::this_pqc_algorithm_v == alg, T>;

            template<class this_t, algorithm alg, class T = void>
            using general_staged_ntt_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_general_staged_ntt && this_t::this_pqc_algorithm_v == alg, T>;

            template<class this_t, algorithm alg, class T = void>
            using forward_staged_ntt_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_general_staged_ntt && this_t::this_pqc_algorithm_v == alg && this_t::this_direction_v == nttDirection::FORWARD, T>;

            template<class this_t, algorithm alg, class T = void>
            using inverse_staged_ntt_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_general_staged_ntt && this_t::this_pqc_algorithm_v == alg && this_t::this_direction_v == nttDirection::INVERSE, T>;

            template<class this_t, algorithm alg, class T = void>
            using staged_ntt_enable_if_t = COMMONDX_STL_NAMESPACE::enable_if_t<is_complete_staged_ntt && this_t::this_pqc_algorithm_v == alg, T>;

            /// ---- Accessors
        public:
            static constexpr auto BlockDim = this_pqc_block_dim_v;
            static constexpr auto Size = this_size_v;

            /// ---- Execution
        public:
            // NTT
            // N.B., have to use a template to use SFINAE
            /*
            template<class this_t = this_type>
            inline __device__ auto execute(typename this_type::this_pqc_precision_t* data, const typename this_type::this_pqc_precision_t* twiddles,
                                           const typename this_type::this_pqc_precision_t p, const std::optional<typename this_type::this_pqc_precision_t> n_inv)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                         return database::forward_ntt<this_type::this_size_v, this_type::this_pqc_precision_t>(data, twiddles, p);
                        } else {
                         return database::inverse_ntt<this_type::this_size_v, this_type::this_pqc_precision_t>(data, twiddles, p, n_inv.value());
                        }
            }
            */

            // Forward NTT and general functions for NTT
            template<class this_t = this_type>
            inline __device__ auto execute(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* twiddles,
                                           const typename this_type::this_pqc_precision_t p)
                    -> forward_gen_ntt_if_t<this_t, algorithm::NTT> {
                        return database::shared_ntt<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, twiddles, p);
            }

            template<class this_t = this_type>
            inline __device__ auto load(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* data)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (has_modulus) {
                            return database::load_montgomery<this_type::this_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data);
                        } else {
                            return database::load<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data);
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto load_to_mont(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* data, [[maybe_unused]] const nttConst<typename this_type::this_pqc_precision_t> ntt_const = nttConst<typename this_type::this_pqc_precision_t>())
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (has_modulus) {
                            return database::load_montgomery<this_type::this_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data);
                        } else {
                            return database::load_to_mont<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, ntt_const);
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto store(typename this_type::this_pqc_precision_t* sdata, typename this_type::this_pqc_precision_t* data)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (has_modulus) {
                            return database::store_montgomery<this_type::this_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data);
                        } else {
                            return database::store<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data);
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto store_from_mont(typename this_type::this_pqc_precision_t* sdata, typename this_type::this_pqc_precision_t* data, [[maybe_unused]] const nttConst<typename this_type::this_pqc_precision_t> ntt_const = nttConst<typename this_type::this_pqc_precision_t>())
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (has_modulus) {
                            return database::store_montgomery<this_type::this_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data);
                        } else {
                            return database::store_from_mont<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, ntt_const);
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto to_mont(typename this_type::this_pqc_precision_t x, const nttConst<typename this_type::this_pqc_precision_t> ntt_const = nttConst<typename this_type::this_pqc_precision_t>())
                    -> algorithm_enable_if_t<this_t, algorithm::NTT, typename this_type::this_pqc_precision_t> {
                        return database::to_montgomery<this_type::this_pqc_precision_t>(x, ntt_const);
            }

            template<class this_t = this_type>
            inline __device__ auto from_mont(typename this_type::this_pqc_precision_t x, const nttConst<typename this_type::this_pqc_precision_t> ntt_const = nttConst<typename this_type::this_pqc_precision_t>())
                    -> algorithm_enable_if_t<this_t, algorithm::NTT, typename this_type::this_pqc_precision_t> {
                        return database::from_montgomery<this_type::this_pqc_precision_t>(x, ntt_const);
            }

            template<class this_t = this_type>
            inline __device__ auto add(typename this_type::this_pqc_precision_t* data1, const typename this_type::this_pqc_precision_t* data2, const typename this_type::this_pqc_precision_t p)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        return database::shared_transformed_addition<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(data1, data2, p);
            }

            template<class this_t = this_type>
            inline __device__ auto mul(typename this_type::this_pqc_precision_t* data1, const typename this_type::this_pqc_precision_t* data2, const typename this_type::this_pqc_precision_t p)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        return database::shared_transformed_multiplication<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(data1, data2, p);
            }

            template<class this_t = this_type>
            inline __device__ auto sub(typename this_type::this_pqc_precision_t* data1, const typename this_type::this_pqc_precision_t* data2, const typename this_type::this_pqc_precision_t p)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        return database::shared_transformed_subtraction<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(data1, data2, p);
            }

            // Inverse NTT
            template<class this_t = this_type>
            inline __device__ auto execute(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* twiddles,
                                           const typename this_type::this_pqc_precision_t p, const typename this_type::this_pqc_precision_t n_inv)
                    -> inverse_gen_ntt_if_t<this_t, algorithm::NTT> {
                        return database::shared_intt<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, twiddles, p, n_inv);
            }

            // Staged NTT functions and general functions for Staged NTT
            template<class this_t = this_type>
            inline __device__ auto stage_1_execute(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* twiddles,
                                           const typename this_type::this_pqc_precision_t p)
                    -> general_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                         return database::shared_ntt_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, twiddles, p);
                        } 
                        else {
                         return database::shared_intt_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, twiddles, p);
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_1_load(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* data, const int id)
                    -> general_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                            if constexpr (has_modulus) {
                                return database::fwd_load_stage_1_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::fwd_load_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        }
                        else {
                            if constexpr (has_modulus) {
                                return database::inv_load_stage_1_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::inv_load_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_1_load_to_mont(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* data, const int id, [[maybe_unused]] const nttConst<typename this_type::this_pqc_precision_t> ntt_const = nttConst<typename this_type::this_pqc_precision_t>())
                    -> general_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                            if constexpr (has_modulus) {
                                return database::fwd_load_stage_1_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::fwd_load_stage_1_to_mont<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id, ntt_const);
                            }
                        }
                        else {
                            if constexpr (has_modulus) {
                                return database::inv_load_stage_1_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::inv_load_stage_1_to_mont<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id, ntt_const);
                            }
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_1_store(typename this_type::this_pqc_precision_t* sdata, typename this_type::this_pqc_precision_t* data, const int id)
                    -> general_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                            if constexpr (has_modulus) {
                                return database::fwd_store_stage_1_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::fwd_store_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        } 
                        else {
                            if constexpr (has_modulus) {
                                return database::inv_store_stage_1_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::inv_store_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_2_load(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* data, const int id)
                    -> general_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                            if constexpr (has_modulus) {
                                return database::fwd_load_stage_2_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::fwd_load_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        }
                        else {
                            if constexpr (has_modulus) {
                                return database::inv_load_stage_2_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::inv_load_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_2_store(typename this_type::this_pqc_precision_t* sdata, typename this_type::this_pqc_precision_t* data, const int id)
                    -> general_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                            if constexpr (has_modulus) {
                                return database::fwd_store_stage_2_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::fwd_store_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        }
                        else {
                            if constexpr (has_modulus) {
                                return database::inv_store_stage_2_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::inv_store_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            }
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_2_store_from_mont(typename this_type::this_pqc_precision_t* sdata, typename this_type::this_pqc_precision_t* data, const int id, [[maybe_unused]] const nttConst<typename this_type::this_pqc_precision_t> ntt_const = nttConst<typename this_type::this_pqc_precision_t>())
                    -> general_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                            if constexpr (has_modulus) {
                                return database::fwd_store_stage_2_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::fwd_store_stage_2_from_mont<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id, ntt_const);
                            }
                        }
                        else {
                            if constexpr (has_modulus) {
                                return database::inv_store_stage_2_montgomery<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(sdata, data, id);
                            } else {
                                return database::inv_store_stage_2_from_mont<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, data, id, ntt_const);
                            }
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_2_execute(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* twiddles,
                                           const typename this_type::this_pqc_precision_t p)
                    -> forward_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        return database::shared_ntt_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, twiddles, p);
            }

            template<class this_t = this_type>
            inline __device__ auto stage_2_execute(typename this_type::this_pqc_precision_t* sdata, const typename this_type::this_pqc_precision_t* twiddles,
                                           const typename this_type::this_pqc_precision_t p, const typename this_type::this_pqc_precision_t n_inv)
                    -> inverse_staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        return database::shared_intt_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(sdata, twiddles, p, n_inv);
            }

            /* Specialized NTT functions for Montgomery multiplication , currently only uint32_t data and twiddles.*/
            // TODO: Add support for other data types.
            template<class this_t = this_type>
            inline __device__ auto execute(uint32_t* data, const uint32_t* twiddles)
                    -> ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                         return database::shared_ntt_montgomery<this_type::this_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(data, twiddles);
                        } else {
                         return database::shared_intt_montgomery<this_type::this_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(data, twiddles);
                        }
            }

            /* Staged versions of the specialized NTT functions for Montgomery multiplication. */
            template<class this_t = this_type>
            inline __device__ auto stage_1_execute(uint32_t* data, const uint32_t* twiddles)
                    -> staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                         return database::shared_ntt_montgomery_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(data, twiddles);
                        } else {
                         return database::shared_intt_montgomery_stage_1<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(data, twiddles);
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto stage_2_execute(uint32_t* data, const uint32_t* twiddles)
                    -> staged_ntt_enable_if_t<this_t, algorithm::NTT> {
                        if constexpr (this_type::this_direction_v == nttDirection::FORWARD) {
                         return database::shared_ntt_montgomery_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(data, twiddles);
                        } else {
                         return database::shared_intt_montgomery_stage_2<this_type::this_size_v, this_type::this_sub_size_v, this_type::this_modulus_v, this_type::this_pqc_block_dim_v.x>(data, twiddles);
                        }
            }

            template<class this_t = this_type>
            inline __device__ auto make_twiddles(typename this_type::this_pqc_precision_t* twiddles,
                                                 const typename this_type::this_pqc_precision_t p, const typename this_type::this_pqc_precision_t g)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        return database::make_twiddle<this_type::this_size_v, this_type::this_pqc_precision_t>(twiddles, p, g);
                    }

            template<class this_t = this_type>
            inline __device__ auto transform_twiddles_to_mont(typename this_type::this_pqc_precision_t* twiddles,
                                                              const typename this_type::this_pqc_precision_t p)
                    -> algorithm_enable_if_t<this_t, algorithm::NTT> {
                        return database::twiddles_to_mont<this_type::this_size_v, this_type::this_pqc_precision_t, this_type::this_pqc_block_dim_v.x>(twiddles, p);
                    }
             
            // Montgomery variant: modulus is compile-time; only the primitive root g is needed at runtime.
            template<class this_t = this_type>
            inline __device__ auto make_twiddles(uint32_t* twiddles, const uint32_t g)
                    -> ntt_enable_if_t<this_t, algorithm::NTT> {
                        return database::make_twiddle<this_type::this_size_v, this_type::this_modulus_v>(twiddles, g);
                    }
        };

    } // namespace detail
} // namespace cupqc

#endif // CUPQC_DETAIL_NTT_EXECUTION_HPP
