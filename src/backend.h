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

#include <cstddef>
#include <stdio.h>
#include <stdlib.h>

namespace megpeak {

class Backend {
public:
    Backend(size_t id) : m_dev_id{id} {}
    virtual void execute() = 0;
    virtual ~Backend() {}

protected:
    //! device id
    size_t m_dev_id;
};

class CPUBackend : public Backend {
public:
    CPUBackend(size_t id) : Backend(id) {}
    void execute() override;
};

class OpenCLBackend : public Backend {
public:
    OpenCLBackend(size_t id) : Backend(id) {}
    void execute() override;
};

#define megpeak_assert(x, format, ...)             \
    if (!(x)) {                                    \
        fprintf(stderr, format "\n", __VA_ARGS__); \
        exit(1);                                   \
    }

}  // namespace megpeak

// vim: syntax=cpp.doxygen
