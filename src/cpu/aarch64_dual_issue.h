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
#if MEGPEAK_AARCH64
#include <arm_neon.h>
#include "src/cpu/common.h"
#include "stdlib.h"

#include <arm_neon.h>

#define eor(i) "eor v" #i ".16b, v" #i ".16b, v" #i ".16b\n"
// clang-format off
#define THROUGHPUT(cb, func)                                                \
    static int func##_throughput() {                                        \
        asm volatile(                                                       \
        UNROLL_CALL(20, eor)                                                \
        "mov x0, %x[RUNS]\n"                                                \
        "1:\n"                                                              \
        UNROLL_CALL(20, cb)                                                 \
        "subs x0, x0, #1 \n"                                                \
        "bne 1b \n"                                                         \
        :                                                                   \
        : [RUNS] "r"(megpeak::RUNS)                                         \
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", \
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    \
          "v19", "v20", "v21", "v22", "x0");                                \
        return megpeak::RUNS * 20;                                          \
    }

#define LATENCY(cb, func)              \
    static int func##_latency() {      \
        asm volatile(                  \
        "eor v0.16b, v0.16b, v0.16b\n" \
        "mov x0, %x[RUNS]\n"           \
        "1:\n"                         \
        UNROLL_CALL(20, cb)            \
        "subs x0, x0, #1 \n"           \
        "bne 1b \n"                    \
        :                              \
        : [RUNS] "r"(megpeak::RUNS)    \
        : "cc", "v0", "x0");           \
        return megpeak::RUNS * 20;     \
    }

#define THROUGHPUT_MEM_ITER(cb, func, iter)                                   \
    static int func##_throughput() {                                          \
        const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());     \
        size_t run_times = megpeak::RUNS;                                     \
        asm volatile(                                                         \
        UNROLL_CALL(iter, eor)                                                \
        "mov x28, %x[RUNS]\n"                                                 \
        "1:\n"                                                                \
        UNROLL_CALL(iter, cb)                                                 \
        "subs  x28, x28, #1 \n"                                               \
        "bne 1b \n"                                                           \
        :                                                                     \
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                           \
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9",   \
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",      \
          "v19", "v20", "v21", "v22", "x0", "x1", "x2", "x3", "x4", "x5",     \
          "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",   \
          "x16", "x17", "x18", "x19", "x20", "x28", "memory");                \
        return megpeak::RUNS * iter;                                          \
    }

#define THROUGHPUT_MEM(cb, func) THROUGHPUT_MEM_ITER(cb, func, 20)

#define LATENCY_MEM(cb, func)                                             \
    static int func##_latency() {                                         \
        const float* a_ptr = reinterpret_cast<float*>(get_mem_align64()); \
        size_t run_times = megpeak::RUNS;                                 \
        asm volatile(                                                     \
            "eor v0.16b, v0.16b, v0.16b\n"                                \
            "mov x28, %x[RUNS]\n"                                         \
            "1:\n"                                                        \
            UNROLL_CALL(20, cb)                                           \
            "subs x28, x28, #1 \n"                                        \
            "bne 1b \n"                                                   \
            :                                                             \
            : [a_ptr] "r"(a_ptr), [RUNS] "r"(run_times)                   \
            : "cc", "v0","v1", "x0", "x1", "x28", "memory");              \
        return megpeak::RUNS * 20;                                        \
    }
//!!!
// clang-format off
/**
 * @brief 测试ins和ldd是否可以双发射
 * 
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      在A55测试的throughput为0.641968 ns，约为1周期，故可推测ins和ldd在A55可以双发射。
 */
static int ins_ldd_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x28, #0\n"                                                  
        "1:\n"                                                          
        "ins v0.d[0], x0\n"
        "ldr d1, [%[a_ptr]]\n"
        "ins v2.d[0], x0\n"
        "ldr d3, [%[a_ptr]]\n"
        "ins v4.d[0], x0\n"
        "ldr d5, [%[a_ptr]]\n"
        "ins v6.d[0], x0\n"
        "ldr d7, [%[a_ptr]]\n"
        "ins v8.d[0], x0\n"
        "ldr d9, [%[a_ptr]]\n"
        "ins v10.d[0], x0\n"
        "ldr d11, [%[a_ptr]]\n"
        "ins v12.d[0], x0\n"
        "ldr d13, [%[a_ptr]]\n"
        "ins v14.d[0], x0\n"
        "ldr d15, [%[a_ptr]]\n"
        "ins v16.d[0], x0\n"
        "ldr d17, [%[a_ptr]]\n"
        "ins v18.d[0], x0\n"
        "ldr d19, [%[a_ptr]]\n"
        "ins v20.d[0], x0\n"
        "ldr d21, [%[a_ptr]]\n"
        "ins v22.d[0], x0\n"
        "ldr d23, [%[a_ptr]]\n"
        "ins v24.d[0], x0\n"
        "ldr d25, [%[a_ptr]]\n"
        "ins v26.d[0], x0\n"
        "ldr d27, [%[a_ptr]]\n"
        "ins v28.d[0], x0\n"
        "ldr d29, [%[a_ptr]]\n"
        "ins v30.d[0], x0\n"
        "ldr d31, [%[a_ptr]]\n"
                                          
        "add  x28, x28, #1 \n"                                            
        "cmp x28, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                        
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", 
          "x19", "x20", "x28", "memory");
    // 16表示将上面的汇编两两一组分为16组，每组包含1条ins和1条ldr，故计算的throughput表示ins+ldr的throughput。
    return megpeak::RUNS * 16; 
}
#define cb(i) "ins v0.d[0], x0\n"\
              "ldr d0, [%[a_ptr]]\n"
LATENCY_MEM(cb, ins_ldd)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试ldd+ldx+ins指令组合的效果
 * 
 */
#define cb(i)        \
    "ldr d" #i ", [%[a_ptr]]\n" \
    "ldr x" #i ", [%[a_ptr]]\n" \
    "ins v20.d[1], x20\n"
THROUGHPUT_MEM(cb, ldd_ldx_ins)
#undef cb
#define cb(i)              \
    "ldr d0, [%[a_ptr]]\n" \
    "ldr x0, [%[a_ptr]]\n" \
    "ins v0.d[1], x0\n"
LATENCY_MEM(cb, ldd_ldx_ins)
#undef cb
// clang-format on

//!!!
// clang-format off
/**
 * @brief 测试ldq+fmlaq指令组合效果
 * 
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      在A55上进行测试，ldq的throughput为1.157480 ns，而ldd的throughput为0.597214 ns，
 *      表明一个周期可以发射一条ldd，而两个周期才能发射一条ldq。
 *      同时fmla的throughput为0.603197 ns，表明一个周期可发射一条。（以上均是从本项目提供的其它测试得到的结果）
 *      而ldq+fmlaq的throughput为2.917809 ns，表明双周期发射的ldq和单周期发射的fmla组合效果很差。
 */
#define cb(i) "ldr q" #i ", [%[a_ptr]]\n" \
              "fmla v" #i ".4s, v" #i ".4s, v" #i ".4s\n"
THROUGHPUT_MEM(cb, ldq_fmlaq)
#undef cb
#define cb(i) "ldr q0, [%[a_ptr]]\n"\
              "fmla v0.4s, v0.4s, v0.4s\n"
LATENCY_MEM(cb, ldq_fmlaq)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试stq + 3xfmla指令组合的性能
 * 
 */
