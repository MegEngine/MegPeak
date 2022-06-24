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

#ifdef MEGPEAK_WITH_OPENCL

#include <MCL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <string>

#define opencl_check(_expr)                                             \
    do {                                                                \
        cl_int _err = (_expr);                                          \
        if (_err != CL_SUCCESS) {                                       \
            fprintf(stderr, "opencl error %d: %s (%s at %s:%s:%d)\n",   \
                    int(_err), megpeak::get_error_string(_err), #_expr, \
                    __FILE__, __func__, __LINE__);                      \
            exit(1);                                                    \
        }                                                               \
    } while (0)

namespace megpeak {
const char* get_error_string(cl_int error);

template <typename T, cl_device_info info>
struct DeviceInfoGetter {
    static T get(cl_device_id dev_id);
};

/**
 * \brief N-dimensional index space, where N is one, two or three.
 */
class NDRange {
private:
    size_t m_size[3];
    size_t m_dimension;

public:
    //! \brief resulting range has zero dimensions.
    NDRange() : m_dimension(0) {
        m_size[0] = 0;
        m_size[1] = 0;
        m_size[2] = 0;
    }

    //! \brief Constructs one-dimensional range.
    NDRange(size_t size0) : m_dimension(1) {
        m_size[0] = size0;
        m_size[1] = 1;
        m_size[2] = 1;
    }

    //! \brief Constructs two-dimensional range.
    NDRange(size_t size0, size_t size1) : m_dimension(2) {
        m_size[0] = size0;
        m_size[1] = size1;
        m_size[2] = 1;
    }

    //! \brief Constructs three-dimensional range.
    NDRange(size_t size0, size_t size1, size_t size2) : m_dimension(3) {
        m_size[0] = size0;
        m_size[1] = size1;
        m_size[2] = size2;
    }

    bool operator<(const NDRange& rhs) const;

    //! \brief Queries the number of dimensions in the range.
    size_t dimension() const { return m_dimension; }

    //! \brief Returns the size of the object in bytes based on the
    // runtime number of dimensions
    size_t size() const { return m_dimension * sizeof(size_t); }

    size_t* get() { return m_dimension ? m_size : nullptr; }

    size_t operator[](size_t idx);
    size_t operator[](size_t idx) const {
        return const_cast<NDRange*>(this)->operator[](idx);
    };

    const size_t* get() const { return const_cast<NDRange*>(this)->get(); }

    size_t total_size() const { return m_size[0] * m_size[1] * m_size[2]; }
};

struct DeviceInfo {
    std::string device_name;
    std::string driver_version;
    std::string extensions;

    uint32_t num_cus;
    uint32_t max_wg_size;
    uint32_t max_alloc_size;
    uint32_t max_global_size;
    uint32_t max_clock_freq;

    bool half_supported;
    bool double_supported;
};

class OpenCLEnv;
class Runner {
public:
    Runner(const OpenCLEnv& env) : m_env{env} {};
    virtual void run() = 0;

protected:
    float run_kernel(cl_kernel kern, const NDRange& global_size,
                     const NDRange& local_size, size_t iters);
    const OpenCLEnv& m_env;
};

class LocalMemRunner : public Runner {
public:
    LocalMemRunner(const OpenCLEnv& env);
    void run() override;

protected:
    void run_step(cl_kernel kernel, size_t global_size, size_t local_size,
                  size_t nr_elems, size_t iters, const std::string& tag);

};

class GlobalMemRunner : public Runner {
public:
    GlobalMemRunner(const OpenCLEnv& env);
    void run() override;

protected:
    void run_step(cl_kernel kernel_go, cl_kernel kernel_lo, size_t global_size,
                  size_t local_size, size_t nr_elems, size_t iters,
                  const std::string& tag);
};

template <typename T>
class CompRunner : public Runner {
public:
    CompRunner(const OpenCLEnv& env);
    void run() override;

protected:
    void run_step(cl_kernel kernel, size_t global_size, size_t local_size,
                  size_t iters, const std::string& tag);
};

/*************************************
 * * cl_qcom_perf_hint extension *
 * *************************************/
#define CL_PERF_HINT_NONE_QCOM 0
typedef cl_uint cl_perf_hint;
#define CL_CONTEXT_PERF_HINT_QCOM 0x40C2
/*cl_perf_hint*/
#define CL_PERF_HINT_HIGH_QCOM 0x40C3
#define CL_PERF_HINT_NORMAL_QCOM 0x40C4
#define CL_PERF_HINT_LOW_QCOM 0x40C5
/*************************************
 * * cl_qcom_priority_hint extension *
 * *************************************/
#define CL_PRIORITY_HINT_NONE_QCOM 0
typedef cl_uint cl_priority_hint;
#define CL_CONTEXT_PRIORITY_HINT_QCOM 0x40C9
/*cl_priority_hint*/
#define CL_PRIORITY_HINT_HIGH_QCOM 0x40CA
#define CL_PRIORITY_HINT_NORMAL_QCOM 0x40CB
#define CL_PRIORITY_HINT_LOW_QCOM 0x40CC

class OpenCLEnv {
public:
    OpenCLEnv(size_t dev_idx);
    ~OpenCLEnv() {
        clReleaseCommandQueue(m_queue);
        clReleaseDevice(m_dev_id);
        clReleaseContext(m_ctx);
    }

    cl_program build_program(const std::string& source_name) const;
    void print_device_info() const;
    void run() const;

private:
    cl_device_id m_dev_id;
    cl_context m_ctx;
    cl_platform_id m_platform;
    cl_command_queue m_queue;
    DeviceInfo m_dev_info;

    friend class Runner;
    friend class LocalMemRunner;
    friend class GlobalMemRunner;
    friend class CompRunner<cl_int>;
    friend class CompRunner<cl_half>;
    friend class CompRunner<cl_double>;
    friend class CompRunner<cl_float>;
};

// clang-format off
static std::unordered_map<std::string, const char*> PROGRAMS = {
        {"compute_dp_kernels",
#include "src/opencl/kernels/compute_dp_kernels.h"
        },
        {"compute_sp_kernels",
#include "src/opencl/kernels/compute_sp_kernels.h"
        },
        {"compute_hp_kernels",
#include "src/opencl/kernels/compute_hp_kernels.h"
        },
        {"compute_int_kernels",
#include "src/opencl/kernels/compute_integer_kernels.h"
        },
        {"global_memory_bandwidth",
#include "src/opencl/kernels/global_bandwidth_kernels.h"
        },
        {
                     "local_memory_bandwidth",
#include "src/opencl/kernels/local_memory_bandwidth.h"
        }
};
// clang-format on

}  // namespace megpeak

#endif

// vim: syntax=cpp.doxygen
