// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef PK_HPP
#define PK_HPP

#include "cupqc.hpp"

#include "workspace.hpp"

namespace cupqc {
    using ML_KEM_512  = decltype(Algorithm<algorithm::ML_KEM>() + SecurityCategory<1>());
    using ML_KEM_768  = decltype(Algorithm<algorithm::ML_KEM>() + SecurityCategory<3>());
    using ML_KEM_1024 = decltype(Algorithm<algorithm::ML_KEM>() + SecurityCategory<5>());
    using ML_DSA_44   = decltype(Algorithm<algorithm::ML_DSA>() + SecurityCategory<2>());
    using ML_DSA_65   = decltype(Algorithm<algorithm::ML_DSA>() + SecurityCategory<3>());
    using ML_DSA_87   = decltype(Algorithm<algorithm::ML_DSA>() + SecurityCategory<5>());
}

#endif // PK_HPP
