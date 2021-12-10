/**
 * \file src/cpu/x86_utils.h
 *
 * This file is part of MegPeak.
 *
 * \copyright Copyright (c) 2019-2019 Megvii Inc. All rights reserved.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace megpeak {
enum class SIMDType {
    SSE,
    SSE2,
    SSE3,
    SSE4_1,
    SSE4_2,
    AVX,
    AVX2,
    FMA,
    AVX512,
    VNNI,
    __NR_SIMD_TYPE  //! total number of SIMD types; used for testing
};

bool is_supported(SIMDType type);
}  // namespace megpeak

// vim: syntax=cpp.doxygen
