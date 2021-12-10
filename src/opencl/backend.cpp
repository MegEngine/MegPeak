/**
 * \file src/opencl/backend.cpp
 *
 * \brief This is benchmark for MegPeak
 *
 * \copyright Copyright (c) 2019-2019 Megvii Inc. All rights reserved.
 */

#include "src/backend.h"
#ifdef MEGPEAK_WITH_OPENCL
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <MCL/cl.h>

#include "src/opencl/common.h"

void megpeak::OpenCLBackend::execute() {
    OpenCLEnv env(m_dev_id);
    env.print_device_info();
    env.run();
}

#else

void megpeak::OpenCLBackend::execute() {
    megpeak_assert(0, "%s", "opencl disabled at compile time");
}
#endif

// vim: syntax=cpp.doxygen