static int stq_fmlaq_lane_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"                                                          
        "str q0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s[0]\n"
        "fmla v17.4s, v17.4s, v17.s[0]\n"    
        "fmla v18.4s, v18.4s, v18.s[0]\n"    
        "str q0, [%[a_ptr]]\n"
        "fmla v19.4s, v19.4s, v19.s[0]\n"            
        "fmla v20.4s, v20.4s, v20.s[0]\n"    
        "fmla v21.4s, v21.4s, v21.s[0]\n"    
        "str q0, [%[a_ptr]]\n"
        "fmla v22.4s, v22.4s, v22.s[0]\n"    
        "fmla v23.4s, v23.4s, v23.s[0]\n"            
        "fmla v24.4s, v24.4s, v24.s[0]\n"    
        "str q0, [%[a_ptr]]\n"
        "fmla v25.4s, v25.4s, v25.s[0]\n"            
        "fmla v26.4s, v26.4s, v26.s[0]\n"    
        "fmla v27.4s, v27.4s, v27.s[0]\n"    
        "str q0, [%[a_ptr]]\n"
        "fmla v28.4s, v28.4s, v28.s[0]\n"    
        "fmla v29.4s, v29.4s, v29.s[0]\n"            
        "fmla v30.4s, v30.4s, v30.s[0]\n"                                      
        "add  x0, x0, #1 \n"                                            
        "cmp x0, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                        
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0", "memory");
    return run_times * 15;
}
#define cb(i) "str q0, [%[a_ptr]]\n"\
              "fmla v0.4s, v0.4s, v0.s[0]\n"
LATENCY_MEM(cb, stq_fmlaq_lane)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试ldd+fmlad指令组合的性能。
 * 
 */
#define cb(i) "ldr d" #i ", [%[a_ptr]]\n" \
              "fmla v" #i ".2s, v" #i ".2s, v" #i ".2s\n"
THROUGHPUT_MEM(cb, ldd_fmlad)
#undef cb
#define cb(i) "ldr d0, [%[a_ptr]]\n"\
              "fmla v0.2s, v0.2s, v0.2s\n"
LATENCY_MEM(cb, ldd_fmlad)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试ldq + 2xfmlaq指令组合的性能
 * 
 */
