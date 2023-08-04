/*
 *   LIBOPENCL_PATH: candidate paths to libopencl.so; multiple paths are
 *   splitted by colons
 **/

#if MEGDNN_SHARED_LIB
#pragma GCC visibility push(default)
#endif

#include "MCL/cl.h"

#include <cstdlib>
#include <string>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#if defined(__APPLE__) || defined(__MACOSX)
static const char* default_so_paths[] = { "/System/Library/Frameworks/OpenCL.framework/OpenCL", "libOpenCL.so" };
#elif defined(__ANDROID__)
static const char* default_so_paths[] = {
#if defined(__aarch64__)
        "/system/lib64/libOpenCL.so",
        "/system/lib64/libOpenCL_system.so",
        "/system/lib64/egl/libGLES_mali.so",
        "/system/vendor/lib64/libOpenCL.so",
        "/system/vendor/lib64/egl/libGLES_mali.so",
        "/system/vendor/lib64/libPVROCL.so",
        "/vendor/lib64/libOpenCL.so",
        "/usr/lib64/libGLESv2.so.2.1.0",
        "/data/data/org.pocl.libs/files/lib64/libpocl.so",
#else
        "/system/lib/libOpenCL.so",
        "/system/lib/libOpenCL_system.so",
        "/system/lib/egl/libGLES_mali.so",
        "/system/vendor/lib/libOpenCL.so",
        "/system/vendor/lib/egl/libGLES_mali.so",
        "/system/vendor/lib/libPVROCL.so",
        "/vendor/lib/libOpenCL.so",
        "/data/data/org.pocl.libs/files/lib/libpocl.so",
#endif
        "libOpenCL.so"};
