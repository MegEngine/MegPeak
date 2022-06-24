/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "src/cpu/common.h"

#if MEGPEAK_ARMV7
#include <arm_neon.h>

#define eor(i) "veor.32 q" #i ", q" #i ", q" #i "\n"

#define THROUGHPUT0(cb, func, start)                                        \
    static int func##_throughput() {                                        \
        asm volatile(                                                       \
        UNROLL_CALL##start(10, eor)                                         \
        "mov r0, #0\n"                                                      \
        "1:\n"                                                              \
        UNROLL_CALL##start(10, cb)                                          \
        "add  r0, r0, #1 \n"                                                \
        "cmp r0, %[RUNS] \n"                                                \
        "blt 1b \n"                                                         \
        :                                                                   \
        : [RUNS] "r"(megpeak::RUNS)                                         \
        : "cc", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", \
          "r0");                                                            \
        return megpeak::RUNS * 10;                                          \
    }

#define LATENCY(cb, func)              \
    static int func##_latency() {      \
        asm volatile(                  \
        "veor.32 q0, q0, q0\n"         \
        "mov r0, #0\n"                 \
        "1:\n"                         \
        UNROLL_CALL(10, cb)            \
        "add  r0, r0, #1 \n"           \
        "cmp r0, %[RUNS] \n"           \
        "blt 1b \n"                    \
        :                              \
        : [RUNS] "r"(megpeak::RUNS)    \
        : "cc", "q0", "r0");           \
        return megpeak::RUNS * 10;     \
    }

#define THROUGHPUT(cb, func) THROUGHPUT0(cb, func, )
#define THROUGHPUT_START6(cb, func) THROUGHPUT0(cb, func, _START6)

#define cb(i) "vpadd.s32 d" #i ", d" #i ", d" #i "\n"
THROUGHPUT(cb, padd)
#undef cb
#define cb(i) "vpadd.s32 d0, d0, d0\n"
LATENCY(cb, padd)
#undef cb

#define cb(i) "vpadal.s16 q" #i ", q" #i "\n"
THROUGHPUT(cb, padal)
#undef cb
#define cb(i) "vpadal.s16 q0, q0\n"
LATENCY(cb, padal)
#undef cb

#define cb(i) "vmla.s32 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT(cb, mla_s32)
#undef cb
#define cb(i) "vmla.s32 q0, q0, q0\n"
LATENCY(cb, mla_s32)
#undef cb

#define cb(i) "vmla.s16 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT(cb, mla_s16)
#undef cb
#define cb(i) "vmla.s16 q0, q0, q0\n"
LATENCY(cb, mla_s16)
#undef cb

#define cb(i) "vmlal.s8 q" #i ", d" #i ", d" #i "\n"
THROUGHPUT_START6(cb, mlal_s8)
#undef cb
#define cb(i) "vmlal.s8 q1, d0, d0\n"
LATENCY(cb, mlal_s8)
#undef cb

#define cb(i) "vmlal.s16 q" #i ", d" #i ", d" #i "\n"
THROUGHPUT_START6(cb, mlal_s16)
#undef cb
#define cb(i) "vmlal.s16 q1, d0, d0\n"
LATENCY(cb, mlal_s16)
#undef cb

#define cb(i) "vmla.s16 q" #i ", q1, d0[0]\n"
THROUGHPUT_START6(cb, mla_s16_lane)
#undef cb
#define cb(i) "vmla.s16 q1, q0, d0[0]\n"
LATENCY(cb, mla_s16_lane)
#undef cb

#define cb(i) "vmlal.s16 q" #i ", d" #i ", d0[0]\n"
THROUGHPUT_START6(cb, mlal_s16_lane)
#undef cb
#define cb(i) "vmlal.s16 q1, d0, d0[0]\n"
LATENCY(cb, mlal_s16_lane)
#undef cb

#define cb(i) "vmla.f32 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT(cb, mla_f32)
#undef cb
#define cb(i) "vmla.f32 q0, q0, q0\n"
LATENCY(cb, mla_f32)
#undef cb

#define cb(i) "vmul.s32 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT(cb, mul_s32)
#undef cb
#define cb(i) "vmul.s32 q0, q0, q0\n"
LATENCY(cb, mul_s32)
#undef cb

#define cb(i) "vmul.f32 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT(cb, mul_f32)
#undef cb
#define cb(i) "vmul.f32 q0, q0, q0\n"
LATENCY(cb, mul_f32)
#undef cb

#define cb(i) "vcvt.f32.s32 q" #i ", q" #i "\n"
THROUGHPUT(cb, cvt)
#undef cb
#define cb(i) "vcvt.f32.s32 q0, q0\n"
LATENCY(cb, cvt)
#undef cb

#define cb(i) "vqrdmulh.s32 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT(cb, qrdmulh)
#undef cb
#define cb(i) "vqrdmulh.s32 q0, q0, q0\n"
LATENCY(cb, qrdmulh)
#undef cb

#define cb(i) "vrshl.s32 q" #i ", q" #i "\n"
THROUGHPUT(cb, rshl)
#undef cb
#define cb(i) "vrshl.s32 q0, q0\n"
LATENCY(cb, rshl)
#undef cb

void megpeak::armv7() {
    //! warmup
    for (size_t i = 0; i < 100; i++) {
        mla_s32_throughput();
        mla_f32_throughput();
    }
    benchmark(padal_throughput, padal_latency, "padal");
    benchmark(padd_throughput, padd_latency, "padd");
    benchmark(mla_s32_throughput, mla_s32_latency, "mla_s32", 8);
    benchmark(mla_s16_throughput, mla_s16_latency, "mla_s16", 16);
    benchmark(mlal_s8_throughput, mlal_s8_latency, "mlal_s8", 16);
    benchmark(mlal_s16_throughput, mlal_s16_latency, "mlal_s16", 8);
    benchmark(mlal_s16_lane_throughput, mlal_s16_lane_latency, "mlal_s16_lane",
              8);
    benchmark(mla_s16_lane_throughput, mla_s16_lane_latency, "mla_s16_lane",
              16);
    benchmark(mla_f32_throughput, mla_f32_latency, "mla_f32", 8);
    benchmark(mul_s32_throughput, mul_s32_latency, "mul_s32");
    benchmark(mul_f32_throughput, mul_f32_latency, "mul_f32");
    benchmark(cvt_throughput, cvt_latency, "cvt");
    benchmark(qrdmulh_throughput, qrdmulh_latency, "qrdmulh");
    benchmark(rshl_throughput, rshl_latency, "rshl");
}
#else
void megpeak::armv7() {}
#endif

// vim: syntax=cpp.doxygen
