// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQCDX_OPERATORS_HPP
#define CUPQCDX_OPERATORS_HPP

#include "operators/operator_type.hpp"

#include "operators/algorithm.hpp"
#include "operators/batches_per_block.hpp"
#include "operators/bigint_on_error.hpp"
#include "operators/bit_width.hpp"
#include "operators/capacity.hpp"
#include "operators/direction.hpp"
#include "operators/field.hpp"
#include "operators/function.hpp"
#include "operators/modulus.hpp"
#include "operators/num_moduli.hpp"
#include "operators/precision.hpp"
#include "operators/security_category.hpp"
#include "operators/size.hpp"
#include "operators/sub_size.hpp"
#include "operators/tpi.hpp"
#include "operators/width.hpp"

#include "commondx/operators/block_dim.hpp"
#include "commondx/operators/sm.hpp"
#include "commondx/operators/execution_operators.hpp"

namespace cupqc {
    //Import selected operators from commonDx
    struct Block: public commondx::Block {};
    struct Thread: public commondx::Thread {};
    struct Warp: public commondx::detail::operator_expression {}; // not yet defined in commondx

    template<unsigned int X, unsigned int Y = 1, unsigned int Z = 1>
    struct BlockDim: public commondx::BlockDim<X, Y, Z> {};
    template<unsigned int Architecture>
    struct SM: public commondx::SM<Architecture> {};
} //namespace cupqc

// Register operators
namespace commondx::detail {
    template<>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::block, cupqc::Block>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<>
    struct get_operator_type<cupqc::Block> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::block;
    };

    template<>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::thread, cupqc::Thread>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<>
    struct get_operator_type<cupqc::Thread> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::thread;
    };

    template<>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::warp, cupqc::Warp>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<>
    struct get_operator_type<cupqc::Warp> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::warp;
    };

    template<unsigned int Architecture>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::sm, cupqc::SM<Architecture>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<unsigned int Architecture>
    struct get_operator_type<cupqc::SM<Architecture>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::sm;
    };

    template<unsigned int X, unsigned int Y, unsigned int Z>
    struct is_operator<cupqc::operator_type, cupqc::operator_type::block_dim, cupqc::BlockDim<X, Y, Z>>:
        COMMONDX_STL_NAMESPACE::true_type {
    };

    template<unsigned int X, unsigned int Y, unsigned int Z>
    struct get_operator_type<cupqc::BlockDim<X, Y, Z>> {
        static constexpr cupqc::operator_type value = cupqc::operator_type::block_dim;
    };
} // namespace commondx::detail


#endif // CUPQCDX_OPERATORS_HPP
