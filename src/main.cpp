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

#include <getopt.h>
#include <stdio.h>
#include <algorithm>
#include <memory>
#include <string>

#include "backend.h"

void usage() {
    fprintf(stderr, "\n");
    fprintf(stderr, "Get the peak performance for the device\n");
    fprintf(stderr,
            "Usage: megpeak [--device|-d] [cpu/opencl] [-i|--dev-id] "
            "<dev_id> \n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -d, --device   default is cpu\n");
    fprintf(stderr, "  -i, --dev-id   device id for the device\n");
    fprintf(stderr, "\n");
}

int main(int argc, char** argv) {
    int c;
    static struct option loptions[] = {{"help", no_argument, NULL, 'h'},
                                       {"device", required_argument, NULL, 'd'},
                                       {"dev-id", required_argument, NULL, 'i'},
                                       {NULL, 0, NULL, 0}};

    size_t dev_id = 0;
    std::string device = "cpu";
    std::unique_ptr<megpeak::Backend> backend;
    while ((c = getopt_long(argc, argv, "h?d:i:", loptions, NULL)) != -1) {
        switch (c) {
            case 'd':
                device = optarg;
                std::transform(device.begin(), device.end(), device.begin(),
                               ::tolower);
                break;
            case 'i':
                dev_id = std::atoi(optarg);
                break;
            default:
                usage();
                exit(-1);
                break;
        }
    }
    if (device == "cpu") {
        backend = std::make_unique<megpeak::CPUBackend>(dev_id);
    } else if (device == "opencl") {
        backend = std::make_unique<megpeak::OpenCLBackend>(dev_id);
    } else {
        fprintf(stderr, "Invalid device: %s\n", device.c_str());
        usage();
        exit(1);
    }
    backend->execute();

    return 0;
}

// vim: syntax=cpp.doxygen