#elif defined(_WIN32)
static const char* default_so_paths[] = {"OpenCL.dll"};
#elif defined(__linux__)
static const char* default_so_paths[] = {
#if defined(__x86_64__) || defined(__amd64__)
        "/usr/lib64/libOpenCL.so", "/usr/local/lib64/libOpenCL.so",
        "/usr/local/cuda/lib64/libOpenCL.so",
        "/opt/intel/opencl/libOpenCL.so",
        //! As in some system like apex, the driver exists here
        "/usr/lib/libOpenCL.so",
#else
        "/usr/lib/libOpenCL.so",
        "/lib/aarch64-linux-gnu/libmali.so",
        "/usr/lib32/libOpenCL.so",
        "/usr/local/lib/libOpenCL.so",
        "/usr/local/lib/libpocl.so",
        "/usr/local/cuda/lib/libOpenCL.so",
#endif
        "libOpenCL.so"};
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define LOGD(fmt, v...)                                                  \
    do {                                                                 \
        __android_log_print(ANDROID_LOG_DEBUG, "megdnn", fmt "\n", ##v); \
        printf(fmt "\n", ##v);                                           \
    } while (0)
#define LOGE(fmt, v...)                                                  \
    do {                                                                 \
        __android_log_print(ANDROID_LOG_ERROR, "megdnn", fmt "\n", ##v); \
        fprintf(stderr, "err: " fmt "\n", ##v);                          \
    } while (0)
#else
#include <cstdio>
#define LOGD(fmt, v...) printf(fmt "\n", ##v)
#define LOGE(fmt, v...) fprintf(stderr, "err: " fmt "\n", ##v)
#endif

#if defined(_WIN32)
#include <io.h>
#include <windows.h>
#define F_OK 0
#define RTLD_LAZY 0
// On the windows platform we use a lib_filename without a full path so
// the win-api "LoadLibrary" would uses a standard search strategy to
// find the lib module. As we cannot access to the lib_filename without a
// full path, we should not use "access(a, b)" to verify it.
#define access(a, b) false

static void* dlopen(const char* file, int) {
    return static_cast<void*>(LoadLibrary(file));
}

static void* dlerror() {
    const char* errmsg = "dlerror not aviable in windows";
    return const_cast<char*>(errmsg);
}

static void* dlsym(void* handle, const char* name) {
    FARPROC symbol = GetProcAddress((HMODULE)handle, name);
    return reinterpret_cast<void*>(symbol);
}

#else
#include <dlfcn.h>
#include <unistd.h>
#endif

static void log_failed_load(int func_idx);
namespace {
template <typename T>
T on_init_failed(int func_idx);

template <>
cl_mem on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
cl_int on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return CL_INVALID_PLATFORM;
}

template <>
cl_context on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
cl_kernel on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
cl_event on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
cl_command_queue on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
cl_sampler on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
cl_program on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
void* on_init_failed(int func_idx) {
    log_failed_load(func_idx);
    return nullptr;
}

template <>
void on_init_failed(int func_idx) {
    log_failed_load(func_idx);
}
}  // anonymous namespace

#if 0
#include <unistd.h>
#include <cstdio>
namespace {
size_t get_graphics_mem() {
    char cmd[128];
    sprintf(cmd,
            "dumpsys meminfo %d | grep Graphics | cut -d: -f2 | tr -d ' ' ",
            getpid());
    FILE* fin = popen(cmd, "r");
    size_t val = 0;
    if (fscanf(fin, "%zu", &val) != 1) {
        val = -1;
    }
    fclose(fin);
    return val;
}

class MemInfoDump {
    const char* m_name;
    size_t m_start;

public:
    MemInfoDump(const char* name) : m_name{name}, m_start{get_graphics_mem()} {}

    ~MemInfoDump() {
        auto now = get_graphics_mem();
        FILE* flog = fopen("log.txt", "a");
        if (!flog) {
            fprintf(stderr, "failed to open log.txt\n");
            abort();
        }
        fprintf(flog, "d=%zu func=%s start=%zu end=%zu\n", now - m_start,
                m_name, m_start, now);
        fclose(flog);
    }
};
}  // namespace
#define ON_ENTRY(x) \
    MemInfoDump _mem_info_dump { #x }
#endif  // use dumpsys to get memory usage at each API call on android

#define _WRAPLIB_API_CALL CL_API_CALL
#define _WRAPLIB_CALLBACK CL_CALLBACK
#include "./libopencl-wrap.h"
#undef _WRAPLIB_CALLBACK
#undef _WRAPLIB_API_CALL

static bool is_valid_path_for_dlopen(const char* path)
{
#if __APPLE__
  if (strstr(path, ".framework") != NULL)
  {
    return true;
  }
  return access(path, F_OK);
#else
    return access(path, F_OK);
#endif
}

static bool open_shared_lib(const char* path, void*& handle) {
    if (!is_valid_path_for_dlopen(path)) {
        handle = dlopen(path, RTLD_LAZY);
        if (handle)
            return true;
        LOGD("opencl lib found but can not be opened: %s err=%s", path,
             dlerror());
    }
    return false;
}

static void* get_library_handle() {
    const char* path = nullptr;
    auto str_cptr = getenv("LIBOPENCL_PATH");
    std::string str;
    void* handle = nullptr;

    if (str_cptr) {
        str = str_cptr;
        char* p = &str[0];
        const char* begin = p;
        while (*p) {
            if (*p == ':') {
                *p = 0;
                if (open_shared_lib(begin, handle)) {
                    path = begin;
                    break;
                }
                begin = p + 1;
            }
            ++p;
        }
        if (open_shared_lib(begin, handle)) {
            path = begin;
        }
    }

    if (!path) {
        for (size_t i = 0; i < (sizeof(default_so_paths) / sizeof(char*));
             i++) {
            if (open_shared_lib(default_so_paths[i], handle)) {
                path = default_so_paths[i];
                break;
            }
        }
    }

    if (!path) {
        LOGE("can not find opencl");
        return nullptr;
    }
    LOGD("use opencl: %s", path);
    return handle;
}

static void log_failed_load(int func_idx) {
    LOGE("failed to load opencl func: %s", g_func_name[func_idx]);
}

static void* resolve_library_func(void* handle, const char* func) {
    if (!handle) {
        LOGE("handle should not be nullptr!");
        return nullptr;
    }
    auto ret = dlsym(handle, func);
    if (!ret) {
        LOGD("failed to load opencl func: %s", func);
    }
    return ret;
}
