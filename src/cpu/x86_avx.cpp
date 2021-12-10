/**
 * \file src/cpu/x86_avx.cpp
 *
 * \brief This is benchmark for MegPeak.
 *
 * \copyright Copyright (c) 2019-2019 Megvii Inc. All rights reserved.
 */

#include "src/cpu/common.h"
#include "src/cpu/x86_utils.h"

#if MEGPEAK_X86

#if __GNUC__ >= 9
#define __AVX512VNNI__ 1
#endif

#define eor(i) "vxorps %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
// clang-format off
#define THROUGHPUT(cb, func, simd)                                 \
    MEGPEAK_ATTRIBUTE_TARGET(simd)                                 \
    static int func##_throughput() {                               \
        asm volatile(                                              \
        UNROLL_CALL(10, eor)                                       \
        "movl %[RUNS], %%eax \n"                                   \
        "1:\n"                                                     \
        UNROLL_CALL(10, cb)                                        \
        "sub  $0x01, %%eax\n"                                      \
        "jne 1b \n"                                                \
        :                                                          \
        :[RUNS] "r"(megpeak::RUNS * 100)                           \
        : "%ymm0", "%ymm1", "%ymm2", "%ymm3", "%ymm4", "%ymm5",    \
           "%ymm6", "%ymm7", "%ymm8", "%ymm9", "%eax", "cc");      \
        return megpeak::RUNS * 100 * 10;                           \
    }

#define LATENCY(cb, func, simd)          \
    MEGPEAK_ATTRIBUTE_TARGET(simd)       \
    static int func##_latency() {        \
        asm volatile(                    \
        "vxorpd %%ymm0, %%ymm0, %%ymm0\n"\
        "movl %[RUNS], %%eax \n"         \
        "1:\n"                           \
        UNROLL_CALL(10, cb)              \
        "sub  $0x01, %%eax\n"            \
        "jne 1b \n"                      \
        :                                \
        :[RUNS] "r"(megpeak::RUNS * 100) \
        : "%ymm0", "%eax", "cc");        \
        return megpeak::RUNS * 100 * 10; \
    }
// clang-format on
#define cb(i) "vfmadd132ps %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vfmadd132ps, "avx2")
#undef cb
#define cb(i) "vfmadd132ps %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vfmadd132ps, "avx2")
#undef cb

#define cb(i) "vfmadd132pd %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vfmadd132pd, "avx2")
#undef cb
#define cb(i) "vfmadd132pd %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vfmadd132pd, "avx")
#undef cb

#define cb(i) "vpmaddwd %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vpmaddwd, "avx2")
#undef cb
#define cb(i) "vpmaddwd %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vpmaddwd, "avx2")
#undef cb

#define cb(i) "vpaddd %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vpaddd, "avx2")
#undef cb
#define cb(i) "vpaddd %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vpaddd, "avx2")
#undef cb

#define cb(i) "vpacksswb %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vpacksswb, "avx2")
#undef cb
#define cb(i) "vpacksswb %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vpacksswb, "avx2")
#undef cb

#define cb(i) "vpackssdw %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vpackssdw, "avx2")
#undef cb
#define cb(i) "vpackssdw %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vpackssdw, "avx2")
#undef cb

#define cb(i) "vpand %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vpand, "avx2")
#undef cb
#define cb(i) "vpand %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vpand, "avx2")
#undef cb
// clang-format off
#define cb(i)                                           \
    "vpmaddwd %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"  \
    "vpaddd %%ymm" #i ", %%ymm" #i ", %%ymm" #i "\n"
THROUGHPUT(cb, vpmaddwd_vpaddd, "avx2")
#undef cb
// clang-format on
#define cb(i)                           \
    "vpmaddwd %%ymm0, %%ymm0, %%ymm0\n" \
    "vpaddd %%ymm0, %%ymm0, %%ymm0\n"
LATENCY(cb, vpmaddwd_vpaddd, "avx2")
#undef cb

