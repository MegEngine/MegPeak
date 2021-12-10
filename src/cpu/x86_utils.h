/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
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
