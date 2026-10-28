// Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef NTT_HPP
#define NTT_HPP

#include "cupqc.hpp"

namespace cupqc {
    // Returns the amount of shared memory required for the NTT.
    template<uint32_t N, typename Precision>
    constexpr size_t ntt_shared_workspace_size() {
        return N * sizeof(Precision);
    }

    template<uint32_t N, uint32_t M, typename Precision>
    constexpr size_t fwd_stage_1_ntt_shared_workspace_size() {
        return (N / M) * sizeof(Precision);
    }

    template<uint32_t N, uint32_t M, typename Precision>
    constexpr size_t fwd_stage_2_ntt_shared_workspace_size() {
        return M * sizeof(Precision);
    }

    template<uint32_t N, uint32_t M, typename Precision>
    constexpr size_t inv_stage_1_ntt_shared_workspace_size() {
        return M * sizeof(Precision);
    }

    template<uint32_t N, uint32_t M, typename Precision>
    constexpr size_t inv_stage_2_ntt_shared_workspace_size() {
        return (N / M) * sizeof(Precision);
    }

    /* Calculates N^{-1} mod p, which is equivalent to N^{p-2} mod p */
    template<uint32_t N, typename Precision>
    Precision n_inv(Precision p) {
        Precision result = 1;
        Precision a = N;
        Precision b = p - 2;
        while (b > 0) {
            if (b & 1) result = static_cast<Precision>((static_cast<uint64_t>(result) * static_cast<uint64_t>(a)) % p);
            a = static_cast<Precision>((static_cast<uint64_t>(a) * static_cast<uint64_t>(a)) % p);
            b >>= 1;
        }
        return result;
    }

    constexpr uint32_t BabyBear = 2013265921;
    constexpr uint32_t BabyBear_primitive_root_10 = 341742893;
    constexpr uint32_t BabyBear_primitive_root_11 = 1340477990;
    constexpr uint32_t BabyBear_primitive_root_12 = 1282623253;
    constexpr uint32_t BabyBear_primitive_root_13 = 298008106;
    constexpr uint32_t BabyBear_primitive_root_14 = 1657000625;
    constexpr uint32_t BabyBear_primitive_root_15 = 2009781145;
    constexpr uint32_t BabyBear_primitive_root_16 = 1421947380;
    constexpr uint32_t BabyBear_primitive_root_17 = 1286330022;
    constexpr uint32_t BabyBear_primitive_root_18 = 1559589183;
    constexpr uint32_t BabyBear_primitive_root_19 = 1049899240;
    constexpr uint32_t BabyBear_primitive_root_20 = 195061667;
    constexpr uint32_t BabyBear_primitive_root_21 = 414040701;
    constexpr uint32_t BabyBear_primitive_root_22 = 570250684;
    constexpr uint32_t BabyBear_primitive_root_23 = 1267047229;
    constexpr uint32_t BabyBear_primitive_root_24 = 1003846038;
    //Derived
    constexpr uint32_t BabyBear_primitive_root_9 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_10) * static_cast<uint64_t>(BabyBear_primitive_root_10) % BabyBear);
    constexpr uint32_t BabyBear_primitive_root_8 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_9) * static_cast<uint64_t>(BabyBear_primitive_root_9) % BabyBear);
    constexpr uint32_t BabyBear_primitive_root_7 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_8) * static_cast<uint64_t>(BabyBear_primitive_root_8) % BabyBear);
    constexpr uint32_t BabyBear_primitive_root_6 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_7) * static_cast<uint64_t>(BabyBear_primitive_root_7) % BabyBear);
    constexpr uint32_t BabyBear_primitive_root_5 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_6) * static_cast<uint64_t>(BabyBear_primitive_root_6) % BabyBear);
    constexpr uint32_t BabyBear_primitive_root_4 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_5) * static_cast<uint64_t>(BabyBear_primitive_root_5) % BabyBear);
    constexpr uint32_t BabyBear_primitive_root_3 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_4) * static_cast<uint64_t>(BabyBear_primitive_root_4) % BabyBear);
    constexpr uint32_t BabyBear_primitive_root_2 = static_cast<uint32_t>(static_cast<uint64_t>(BabyBear_primitive_root_3) * static_cast<uint64_t>(BabyBear_primitive_root_3) % BabyBear);

    constexpr uint32_t KoalaBear = 2130706433;
    constexpr uint32_t KoalaBear_primitive_root_10 = 860702919;
    constexpr uint32_t KoalaBear_primitive_root_11 = 2000983452;
    constexpr uint32_t KoalaBear_primitive_root_12 = 1989134074;
    constexpr uint32_t KoalaBear_primitive_root_13 = 809067698;
    constexpr uint32_t KoalaBear_primitive_root_14 = 1047035213;
    constexpr uint32_t KoalaBear_primitive_root_15 = 1168510561;
    constexpr uint32_t KoalaBear_primitive_root_16 = 1848593786;
    constexpr uint32_t KoalaBear_primitive_root_17 = 373019801;
    constexpr uint32_t KoalaBear_primitive_root_18 = 1816824389;
    constexpr uint32_t KoalaBear_primitive_root_19 = 339671193;
    constexpr uint32_t KoalaBear_primitive_root_20 = 1364057261;
    constexpr uint32_t KoalaBear_primitive_root_21 = 1213133211;
    constexpr uint32_t KoalaBear_primitive_root_22 = 542991299;
    constexpr uint32_t KoalaBear_primitive_root_23 = 1760025929;
    constexpr uint32_t KoalaBear_primitive_root_24 = 1791270792;
    //Derived
    constexpr uint32_t KoalaBear_primitive_root_9 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_10) * static_cast<uint64_t>(KoalaBear_primitive_root_10) % KoalaBear);
    constexpr uint32_t KoalaBear_primitive_root_8 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_9) * static_cast<uint64_t>(KoalaBear_primitive_root_9) % KoalaBear);
    constexpr uint32_t KoalaBear_primitive_root_7 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_8) * static_cast<uint64_t>(KoalaBear_primitive_root_8) % KoalaBear);
    constexpr uint32_t KoalaBear_primitive_root_6 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_7) * static_cast<uint64_t>(KoalaBear_primitive_root_7) % KoalaBear);
    constexpr uint32_t KoalaBear_primitive_root_5 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_6) * static_cast<uint64_t>(KoalaBear_primitive_root_6) % KoalaBear);
    constexpr uint32_t KoalaBear_primitive_root_4 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_5) * static_cast<uint64_t>(KoalaBear_primitive_root_5) % KoalaBear);
    constexpr uint32_t KoalaBear_primitive_root_3 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_4) * static_cast<uint64_t>(KoalaBear_primitive_root_4) % KoalaBear);
    constexpr uint32_t KoalaBear_primitive_root_2 = static_cast<uint32_t>(static_cast<uint64_t>(KoalaBear_primitive_root_3) * static_cast<uint64_t>(KoalaBear_primitive_root_3) % KoalaBear);
}

#endif // NTT_HPP
