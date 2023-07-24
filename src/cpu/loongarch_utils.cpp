/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2023 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 */

#if MEGPEAK_LOONGARCH
#include "src/cpu/loongarch_utils.h"

using namespace megpeak;
namespace {

#define LA_HWCAP_LSX    (1 << 4)
#define LA_HWCAP_LASX   (1 << 5)

int feature_detect_lasx() {
    int flag  = (int)getauxval(AT_HWCAP);

    if (flag & LA_HWCAP_LASX)
        return true;
    else
        return false;
}

int feature_detect_lsx() {
    int flag  = (int)getauxval(AT_HWCAP);

    if (flag & LA_HWCAP_LSX)
        return true;
    else
        return false;
}


SIMDType disabled_simd_type_thresh = SIMDType::__NR_SIMD_TYPE;
} // namespace

bool megpeak::is_supported(SIMDType type) {
    bool static is_lasx_supported = feature_detect_lasx();
    bool static is_lsx_supported  = feature_detect_lsx();
    if (type >= disabled_simd_type_thresh)
        return false;

    switch (type) {
        case SIMDType::LSX:
            return is_lsx_supported;
        case SIMDType::LASX:
            return is_lasx_supported;
        default:
            break;
    }
    printf("unknown cpu feature.\n");
    return false;
}
#endif
