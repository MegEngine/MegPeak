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

#if MEGPEAK_AARCH64
#include <arm_neon.h>
#define eor(i) "eor v" #i ".16b, v" #i ".16b, v" #i ".16b\n"

#define THROUGHPUT(cb, func)                                                \
    static int func##_throughput() {                                        \
        asm volatile(                                                       \
        UNROLL_CALL(20, eor)                                                \
        "mov x0, %x[RUNS]\n"                                                \
        "1:\n"                                                              \
        UNROLL_CALL(20, cb)                                                 \
        "subs  x0, x0, #1 \n"                                               \
        "bne 1b \n"                                                         \
        :                                                                   \
        : [RUNS] "r"(megpeak::RUNS)                                         \
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", \
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    \
          "v19", "x0");                                                     \
        return megpeak::RUNS * 20;                                          \
    }

#define LATENCY(cb, func)              \
    static int func##_latency() {      \
        asm volatile(                  \
        "eor v0.16b, v0.16b, v0.16b\n" \
        "mov x0, #0\n"                 \
        "1:\n"                         \
        UNROLL_CALL(20, cb)            \
        "add  x0, x0, #1 \n"           \
        "cmp x0, %x[RUNS] \n"          \
        "blt 1b \n"                    \
        :                              \
        : [RUNS] "r"(megpeak::RUNS)    \
        : "cc", "v0", "x0");           \
        return megpeak::RUNS * 20;     \
    }

#define cb(i) "fmla v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fmla)
#undef cb
#define cb(i) "fmla v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, fmla)
#undef cb

#define cb(i) "fadd v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fadd)
#undef cb
#define cb(i) "fadd v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, fadd)
#undef cb

#define cb(i) "addp v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, addp)
#undef cb
#define cb(i) "addp v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, addp)
#undef cb

#define cb(i) "fmul v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fmul)
#undef cb
#define cb(i) "fmul v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, fmul)
#undef cb

#define cb(i) "mul v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, mul)
#undef cb
#define cb(i) "mul v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, mul)
#undef cb

#define cb(i) "add v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, add)
#undef cb
#define cb(i) "add v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, add)
#undef cb

#define cb(i) "usubl v" #i ".4s, v" #i ".4h, v" #i ".4h\n"
THROUGHPUT(cb, usubl)
#undef cb
#define cb(i) "usubl v0.4s, v0.4h, v0.4h\n"
LATENCY(cb, usubl)
#undef cb

#define cb(i) "abs v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, abs)
#undef cb
#define cb(i) "abs v0.4s, v0.4s\n"
LATENCY(cb, abs)
#undef cb

#define cb(i) "fcvtzs v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fcvtzs)
#undef cb
#define cb(i) "fcvtzs v0.4s, v0.4s\n"
LATENCY(cb, fcvtzs)
#undef cb

#define cb(i) "fcvtns v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fcvtns)
#undef cb
#define cb(i) "fcvtns v0.4s, v0.4s\n"
LATENCY(cb, fcvtns)
#undef cb

#define cb(i) "fcvtms v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fcvtms)
#undef cb
#define cb(i) "fcvtms v0.4s, v0.4s\n"
LATENCY(cb, fcvtms)
#undef cb

#define cb(i) "fcvtps v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fcvtps)
#undef cb
#define cb(i) "fcvtps v0.4s, v0.4s\n"
LATENCY(cb, fcvtps)
#undef cb

#define cb(i) "fcvtas v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fcvtas)
#undef cb
#define cb(i) "fcvtas v0.4s, v0.4s\n"
LATENCY(cb, fcvtas)
#undef cb

#define cb(i) "fcvtn v" #i ".4h, v" #i ".4s\n"
THROUGHPUT(cb, fcvtn)
#undef cb
#define cb(i) "fcvtn v0.4h, v0.4s\n"
LATENCY(cb, fcvtn)
#undef cb

#define cb(i) "fcvtl v" #i ".4s, v" #i ".4h\n"
THROUGHPUT(cb, fcvtl)
#undef cb
#define cb(i) "fcvtl v0.4s, v0.4h\n"
LATENCY(cb, fcvtl)
#undef cb

#define cb(i) "scvtf v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, scvtf)
#undef cb
#define cb(i) "scvtf v0.4s, v0.4s\n"
LATENCY(cb, scvtf)
#undef cb