static int ldq_fmlaq_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"                                                          
        "ldr q0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.4s\n"
        "fmla v17.4s, v17.4s, v17.4s\n"    
        "ldr q2, [%[a_ptr]]\n"
        "fmla v18.4s, v18.4s, v18.4s\n"    
        "fmla v19.4s, v19.4s, v19.4s\n"    
        "ldr q4, [%[a_ptr]]\n"
        "fmla v20.4s, v20.4s, v20.4s\n"    
        "fmla v21.4s, v21.4s, v21.4s\n"    
        "ldr q6, [%[a_ptr]]\n"
        "fmla v22.4s, v22.4s, v22.4s\n"    
        "fmla v23.4s, v23.4s, v23.4s\n"    
        "ldr q8, [%[a_ptr]]\n"
        "fmla v24.4s, v24.4s, v24.4s\n"    
        "fmla v25.4s, v25.4s, v25.4s\n"    
        "ldr q10, [%[a_ptr]]\n"
        "fmla v26.4s, v26.4s, v26.4s\n"    
        "fmla v27.4s, v27.4s, v27.4s\n"    
        "ldr q12, [%[a_ptr]]\n"
        "fmla v28.4s, v28.4s, v28.4s\n"    
        "fmla v29.4s, v29.4s, v29.4s\n"    
        "ldr q14, [%[a_ptr]]\n"
        "fmla v30.4s, v30.4s, v30.4s\n"    
        "fmla v31.4s, v31.4s, v31.4s\n"                                     
        "add  x0, x0, #1 \n"                                            
        "cmp x0, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                        
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0", "memory");
    return megpeak::RUNS * 16;
}
#define cb(i) "ldr q0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.4s\n"
LATENCY_MEM(cb, ldq_fmlaq_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试ldq + 2xfmlaq_lane指令组合的性能
 * 
 */
static int ldq_fmlaq_lane_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"                                                          
        "ldr q0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"
        "fmla v17.4s, v17.4s, v17.s [0]\n"    
        "ldr q2, [%[a_ptr]]\n"
        "fmla v18.4s, v18.4s, v18.s [0]\n"    
        "fmla v19.4s, v19.4s, v19.s [0]\n"    
        "ldr q4, [%[a_ptr]]\n"
        "fmla v20.4s, v20.4s, v20.s [0]\n"    
        "fmla v21.4s, v21.4s, v21.s [0]\n"    
        "ldr q6, [%[a_ptr]]\n"
        "fmla v22.4s, v22.4s, v22.s [0]\n"    
        "fmla v23.4s, v23.4s, v23.s [0]\n"    
        "ldr q8, [%[a_ptr]]\n"
        "fmla v24.4s, v24.4s, v24.s [0]\n"    
        "fmla v25.4s, v25.4s, v25.s [0]\n"    
        "ldr q10, [%[a_ptr]]\n"
        "fmla v26.4s, v26.4s, v26.s [0]\n"    
        "fmla v27.4s, v27.4s, v27.s [0]\n"    
        "ldr q12, [%[a_ptr]]\n"
        "fmla v28.4s, v28.4s, v28.s [0]\n"    
        "fmla v29.4s, v29.4s, v29.s [0]\n"    
        "ldr q14, [%[a_ptr]]\n"
        "fmla v30.4s, v30.4s, v30.s [0]\n"    
        "fmla v31.4s, v31.4s, v31.s [0]\n"                                     
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
#define cb(i) "ldr q0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldq_fmlaq_lane_sep)
#undef cb
// clang-format on

// !!!
// clang-format off
/**
 * @brief 测试ldd+fmlaq指令组合的效果
 *
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      在A55上进行测试，而ldd的throughput为0.597214 ns，表明一个周期可以发射一条ldd。
 *      同时fmla的throughput为0.603197 ns，表明一个周期可发射一条。（以上均是从本项目提供的其它测试得到的结果）
 * 
 *      而ldd+fmlaq的throughput为0.630208 ns，约为一个周期，
 *      表明ldd+fmlaq可以双发射（注意：本测试中ldd和fmla没有寄存器资源的依赖）。
 */
static int ldd_fmlaq_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"                                                          
        "ldr d0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.4s\n"
        "ldr d1, [%[a_ptr]]\n"
        "fmla v17.4s, v17.4s, v17.4s\n"    
        "ldr d2, [%[a_ptr]]\n"
        "fmla v18.4s, v18.4s, v18.4s\n"    
        "ldr d3, [%[a_ptr]]\n"
        "fmla v19.4s, v19.4s, v19.4s\n"    
        "ldr d4, [%[a_ptr]]\n"
        "fmla v20.4s, v20.4s, v20.4s\n"    
        "ldr d5, [%[a_ptr]]\n"
        "fmla v21.4s, v21.4s, v21.4s\n"    
        "ldr d6, [%[a_ptr]]\n"
        "fmla v22.4s, v22.4s, v22.4s\n"    
        "ldr d7, [%[a_ptr]]\n"
        "fmla v23.4s, v23.4s, v23.4s\n"    
        "ldr d8, [%[a_ptr]]\n"
        "fmla v24.4s, v24.4s, v24.4s\n"    
        "ldr d9, [%[a_ptr]]\n"
        "fmla v25.4s, v25.4s, v25.4s\n"    
        "ldr d10, [%[a_ptr]]\n"
        "fmla v26.4s, v26.4s, v26.4s\n"    
        "ldr d11, [%[a_ptr]]\n"
        "fmla v27.4s, v27.4s, v27.4s\n"    
        "ldr d12, [%[a_ptr]]\n"
        "fmla v28.4s, v28.4s, v28.4s\n"    
        "ldr d13, [%[a_ptr]]\n"
        "fmla v29.4s, v29.4s, v29.4s\n"    
        "ldr d14, [%[a_ptr]]\n"
        "fmla v30.4s, v30.4s, v30.4s\n"    
        "ldr d15, [%[a_ptr]]\n"
        "fmla v31.4s, v31.4s, v31.4s\n"                                     
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
#define cb(i) "ldr d0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.4s\n"
LATENCY_MEM(cb, ldd_fmlaq_sep)
#undef cb
// clang-format on

// !!!
// clang-format off
/**
 * @brief 测试ins+fmlaq_lane指令组合的性能
 * 
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      测得ins+fmlaq_lane的throughput为0.621191 ns，所以可推测ins和fmlaq_lane可以双发射。
 */
#define cb(i) "ins v21.d[0], x1\n"\
        "fmla v"#i".4s, v"#i".4s, v"#i".s [0]\n"
THROUGHPUT_MEM(cb, ins_fmlaq_lane_sep)
#undef cb
#define cb(i) "ins v0.d[0], x0\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ins_fmlaq_lane_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试dupd + fmlaq_lane指令组合的性能
 * 
 */
#define cb(i) "dup v21.2d, x1\n"\
        "fmla v"#i".4s, v"#i".4s, v"#i".s [0]\n"
THROUGHPUT_MEM(cb, dupd_fmlaq_lane_sep)
#undef cb
#define cb(i) "dup v0.2d, x0\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, dupd_fmlaq_lane_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试ldq + stq指令组合的性能
 * 
 */
#define cb(i) "ldr q" #i ", [%[a_ptr]]\n" \
              "str q" #i ", [%[a_ptr]]\n"
THROUGHPUT_MEM(cb, ldqstq)
#undef cb
#define cb(i) "ldr q0, [%[a_ptr]]\n"\
              "str q0, [%[a_ptr]]\n"
LATENCY_MEM(cb, ldqstq)
#undef cb
// clang-format on

/**
 * @brief 测试dupd_lane + smlal_s8指令组合的性能
 *
 */
#define cb(i)                \
    "dup v20.8b, v21.b[0]\n" \
    "smlal v" #i ".8h, v" #i ".8b, v" #i ".8b\n"
THROUGHPUT_MEM(cb, dupd_lane_smlal_s8)
#undef cb
#define cb(i)              \
    "dup v0.8b, v0.b[0]\n" \
    "smlal v0.8h, v0.8b, v0.8b\n"
LATENCY_MEM(cb, dupd_lane_smlal_s8)
#undef cb
//!!!
// clang-format off
/**
 * @brief 测试ldd+fmlaq_lane指令的组合效果。
 * 
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      测试ldd+fmlaq_lane的throughput为0.625384 ns，约为一个周期，所以可以推测ldd和fmlaq_lane可以双发射。
 */
#define cb(i) "ldr d21, [%[a_ptr]]\n"\
        "fmla v"#i".4s, v"#i".4s, v"#i".s [0]\n"
THROUGHPUT_MEM(cb, ldd_fmlaq_lane_sep)
#undef cb
#define cb(i) "ldr d0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldd_fmlaq_lane_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试lds + fmlaq_lane指令组合的性能
 * 
 */
#define cb(i) "ldr s21, [%[a_ptr]]\n"\
        "fmla v"#i".4s, v"#i".4s, v"#i".s [0]\n"
THROUGHPUT_MEM(cb, lds_fmlaq_lane_sep)
#undef cb
#define cb(i) "ldr s0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, lds_fmlaq_lane_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试ldd + smlalq指令组合的性能
 * 
 */
#define cb(i) "ldr d21, [%[a_ptr]]\n"\
              "smlal v"#i".8h, v"#i".8b, v"#i".8b\n"
THROUGHPUT_MEM(cb, ldd_smlalq_sep)
#undef cb
#define cb(i) "ldr d1, [%[a_ptr]]\n"\
              "smlal v1.8h, v1.8b, v1.8b\n"
LATENCY_MEM(cb, ldd_smlalq_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试ldq + smlalq指令组合的性能
 * 
 */
#define cb(i) "ldr q21, [%[a_ptr]]\n"\
              "smlal v"#i".8h, v"#i".8b, v"#i".8b\n"
THROUGHPUT_MEM(cb, ldq_smlalq_sep)
#undef cb
#define cb(i) "ldr q1, [%[a_ptr]]\n"\
              "smlal v1.8h, v1.8b, v1.8b\n"
LATENCY_MEM(cb, ldq_smlalq_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 2xldd + smlalq指令组合的性能
 * 
 */
#define cb(i) "ld1 {v21.2s, v22.2s}, [%[a_ptr]]\n"\
              "smlal v"#i".8h, v"#i".8b, v"#i".8b\n"
THROUGHPUT_MEM(cb, lddx2_smlalq_sep)
#undef cb
#define cb(i) "ld1 {v0.2s, v1.2s}, [%[a_ptr]]\n"\
              "smlal v1.8h, v1.8b, v1.8b\n"
LATENCY_MEM(cb, lddx2_smlalq_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 smlal + ldx + 3xsmlal + ldd + ins指令组合的性能
 * 
 */
static int ldx_ins_smlalq_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "smlal v16.8h, v16.8b, v16.8b \n"
        "ldr x2, [%[a_ptr]]\n"
        "smlal v17.8h, v17.8b, v17.8b \n"    
        "smlal v18.8h, v18.8b, v18.8b \n"    
        "smlal v19.8h, v19.8b, v19.8b \n"    
        "ldr d3, [%[a_ptr]]\n"
        "ins v4.d[0], x5\n"

        "smlal v20.8h, v20.8b, v20.8b \n" 
        "ldr x2, [%[a_ptr]]\n"   
        "smlal v21.8h, v21.8b, v21.8b \n"    
        "smlal v22.8h, v22.8b, v22.8b \n"    
        "smlal v23.8h, v23.8b, v23.8b \n"    
        "ldr d3, [%[a_ptr]]\n"
        "ins v8.d[0], x5\n"

        "smlal v24.8h, v24.8b, v24.8b \n" 
        "ldr x2, [%[a_ptr]]\n"   
        "smlal v25.8h, v25.8b, v25.8b \n"    
        "smlal v26.8h, v26.8b, v26.8b \n"    
        "smlal v27.8h, v27.8b, v27.8b \n"    
        "ldr d3, [%[a_ptr]]\n"
        "ins v12.d[0], x5\n"

        "smlal v28.8h, v28.8b, v28.8b \n"   
        "ldr x2, [%[a_ptr]]\n" 
        "smlal v29.8h, v29.8b, v29.8b \n"    
        "smlal v30.8h, v30.8b, v30.8b \n"    
        "smlal v31.8h, v31.8b, v31.8b \n"
        "ldr d3, [%[a_ptr]]\n"
        "ins v15.d[0], x5\n"             

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
#define cb(i) "ldr x0, [%[a_ptr]]\n"\
              "ins v1.d[0], x0\n"\
              "smlal v1.8h, v1.8b, v1.8b\n"
LATENCY_MEM(cb, ldx_ins_smlalq_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 2xsmlal + ins指令组合的性能
 * 
 */
static int ins_smlalq_sep_1_2_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "smlal v16.8h, v16.8b, v16.8b \n"
        "smlal v17.8h, v17.8b, v17.8b \n"    
        "ins v3.d[0], x5\n"
        "smlal v18.8h, v18.8b, v18.8b \n"    
        "smlal v19.8h, v19.8b, v19.8b \n"    
        "ins v3.d[0], x5\n"
        "smlal v20.8h, v20.8b, v20.8b \n"    
        "smlal v21.8h, v21.8b, v21.8b \n"    
        "ins v3.d[0], x5\n"
        "smlal v22.8h, v22.8b, v22.8b \n"    
        "smlal v23.8h, v23.8b, v23.8b \n"           
        "ins v3.d[0], x5\n"
        "smlal v24.8h, v24.8b, v24.8b \n"    
        "smlal v25.8h, v25.8b, v25.8b \n"    
        "ins v3.d[0], x5\n"
        "smlal v26.8h, v26.8b, v26.8b \n"    
        "smlal v27.8h, v27.8b, v27.8b \n"    
        "ins v3.d[0], x5\n"
        "smlal v28.8h, v28.8b, v28.8b \n"    
        "smlal v29.8h, v29.8b, v29.8b \n"    
        "ins v3.d[0], x5\n"
        "smlal v30.8h, v30.8b, v30.8b \n"    
        "smlal v31.8h, v31.8b, v31.8b \n"
        "ins v15.d[0], x5\n"             
                                
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
#define cb(i) "ins v1.d[0], x0\n"\
              "smlal v1.8h, v1.8b, v1.8b\n"
LATENCY_MEM(cb, ins_smlalq_sep_1_2)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 ldx + fmlaq_lane 指令组合的性能
 * 
 */
#define cb(i) "ldr x"#i", [%[a_ptr]]\n"\
        "fmla v"#i".4s, v"#i".4s, v"#i".s [0]\n"
THROUGHPUT_MEM(cb, ldx_fmlaq_lane_sep)
#undef cb
#define cb(i) "ldr x0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldx_fmlaq_lane_sep)
#undef cb
// clang-format on
//!!!
// clang-format off
/**
 * @brief 和ldd+fmlaq_lane组合进行对比，测试ldd + 4xfmlaq_lane（增加计算密度）的性能。
 * 
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      ldd+fmlaq_lane: throughput: 0.625384 ns 12.792140 GFlops
 *      ldd + 4xfmlaq_lane: throughput: 0.638399 ns 12.531342 GFlops
 *      以上结果表明，在A55上，上述指令组合增加计算密度不会有性能提升，即ldd+fmlaq_lane已经可以充分发挥A55的计算能力。
 */
static int ldd_fmlaq_lane_1_4_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "ldr d0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "fmla v17.4s, v17.4s, v17.s [0]\n"          
        "fmla v18.4s, v18.4s, v18.s [0]\n"         
        "fmla v19.4s, v19.4s, v19.s [0]\n"  
        
        "ldr d2, [%[a_ptr]]\n"        
        "fmla v20.4s, v20.4s, v20.s [0]\n"   
        "fmla v21.4s, v21.4s, v21.s [0]\n"         
        "fmla v22.4s, v22.4s, v22.s [0]\n"             
        "fmla v23.4s, v23.4s, v23.s [0]\n"    

        "ldr d6, [%[a_ptr]]\n"
        "fmla v24.4s, v24.4s, v24.s [0]\n"    
        "fmla v25.4s, v25.4s, v25.s [0]\n"           
        "fmla v26.4s, v26.4s, v26.s [0]\n"            
        "fmla v27.4s, v27.4s, v27.s [0]\n"  

        "ldr d11, [%[a_ptr]]\n" 
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "fmla v29.4s, v29.4s, v29.s [0]\n"            
        "fmla v30.4s, v30.4s, v30.s [0]\n"            
        "fmla v31.4s, v31.4s, v31.s [0]\n"   

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
          "x19", "x20", "x22");
    return megpeak::RUNS * 16;
}
#define cb(i) "ldr x0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldd_fmlaq_lane_1_4_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 ldd + 4xmla_s16_lane 指令组合的性能
 * 
 */
static int ldd_mla_s16_lane_1_4_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "ldr d0, [%[a_ptr]]\n"
        "mla v16.8h, v16.8h, v1.h [0]\n"        
        "mla v17.8h, v17.8h, v1.h [0]\n"          
        "mla v18.8h, v18.8h, v1.h [0]\n"         
        "mla v19.8h, v19.8h, v1.h [0]\n"  
        
        "ldr d2, [%[a_ptr]]\n"        
        "mla v20.8h, v20.8h, v1.h [0]\n"   
        "mla v21.8h, v21.8h, v1.h [0]\n"         
        "mla v22.8h, v22.8h, v1.h [0]\n"             
        "mla v23.8h, v23.8h, v1.h [0]\n"    

        "ldr d6, [%[a_ptr]]\n"
        "mla v24.8h, v24.8h, v1.h [0]\n"    
        "mla v25.8h, v25.8h, v1.h [0]\n"           
        "mla v26.8h, v26.8h, v1.h [0]\n"            
        "mla v27.8h, v27.8h, v1.h [0]\n"  

        "ldr d11, [%[a_ptr]]\n" 
        "mla v28.8h, v28.8h, v1.h [0]\n"            
        "mla v29.8h, v29.8h, v1.h [0]\n"            
        "mla v30.8h, v30.8h, v1.h [0]\n"            
        "mla v31.8h, v31.8h, v1.h [0]\n"   

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
          "x19", "x20", "x22");
    return megpeak::RUNS * 16;
}
#define cb(i) "ldr d1, [%[a_ptr]]\n"\
              "mla v1.8h, v1.8h, v1.h [0]\n"
LATENCY_MEM(cb, ldd_mla_s16_lane_1_4_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 sshll + ins 指令组合的性能
 * 
 */
static int sshll_ins_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "sshll v0.8h, v0.8b, #0\n"
        "ins v1.d[0], x0\n"
        "sshll v2.8h, v2.8b, #0\n"
        "ins v3.d[0], x0\n"
        "sshll v4.8h, v4.8b, #0\n"
        "ins v5.d[0], x0\n"
        "sshll v6.8h, v6.8b, #0\n"
        "ins v7.d[0], x0\n"
        "sshll v8.8h, v8.8b, #0\n"
        "ins v9.d[0], x0\n"
        "sshll v10.8h, v10.8b, #0\n"
        "ins v11.d[0], x0\n"
        "sshll v12.8h, v12.8b, #0\n"
        "ins v13.d[0], x0\n"
        "sshll v14.8h, v14.8b, #0\n"
        "ins v15.d[0], x0\n"
        "sshll v16.8h, v16.8b, #0\n"
        "ins v17.d[0], x0\n"
        "sshll v18.8h, v18.8b, #0\n"
        "ins v19.d[0], x0\n"
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
          "x19", "x20", "x22");
    return megpeak::RUNS * 10;
}
#define cb(i) "ins v1.d[0], x0\n"\
              "sshll v1.8h, v1.8b, #0\n"              
