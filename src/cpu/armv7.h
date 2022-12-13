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

#if MEGPEAK_ARMV7
#include <arm_neon.h>

#define eor(i) "veor.32 q" #i ", q" #i ", q" #i "\n"
// clang-format off
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
          "r0", "r1");                                                      \
        return megpeak::RUNS * 10;                                          \
    }

#define THROUGHPUT_MEM_N(cb, func, iter_number)                             \
    static int func##_throughput() {                                        \
        const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());   \
        asm volatile(                                                       \
        UNROLL_CALL(10, eor)                                                \
        "mov r10, #0\n"                                                     \
        "1:\n"                                                              \
        UNROLL_CALL(iter_number, cb)                                        \
        "add  r10, r10, #1 \n"                                              \
        "cmp r10, %[RUNS] \n"                                               \
        "blt 1b \n"                                                         \
        :                                                                   \
        : [RUNS] "r"(megpeak::RUNS),  [a_ptr] "r"(a_ptr)                    \
        : "cc", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", \
        "q10", "q11", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", \
        "r9", "r10",  "memory");                                            \
        return megpeak::RUNS * iter_number;                                 \
    }

#define THROUGHPUT_MEM(cb, func) THROUGHPUT_MEM_N(cb, func, 10)

#define THROUGHPUT_MEM_START6(cb, func)                                     \
    static int func##_throughput() {                                        \
        const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());   \
        asm volatile(                                                       \
        UNROLL_CALL_START6(10, eor)                                         \
        "mov r10, #0\n"                                                     \
        "1:\n"                                                              \
        UNROLL_CALL_START6(10, cb)                                          \
        "add  r10, r10, #1 \n"                                              \
        "cmp r10, %[RUNS] \n"                                               \
        "blt 1b \n"                                                         \
        :                                                                   \
        : [RUNS] "r"(megpeak::RUNS),  [a_ptr] "r"(a_ptr)                    \
        : "cc", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", \
        "q10", "q11", "q12", "q13", "q14", "q15", "r0", "r1", "r2", "r3",   \
        "r4", "r5", "r6", "r7", "r8", "r9", "r10",  "memory");              \
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

#define LATENCY_MEM(cb, func)                                             \
    static int func##_latency() {                                         \
        const float* a_ptr = reinterpret_cast<float*>(get_mem_align64()); \
        asm volatile(                                                     \
        "veor.32 q0, q0, q0\n"                                            \
        "mov r0, #0\n"                                                    \
        "1:\n"                                                            \
        UNROLL_CALL(10, cb)                                               \
        "add  r0, r0, #1 \n"                                              \
        "cmp r0, %[RUNS] \n"                                              \
        "blt 1b \n"                                                       \
        :                                                                 \
        : [RUNS] "r"(megpeak::RUNS),  [a_ptr] "r"(a_ptr)                  \
        : "cc", "q0", "r0", "r1");                                        \
        return megpeak::RUNS * 10;                                        \
    }
// clang-format on
#define THROUGHPUT(cb, func) THROUGHPUT0(cb, func, )
#define THROUGHPUT_START6(cb, func) THROUGHPUT0(cb, func, _START6)

#define cb(i) "nop\n"
THROUGHPUT(cb, nop)
#undef cb
#define cb(i) "nop\n"
LATENCY(cb, nop)
#undef cb

#define cb(i) "veor.f32 d" #i ", d" #i ", d" #i "\n"
THROUGHPUT(cb, eor_d)
#undef cb
#define cb(i) "veor.f32 d0, d0, d0\n"
LATENCY(cb, eor_d)
#undef cb

#define cb(i) "veor.f32 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT(cb, eor_q)
#undef cb
#define cb(i) "veor.f32 q0, q0, q0\n"
LATENCY(cb, eor_q)
#undef cb

#define cb(i) "add r" #i ", r" #i ", #1\n "
THROUGHPUT_MEM_N(cb, add_imm, 9)
#undef cb
#define cb(i) "add r1, r1, #1\n"
LATENCY_MEM(cb, add_imm)
#undef cb

#define cb(i) "add r" #i ", r" #i ", r" #i "\n "
THROUGHPUT_MEM_N(cb, add_reg, 9)
#undef cb
#define cb(i) "add r1, r1, r1\n"
LATENCY_MEM(cb, add_reg)
#undef cb

#define cb(i) "vld1.32 {d" #i "}, [%[a_ptr]] \n "
THROUGHPUT_MEM(cb, ld1_d)
#undef cb
#define cb(i) "vld1.32 {d0}, [%[a_ptr]]\n"
LATENCY_MEM(cb, ld1_d)
#undef cb

