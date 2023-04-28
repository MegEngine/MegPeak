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

#pragma once
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <stdlib.h>
#include <string>

#ifndef __APPLE__
#include <malloc.h>
#endif

namespace megpeak {
constexpr static uint32_t RUNS = 800000;
#define MEGPEAK_ATTRIBUTE_TARGET(simd) __attribute__((target(simd)))

class Timer {
    std::chrono::high_resolution_clock::time_point m_start;

public:
    Timer() { reset(); }

    void reset() { m_start = std::chrono::high_resolution_clock::now(); }

    double get_secs() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now -
                                                                    m_start)
                       .count() *
               1e-9;
    }

    double get_nsecs() const { return get_secs() * 1e9; }

    double get_msecs() const { return get_secs() * 1e3; }

    double get_secs_reset() {
        auto ret = get_secs();
        reset();
        return ret;
    }

    double get_msecs_reset() { return get_secs_reset() * 1e3; }
};

inline static float get_relative_diff(float lhs, float rhs) {
    float abs_diff = std::fabs(rhs - lhs);
    float base = std::max(std::max(std::fabs(lhs), std::fabs(rhs)), 1.0f);
    float rel_diff = abs_diff / base;
    return rel_diff;
}

/**
 * latency:
 *
 *       xor reg, reg
 *       mov r0, count
 * loop:
 *       op reg, reg
 *       op reg, reg
 *       ...
 *       op reg, reg
 *       dec r0
 *       jne loop
 *
 *
 * throughput
 *
 *       xor reg0, reg0
 *       ....
 *       xor reg19, reg19
 *       mov r0, count
 * loop:
 *       op reg0, reg0
 *       op reg1, reg1
 *       ...
 *       op reg19, reg19
 *       dec 0r
 *       jne loop
 *
 */
inline static void benchmark(std::function<int()> throughtput_func,
                             std::function<int()> latency_func,
                             const char* inst, size_t inst_simd = 4,
                             std::string msg = "") {
    Timer timer;
    auto runs = throughtput_func();
    float throuphput_used = timer.get_nsecs() / runs;
    timer.reset();
    runs = latency_func();
    float latency_used = timer.get_nsecs() / runs;
    printf("%s throughput: %f ns %f GFlops latency: %f ns :%s\n", inst,
           throuphput_used, 1.f / throuphput_used * inst_simd, latency_used,
           msg.c_str());
}