#define cb(i) "sqrdmulh v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, sqrdmulh)
#undef cb
#define cb(i) "sqrdmulh v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, sqrdmulh)
#undef cb

#define cb(i) "tbl v" #i ".8b, {v" #i ".16b}, v" #i ".8b\n"
THROUGHPUT(cb, tbl)
#undef cb
#define cb(i) "tbl v0.8b, {v0.16b}, v0.8b\n"
LATENCY(cb, tbl)
#undef cb

#define cb(i) "smlal v" #i ".4s, v" #i ".4h, v" #i ".4h\n"
THROUGHPUT(cb, smlal)
#undef cb
#define cb(i) "smlal v0.4s, v0.4h, v0.4h\n"
LATENCY(cb, smlal)
#undef cb

#define cb(i) "smull v" #i ".4s, v" #i ".4h, v" #i ".4h\n"
THROUGHPUT(cb, smull)
#undef cb
#define cb(i) "smull v0.4s, v0.4h, v0.4h\n"
LATENCY(cb, smull)
#undef cb

#define cb(i) "sadalp v" #i ".4s, v" #i ".8h\n"
THROUGHPUT(cb, sadalp)
#undef cb
#define cb(i) "sadalp v0.4s, v0.8h\n"
LATENCY(cb, sadalp)
#undef cb

#define cb(i) "mla v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, mla)
#undef cb
#define cb(i) "mla v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, mla)
#undef cb

#if __ARM_FEATURE_DOTPROD

#define cb(i) "sdot v" #i ".4s, v" #i ".16b, v" #i ".16b\n"
THROUGHPUT(cb, sdot)
#undef cb
#define cb(i) "sdot v0.4s, v0.16b, v0.16b\n"
LATENCY(cb, sdot)
#undef cb

#endif

#if defined(__ARM_FEATURE_MATMUL_INT8)
#define cb(i) "smmla  v" #i ".4s, v" #i ".16b, v" #i ".16b\n"
THROUGHPUT(cb, smmla)
#undef cb
#define cb(i) "smmla v0.4s, v0.16b, v0.16b\n"
LATENCY(cb, smmla)
#undef cb
#endif

void megpeak::aarch64() {
    //! warmup
    for (size_t i = 0; i < 100; i++) {
        fmla_throughput();
    }
    benchmark(fmla_throughput, fmla_latency, "fmla", 8);
    benchmark(mla_throughput, mla_latency, "mla", 8);
    benchmark(fmul_throughput, fmul_latency, "fmul");
    benchmark(mul_throughput, mul_latency, "mul");
#if defined(__ARM_FEATURE_MATMUL_INT8)
    benchmark(smmla_throughput, smmla_latency, "smmla", 64);
#endif
    benchmark(addp_throughput, addp_latency, "addp");
#if __ARM_FEATURE_DOTPROD
    benchmark(sdot_throughput, sdot_latency, "sdot", 32);
#endif
    benchmark(sadalp_throughput, sadalp_latency, "sadalp");
    benchmark(add_throughput, add_latency, "add");
    benchmark(fadd_throughput, fadd_latency, "fadd");
    benchmark(smull_throughput, smull_latency, "smull");
    benchmark(smlal_throughput, smlal_latency, "smlal", 8);
    benchmark(tbl_throughput, tbl_latency, "tbl", 16);
    benchmark(sqrdmulh_throughput, sqrdmulh_latency, "sqrdmulh");
    benchmark(usubl_throughput, usubl_latency, "usubl");
    benchmark(abs_throughput, abs_latency, "abs");
    benchmark(fcvtzs_throughput, fcvtzs_latency, "fcvtzs");
    benchmark(scvtf_throughput, scvtf_latency, "scvtf");
    benchmark(fcvtns_throughput, fcvtns_latency, "fcvtns");
    benchmark(fcvtms_throughput, fcvtms_latency, "fcvtms");
    benchmark(fcvtps_throughput, fcvtps_latency, "fcvtps");
    benchmark(fcvtas_throughput, fcvtas_latency, "fcvtas");
    benchmark(fcvtn_throughput, fcvtn_latency, "fcvtn");
    benchmark(fcvtl_throughput, fcvtl_latency, "fcvtl");
}
#else
void megpeak::aarch64() {}
#endif

// vim: syntax=cpp.doxygen
