/**
 * \file src/backend.h
 *
 * This file is part of MegPeak.
 *
 * \copyright Copyright (c) 2019-2019 Megvii Inc. All rights reserved.
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
