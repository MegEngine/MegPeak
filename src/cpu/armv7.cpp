/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "src/cpu/armv7.h"
#include "src/cpu/common.h"

#if MEGPEAK_ARMV7

#define BENCHMARK(name, computation) \
    benchmark(name##_throughput, name##_latency, #name, computation)
void megpeak::armv7() {
    //! warmup
    for (size_t i = 0; i < 100; i++) {
        mla_s32_throughput();
        mla_f32_throughput();
    }
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(nop_throughput, nop_latency, "nop", 1);
    benchmark(eor_d_throughput, eor_d_latency, "eor_d", 2);
    benchmark(eor_q_throughput, eor_q_latency, "eor_q", 4);

    benchmark(ld1_d_throughput, ld1_d_latency, "ld1_d", 2);
    BENCHMARK(vld1d_x2, 4);
    benchmark(ld1_q_throughput, ld1_q_latency, "ld1_q", 4);
    BENCHMARK(add_imm, 1);
    BENCHMARK(add_reg, 1);
    BENCHMARK(pld, 4);
    benchmark(st1_d_throughput, st1_d_latency, "st1_d", 2);
    benchmark(ldst1_d_throughput, ldst1_d_latency, "ldst1_d", 2);
#endif

    benchmark(padal_throughput, padal_latency, "padal");
    benchmark(padd_throughput, padd_latency, "padd");
    benchmark(mla_s32_throughput, mla_s32_latency, "mla_s32", 8);

#if MEGPEAK_WITH_ALL_BENCHMARK
    BENCHMARK(vmovl_s8, 8);
    BENCHMARK(mla_s16, 16);
    BENCHMARK(mla_s16_lane, 16);
    BENCHMARK(mla_s16_lane_d, 8);
    BENCHMARK(ldrd_mla_s16_lane, 16);
    BENCHMARK(ldrd_mla_s16_lane_1_4, 16);

    benchmark(ld1d_mlad_s16_throughput, ld1d_mlad_s16_latency, "ld1d_mlad_s16",
              8);
#endif

    benchmark(mlal_s8_throughput, mlal_s8_latency, "mlal_s8", 16);
    benchmark(mlal_s16_throughput, mlal_s16_latency, "mlal_s16", 8);

    benchmark(mlal_s16_lane_throughput, mlal_s16_lane_latency, "mlal_s16_lane",
              8);

    benchmark(mla_f32_throughput, mla_f32_latency, "mla_f32", 8);
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(mla_f32_d_throughput, mla_f32_d_latency, "mla_f32_d", 4);
    BENCHMARK(mla_f32d_lane, 4);
    BENCHMARK(mla_f32q_lane, 8);
    BENCHMARK(ld1d_fmlad, 4);
    BENCHMARK(ld1d_fmlaq, 8);
    BENCHMARK(ld1dx2_fmlad_lane, 4);
    BENCHMARK(ld1dx2_fmlaq_lane, 8);
#endif

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
