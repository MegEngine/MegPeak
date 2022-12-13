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
#pragma once
#include <malloc.h>
#include "src/cpu/common.h"
#include "stdlib.h"

#if MEGPEAK_AARCH64
#include "src/cpu/aarch64_dual_issue.h"
#include <arm_neon.h>

#define cb(i) "ldr d" #i ", [%[a_ptr]]\n"
THROUGHPUT_MEM(cb, ldd)
#undef cb
#define cb(i) "ldr d0, [%[a_ptr]]\n"
LATENCY_MEM(cb, ldd)
#undef cb

#define cb(i) "ld1 {v0.2s, v1.2s}, [%[a_ptr]]\n"
THROUGHPUT_MEM(cb, lddx2)
#undef cb
#define cb(i) "ld1 {v0.2s, v1.2s}, [%[a_ptr]]\n"
LATENCY_MEM(cb, lddx2)
#undef cb

#define cb(i) "ld1 {v0.4s}, [%[a_ptr]]\n"
THROUGHPUT_MEM(cb, ld1q)
#undef cb
#define cb(i) "ld1 {v0.4s}, [%[a_ptr]]\n"
LATENCY_MEM(cb, ld1q)
#undef cb

#define cb(i) "nop\n"
THROUGHPUT_MEM(cb, nop)
#undef cb
#define cb(i) "nop\n"
LATENCY_MEM(cb, nop)
#undef cb

#define cb(i) "ldr q" #i ", [%[a_ptr]]\n"
THROUGHPUT_MEM(cb, ldq)
#undef cb
#define cb(i) "ldr q0, [%[a_ptr]]\n"
LATENCY_MEM(cb, ldq)
#undef cb

#define cb(i) "str q" #i ", [%[a_ptr]]\n"
THROUGHPUT_MEM(cb, stq)
#undef cb
#define cb(i) "str q0, [%[a_ptr]]\n"
LATENCY_MEM(cb, stq)
#undef cb

#define cb(i) "dup v" #i ".8b, v21.b[0]\n"
THROUGHPUT_MEM(cb, dupd_lane_s8)
#undef cb
#define cb(i) "dup v0.8b, v0.b[0]\n"
LATENCY_MEM(cb, dupd_lane_s8)
#undef cb

#define cb(i) "mla v" #i ".8h, v" #i ".8h, v15.h[0]\n"
THROUGHPUT_MEM_ITER(cb, mlaq_lane_s16, 15)
#undef cb
#define cb(i) "mla v0.8h, v0.8h, v0.h[0]\n"
LATENCY_MEM(cb, mlaq_lane_s16)
#undef cb

static int ldpq_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"                                                          
        "ldp q0, q1, [%[a_ptr]]\n"
        "ldp q2, q3, [%[a_ptr]]\n"
        "ldp q4, q5, [%[a_ptr]]\n"
        "ldp q6, q7, [%[a_ptr]]\n"
        "ldp q8, q9, [%[a_ptr]]\n" 
        "ldp q10, q11, [%[a_ptr]]\n"
        "ldp q12, q13, [%[a_ptr]]\n"
        "ldp q14, q15, [%[a_ptr]]\n"
        "ldp q16, q17, [%[a_ptr]]\n"
        "ldp q18, q19, [%[a_ptr]]\n"
        "ldp q20, q21, [%[a_ptr]]\n"
        "ldp q22, q23, [%[a_ptr]]\n"
        "ldp q24, q25, [%[a_ptr]]\n"
        "ldp q26, q27, [%[a_ptr]]\n"
        "ldp q28, q29, [%[a_ptr]]\n"
        "ldp q30, q31, [%[a_ptr]]\n"
        "add  x0, x0, #1 \n"                                            
        "cmp x0, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                       
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0");
    return megpeak::RUNS * 16;
}

#define cb(i) "ldp q0, q1, [%[a_ptr]]\n"
LATENCY_MEM(cb, ldpq)
#undef cb

#define cb(i) "eor v" #i ".16b, v" #i ".16b, v" #i ".16b\n"
THROUGHPUT(cb, eor)
#undef cb
#define cb(i) "eor v0.16b, v0.16b, v0.16b\n"
LATENCY(cb, eor)
#undef cb

#define cb(i) "fmla v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fmla)
#undef cb
#define cb(i) "fmla v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, fmla)
#undef cb

#define cb(i) "fmla v" #i ".2s, v" #i ".2s, v" #i ".2s\n"
THROUGHPUT(cb, fmlad)
#undef cb
#define cb(i) "fmla v0.2s, v0.2s, v0.2s\n"
LATENCY(cb, fmlad)
#undef cb

// clang-format off
#define cb(i) "fmla v" #i ".4s, v" #i ".4s, v" #i ".4s\n"\
              "fmla v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT(cb, fmla_x2)
#undef cb
#define cb(i)                    \
    "fmla v0.4s, v0.4s, v0.4s\n" \
    "fmla v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, fmla_x2)
#undef cb
// clang-format on

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

#define cb(i) "ins v" #i ".d[0], x1\n"
THROUGHPUT_MEM(cb, ins)
#undef cb
#define cb(i) "ins v0.d[0], x0\n"
LATENCY_MEM(cb, ins)
#undef cb

#define cb(i) "smlal v" #i ".4s, v" #i ".4h, v" #i ".4h\n"
THROUGHPUT(cb, smlal)
#undef cb
#define cb(i) "smlal v0.4s, v0.4h, v0.4h\n"
LATENCY(cb, smlal)
#undef cb

#define cb(i) "smlal v" #i ".8h, v" #i ".8b, v" #i ".8b\n"
THROUGHPUT(cb, smlal_8b)
#undef cb
#define cb(i) "smlal v0.8h, v0.8b, v0.8b\n"
LATENCY(cb, smlal_8b)
#undef cb
// clang-format off


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

#define cb(i) "sshll v" #i ".8h, v" #i ".8b, #0\n"
THROUGHPUT(cb, sshll)
#undef cb
#define cb(i) "sshll v0.8h, v0.8b, #0\n"
LATENCY(cb, sshll)
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

// clang-format off
static int prefetch_very_long_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "prfm pldl1keep, [%[a_ptr], #1024]\n"
        "prfm pldl1keep, [%[a_ptr], #2048]\n"        
        "prfm pldl1keep, [%[a_ptr], #3072]\n"        
        "prfm pldl1keep, [%[a_ptr], #4096]\n"        
        "prfm pldl1keep, [%[a_ptr], #8192]\n"        
        "prfm pldl1keep, [%[a_ptr], #12288]\n"        
        "prfm pldl1keep, [%[a_ptr], #16384]\n"        
        "prfm pldl1keep, [%[a_ptr], #20480]\n"        
        "prfm pldl1keep, [%[a_ptr], #24576]\n"        
        "prfm pldl1keep, [%[a_ptr], #28672]\n"        
        "prfm pldl1keep, [%[a_ptr], #32760]\n"                   
                       
        "add  x20, x20, #1 \n"                                            
        "cmp x20, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                        
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", 
          "x19", "x20");
    return megpeak::RUNS * 16;
}
#define cb(i) "prfm pldl1keep, [%[a_ptr], #1024]\n"
LATENCY_MEM(cb, prefetch_very_long)
#undef cb
#endif