static int vld1d_x2_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    asm volatile(                                                       
        UNROLL_CALL(10, eor)                                        
        "mov r0, #0\n"                                              
        "1:\n"      
        "vld1.32 {d0, d1}, [%[a_ptr]]\n"
        "vld1.32 {d2, d3}, [%[a_ptr]]\n"
        "vld1.32 {d4, d5}, [%[a_ptr]]\n"
        "vld1.32 {d6, d7}, [%[a_ptr]]\n"
        "vld1.32 {d8, d9}, [%[a_ptr]]\n"
        "vld1.32 {d10, d11}, [%[a_ptr]]\n"
        "vld1.32 {d12, d13}, [%[a_ptr]]\n"
        "vld1.32 {d14, d15}, [%[a_ptr]]\n"
        "vld1.32 {d16, d17}, [%[a_ptr]]\n"
        "vld1.32 {d18, d19}, [%[a_ptr]]\n"
        "add  r0, r0, #1 \n"                                        
        "cmp r0, %[RUNS] \n"                                        
        "blt 1b \n"                                                 
        :                                                           
        : [RUNS] "r"(megpeak::RUNS),  [a_ptr] "r"(a_ptr)            
        : "cc", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9",
        "q10", "q11", "r0","r1", "memory");
    return megpeak::RUNS * 10;
}

#define cb(i) "vld1.32 {d0, d1}, [%[a_ptr]]\n"
LATENCY_MEM(cb, vld1d_x2)
#undef cb

#define cb(i) "pld [%[a_ptr]] \n "
THROUGHPUT_MEM(cb, pld)
#undef cb
#define cb(i) "pld [%[a_ptr]]\n"
LATENCY_MEM(cb, pld)
#undef cb

#define cb(i) "vst1.32 {d" #i "}, [%[a_ptr]] \n "
THROUGHPUT_MEM(cb, st1_d)
#undef cb
#define cb(i) "vst1.32 {d0}, [%[a_ptr]]\n"
LATENCY_MEM(cb, st1_d)
#undef cb

// clang-format off
#define cb(i)\
    "vld1.32 {d" #i "}, [%[a_ptr]] \n "\
    "vst1.32 {d" #i "}, [%[a_ptr]] \n "
THROUGHPUT_MEM(cb, ldst1_d)
#undef cb
#define cb(i)\
    "vld1.32 {d0}, [%[a_ptr]]\n"\
    "vst1.32 {d0}, [%[a_ptr]]\n"
LATENCY_MEM(cb, ldst1_d)
#undef cb
// clang-format on

#define cb(i) "vld1.32 {q" #i "}, [%[a_ptr]] \n "
THROUGHPUT_MEM(cb, ld1_q)
#undef cb
#define cb(i) "vld1.32 {q0}, [%[a_ptr]]\n"
LATENCY_MEM(cb, ld1_q)
#undef cb

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

#define cb(i) "vmlal.s8 q" #i ", d0, d0\n"
THROUGHPUT_START6(cb, mlal_s8)
#undef cb
#define cb(i) "vmlal.s8 q1, d0, d0\n"
LATENCY(cb, mlal_s8)
#undef cb

#define cb(i) "vmovl.s8 q" #i ", d0\n"
THROUGHPUT_START6(cb, vmovl_s8)
#undef cb
#define cb(i) "vmovl.s8 q0, d0\n"
LATENCY(cb, vmovl_s8)
#undef cb

#define cb(i) "vmlal.s16 q" #i ", d0, d0\n"
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

#define cb(i) "vmla.s16 d" #i ", d1, d0[0]\n"
THROUGHPUT_START6(cb, mla_s16_lane_d)
#undef cb
#define cb(i) "vmla.s16 d1, d0, d0[0]\n"
LATENCY(cb, mla_s16_lane_d)
#undef cb

static int ldrd_mla_s16_lane_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    asm volatile(                                                       
        UNROLL_CALL(10, eor)                                        
        "mov r0, #0\n"                                              
        "1:\n"      
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q2, q0, d0[0]\n"  
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q3, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q4, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q5, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q6, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q7, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q8, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q9, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q10, q0, d0[0]\n" 
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q11, q0, d0[0]\n" 
        "add  r0, r0, #1 \n"                                        
        "cmp r0, %[RUNS] \n"                                        
        "blt 1b \n"                                                 
        :                                                           
        : [RUNS] "r"(megpeak::RUNS),  [a_ptr] "r"(a_ptr)            
        : "cc", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9",
        "q10", "q11", "r0","r1", "memory");
    return megpeak::RUNS * 10;
}

#define cb(i)                    \
    "vld1.32 {d0}, [%[a_ptr]]\n" \
    "vmla.s16 q1, q0, d0[0]\n"
LATENCY_MEM(cb, ldrd_mla_s16_lane)
#undef cb

