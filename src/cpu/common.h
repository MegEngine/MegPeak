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
#include <chrono>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>
#include <functional>

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
                             const char* inst, size_t inst_simd = 4) {
    Timer timer;
    auto runs = throughtput_func();
    float throuphput_used = timer.get_nsecs() / runs;
    timer.reset();
    runs = latency_func();
    float latency_used = timer.get_nsecs() / runs;
    printf("%s throughput: %f ns %f GFlops latency: %f ns\n", inst,
           throuphput_used, 1.f / throuphput_used * inst_simd, latency_used);
}

#define UNROLL_RAW5(cb, v0, a...) \
    cb(0, ##a) cb(1, ##a) cb(2, ##a) cb(3, ##a) cb(4, ##a)
#define UNROLL_RAW10(cb, v0, a...) \
    UNROLL_RAW5(cb, v0, ##a)       \
    cb(5, ##a) cb(6, ##a) cb(7, ##a) cb(8, ##a) cb(9, ##a)
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

void aarch64();
void armv7();
void x86_avx();
void x86_sse();
}  // namespace megpeak

// vim: syntax=cpp.doxygen