LATENCY_MEM(cb, sshll_ins_sep)
#undef cb
// clang-format on

/**
 * @brief 测试 ldq + 4xfmlaq_lane 指令组合的性能
 *
 */
static int ldq_fmlaq_lane_1_4_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "ldr q0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "fmla v17.4s, v17.4s, v17.s [0]\n"          
        "fmla v18.4s, v18.4s, v18.s [0]\n"         
        "fmla v19.4s, v19.4s, v19.s [0]\n"  
        
        "ldr q2, [%[a_ptr]]\n"        
        "fmla v20.4s, v20.4s, v20.s [0]\n"   
        "fmla v21.4s, v21.4s, v21.s [0]\n"         
        "fmla v22.4s, v22.4s, v22.s [0]\n"             
        "fmla v23.4s, v23.4s, v23.s [0]\n"    

        "ldr q6, [%[a_ptr]]\n"
        "fmla v24.4s, v24.4s, v24.s [0]\n"    
        "fmla v25.4s, v25.4s, v25.s [0]\n"           
        "fmla v26.4s, v26.4s, v26.s [0]\n"            
        "fmla v27.4s, v27.4s, v27.s [0]\n"  

        "ldr q11, [%[a_ptr]]\n" 
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "fmla v29.4s, v29.4s, v29.s [0]\n"            
        "fmla v30.4s, v30.4s, v30.s [0]\n"            
        "fmla v31.4s, v31.4s, v31.s [0]\n"   

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
          "x19", "x20", "x22");
    return megpeak::RUNS * 16;
}
#define cb(i)               \
    "ldr q1, [%[a_ptr]] \n" \
    "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldq_fmlaq_lane_1_4_sep)
