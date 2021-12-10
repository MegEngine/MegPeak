/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 */

#ifdef MEGPEAK_WITH_OPENCL
#include "src/opencl/common.h"
#include "src/backend.h"

#include <vector>
#include <cstring>
#include <string>
#include <cmath>

using namespace megpeak;
using cl_int1 = cl_int;

namespace {
static NDRange NullNDRange;

std::string to_string(const std::vector<char> str) {
    std::string ret(str.begin(), str.end());
    return ret;
}

template <typename T>
void populate(std::vector<T>& arr) {
    for (size_t i = 0; i < arr.size(); i++) {
        arr[i] = i;
    }
}

size_t get_max_work_group_size(cl_device_id dev_id, cl_kernel kern) {
    size_t lws = 0;
    opencl_check(clGetKernelWorkGroupInfo(kern, dev_id,
                                          CL_KERNEL_WORK_GROUP_SIZE,
                                          sizeof(size_t), &lws, nullptr));
    return lws;
}

uint32_t round_to_power2(uint32_t number) {
    return pow(2, floorf(::log2f(number)));
}

NDRange get_global_size_divdown(const NDRange& gs, const NDRange& ls) {
    NDRange ret = gs;
    for (size_t i = 0; i < gs.dimension(); i++) {
        size_t local_size = ls.get()[i];
        size_t origin = gs.get()[i];
        ret.get()[i] = origin / local_size * local_size;
    }
    return ret;
}

}  // namespace

