/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied.
 */

#include "src/cpu/aarch64.h"
#include "src/cpu/aarch64_dual_issue.h"
#include "src/cpu/common.h"

#if MEGPEAK_AARCH64
// clang-format on

void megpeak::aarch64() {
  //! warmup
  for (size_t i = 0; i < 100; i++) {
    fmla_throughput();
  }
#if MEGPEAK_WITH_ALL_BENCHMARK
  benchmark(nop_throughput, nop_latency, "nop", 1);
#endif
  benchmark(ldd_throughput, ldd_latency, "ldd", 2);
  benchmark(ldq_throughput, ldq_latency, "ldq", 4);
  benchmark(stq_throughput, stq_latency, "stq", 4);
  benchmark(ldpq_throughput, ldpq_latency, "ldpq", 8);
  benchmark(lddx2_throughput, lddx2_latency, "lddx2", 4);
  benchmark(ld1q_throughput, ld1q_latency, "ld1q", 4);

  benchmark(eor_throughput, eor_latency, "eor", 4);
  benchmark(fmla_throughput, fmla_latency, "fmla", 8);
  benchmark(fmlad_throughput, fmlad_latency, "fmlad", 4);
  benchmark(fmla_x2_throughput, fmla_x2_latency, "fmla_x2", 16);
  benchmark(mla_throughput, mla_latency, "mla", 8);
  benchmark(fmul_throughput, fmul_latency, "fmul");
  benchmark(mul_throughput, mul_latency, "mul");
  benchmark(addp_throughput, addp_latency, "addp");

#if __ARM_FEATURE_DOTPROD
  benchmark(sdot_throughput, sdot_latency, "sdot", 32);
#endif
  benchmark(sadalp_throughput, sadalp_latency, "sadalp");
  benchmark(add_throughput, add_latency, "add");
  benchmark(fadd_throughput, fadd_latency, "fadd");
  benchmark(smull_throughput, smull_latency, "smull");
  benchmark(smlal_throughput, smlal_latency, "smlal_4b", 8);

  benchmark(smlal_8b_throughput, smlal_8b_latency, "smlal_8b", 16);

  //! for 8x8x16
  benchmark(dupd_lane_s8_throughput, dupd_lane_s8_latency, "dupd_lane_s8", 8);
  benchmark(mlaq_lane_s16_throughput, mlaq_lane_s16_latency, "mlaq_lane_s16",
            16);
  benchmark(sshll_throughput, sshll_latency, "sshll", 8);

  benchmark(tbl_throughput, tbl_latency, "tbl", 16);
  benchmark(ins_throughput, ins_latency, "ins", 2);

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
#if MEGPEAK_WITH_ALL_BENCHMARK
  benchmark(prefetch_very_long_throughput, prefetch_very_long_latency,
            "prefetch_very_long");
#endif

  aarch64_dual_issue();
}
#else
void megpeak::aarch64() {}
#endif

// vim: syntax=cpp.doxygen