#undef cb

//!!!
// clang-format off
/**
 * @brief 和ins+fmlaq_lane组合进行对比，测试ins + 4xfmlaq_lane（增加计算密度）的性能。
 * 
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      ins+fmlaq_lane: throughput: 0.621191 ns 12.878477 GFlops
 *      ldd + 4xfmlaq_lane: throughput: 0.638135 ns 12.536536 GFlops
 *      以上结果表明，在A55上，上述指令组合增加计算密度不会有性能提升，即ins+fmlaq_lane已经可以充分发挥A55的计算能力。
 */
static int ins_fmlaq_lane_1_4_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "ins v0.d[0], x0\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "fmla v17.4s, v17.4s, v17.s [0]\n"          
        "fmla v18.4s, v18.4s, v18.s [0]\n"         
        "fmla v19.4s, v19.4s, v19.s [0]\n"  
        
        "ins v0.d[0], x0\n"     
        "fmla v20.4s, v20.4s, v20.s [0]\n"   
        "fmla v21.4s, v21.4s, v21.s [0]\n"         
        "fmla v22.4s, v22.4s, v22.s [0]\n"             
        "fmla v23.4s, v23.4s, v23.s [0]\n"    

       "ins v0.d[0], x0\n"
        "fmla v24.4s, v24.4s, v24.s [0]\n"    
        "fmla v25.4s, v25.4s, v25.s [0]\n"           
        "fmla v26.4s, v26.4s, v26.s [0]\n"            
        "fmla v27.4s, v27.4s, v27.s [0]\n"  

        "ins v0.d[0], x0\n"
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "fmla v29.4s, v29.4s, v29.s [0]\n"            
        "fmla v30.4s, v30.4s, v30.s [0]\n"            
        "fmla v31.4s, v31.4s, v31.s [0]\n"   

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
          "x19", "x20", "x22");
    return megpeak::RUNS * 16;
}
#define cb(i) "ins v1.d[0], x0\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ins_fmlaq_lane_1_4_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 ldd + ins + fmlaq_lane + ldx + fmlaq_lane + nop + fmlaq_lane + nop 指令组合的性能
 * 
 */
static int ldd_fmlaq_lane_1_3_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                                  
        "ldr d0, [%[a_ptr]]\n"
        "ins v1.d[0], x1\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "ldr x2, [%[a_ptr]]\n"  
        "fmla v17.4s, v17.4s, v17.s [0]\n"         
        "nop\n" 
        "fmla v18.4s, v18.4s, v18.s [0]\n" 
        "nop\n" 

        "ldr d2, [%[a_ptr]]\n"         
        "ins v3.d[0], x1\n"
        "fmla v19.4s, v19.4s, v19.s [0]\n"               
        "ldr x2, [%[a_ptr]]\n" 
        "fmla v20.4s, v20.4s, v20.s [0]\n"            
        "nop\n" 
        "fmla v21.4s, v21.4s, v21.s [0]\n"
        "nop\n"          
        
        "ldr d4, [%[a_ptr]]\n"
        "ins v5.d[0], x1\n"
        "fmla v22.4s, v22.4s, v22.s [0]\n"             
        "ldr x2, [%[a_ptr]]\n"   
        "fmla v23.4s, v23.4s, v23.s [0]\n"   
        "nop\n"                 
        "fmla v24.4s, v24.4s, v24.s [0]\n"    
        "nop\n" 

        "ldr d6, [%[a_ptr]]\n"
        "ins v7.d[0], x1\n"
        "fmla v25.4s, v25.4s, v25.s [0]\n"           
        "ldr x2, [%[a_ptr]]\n"  
        "fmla v26.4s, v26.4s, v26.s [0]\n"                    
        "nop\n" 
        "fmla v27.4s, v27.4s, v27.s [0]\n"  
        "nop\n" 

        "ldr d11, [%[a_ptr]]\n"     
        "ins v12.d[0], x1\n"    
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "ldr x2, [%[a_ptr]]\n"  
        "fmla v29.4s, v29.4s, v29.s [0]\n" 
        "add  x20, x20, #1 \n" 
        "fmla v30.4s, v30.4s, v30.s [0]\n"  
        // "nop\n"           

                                                   
        "cmp x20, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                        
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", 
          "x19", "x20", "x22");
    return megpeak::RUNS * 15;
}
#define cb(i) "ldr x0, [%[a_ptr]]\n"\
              "ins v1.d[0], x0\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldd_fmlaq_lane_1_3_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 ins + 3xfmlaq_lane 指令组合的性能
 * 
 */
static int ins_fmlaq_lane_1_3_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                                  
        "ins v1.d[0], x1\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "fmla v17.4s, v17.4s, v17.s [0]\n" 
        "fmla v18.4s, v18.4s, v18.s [0]\n"  
      
        "ins v3.d[0], x1\n"
        "fmla v19.4s, v19.4s, v19.s [0]\n"               
        "fmla v20.4s, v20.4s, v20.s [0]\n"   
        "fmla v21.4s, v21.4s, v21.s [0]\n"         
        
        "ins v5.d[0], x1\n"
        "fmla v22.4s, v22.4s, v22.s [0]\n"             
        "fmla v23.4s, v23.4s, v23.s [0]\n"          
        "fmla v24.4s, v24.4s, v24.s [0]\n"    

        "ins v7.d[0], x1\n"
        "fmla v25.4s, v25.4s, v25.s [0]\n"           
        "fmla v26.4s, v26.4s, v26.s [0]\n"          
        "fmla v27.4s, v27.4s, v27.s [0]\n"  

        "ins v12.d[0], x1\n"    
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "fmla v29.4s, v29.4s, v29.s [0]\n" 
        "fmla v30.4s, v30.4s, v30.s [0]\n"            

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
          "x19", "x20", "x22");
    return megpeak::RUNS * 15;
}
#define cb(i) "ldr x0, [%[a_ptr]]\n"\
              "ins v1.d[0], x0\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ins_fmlaq_lane_1_3_sep)