#define cb(i) "vpmaddwd %%zmm" #i ", %%zmm" #i ", %%zmm" #i "\n"
THROUGHPUT(cb, vpmaddwd_512, "avx512bw")
#undef cb
#define cb(i) "vpmaddwd %%zmm0, %%zmm0, %%zmm0\n"
LATENCY(cb, vpmaddwd_512, "avx512bw")
#undef cb

#define cb(i) "vpaddd %%zmm" #i ", %%zmm" #i ", %%zmm" #i "\n"
THROUGHPUT(cb, vpaddd_512, "avx512bw")
#undef cb
#define cb(i) "vpaddd %%zmm0, %%zmm0, %%zmm0\n"
LATENCY(cb, vpaddd_512, "avx512f")
#undef cb

#define cb(i) "vfmadd132ps %%zmm" #i ", %%zmm" #i ", %%zmm" #i "\n"
THROUGHPUT(cb, vfmadd132ps_512, "avx512f")
#undef cb
#define cb(i) "vfmadd132ps %%zmm0, %%zmm0, %%zmm0\n"
LATENCY(cb, vfmadd132ps_512, "avx512f")
#undef cb

#if __AVX512VNNI__

#define cb(i) "vpdpbusd %%zmm" #i ", %%zmm" #i ", %%zmm" #i "\n"
THROUGHPUT(cb, vpdpbusd, "avx512vnni")
#undef cb
#define cb(i) "vpdpbusd %%zmm0, %%zmm0, %%zmm0\n"
LATENCY(cb, vpdpbusd, "avx512vnni")
#undef cb

#endif

void megpeak::x86_avx() {
    if (is_supported(SIMDType::FMA) && is_supported(SIMDType::AVX)) {
        //! warmup
        for (size_t i = 0; i < 10; i++) {
            vfmadd132ps_throughput();
        }
        //! fma is 256 width, mul + add = 2 ops
        benchmark(vfmadd132ps_throughput, vfmadd132ps_latency,
                  "vfmadd132ps_avx", 8 * 2);
        benchmark(vfmadd132pd_throughput, vfmadd132pd_latency,
                  "vfmadd132pd_avx", 4 * 2);
    }
    if (is_supported(SIMDType::AVX2)) {
        //! warmup
        for (size_t i = 0; i < 10; i++) {
            vpmaddwd_throughput();
        }
        benchmark(vpmaddwd_throughput, vpmaddwd_latency, "vpmaddwd_avx2",
                  16 + 8);
        benchmark(vpaddd_throughput, vpaddd_latency, "vpaddd_avx2", 8);
        benchmark(vpand_throughput, vpand_latency, "vpand_avx2", 8);
        benchmark(vpmaddwd_vpaddd_throughput, vpmaddwd_vpaddd_latency,
                  "vpmaddwd_vpaddd_avx2", 16 + 8 + 8);

        benchmark(vpackssdw_throughput, vpackssdw_latency, "vpackssdw_avx2",
                  16);
        benchmark(vpacksswb_throughput, vpacksswb_latency, "vpacksswb_avx2",
                  32);
    }
    if (is_supported(SIMDType::AVX512)) {
        //! warmup
        for (size_t i = 0; i < 10; i++) {
            vpmaddwd_512_throughput();
        }
        benchmark(vpmaddwd_512_throughput, vpmaddwd_512_latency, "vpmaddwd_512",
                  32 + 16);
        benchmark(vpaddd_512_throughput, vpaddd_512_latency, "vpaddd_512", 16);
    }

    if (is_supported(SIMDType::AVX512)) {
        benchmark(vfmadd132ps_512_throughput, vfmadd132ps_512_latency,
                  "vfmadd132ps_512", 16 * 2);
    }
#if __AVX512VNNI__
    if (is_supported(SIMDType::VNNI)) {
        benchmark(vpdpbusd_throughput, vpdpbusd_latency, "vpdpbusd_vnni", 112);
    }
#endif
}
#else
void megpeak::x86_avx() {}
#endif

// vim: syntax=cpp.doxygen