const char* megpeak::get_error_string(cl_int error) {
#define ERROR(_err) \
    case _err:      \
        return #_err;

    switch (error) {
        ERROR(CL_SUCCESS)
        ERROR(CL_DEVICE_NOT_FOUND)
        ERROR(CL_DEVICE_NOT_AVAILABLE)
        ERROR(CL_COMPILER_NOT_AVAILABLE)
        ERROR(CL_MEM_OBJECT_ALLOCATION_FAILURE)
        ERROR(CL_OUT_OF_RESOURCES)
        ERROR(CL_OUT_OF_HOST_MEMORY)
        ERROR(CL_PROFILING_INFO_NOT_AVAILABLE)
        ERROR(CL_MEM_COPY_OVERLAP)
        ERROR(CL_IMAGE_FORMAT_MISMATCH)
        ERROR(CL_IMAGE_FORMAT_NOT_SUPPORTED)
        ERROR(CL_BUILD_PROGRAM_FAILURE)
        ERROR(CL_MAP_FAILURE)
        ERROR(CL_MISALIGNED_SUB_BUFFER_OFFSET)
        ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        ERROR(CL_COMPILE_PROGRAM_FAILURE)
        ERROR(CL_LINKER_NOT_AVAILABLE)
        ERROR(CL_LINK_PROGRAM_FAILURE)
        ERROR(CL_DEVICE_PARTITION_FAILED)
        ERROR(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
        ERROR(CL_INVALID_VALUE)
        ERROR(CL_INVALID_DEVICE_TYPE)
        ERROR(CL_INVALID_PLATFORM)
        ERROR(CL_INVALID_DEVICE)
        ERROR(CL_INVALID_CONTEXT)
        ERROR(CL_INVALID_QUEUE_PROPERTIES)
        ERROR(CL_INVALID_COMMAND_QUEUE)
        ERROR(CL_INVALID_HOST_PTR)
        ERROR(CL_INVALID_MEM_OBJECT)
        ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        ERROR(CL_INVALID_IMAGE_SIZE)
        ERROR(CL_INVALID_SAMPLER)
        ERROR(CL_INVALID_BINARY)
        ERROR(CL_INVALID_BUILD_OPTIONS)
        ERROR(CL_INVALID_PROGRAM)
        ERROR(CL_INVALID_PROGRAM_EXECUTABLE)
        ERROR(CL_INVALID_KERNEL_NAME)
        ERROR(CL_INVALID_KERNEL_DEFINITION)
        ERROR(CL_INVALID_KERNEL)
        ERROR(CL_INVALID_ARG_INDEX)
        ERROR(CL_INVALID_ARG_VALUE)
        ERROR(CL_INVALID_ARG_SIZE)
        ERROR(CL_INVALID_KERNEL_ARGS)
        ERROR(CL_INVALID_WORK_DIMENSION)
        ERROR(CL_INVALID_WORK_GROUP_SIZE)
        ERROR(CL_INVALID_WORK_ITEM_SIZE)
        ERROR(CL_INVALID_GLOBAL_OFFSET)
        ERROR(CL_INVALID_EVENT_WAIT_LIST)
        ERROR(CL_INVALID_EVENT)
        ERROR(CL_INVALID_OPERATION)
        ERROR(CL_INVALID_GL_OBJECT)
        ERROR(CL_INVALID_BUFFER_SIZE)
        ERROR(CL_INVALID_MIP_LEVEL)
        ERROR(CL_INVALID_GLOBAL_WORK_SIZE)
        ERROR(CL_INVALID_PROPERTY)
        ERROR(CL_INVALID_IMAGE_DESCRIPTOR)
        ERROR(CL_INVALID_COMPILER_OPTIONS)
        ERROR(CL_INVALID_LINKER_OPTIONS)
        ERROR(CL_INVALID_DEVICE_PARTITION_COUNT)

        default:
            return "unknown error";
    }
}

////////////////////////////// NDRange /////////////////////////////////

size_t NDRange::operator[](size_t idx) {
    megpeak_assert(idx < m_dimension, "invalid index: %zu expected < %zu", idx,
                   m_dimension);
    return m_size[idx];
}

bool NDRange::operator<(const NDRange& rhs) const {
    return std::tie(m_dimension, m_size[0], m_size[1], m_size[2]) <
           std::tie(rhs.m_dimension, rhs.m_size[0], rhs.m_size[1],
                    rhs.m_size[2]);
}

////////////////////////////// DeviceInfo //////////////////////////////
namespace megpeak {
template <typename T, cl_device_info info>
struct DeviceInfoGetter<T[], info> {
    static std::vector<T> get(cl_device_id dev_id) {
        size_t res_size = 0;
        opencl_check(clGetDeviceInfo(dev_id, info, 0, nullptr, &res_size));
        std::vector<T> res(res_size, T{0});
        opencl_check(clGetDeviceInfo(dev_id, info, res_size, &res[0], nullptr));
        return res;
    }
};

#define cb(_type)                                                           \
    template <cl_device_info info>                                          \
    struct DeviceInfoGetter<_type, info> {                                  \
        static _type get(cl_device_id dev_id) {                             \
            _type res = 0;                                                  \
            opencl_check(clGetDeviceInfo(dev_id, info, sizeof(_type), &res, \
                                         nullptr));                         \
            return res;                                                     \
        }                                                                   \
    };
cb(cl_uint) cb(cl_ulong)
#undef cb
}  // namespace megpeak

//////////////////////////////////// Runner //////////////////////////////////
float Runner::run_kernel(cl_kernel kern, const NDRange& global_size,
                         const NDRange& local_size, size_t iters) {
    megpeak_assert(global_size.dimension() == local_size.dimension() &&
                           local_size.dimension() == 1,
                   "only support 1-dim ndrange, got: %zu",
                   local_size.dimension());
    auto gs = get_global_size_divdown(global_size, local_size);
    float used = 0;
    opencl_check(clEnqueueNDRangeKernel(
            m_env.m_queue, kern, global_size.dimension(), nullptr,
            gs.get(), local_size.get(), 0, nullptr, nullptr));
    clFinish(m_env.m_queue);

    cl_event ev;
    for (size_t i = 0; i < iters; i++) {
        opencl_check(clEnqueueNDRangeKernel(
                m_env.m_queue, kern, global_size.dimension(), nullptr,
                gs.get(), local_size.get(), 0, nullptr, &ev));
        opencl_check(clWaitForEvents(1, &ev));
        cl_ulong start, end;
        opencl_check(clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START,
                                             sizeof(cl_ulong), &start,
                                             nullptr));
        opencl_check(clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END,
                                             sizeof(cl_ulong), &end, nullptr));

        used += (end - start) / 1000.0;
    }
    opencl_check(clReleaseEvent(ev));
    return used / iters;
}

///////////////////////// LocalMemRunner /////////////////////////////////////
LocalMemRunner::LocalMemRunner(const OpenCLEnv& env) : Runner(env) {}

