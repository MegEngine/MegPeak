/**
 * \file src/cpu/backend.cpp
 *
 * \brief This is benchmark for MegPeak
 *
 * \copyright Copyright (c) 2019-2019 Megvii Inc. All rights reserved.
 */

#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <memory>

#include "src/cpu/common.h"
#include "src/backend.h"

using namespace megpeak;
namespace {
uint8_t bandwidth() {
    constexpr size_t NR_WARMUP = 5, NR_RUNS = 40, NR_BYTES = 1024 * 1024 * 100;
    std::unique_ptr<uint8_t[]> src{new uint8_t[NR_BYTES]};
    std::unique_ptr<uint8_t[]> dst{new uint8_t[NR_BYTES]};

    volatile uint8_t res = 0;
    for (size_t i = 0; i < NR_WARMUP; i++) {
        memcpy(dst.get(), src.get(), NR_BYTES);
        res += dst[0];
    }

    Timer timer;
    for (size_t i = 0; i < NR_RUNS; i++) {
        memcpy(dst.get(), src.get(), NR_BYTES);
        res += dst[0];
    }
    float used = timer.get_msecs() / NR_RUNS;
    printf("bandwidth: %f Gbps\n",
           2 * NR_BYTES / (1024.0 * 1024.0 * 1024.0) * 1000 / used);
    return res;
}

void cpu_set_affinity(int dev_id) {
    cpu_set_t cst;
    CPU_ZERO(&cst);
    CPU_SET(dev_id, &cst);
    sched_setaffinity(0, sizeof(cst), &cst);
}

void get_cpu_info() {
    // cpu_set_t cst;
    // CPU_ZERO(&cst);
    // sched_getaffinity(0, sizeof(cst), &cst);
    size_t cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    printf("there are %zu cores\n", cpu_num);
}

}  // namespace

void CPUBackend::execute() {
    get_cpu_info();
    cpu_set_affinity(m_dev_id);
    printf("currently use core %zu\n", m_dev_id);
    bandwidth();
    aarch64();
    armv7();
    x86_avx();
    x86_sse();
}

// vim: syntax=cpp.doxygen
