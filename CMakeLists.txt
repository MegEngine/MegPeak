cmake_minimum_required(VERSION 3.15.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

project(MegEngine LANGUAGES C CXX ASM)
set(CMAKE_CXX_STANDARD 14)

message(${PROJECT_SOURCE_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

option(MEGPEAK_ENABLE_OPENCL "Build MegPeak with OpenCL." OFF)
option(MEGPEAK_ENABLE_ALL_BENCHMARK "Build MegPeak with all benchmark." OFF)
option(MEGPEAK_USE_CPUINFO   "Use cpuinfo library"        OFF)
option(MEGPEAK_ENABLE_DOT       "Use arm dotprod instruction."   OFF)
option(MEGPEAK_ENABLE_MMA       "Use arm mma instruction"        OFF)
option(MEGPEAK_ENABLE_BFMMA       "Use arm bfmma instruction"        OFF)

if(CMAKE_TOOLCHAIN_FILE)
    message(STATUS "cross compile MegPeak.")
    if(ANDROID_TOOLCHAIN_ROOT)
        if(NOT "${ANDROID_ARCH_NAME}" STREQUAL "")
            set(ANDROID_ARCH ${ANDROID_ARCH_NAME})
        endif()
        if(${ANDROID_ARCH} STREQUAL "arm")
            set(MGEPEAK_ARCH "armv7")
        elseif(${ANDROID_ARCH} STREQUAL "arm64")
            set(MGEPEAK_ARCH "aarch64")
        else()
            message(FATAL_ERROR "DO NOT SUPPORT ANDROID ARCH NOW")
        endif()
    elseif(NOT "${ARM_CROSS_BUILD_ARCH}" STREQUAL "")
        set(MGEPEAK_ARCH ${ARM_CROSS_BUILD_ARCH})
    else()
        message(FATAL_ERROR "Unknown cross-compiling settings.")
    endif()
else()
    message("CMAKE SYSTEM PROCESSOR :${CMAKE_SYSTEM_PROCESSOR}.")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        set(MGEPEAK_ARCH "x86")
    elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "loongarch64")
        set(MGEPEAK_ARCH "loongarch")
    else()
        set(MGEPEAK_ARCH "aarch64")
    endif()
endif()

message(STATUS "CONFIG MGEPEAK_ARCH TO ${MGEPEAK_ARCH}")

file(GLOB_RECURSE SRC src/*.cpp src/*.h )

add_executable(megpeak ${SRC})
target_include_directories(megpeak PUBLIC ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "-Ofast -g ${CMAKE_CXX_FLAGS}")
if(${MGEPEAK_ARCH} STREQUAL "aarch64")
    set(CMAKE_CXX_FLAGS " -DMEGPEAK_AARCH64=1 ${CMAKE_CXX_FLAGS}")
elseif(${MGEPEAK_ARCH} STREQUAL "armv7")
    set(CMAKE_CXX_FLAGS " -DMEGPEAK_ARMV7=1 ${CMAKE_CXX_FLAGS}")
elseif(${MGEPEAK_ARCH} STREQUAL "x86")
    set(CMAKE_CXX_FLAGS " -DMEGPEAK_X86=1 ${CMAKE_CXX_FLAGS}")
elseif(${MGEPEAK_ARCH} STREQUAL "loongarch")
    set(CMAKE_CXX_FLAGS " -DMEGPEAK_LOONGARCH=1 ${CMAKE_CXX_FLAGS}")
else()
    message(FATAL_ERROR "Unknown MEGPEAK_ARCH ${MEGPEAK_ARCH}.")
endif()


if(MEGPEAK_ENABLE_DOT)
    message(STATUS "Enable dotprod feature in armv8.2-a")
    set(CMAKE_CXX_FLAGS " -march=armv8.2-a+dotprod ${CMAKE_CXX_FLAGS}")
endif()
if(MEGPEAK_ENABLE_MMA)
    message(STATUS "Enable smmla feature")
    add_compile_definitions(MEGPEAK_ENABLE_MMA)
endif()
if(MEGPEAK_ENABLE_BFMMA)
    message(STATUS "Enable bfmmla feature")
    add_compile_definitions(MEGPEAK_ENABLE_BFMMA)
endif()

if(MEGPEAK_ENABLE_OPENCL)
    set(CMAKE_CXX_FLAGS " -DMEGPEAK_WITH_OPENCL=1 ${CMAKE_CXX_FLAGS}")

    message(STATUS "megpeak build with opencl.")
    add_subdirectory(opencl-stub)
    target_include_directories(megpeak PUBLIC ${PROJECT_SOURCE_DIR}/opencl-stub/include/)
    target_link_libraries(megpeak OpenCL)
endif()

if(MEGPEAK_ENABLE_ALL_BENCHMARK)
    set(CMAKE_CXX_FLAGS " -DMEGPEAK_WITH_ALL_BENCHMARK=1 ${CMAKE_CXX_FLAGS}")
    message(STATUS "megpeak build with all benchmark.")
endif()

if(UNIX)
    target_link_libraries(megpeak dl)
endif()
if(ANDROID)
    target_link_libraries(megpeak log)
endif()

if(MEGPEAK_USE_CPUINFO)
    add_definitions(-DMEGPEAK_USE_CPUINFO)
    set(CPUINFO_BUILD_UNIT_TESTS OFF CACHE BOOL "")
    set(CPUINFO_BUILD_MOCK_TESTS OFF CACHE BOOL "")
    set(CPUINFO_BUILD_BENCHMARKS OFF CACHE BOOL "")
    add_subdirectory(cpuinfo)
    target_link_libraries(megpeak cpuinfo::cpuinfo)
endif()