void LocalMemRunner::run() {
    constexpr static size_t nr_elems = 1024 * 1024;
    constexpr static size_t iters = 50;
    size_t block_size = m_env.m_dev_info.max_wg_size;

    std::vector<int> elems(nr_elems, 0);
    populate(elems);
    printf("Local memory bandwidth (GBPS)\n");

    cl_int err;
    cl_mem src = clCreateBuffer(m_env.m_ctx, CL_MEM_READ_WRITE,
                                nr_elems * sizeof(int), nullptr, &err);
    opencl_check(err);
    opencl_check(clEnqueueWriteBuffer(m_env.m_queue, src, CL_TRUE, 0,
                                      nr_elems * sizeof(int), elems.data(), 0,
                                      nullptr, nullptr));

    cl_program program = m_env.build_program("local_memory_bandwidth");

#define cb(i)                                                                  \
    auto kernel_v##i =                                                         \
            clCreateKernel(program, "krn_localmem_juggling_int" #i, &err);     \
    block_size = std::min<size_t>(                                             \
            block_size, get_max_work_group_size(m_env.m_dev_id, kernel_v##i)); \
    opencl_check(err);                                                         \
    err = clSetKernelArg(kernel_v##i, 0, sizeof(cl_mem), &src);                \
    err |= clSetKernelArg(kernel_v##i, 1, block_size * sizeof(cl_int##i) * 6,  \
                          nullptr);                                            \
    opencl_check(err);                                                         \
    run_step(kernel_v##i, nr_elems / i, block_size, nr_elems, iters,           \
             "int" #i);                                                        \
    clReleaseKernel(kernel_v##i);

    cb(1);
    cb(2);
    cb(4);
#undef cb
    clReleaseProgram(program);
}

void LocalMemRunner::run_step(cl_kernel kernel, size_t global_size,
                              size_t local_size, size_t nr_elems, size_t iters,
                              const std::string& tag) {
    float used = run_kernel(kernel, global_size, local_size, iters);

    constexpr size_t TOTAL_ITERATIONS = 16;
    float memory =
            (6LL + 4 * 5 * TOTAL_ITERATIONS + 6) * nr_elems * sizeof(int);
    float gdps = memory / used / 1e3;
    printf("    %s : %f\n", tag.c_str(), gdps);
}

///////////////////////// GlobalMemRunner /////////////////////////////////////
GlobalMemRunner::GlobalMemRunner(const OpenCLEnv& env) : Runner(env) {}

void GlobalMemRunner::run() {
    constexpr static size_t FETCH_PER_WI = 16;
    constexpr static size_t iters = 50;
    uint32_t nr_elems = m_env.m_dev_info.max_alloc_size / sizeof(float) / 2;
    nr_elems = round_to_power2(nr_elems);

    std::vector<float> elems(nr_elems, 0);
    populate(elems);
    printf("Global memory bandwidth (GBPS)\n");
    size_t block_size = m_env.m_dev_info.max_wg_size;

    cl_int err;
    cl_mem src = clCreateBuffer(m_env.m_ctx, CL_MEM_READ_ONLY,
                                nr_elems * sizeof(float), nullptr, &err);
    opencl_check(err);
    cl_mem dst = clCreateBuffer(m_env.m_ctx, CL_MEM_WRITE_ONLY,
                                nr_elems * sizeof(float), nullptr, &err);
    opencl_check(err);
    opencl_check(clEnqueueWriteBuffer(m_env.m_queue, src, CL_TRUE, 0,
                                      nr_elems * sizeof(float), elems.data(), 0,
                                      nullptr, nullptr));

    cl_program program = m_env.build_program("global_memory_bandwidth");

#define cb(i)                                                               \
    auto kernel_v##i##_global = clCreateKernel(                             \
            program, "global_bandwidth_v" #i "_global_offset", &err);       \
    opencl_check(err);                                                      \
    block_size = std::min<size_t>(                                          \
            block_size,                                                     \
            get_max_work_group_size(m_env.m_dev_id, kernel_v##i##_global)); \
    err = clSetKernelArg(kernel_v##i##_global, 0, sizeof(cl_mem), &src);    \
    err |= clSetKernelArg(kernel_v##i##_global, 1, sizeof(cl_mem), &dst);   \
    opencl_check(err);                                                      \
    auto kernel_v##i##_local = clCreateKernel(                              \
            program, "global_bandwidth_v" #i "_local_offset", &err);        \
    opencl_check(err);                                                      \
    block_size = std::min<size_t>(                                          \
            block_size,                                                     \
            get_max_work_group_size(m_env.m_dev_id, kernel_v##i##_local));  \
    err = clSetKernelArg(kernel_v##i##_local, 0, sizeof(cl_mem), &src);     \
    err |= clSetKernelArg(kernel_v##i##_local, 1, sizeof(cl_mem), &dst);    \
    opencl_check(err);                                                      \
    run_step(kernel_v##i##_global, kernel_v##i##_local,                     \
             nr_elems / i / FETCH_PER_WI, block_size, nr_elems, iters,      \
             "float" #i);                                                   \
    clReleaseKernel(kernel_v##i##_global);                                  \
    clReleaseKernel(kernel_v##i##_local);

    cb(1);
    cb(2);
    cb(4);
    cb(8);
    cb(16);
#undef cb
    clReleaseProgram(program);
}

void GlobalMemRunner::run_step(cl_kernel kernel_go, cl_kernel kernel_lo,
                               size_t global_size, size_t local_size,
                               size_t nr_elems, size_t iters,
                               const std::string& tag) {
    float used_go = run_kernel(kernel_go, global_size, local_size, iters);
    float used_lo = run_kernel(kernel_lo, global_size, local_size, iters);

    float used = std::min<float>(used_lo, used_go);
    float gdps = static_cast<float>(nr_elems) * sizeof(float) / used / 1e3;
    printf("    %s : %f\n", tag.c_str(), gdps);
}

///////////////////////// CompRunner /////////////////////////////////////
namespace megpeak {
template <typename T>
CompRunner<T>::CompRunner(const OpenCLEnv& env) : Runner(env) {}

template <typename T>
void CompRunner<T>::run() {
    std::string src_tag, kernel_tag, tag;
    if (std::is_same<T, cl_int>::value) {
        src_tag = "compute_int_kernels";
        kernel_tag = "compute_integer_v";
        tag = "int";
    } else if (std::is_same<T, cl_half>::value) {
        if (!m_env.m_dev_info.half_supported) {
            fprintf(stderr, "half not suppprted, skip\n");
            return;
        }
        src_tag = "compute_hp_kernels";
        kernel_tag = "compute_hp_v";
        tag = "half";
    } else if (std::is_same<T, cl_float>::value) {
        src_tag = "compute_sp_kernels";
        kernel_tag = "compute_sp_v";
        tag = "float";
    } else if (std::is_same<T, cl_double>::value) {
        if (!m_env.m_dev_info.double_supported) {
            fprintf(stderr, "double not suppprted, skip\n");
            return;
        }
        src_tag = "compute_dp_kernels";
        kernel_tag = "compute_dp_v";
        tag = "double";
    } else {
        fprintf(stderr, "unknown type, skip\n");
        return;
    }

    constexpr static size_t iters = 10;
    size_t global_size =
            m_env.m_dev_info.num_cus * 2048 * m_env.m_dev_info.max_wg_size;
    global_size = std::min<size_t>(global_size * sizeof(T),
                                   m_env.m_dev_info.max_alloc_size);
    global_size = round_to_power2(global_size);
    global_size = global_size / sizeof(T);
    size_t block_size = m_env.m_dev_info.max_wg_size;

    std::vector<T> elems(global_size, 0);
    printf("%s compute (GFLOPS)\n", tag.c_str());

    cl_int err;
    cl_mem dst = clCreateBuffer(m_env.m_ctx, CL_MEM_WRITE_ONLY,
                                global_size * sizeof(T), nullptr, &err);
    opencl_check(err);
    cl_program program = m_env.build_program(src_tag);
#if MEGPEAK_AARCH64 || MEGPEAK_ARMV7
    //! As in android, the opencl driver may cause CL_INVALID_ARG_SIZE if we
    //! pass a as half in half compute, so here we make dtype of a float.
    //! we should not make a as float in half compute in k8.
    //! FIXME: I don't known why.
    float a = static_cast<float>(1.3f);
#else
    T a = static_cast<T>(1.3f);
#endif

#define cb(i)                                                                  \
    auto kernel_v##i = clCreateKernel(                                         \
            program, (kernel_tag + std::to_string(i)).c_str(), &err);          \
    block_size = std::min<size_t>(                                             \
            block_size, get_max_work_group_size(m_env.m_dev_id, kernel_v##i)); \
    opencl_check(err);                                                         \
    err = clSetKernelArg(kernel_v##i, 0, sizeof(cl_mem), &dst);                \
    err |= clSetKernelArg(kernel_v##i, 1, sizeof(a), &a);                      \
    opencl_check(err);                                                         \
    run_step(kernel_v##i, global_size, block_size, iters,                      \
             tag + std::to_string(i));                                         \
    clReleaseKernel(kernel_v##i);

    cb(1);
    cb(2);
    cb(4);
    cb(8);
    cb(16);
#undef cb
    clReleaseProgram(program);
}

template <typename T>
void CompRunner<T>::run_step(cl_kernel kernel, size_t global_size,
                             size_t local_size, size_t iters,
                             const std::string& tag) {
    constexpr static size_t work_per_wi = 4096;
    float used = run_kernel(kernel, global_size, local_size, iters);
    float gflops = static_cast<float>(global_size) * work_per_wi / used / 1e3;
    printf("    %s : %f\n", tag.c_str(), gflops);
}

}  // namespace megpeak

////////////////////////////// OpenCLEnv //////////////////////////////
OpenCLEnv::OpenCLEnv(size_t dev_idx) {
    cl_uint nr_platforms = 0;

    opencl_check(clGetPlatformIDs(0, nullptr, &nr_platforms));
    megpeak_assert(nr_platforms > 0, "No opencl platform: %u", nr_platforms);

    std::vector<cl_platform_id> platform_ids(nr_platforms);
    opencl_check(clGetPlatformIDs(nr_platforms, platform_ids.data(), nullptr));

    cl_uint nr_devices = 0;
    std::vector<cl_device_id> device_ids;
    for (auto pid : platform_ids) {
        opencl_check(clGetDeviceIDs(pid, CL_DEVICE_TYPE_GPU, 0, nullptr,
                                    &nr_devices));
        if (!nr_devices) {
            continue;
        }

        device_ids.resize(nr_devices);
        opencl_check(clGetDeviceIDs(pid, CL_DEVICE_TYPE_GPU, nr_devices,
                                    device_ids.data(), nullptr));
        m_platform = pid;
        break;
    }
    megpeak_assert(dev_idx < device_ids.size(),
                   "Invalid device id: %zu, max size is %zu", dev_idx,
                   device_ids.size());
    m_dev_id = device_ids[dev_idx];

    size_t property_pos = 0;
    cl_context_properties context_properties[32];

    auto add_prop = [&](cl_context_properties key, cl_context_properties val) {
        context_properties[property_pos++] = key;
        context_properties[property_pos++] = val;
        megpeak_assert(
                property_pos < sizeof(context_properties) /
                                       sizeof(context_properties[0]),
                "invalid property_pos(%zu) > max_prop_size(%zu)", property_pos,
                sizeof(context_properties) / sizeof(context_properties[0]));
    };

    add_prop(CL_CONTEXT_PLATFORM, (cl_context_properties)m_platform);

    m_dev_info.extensions = to_string(
            DeviceInfoGetter<char[], CL_DEVICE_EXTENSIONS>::get(m_dev_id));
    if ((m_dev_info.extensions.find("cl_qcom_perf_hint") !=
         std::string::npos)) {
        add_prop(CL_CONTEXT_PERF_HINT_QCOM, CL_PERF_HINT_HIGH_QCOM);
        add_prop(CL_CONTEXT_PRIORITY_HINT_QCOM, CL_PRIORITY_HINT_HIGH_QCOM);
    }

    context_properties[property_pos++] = 0;

    cl_int err_num;
    m_ctx = clCreateContext(context_properties, 1, &m_dev_id, nullptr, nullptr,
                            &err_num);
    opencl_check(err_num);

    m_queue = clCreateCommandQueue(m_ctx, m_dev_id, CL_QUEUE_PROFILING_ENABLE,
                                   &err_num);
    opencl_check(err_num);

    // init device info
    m_dev_info.num_cus =
            DeviceInfoGetter<cl_uint, CL_DEVICE_MAX_COMPUTE_UNITS>::get(
                    m_dev_id);
    m_dev_info.device_name =
            to_string(DeviceInfoGetter<char[], CL_DEVICE_NAME>::get(m_dev_id));
    m_dev_info.driver_version = to_string(
            DeviceInfoGetter<char[], CL_DRIVER_VERSION>::get(m_dev_id));
    m_dev_info.extensions = to_string(
            DeviceInfoGetter<char[], CL_DEVICE_EXTENSIONS>::get(m_dev_id));

    m_dev_info.max_wg_size = std::max<size_t>(
            DeviceInfoGetter<size_t[], CL_DEVICE_MAX_WORK_ITEM_SIZES>::get(
                    m_dev_id)[0],
            256);
    // FIXME limit max-workgroup size for qualcomm platform to 128
    // Kernel launch fails for workgroup size 256(CL_DEVICE_MAX_WORK_ITEM_SIZES)
    std::string vendor = to_string(
            DeviceInfoGetter<char[], CL_DEVICE_VENDOR>::get(m_dev_id));
    if ((vendor.find("QUALCOMM") != std::string::npos) ||
        (vendor.find("qualcomm") != std::string::npos)) {
        m_dev_info.max_wg_size = std::min<size_t>(m_dev_info.max_wg_size, 128);
    }

    if ((vendor.find("NVIDIA") != std::string::npos) ||
        (vendor.find("nvidia") != std::string::npos)) {
        m_dev_info.max_wg_size = std::min<size_t>(m_dev_info.max_wg_size, 256);
    }

    m_dev_info.max_alloc_size = static_cast<uint32_t>(
            DeviceInfoGetter<cl_ulong, CL_DEVICE_MAX_MEM_ALLOC_SIZE>::get(
                    m_dev_id));
    m_dev_info.max_global_size = static_cast<uint32_t>(
            DeviceInfoGetter<cl_ulong, CL_DEVICE_GLOBAL_MEM_SIZE>::get(
                    m_dev_id));
    m_dev_info.max_clock_freq = static_cast<uint32_t>(
            DeviceInfoGetter<cl_uint, CL_DEVICE_MAX_CLOCK_FREQUENCY>::get(
                    m_dev_id));

    // set extensions
    m_dev_info.double_supported = false;
    m_dev_info.half_supported = false;
    if ((m_dev_info.extensions.find("cl_khr_fp16") != std::string::npos)) {
        m_dev_info.half_supported = true;
    }

    if ((m_dev_info.extensions.find("cl_khr_fp64") != std::string::npos) ||
        (m_dev_info.extensions.find("cl_amd_fp64") != std::string::npos)) {
        m_dev_info.double_supported = true;
    }
}

cl_program OpenCLEnv::build_program(const std::string& source_name) const {
    const char* program_buffer = megpeak::PROGRAMS[source_name];
    size_t program_size = strlen(program_buffer);

    cl_int err;
    cl_program program = clCreateProgramWithSource(
            m_ctx, 1, (const char**)&program_buffer, &program_size, &err);
    opencl_check(err);

    const char* compile_options = "-cl-fast-relaxed-math -cl-mad-enable";
    err = clBuildProgram(program, 1, &m_dev_id, compile_options, NULL, NULL);

    if (err != CL_SUCCESS) {
        size_t str_len = 0;
        std::string build_log;
        clGetProgramBuildInfo(program, m_dev_id, CL_PROGRAM_BUILD_LOG, 0,
                              nullptr, &str_len);
        build_log.resize(str_len);
        clGetProgramBuildInfo(program, m_dev_id, CL_PROGRAM_BUILD_LOG, str_len,
                              &build_log[0], &str_len);
        megpeak_assert(0, "clBuildProgram error: src=%s opt=%s errno=%s\n%s",
                       source_name.c_str(), compile_options, build_log.c_str(),
                       megpeak::get_error_string(err));
    }
    opencl_check(err);

    return program;
}

void OpenCLEnv::print_device_info() const {
    printf("Device: %s\n", m_dev_info.device_name.c_str());
    printf("Extensions: %s\n", m_dev_info.extensions.c_str());
    printf("Driver version: %s\n", m_dev_info.driver_version.c_str());
    printf("Compute units: %u\n", m_dev_info.num_cus);
    printf("Clock frequency: %u\n", m_dev_info.max_clock_freq);
}

void OpenCLEnv::run() const {
    LocalMemRunner lmr(*this);
    lmr.run();
    GlobalMemRunner gmr(*this);
    gmr.run();

    CompRunner<cl_int> int_runner(*this);
    int_runner.run();
    CompRunner<cl_float> float_runner(*this);
    float_runner.run();
    CompRunner<cl_half> half_runner(*this);
    half_runner.run();
    CompRunner<cl_double> double_runner(*this);
    double_runner.run();
}

#endif

// vim: syntax=cpp.doxygen