#undef cb
// clang-format on

/**
 * @brief 测试 ldq + 3xfmlaq_lane 指令组合的性能
 *
 */
static int ldq_fmlaq_lane_1_3_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "ldr q0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "fmla v17.4s, v17.4s, v17.s [0]\n"          
        "fmla v18.4s, v18.4s, v18.s [0]\n"         
        "ldr q2, [%[a_ptr]]\n"        
        "fmla v19.4s, v19.4s, v19.s [0]\n"                  
        "fmla v20.4s, v20.4s, v20.s [0]\n"   
        "fmla v21.4s, v21.4s, v21.s [0]\n"  
        "ldr q6, [%[a_ptr]]\n"       
        "fmla v22.4s, v22.4s, v22.s [0]\n"             
        "fmla v23.4s, v23.4s, v23.s [0]\n"            
        "fmla v24.4s, v24.4s, v24.s [0]\n"    
        "ldr q7, [%[a_ptr]]\n" 
        "fmla v25.4s, v25.4s, v25.s [0]\n"           
        "fmla v26.4s, v26.4s, v26.s [0]\n"            
        "fmla v27.4s, v27.4s, v27.s [0]\n"  
        "ldr q7, [%[a_ptr]]\n" 
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "fmla v29.4s, v29.4s, v29.s [0]\n"            
        "fmla v30.4s, v30.4s, v30.s [0]\n"                    

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
          "x19", "x20", "x22");
    return megpeak::RUNS * 15;
}
#define cb(i)               \
    "ldr q1, [%[a_ptr]] \n" \
    "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldq_fmlaq_lane_1_3_sep)
#undef cb

/**
 * @brief 测试 ldq + 2xfmlaq_lane 指令组合的性能
 *
 */
static int ldq_fmlaq_lane_1_2_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x20, #0\n"                                                  
        "1:\n"                                                          
        "ldr q0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "fmla v17.4s, v17.4s, v17.s [0]\n"          
        "ldr q2, [%[a_ptr]]\n"    
        "fmla v18.4s, v18.4s, v18.s [0]\n"                     
        "fmla v19.4s, v19.4s, v19.s [0]\n"                  
        "ldr q4, [%[a_ptr]]\n"   
        "fmla v20.4s, v20.4s, v20.s [0]\n"   
        "fmla v21.4s, v21.4s, v21.s [0]\n"  
        "ldr q6, [%[a_ptr]]\n"   
        "fmla v22.4s, v22.4s, v22.s [0]\n"             
        "fmla v23.4s, v23.4s, v23.s [0]\n"            
        "ldr q7, [%[a_ptr]]\n" 
        "fmla v24.4s, v24.4s, v24.s [0]\n"            
        "fmla v25.4s, v25.4s, v25.s [0]\n"           
        "ldr q8, [%[a_ptr]]\n" 
        "fmla v26.4s, v26.4s, v26.s [0]\n"            
        "fmla v27.4s, v27.4s, v27.s [0]\n"  
        "ldr q9, [%[a_ptr]]\n" 
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "fmla v29.4s, v29.4s, v29.s [0]\n"            
        "ldr q10, [%[a_ptr]]\n" 
        "fmla v30.4s, v30.4s, v30.s [0]\n"      
        "fmla v31.4s, v31.4s, v31.s [0]\n"                    

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
          "x19", "x20", "x22");
    return megpeak::RUNS * 16;
}
#define cb(i)               \
    "ldr q1, [%[a_ptr]] \n" \
    "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldq_fmlaq_lane_1_2_sep)
#undef cb
//!!!
// clang-format off
/**
 * @brief 测试ldd+fmla + ldx+fmla + ins+fmla指令组合的性能，下面代码中两条指令为一组。
 * 
 *      以A55为例，其频率为1.8GHz，周期为1/1.8ns,约为0.56ns。
 *      ldd+fmla + ldx+fmla + ins+fmla的throughput为0.803760 ns，表明fmla和上面三条指令的组合可以双发射。
 */
static int ldd_ldx_ins_fmlaq_lane_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"                                                          
        "ldr d0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"
        "ldr x1, [%[a_ptr]]\n"
        "fmla v17.4s, v17.4s, v17.s [0]\n"    
        "ins v2.d[1], x2\n"
        "fmla v18.4s, v18.4s, v18.s [0]\n"    
        "ldr d3, [%[a_ptr]]\n"
        "fmla v19.4s, v19.4s, v19.s [0]\n"    
        "ldr x4, [%[a_ptr]]\n"
        "fmla v20.4s, v20.4s, v20.s [0]\n"    
        "ins v5.d[1], x5\n"
        "fmla v21.4s, v21.4s, v21.s [0]\n"    
        "ldr d6, [%[a_ptr]]\n"
        "fmla v22.4s, v22.4s, v22.s [0]\n"    
        "ldr x7, [%[a_ptr]]\n"
        "fmla v23.4s, v23.4s, v23.s [0]\n"    
        "ins v8.d[1], x8\n"
        "fmla v24.4s, v24.4s, v24.s [0]\n"    
        "ldr d9, [%[a_ptr]]\n"
        "fmla v25.4s, v25.4s, v25.s [0]\n"    
        "ldr x10, [%[a_ptr]]\n"
        "fmla v26.4s, v26.4s, v26.s [0]\n"    
        "ins v11.d[1], x11\n"
        "fmla v27.4s, v27.4s, v27.s [0]\n"    
        "ldr x12, [%[a_ptr]]\n"
        "fmla v28.4s, v28.4s, v28.s [0]\n"    
        "ldr x13, [%[a_ptr]]\n"
        "fmla v29.4s, v29.4s, v29.s [0]\n"    
        "ins v14.d[1], x14\n"
        "fmla v30.4s, v30.4s, v30.s [0]\n"    
        "ldr d15, [%[a_ptr]]\n"
        "fmla v31.4s, v31.4s, v31.s [0]\n"                                     
        "add  x0, x0, #1 \n"                                            
        "cmp x0, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                        
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",\
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", \
          "x19", "x20");
    return megpeak::RUNS * 16;
}
#define cb(i) "ldr d0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldd_ldx_ins_fmlaq_lane_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 ldd + 2xfmlaq_lane + ldx + fmlaq_lane + ins + fmlaq_lane 指令组合的性能
 * 
 */
