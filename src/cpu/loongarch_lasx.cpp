/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2023 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 */

#include "src/cpu/common.h"
#include "src/cpu/loongarch.h"
#include "src/cpu/loongarch_utils.h"

#if MEGPEAK_LOONGARCH
void megpeak::loongarch_lasx() {
    if (is_supported(SIMDType::LASX)) {
        benchmark(xvld_throughput, xvld_latency, "xvld", 8);
        benchmark(xvst_throughput, xvst_latency, "xvst", 8);
        benchmark(xvldx_throughput, xvldx_latency, "xvldx", 8);
        benchmark(xvstx_throughput, xvstx_latency, "xvstx", 8);
        benchmark(xvldrepl_b_throughput, xvldrepl_b_latency, "xvldrepl.b", 8);
        benchmark(xvldrepl_h_throughput, xvldrepl_h_latency, "xvldrepl.h", 8);
        benchmark(xvldrepl_w_throughput, xvldrepl_w_latency, "xvldrepl.w", 8);
        benchmark(xvldrepl_d_throughput, xvldrepl_d_latency, "xvldrepl.d", 8);
        benchmark(xvstelm_b_throughput, xvstelm_b_latency, "xvstelm.b", 8);
        benchmark(xvstelm_h_throughput, xvstelm_h_latency, "xvstelm.h", 8);
        benchmark(xvstelm_w_throughput, xvstelm_w_latency, "xvstelm.w", 8);
        benchmark(xvstelm_d_throughput, xvstelm_d_latency, "xvstelm.d", 8);
        benchmark(xvfadd_s_throughput, xvfadd_s_latency, "xvfadd.s", 8);
        benchmark(xvfadd_d_throughput, xvfadd_d_latency, "xvfadd.d", 4);
        benchmark(xvfsub_s_throughput, xvfsub_s_latency, "xvfsub.s", 8);
        benchmark(xvfsub_d_throughput, xvfsub_d_latency, "xvfsub.d", 4);
        benchmark(xvfadd_s_throughput, xvfadd_s_latency, "xvfadd.s", 8);
        benchmark(xvfadd_d_throughput, xvfadd_d_latency, "xvfadd.d", 4);
        benchmark(xvfmul_s_throughput, xvfmul_s_latency, "xvfmul.s", 8);
        benchmark(xvfmul_d_throughput, xvfmul_d_latency, "xvfmul.d", 4);
        benchmark(xvfdiv_s_throughput, xvfdiv_s_latency, "xvfdiv.s", 8);
        benchmark(xvfdiv_d_throughput, xvfdiv_d_latency, "xvfdiv.d", 4);
        benchmark(xvfmadd_s_throughput, xvfmadd_s_latency, "xvfmadd.s", 8 * 2);
        benchmark(xvfmadd_d_throughput, xvfmadd_d_latency, "xvfmadd.d", 4 * 2);
        benchmark(xvfmsub_s_throughput, xvfmsub_s_latency, "xvfmsub.s", 8 * 2);
        benchmark(xvfmsub_d_throughput, xvfmsub_d_latency, "xvfmsub.d", 4 * 2);
        benchmark(xvfnmadd_s_throughput, xvfnmadd_s_latency, "xvfnmadd.s", 8 * 3);
        benchmark(xvfnmadd_d_throughput, xvfnmadd_d_latency, "xvfnmadd.d", 4 * 3);
        benchmark(xvfnmsub_s_throughput, xvfnmsub_s_latency, "xvfnmsub.s", 8 * 3);
        benchmark(xvfnmsub_d_throughput, xvfnmsub_d_latency, "xvfnmsub.d", 4 * 3);
        benchmark(xvfmax_s_throughput, xvfmax_s_latency, "xvfmax.s", 8);
        benchmark(xvfmax_d_throughput, xvfmax_d_latency, "xvfmax.d", 4);
        benchmark(xvfmin_s_throughput, xvfmin_s_latency, "xvfmin.s", 8);
        benchmark(xvfmin_d_throughput, xvfmin_d_latency, "xvfmin.d", 4);
        benchmark(xvfmaxa_s_throughput, xvfmaxa_s_latency, "xvfmaxa.s", 8);
        benchmark(xvfmaxa_d_throughput, xvfmaxa_d_latency, "xvfmaxa.d", 4);
        benchmark(xvfmina_s_throughput, xvfmina_s_latency, "xvfmina.s", 8);
        benchmark(xvfmina_d_throughput, xvfmina_d_latency, "xvfmina.d", 4);
        benchmark(xvflogb_s_throughput, xvflogb_s_latency, "xvflogb.s", 8);
        benchmark(xvflogb_d_throughput, xvflogb_d_latency, "xvflogb.d", 4);
        benchmark(xvfclass_s_throughput, xvfclass_s_latency, "xvfclass.s", 8);
        benchmark(xvfclass_d_throughput, xvfclass_d_latency, "xvfclass.d", 4);
        benchmark(xvfsqrt_s_throughput, xvfsqrt_s_latency, "xvfsqrt.s", 8);
        benchmark(xvfsqrt_d_throughput, xvfsqrt_d_latency, "xvfsqrt.d", 4);
        benchmark(xvfrecip_s_throughput, xvfrecip_s_latency, "xvfrecip.s", 8);
        benchmark(xvfrecip_d_throughput, xvfrecip_d_latency, "xvfrecip.d", 4);
        benchmark(xvfrsqrt_s_throughput, xvfrsqrt_s_latency, "xvfrsqrt.s", 8 * 2);
        benchmark(xvfrsqrt_d_throughput, xvfrsqrt_d_latency, "xvfrsqrt.d", 4 * 2);
    }
}
#else
void megpeak::loongarch_lasx() {}
#endif
