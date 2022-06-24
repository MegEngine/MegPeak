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