static int ldd_nop_ldx_ins_fmlaq_lane_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"           

        "ldr d0, [%[a_ptr]]\n"
        "fmla v16.4s, v16.4s, v16.s [0]\n"        
        "fmla v17.4s, v17.4s, v17.s [0]\n"    
        "ldr x1, [%[a_ptr]]\n"
        "fmla v18.4s, v18.4s, v18.s [0]\n"    
        "ins v2.d[1], x2\n"
        "fmla v19.4s, v19.4s, v19.s [0]\n"    
        
        "ldr d3, [%[a_ptr]]\n"
        "fmla v20.4s, v20.4s, v20.s [0]\n"            
        "fmla v21.4s, v21.4s, v21.s [0]\n"    
        "ldr x4, [%[a_ptr]]\n"
        "fmla v22.4s, v22.4s, v22.s [0]\n"    
        "ins v5.d[1], x5\n"
        "fmla v23.4s, v23.4s, v23.s [0]\n"    

        "ldr d6, [%[a_ptr]]\n"
        "fmla v24.4s, v24.4s, v24.s [0]\n"            
        "fmla v25.4s, v25.4s, v25.s [0]\n"    
        "ldr x7, [%[a_ptr]]\n"              
        "fmla v26.4s, v26.4s, v26.s [0]\n"    
        "ins v8.d[1], x11\n"                  
        "fmla v27.4s, v27.4s, v27.s [0]\n"            

        "ldr d9, [%[a_ptr]]\n"
        "fmla v28.4s, v28.4s, v28.s [0]\n"            
        "fmla v29.4s, v29.4s, v29.s [0]\n"    
        "ldr x10, [%[a_ptr]]\n"
        "fmla v30.4s, v30.4s, v30.s [0]\n"    
        "ins v11.d[1], x14\n"        
        "fmla v31.4s, v31.4s, v31.s [0]\n"                                     
        
        "add  x0, x0, #1 \n"                                            
        "cmp x0, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                        
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",\
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", \
          "x19", "x20", "memory");
    return megpeak::RUNS * 16;
}
#define cb(i) "ldr d0, [%[a_ptr]]\n"\
              "fmla v1.4s, v1.4s, v1.s [0]\n"
LATENCY_MEM(cb, ldd_nop_ldx_ins_fmlaq_lane_sep)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 addp + smlal_8b 指令组合的性能
 * 
 */
#define cb(i) \
"addp v21.8h, v22.8h, v22.8h\n"\
"smlal v" #i ".8h, v" #i ".8b, v" #i ".8b\n"
THROUGHPUT(cb, smlal_8b_addp)
#undef cb
#define cb(i) \
"smlal v0.8h, v0.8b, v0.8b\n"\
"addp v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, smlal_8b_addp)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 dupd + smlal_8b 指令组合的性能
 * 
 */
#define cb(i) \
"dup v21.8b, v22.b[0]\n"\
"smlal v" #i ".8h, v" #i ".8b, v" #i ".8b\n"
THROUGHPUT(cb, smlal_8b_dupd)
#undef cb
#define cb(i) \
"dup v0.8b, v0.b[0]\n"\
"addp v0.4s, v0.4s, v0.4s\n"
LATENCY(cb, smlal_8b_dupd)
#undef cb
// clang-format on

/**
 * @brief 测试 smlal + sadalp 指令组合的性能
 *
 */
#define cb(i)                            \
    "smlal v" #i ".8h, v" #i ".8b, v" #i \
    ".8b\n"                              \
    "sadalp v" #i ".4s, v" #i ".8h \n"
THROUGHPUT(cb, smlal_sadalp)
#undef cb
#define cb(i)                     \
    "smlal v0.8h, v0.8b, v0.8b\n" \
    "sadalp v0.4s, v0.8h\n"
LATENCY(cb, smlal_sadalp)
#undef cb

/**
 * @brief 测试 smull + smlal + sadalp 指令组合的性能
 *
 */
#define cb(i)                            \
    "smull v" #i ".8h, v" #i ".8b, v" #i \
    ".8b\n"                              \
    "smlal v" #i ".8h, v" #i ".8b, v" #i \
    ".8b\n"                              \
    "sadalp v" #i ".4s, v" #i ".8h \n"
THROUGHPUT(cb, smull_smlal_sadalp)
#undef cb
#define cb(i)                     \
    "smull v0.8h, v0.8b, v0.8b\n" \
    "smlal v0.8h, v0.8b, v0.8b\n" \
    "sadalp v0.4s, v0.8h\n"
LATENCY(cb, smull_smlal_sadalp)
#undef cb

/**
 * @brief 测试 smull + smlal + sadalp 指令组合的性能，指令间没有寄存器资源的依赖
 *
 */
static int smull_smlal_sadalp_sep_throughput() {
    const float* a_ptr = reinterpret_cast<float*>(get_mem_align64());
    size_t run_times = megpeak::RUNS;
    asm volatile(                                                   
        UNROLL_CALL(20, eor)                                            
        "mov x0, #0\n"                                                  
        "1:\n"                                                          
        "smull v0.8h, v0.8b, v0.8b\n"
        "smlal v1.8h, v1.8b, v1.8b\n"
        "sadalp v2.4s, v2.8h\n"

        "smull v3.8h, v3.8b, v3.8b\n"
        "smlal v4.8h, v4.8b, v4.8b\n" 
        "sadalp v5.4s, v5.8h\n"

        "smull v6.8h, v6.8b, v6.8b\n"
        "smlal v7.8h, v7.8b, v7.8b\n"
        "sadalp v8.4s, v8.8h\n"

        "smull v9.8h, v9.8b, v9.8b\n"
        "smlal v10.8h, v10.8b, v10.8b\n"
        "sadalp v11.4s, v11.8h\n"

        "smull v12.8h, v12.8b, v12.8b\n"
        "smlal v13.8h, v13.8b, v13.8b\n"
        "sadalp v14.4s, v14.8h\n"

        "smull v15.8h, v15.8b, v15.8b\n"
        "smlal v16.8h, v16.8b, v16.8b\n"
        "sadalp v17.4s, v17.8h\n"

        "add  x0, x0, #1 \n"                                            
        "cmp x0, %[RUNS] \n"                                            
        "blt 1b \n"                                                     
        :                                                               
        : [RUNS] "r"(run_times), [a_ptr] "r"(a_ptr)                                       
        : "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", 
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",    
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",    
          "v29", "v30", "v31", "x0");
    return megpeak::RUNS * 12;
}
#define cb(i)                     \
    "smull v0.8h, v0.8b, v0.8b\n" \
    "smlal v0.8h, v0.8b, v0.8b\n" \
    "sadalp v0.4s, v0.8h\n"
LATENCY(cb, smull_smlal_sadalp_sep)
#undef cb
// clang-format on

/**
 * @brief 测试 ldd + sshll 指令组合的性能
 *
 */
#define cb(i)                \
    "ldr d21, [%[a_ptr]] \n" \
    "sshll v" #i ".8h, v" #i ".8b, #0\n"
THROUGHPUT_MEM(cb, ldrd_sshll)
#undef cb
#define cb(i)               \
    "ldr d0, [%[a_ptr]] \n" \
    "sshll v0.8h, v0.8b, #0\n"
LATENCY_MEM(cb, ldrd_sshll)
#undef cb

#if __ARM_FEATURE_DOTPROD
// clang-format off
/**
 * @brief 测试 ldd + sdot_lane 指令组合的性能
 * 
 */
#define cb(i) "ldr d21, [%[a_ptr]]\n"\
              "sdot v" #i ".4s, v" #i ".16b, v" #i ".16b\n"
THROUGHPUT_MEM(cb, ldrd_sdot_lane)
#undef cb
#define cb(i) "ldr d1, [%[a_ptr]]\n"\
              "sdot v1.4s, v1.16b, v1.4b[0]\n"
LATENCY_MEM(cb, ldrd_sdot_lane)
#undef cb
// clang-format on

// clang-format off
/**
 * @brief 测试 ldq + sdot_lane 指令组合的性能
 * 
 */
#define cb(i) "ldr q21, [%[a_ptr]]\n"\
              "sdot v" #i ".4s, v" #i ".16b, v" #i ".16b\n"
THROUGHPUT_MEM(cb, ldrq_sdot_lane)
#undef cb
#define cb(i) "ldr q1, [%[a_ptr]]\n"\
              "sdot v1.4s, v1.16b, v1.4b[0]\n"