static int ldrd_mla_s16_lane_1_4_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    asm volatile(                                                       
        UNROLL_CALL(10, eor)                                        
        "mov r0, #0\n"                                              
        "1:\n"      
        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q2, q0, d0[0]\n"  
        "vmla.s16 q3, q0, d0[0]\n" 
        "vmla.s16 q4, q0, d0[0]\n" 
        "vmla.s16 q5, q0, d0[0]\n" 

        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q6, q0, d0[0]\n"         
        "vmla.s16 q7, q0, d0[0]\n"         
        "vmla.s16 q8, q0, d0[0]\n"         
        "vmla.s16 q9, q0, d0[0]\n" 

        "vld1.32 {d3}, [%[a_ptr]]\n"
        "vmla.s16 q10, q0, d0[0]\n" 
        "vmla.s16 q11, q0, d0[0]\n"
        "vmla.s16 q12, q0, d0[0]\n"
        "vmla.s16 q13, q0, d0[0]\n"

        "add  r0, r0, #1 \n"                                        
        "cmp r0, %[RUNS] \n"                                        
        "blt 1b \n"                                                 
        :                                                           
        : [RUNS] "r"(megpeak::RUNS),  [a_ptr] "r"(a_ptr)            
        : "cc", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9",
        "q10", "q11", "q12", "q13", "r0","r1", "memory");
    return megpeak::RUNS * 12;
}

#define cb(i)                    \
    "vld1.32 {d0}, [%[a_ptr]]\n" \
    "vmla.s16 q1, q0, d0[0]\n"
LATENCY_MEM(cb, ldrd_mla_s16_lane_1_4)
#undef cb

#define cb(i) "vmlal.s16 q" #i ", d" #i ", d0[0]\n"
THROUGHPUT_START6(cb, mlal_s16_lane)
#undef cb
#define cb(i) "vmlal.s16 q1, d0, d0[0]\n"
LATENCY(cb, mlal_s16_lane)
#undef cb

#define cb(i)                       \
    "vld1.32 {d22}, [%[a_ptr]] \n " \
    "vmla.f32 d" #i ", d" #i ", d" #i "\n"
THROUGHPUT_MEM(cb, ld1d_fmlad)
#undef cb
#define cb(i)                    \
    "vld1.32 {d0}, [%[a_ptr]]\n" \
    "vmla.f32 d0, d0, d0\n"
LATENCY_MEM(cb, ld1d_fmlad)
#undef cb

#define cb(i)                       \
    "vld1.32 {d22}, [%[a_ptr]] \n " \
    "vmla.f32 q" #i ", q" #i ", q" #i "\n"
THROUGHPUT_MEM(cb, ld1d_fmlaq)
#undef cb
#define cb(i)                    \
    "vld1.32 {d0}, [%[a_ptr]]\n" \
    "vmla.f32 q0, q0, q0\n"
LATENCY_MEM(cb, ld1d_fmlaq)
#undef cb

#define cb(i)                          \
    "vld1.32 {d2, d3}, [%[a_ptr]] \n " \
    "vmla.f32 q" #i ", q" #i ", d0[0]\n"
THROUGHPUT_MEM_START6(cb, ld1dx2_fmlaq_lane)
#undef cb
#define cb(i)                        \
    "vld1.32 {d0, d1}, [%[a_ptr]]\n" \
    "vmla.f32 q0, q0, d0[0]\n"
LATENCY_MEM(cb, ld1dx2_fmlaq_lane)
#undef cb

#define cb(i)                          \
    "vld1.32 {d2, d3}, [%[a_ptr]] \n " \
    "vmla.f32 d" #i ", d" #i ", d0[0]\n"
THROUGHPUT_MEM_START6(cb, ld1dx2_fmlad_lane)
#undef cb
#define cb(i)                        \
    "vld1.32 {d0, d1}, [%[a_ptr]]\n" \
    "vmla.f32 d0, d0, d0[0]\n"
LATENCY_MEM(cb, ld1dx2_fmlad_lane)
#undef cb

#define cb(i)                       \
    "vld1.32 {d10}, [%[a_ptr]] \n " \
    "vmla.s16 d" #i ", d" #i ", d" #i "\n"
THROUGHPUT_MEM(cb, ld1d_mlad_s16)
#undef cb
#define cb(i)                    \
    "vld1.32 {d0}, [%[a_ptr]]\n" \
    "vmla.s16 d0, d0, d0\n"
LATENCY_MEM(cb, ld1d_mlad_s16)
#undef cb

#define cb(i) "vmla.f32 d" #i ", d" #i ", d" #i "\n"
THROUGHPUT(cb, mla_f32_d)
#undef cb
#define cb(i) "vmla.f32 d0, d0, d0\n"
LATENCY(cb, mla_f32_d)
#undef cb

#define cb(i) "vmla.f32 q" #i ", q" #i ", d0[0]\n"
THROUGHPUT_START6(cb, mla_f32q_lane)
#undef cb
#define cb(i) "vmla.f32 q0, q0, d0[0]\n"
LATENCY(cb, mla_f32q_lane)
#undef cb

#define cb(i) "vmla.f32 d" #i ", d" #i ", d0[0]\n"
THROUGHPUT_START6(cb, mla_f32d_lane)
#undef cb
#define cb(i) "vmla.f32 d0, d0, d0[0]\n"
LATENCY(cb, mla_f32d_lane)
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

#endif

// vim: syntax=cpp.doxygen