#define UNROLL_RAW5(cb, v0, a...) \
    cb(0, ##a) cb(1, ##a) cb(2, ##a) cb(3, ##a) cb(4, ##a)

#define UNROLL_RAW8(cb, v0, a...) \
    UNROLL_RAW5(cb, v0, ##a)      \
    cb(5, ##a) cb(6, ##a) cb(7, ##a)

#define UNROLL_RAW9(cb, v0, a...) \
    UNROLL_RAW5(cb, v0, ##a)      \
    cb(5, ##a) cb(6, ##a) cb(7, ##a) cb(8, ##a)

#define UNROLL_RAW10(cb, v0, a...) \
    UNROLL_RAW5(cb, v0, ##a)       \
    cb(5, ##a) cb(6, ##a) cb(7, ##a) cb(8, ##a) cb(9, ##a)
#define UNROLL_RAW15(cb, v0, a...) \
    UNROLL_RAW10(cb, v0, ##a)      \
    cb(10, ##a) cb(11, ##a) cb(12, ##a) cb(13, ##a) cb(14, ##a)
#define UNROLL_RAW20(cb, v0, a...)                                          \
    UNROLL_RAW10(cb, v0, ##a)                                               \
    cb(10, ##a) cb(11, ##a) cb(12, ##a) cb(13, ##a) cb(14, ##a) cb(15, ##a) \
            cb(16, ##a) cb(17, ##a) cb(18, ##a) cb(19, ##a)

#define UNROLL_RAW5_START6(cb, v0, a...) \
    cb(6, ##a) cb(7, ##a) cb(8, ##a) cb(9, ##a) cb(10, ##a)
#define UNROLL_RAW10_START6(cb, v0, a...) \
    UNROLL_RAW5_START6(cb, v0, ##a)       \
    cb(11, ##a) cb(12, ##a) cb(13, ##a) cb(14, ##a) cb(15, ##a)
#define UNROLL_RAW20_START6(cb, v0, a...)                                   \
    UNROLL_RAW10_START6(cb, v0, ##a)                                        \
    cb(16, ##a) cb(17, ##a) cb(18, ##a) cb(19, ##a) cb(20, ##a) cb(21, ##a) \
            cb(22, ##a) cb(23, ##a) cb(24, ##a) cb(25, ##a)

#define UNROLL_CALL0(step, cb, v...) UNROLL_RAW##step(cb, 0, ##v)
#define UNROLL_CALL(step, cb, v...) UNROLL_CALL0(step, cb, ##v)
//! As some arm instruction, the second/third operand must be [d0-d7], so the
//! iteration should start from a higher number, otherwise may cause data
//! dependence
#define UNROLL_CALL0_START6(step, cb, v...) \
    UNROLL_RAW##step##_START6(cb, 0, ##v)
#define UNROLL_CALL_START6(step, cb, v...) UNROLL_CALL0_START6(step, cb, ##v)

// dec to bin
#define _DEC_BIN_0 "00000"
#define _DEC_BIN_1 "00001"
#define _DEC_BIN_2 "00010"
#define _DEC_BIN_3 "00011"
#define _DEC_BIN_4 "00100"
#define _DEC_BIN_5 "00101"
#define _DEC_BIN_6 "00110"
#define _DEC_BIN_7 "00111"
#define _DEC_BIN_8 "01000"
#define _DEC_BIN_9 "01001"
#define _DEC_BIN_10 "01010"
#define _DEC_BIN_11 "01011"
#define _DEC_BIN_12 "01100"
#define _DEC_BIN_13 "01101"
#define _DEC_BIN_14 "01110"
#define _DEC_BIN_15 "01111"
#define _DEC_BIN_16 "10000"
#define _DEC_BIN_17 "10001"
#define _DEC_BIN_18 "10010"
#define _DEC_BIN_19 "10011"
#define _DEC_BIN_20 "10100"
#define _DEC_BIN_21 "10101"
#define _DEC_BIN_22 "10110"
#define _DEC_BIN_23 "10111"
#define _DEC_BIN_24 "11000"
#define _DEC_BIN_25 "11001"
#define _DEC_BIN_26 "11010"
#define _DEC_BIN_27 "11011"
#define _DEC_BIN_28 "11100"
#define _DEC_BIN_29 "11101"
#define _DEC_BIN_30 "11110"
#define _DEC_BIN_31 "11111"
#define DEC_TO_BIN(val) _DEC_BIN_##val

// new instruction op code & flag
#define sdot_code "01001111100" // code[1] = 0 if compute 2s/8b; code[-1] = 1 if offset = 1/3
#define sdot_flag "111000"  // flag[4] = 1 if offset = 2/3
#define smmla_code "01001110100"
#define smmla_flag "101001"
#define bfmmla_code "01101110010"
#define bfmmla_flag "111011"

#define AARCH64_BINARY_INST(op, dst, src1, src2) ".inst 0b" op##_code DEC_TO_BIN(dst) op##_flag DEC_TO_BIN(src1) DEC_TO_BIN(src2) "\n"
// new instruction define
#define SDOT(vd, vn, vm) AARCH64_BINARY_INST(sdot, vm, vn, vd)
#define SMMLA(vd, vn, vm) AARCH64_BINARY_INST(smmla, vm, vn, vd)
#define BFMMLA(vd, vn, vm) AARCH64_BINARY_INST(bfmmla, vm, vn, vd)

void aarch64();
void armv7();
void x86_avx();
void x86_sse();
}  // namespace megpeak
namespace {
/**
 * @brief Get the mem align64 object, only for single thread
 *
 * @return void*
 */
static inline void* get_mem_align64() {
    static void* mem_algn64 = nullptr;
    size_t alignment = 64;
    size_t size = 2048;
    if (mem_algn64 == nullptr) {
#ifdef WIN32
        mem_algn64 = _aligned_malloc(size, alignment);
#elif defined(__ANDROID__) || defined(ANDROID)
        mem_algn64 = memalign(alignment, size);
#else
        posix_memalign(&mem_algn64, alignment, size);
#endif
    }
    return mem_algn64;
}
}  // namespace

// vim: syntax=cpp.doxygen