LATENCY_MEM(cb, ldrq_sdot_lane)
#undef cb
// clang-format on

#endif

static void aarch64_dual_issue() {
    using namespace megpeak;
    benchmark(ins_ldd_throughput, ins_ldd_latency, "ins_ldd", 2,
              "Test ldd ins dual issue");
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(ldd_ldx_ins_throughput, ldd_ldx_ins_latency, "ldd_ldx_ins", 4);
    benchmark(ldqstq_throughput, ldqstq_latency, "ldqstq", 4,
              "Test ldq stq dual issue");
#endif
    benchmark(ldq_fmlaq_throughput, ldq_fmlaq_latency, "ldq_fmlaq", 8);
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(stq_fmlaq_lane_throughput, stq_fmlaq_lane_latency,
              "stq_fmlaq_lane", 8, "Test stq fmlaq_lane dual issue");

    benchmark(ldd_fmlad_throughput, ldd_fmlad_latency, "ldd_fmlad", 4,
              "Test ldd fmlad dual issue");
    benchmark(ldq_fmlaq_sep_throughput, ldq_fmlaq_sep_latency, "ldq_fmlaq_sep",
              8, "Test throughput ldq + 2 x fmlaq");
    benchmark(ldq_fmlaq_lane_sep_throughput, ldq_fmlaq_lane_sep_latency,
              "ldq_fmlaq_lane_sep", 8,
              "Test compute throughput ldq + 2 x fmlaq_lane");
#endif
    benchmark(ldd_fmlaq_sep_throughput, ldd_fmlaq_sep_latency, "ldd_fmlaq_sep",
              8, "Test compute throughput ldq + fmlaq");
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(lds_fmlaq_lane_sep_throughput, lds_fmlaq_lane_sep_latency,
              "lds_fmlaq_lane_sep", 8);
#endif
    benchmark(ldd_fmlaq_lane_sep_throughput, ldd_fmlaq_lane_sep_latency,
              "ldd_fmlaq_lane_sep", 8,
              "Test compute throughput ldd + fmlaq_lane");
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(ldx_fmlaq_lane_sep_throughput, ldx_fmlaq_lane_sep_latency,
              "ldx_fmlaq_lane_sep", 8);
#endif
    benchmark(ldd_ldx_ins_fmlaq_lane_sep_throughput,
              ldd_ldx_ins_fmlaq_lane_sep_latency, "ldd_ldx_ins_fmlaq_lane_sep",
              8, "Test compute throughput ldd+fmlaq+ldx+fmlaq+ins+fmlaq");
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(ldd_nop_ldx_ins_fmlaq_lane_sep_throughput,
              ldd_nop_ldx_ins_fmlaq_lane_sep_latency,
              "ldd_nop_ldx_ins_fmlaq_lane_sep", 8);
#endif
    benchmark(ins_fmlaq_lane_1_4_sep_throughput, ins_fmlaq_lane_1_4_sep_latency,
              "ins_fmlaq_lane_1_4_sep", 8,
              "Test compute throughput ins + 4 x fmlaq_lane");
    benchmark(ldd_fmlaq_lane_1_4_sep_throughput, ldd_fmlaq_lane_1_4_sep_latency,
              "ldd_fmlaq_lane_1_4_sep", 8,
              "Test compute throughput ldd + 4 x fmlaq_lane");
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(ldq_fmlaq_lane_1_4_sep_throughput, ldq_fmlaq_lane_1_4_sep_latency,
              "ldq_fmlaq_lane_1_4_sep", 8,
              "Test compute throughput ldq + 4 x fmlaq_lane");
    benchmark(ins_fmlaq_lane_1_3_sep_throughput, ins_fmlaq_lane_1_3_sep_latency,
              "ins_fmlaq_lane_1_3_sep", 8,
              "Test compute throughput ins + 3 x fmlaq_lane");
    benchmark(ldd_fmlaq_lane_1_3_sep_throughput, ldd_fmlaq_lane_1_3_sep_latency,
              "ldd_fmlaq_lane_1_3_sep", 8);
    benchmark(ldq_fmlaq_lane_1_3_sep_throughput, ldq_fmlaq_lane_1_3_sep_latency,
              "ldq_fmlaq_lane_1_3_sep", 8,
              "Test compute throughput ldq + 3 x fmlaq_lane");
    benchmark(ldq_fmlaq_lane_1_2_sep_throughput, ldq_fmlaq_lane_1_2_sep_latency,
              "ldq_fmlaq_lane_1_2_sep", 8,
              "Test compute throughput ldq + 2 x fmlaq_lane");
#endif
    benchmark(ins_fmlaq_lane_sep_throughput, ins_fmlaq_lane_sep_latency,
              "ins_fmlaq_lane_sep", 8);
#if MEGPEAK_WITH_ALL_BENCHMARK
    benchmark(dupd_fmlaq_lane_sep_throughput, dupd_fmlaq_lane_sep_latency,
              "dupd_fmlaq_lane_sep", 8);
    benchmark(smlal_8b_addp_throughput, smlal_8b_addp_latency, "smlal_8b_addp",
              16);
    benchmark(smlal_8b_dupd_throughput, smlal_8b_dupd_latency, "smlal_8b_dupd",
              16);
    benchmark(ldd_smlalq_sep_throughput, ldd_smlalq_sep_latency,
              "ldd_smlalq_sep_8b", 16, "Test ldd smlalq dual issue");
    benchmark(ldq_smlalq_sep_throughput, ldq_smlalq_sep_latency,
              "ldq_smlalq_sep", 16, "Test ldq smlalq dual issue");
    benchmark(lddx2_smlalq_sep_throughput, lddx2_smlalq_sep_latency,
              "lddx2_smlalq_sep", 16);
    benchmark(smlal_sadalp_throughput, smlal_sadalp_latency, "smlal_sadalp",
              16);
    benchmark(smull_smlal_sadalp_throughput, smull_smlal_sadalp_latency,
              "smull_smlal_sadalp", 32, "Test smull smlal dual issue");
    benchmark(smull_smlal_sadalp_sep_throughput, smull_smlal_sadalp_sep_latency,
              "smull_smlal_sadalp_sep", 16);
    benchmark(ins_smlalq_sep_1_2_throughput, ins_smlalq_sep_1_2_latency,
              "ins_smlalq_sep_1_2", 16);
    benchmark(ldx_ins_smlalq_sep_throughput, ldx_ins_smlalq_sep_latency,
              "ldx_ins_smlalq_sep", 16);

#if __ARM_FEATURE_DOTPROD
    benchmark(ldrd_sdot_lane_throughput, ldrd_sdot_lane_latency,
              "ldrd_sdot_lane", 32);
    benchmark(ldrq_sdot_lane_throughput, ldrq_sdot_lane_latency,
              "ldrq_sdot_lane", 32);
#endif
    benchmark(dupd_lane_smlal_s8_throughput, dupd_lane_smlal_s8_latency,
              "dupd_lane_smlal_s8", 16);

    benchmark(ldd_mla_s16_lane_1_4_sep_throughput,
              ldd_mla_s16_lane_1_4_sep_latency, "ldd_mla_s16_lane_1_4_sep", 16);

    benchmark(ldrd_sshll_throughput, ldrd_sshll_latency, "ldrd_sshll", 8);
    benchmark(sshll_ins_sep_throughput, sshll_ins_sep_latency, "sshll_ins_sep",
              8);
#endif
}
#endif
