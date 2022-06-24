/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 */

#include "src/cpu/x86_utils.h"

#if MEGPEAK_X86
#include <xmmintrin.h>

#ifdef _WIN32
// For __cpuid
#include <intrin.h>
#endif

using namespace megpeak;
namespace {
struct CPUID {
    uint32_t eax, ebx, ecx, edx;
    CPUID() {
#if defined(_WIN32)
        int cpuInfo[4];
        __cpuid(cpuInfo, 1);
        eax = cpuInfo[0];
        ebx = cpuInfo[1];
        ecx = cpuInfo[2];
        edx = cpuInfo[3];
#else
        asm volatile("cpuid\n"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(1)
                     : "cc");
#endif
    }
} cpuid;

bool bit(unsigned x, unsigned y) {
    return (x >> y) & 1;
}

bool feature_detect_avx2() {
    uint32_t eax, ebx, ecx, edx;

    // check cpu support
#if defined(_WIN32)
    int cpuInfo[4];
    __cpuid(cpuInfo, 7);
    eax = cpuInfo[0];
    ebx = cpuInfo[1];
    ecx = cpuInfo[2];
    edx = cpuInfo[3];
#else
    asm volatile("cpuid\n"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(7), "c"(0)
                 : "cc");
#endif

    if (!(bit(ebx, 3) && bit(ebx, 5) && bit(ebx, 8)))
        return false;

    // check os support
    asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));

    return (eax & 6) == 6;
}

bool feature_detect_avx512() {
    uint32_t eax, ebx, ecx, edx;

    // check cpu support
#if defined(_WIN32)
    int cpuInfo[4];
    __cpuid(cpuInfo, 7);
    eax = cpuInfo[0];
    ebx = cpuInfo[1];
    ecx = cpuInfo[2];
    edx = cpuInfo[3];
#else
    asm volatile("cpuid\n"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(7), "c"(0)
                 : "cc");
#endif
    // avx512f  ---> 16 ebx
    if (!(bit(ebx, 16))) {
        return false;
    }

    // check os support
    asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));

    return (eax & 6) == 6;
}

bool feature_detect_vnni() {
    uint32_t eax, ebx, ecx, edx;

    // check cpu support
#if defined(_WIN32)
    int cpuInfo[4];
    __cpuid(cpuInfo, 7);
    eax = cpuInfo[0];
    ebx = cpuInfo[1];
    ecx = cpuInfo[2];
    edx = cpuInfo[3];
#else
    asm volatile("cpuid\n"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(7), "c"(0)
                 : "cc");
#endif
    // avx512f  ---> 16 ebx
    // avx512dq ---> 17 ebx
    // avx512bw ---> 30 ebx
    // avx512vl ---> 31 ebx
    // avx512vnni --->11 ecx
    if (!(bit(ebx, 16) && bit(ebx, 17) && bit(ebx, 30) && bit(ebx, 31) &&
          bit(ecx, 11))) {
        return false;
    }
    // check os support
    asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return (eax & 6) == 6;
}

bool feature_detect_avx_fma(int ftr) {
    // see Detecting Availability and Support in
    // https://software.intel.com/en-us/articles/introduction-to-intel-advanced-vector-extensions

    // check CPU support
    if (!(bit(cpuid.ecx, 27) && bit(cpuid.ecx, ftr)))
        return false;

    // check OS support
    uint32_t edx, eax;
    asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));

    return (eax & 6) == 6;
}

bool is_avx_supported = feature_detect_avx_fma(28);
bool is_fma_supported = feature_detect_avx_fma(12);
bool is_avx2_supported = feature_detect_avx2();
bool is_avx512_supported = feature_detect_avx512();
bool is_vnni_supported = feature_detect_vnni();

SIMDType disabled_simd_type_thresh = SIMDType::__NR_SIMD_TYPE;
}  // namespace

bool megpeak::is_supported(SIMDType type) {
    if (type >= disabled_simd_type_thresh)
        return false;

    switch (type) {
        case SIMDType::SSE:
            return bit(cpuid.edx, 25);
        case SIMDType::SSE2:
            return bit(cpuid.edx, 26);
        case SIMDType::SSE3:
            return bit(cpuid.ecx, 0);
        case SIMDType::SSE4_1:
            return bit(cpuid.ecx, 19);
        case SIMDType::SSE4_2:
            return bit(cpuid.ecx, 20);
        case SIMDType::AVX:
            return is_avx_supported;
        case SIMDType::FMA:
            return is_fma_supported;
        case SIMDType::AVX2:
            return is_avx2_supported;
        case SIMDType::AVX512:
            return is_avx512_supported;
        case SIMDType::VNNI:
            return is_vnni_supported;
        default:
            break;
    }
    printf("unknown cpu feature.\n");
    return false;
}
#endif

// vim: syntax=cpp.doxygen
