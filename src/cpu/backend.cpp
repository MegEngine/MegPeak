/**
 * MegPeaK is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2021-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied.
 */

#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <memory>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_host.h>
#else
#include <sched.h>
#include <sys/sysinfo.h>
#endif

#include "src/backend.h"
#include "src/cpu/common.h"

#ifdef MEGPEAK_USE_CPUINFO
#include "cpuinfo.h"
#endif

using namespace megpeak;
namespace {
uint8_t bandwidth() {
    constexpr size_t NR_WARMUP = 5, NR_RUNS = 40, NR_BYTES = 1024 * 1024 * 100;
    std::unique_ptr<uint8_t[]> src{new uint8_t[NR_BYTES]};
    std::unique_ptr<uint8_t[]> dst{new uint8_t[NR_BYTES]};

    volatile uint8_t res = 0;
    for (size_t i = 0; i < NR_WARMUP; i++) {
        memcpy(dst.get(), src.get(), NR_BYTES);
        res += dst[0];
    }

    Timer timer;
    for (size_t i = 0; i < NR_RUNS; i++) {
        memcpy(dst.get(), src.get(), NR_BYTES);
        res += dst[0];
    }
    float used = timer.get_msecs() / NR_RUNS;
    printf("bandwidth: %f Gbps\n",
           2 * NR_BYTES / (1024.0 * 1024.0 * 1024.0) * 1000 / used);
    return res;
}

void cpu_set_affinity(int dev_id) {
#if defined(__APPLE__)
#pragma message("set_cpu_affinity not enabled on apple platform")
    printf("cpu core affinity is not usable in apple os\n");
#else
    cpu_set_t cst;
    CPU_ZERO(&cst);
    CPU_SET(dev_id, &cst);
    sched_setaffinity(0, sizeof(cst), &cst);
#endif
}

size_t get_cpu_count() {
    size_t cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    return cpu_num;
}

#ifdef MEGPEAK_USE_CPUINFO
const char* vendor_to_string(enum cpuinfo_vendor vendor) {
    switch (vendor) {
        case cpuinfo_vendor_unknown:
            return "unknown";
        case cpuinfo_vendor_intel:
            return "Intel";
        case cpuinfo_vendor_amd:
            return "AMD";
        case cpuinfo_vendor_huawei:
            return "Huawei";
        case cpuinfo_vendor_hygon:
            return "Hygon";
        case cpuinfo_vendor_arm:
            return "ARM";
        case cpuinfo_vendor_qualcomm:
            return "Qualcomm";
        case cpuinfo_vendor_apple:
            return "Apple";
        case cpuinfo_vendor_samsung:
            return "Samsung";
        case cpuinfo_vendor_nvidia:
            return "Nvidia";
        case cpuinfo_vendor_mips:
            return "MIPS";
        case cpuinfo_vendor_ibm:
            return "IBM";
        case cpuinfo_vendor_ingenic:
            return "Ingenic";
        case cpuinfo_vendor_via:
            return "VIA";
        case cpuinfo_vendor_cavium:
            return "Cavium";
        case cpuinfo_vendor_broadcom:
            return "Broadcom";
        case cpuinfo_vendor_apm:
            return "Applied Micro";
        default:
            return NULL;
    }
}

const char* uarch_to_string(enum cpuinfo_uarch uarch) {
    switch (uarch) {
        case cpuinfo_uarch_unknown:
            return "unknown";
        case cpuinfo_uarch_p5:
            return "P5";
        case cpuinfo_uarch_quark:
            return "Quark";
        case cpuinfo_uarch_p6:
            return "P6";
        case cpuinfo_uarch_dothan:
            return "Dothan";
        case cpuinfo_uarch_yonah:
            return "Yonah";
        case cpuinfo_uarch_conroe:
            return "Conroe";
        case cpuinfo_uarch_penryn:
            return "Penryn";
        case cpuinfo_uarch_nehalem:
            return "Nehalem";
        case cpuinfo_uarch_sandy_bridge:
            return "Sandy Bridge";
        case cpuinfo_uarch_ivy_bridge:
            return "Ivy Bridge";
        case cpuinfo_uarch_haswell:
            return "Haswell";
        case cpuinfo_uarch_broadwell:
            return "Broadwell";
        case cpuinfo_uarch_sky_lake:
            return "Sky Lake";
        case cpuinfo_uarch_palm_cove:
            return "Palm Cove";
        case cpuinfo_uarch_sunny_cove:
            return "Sunny Cove";
        case cpuinfo_uarch_willamette:
            return "Willamette";
        case cpuinfo_uarch_prescott:
            return "Prescott";
        case cpuinfo_uarch_bonnell:
            return "Bonnell";
        case cpuinfo_uarch_saltwell:
            return "Saltwell";
        case cpuinfo_uarch_silvermont:
            return "Silvermont";
        case cpuinfo_uarch_airmont:
            return "Airmont";
        case cpuinfo_uarch_goldmont:
            return "Goldmont";
        case cpuinfo_uarch_goldmont_plus:
            return "Goldmont Plus";
        case cpuinfo_uarch_knights_ferry:
            return "Knights Ferry";
        case cpuinfo_uarch_knights_corner:
            return "Knights Corner";
        case cpuinfo_uarch_knights_landing:
            return "Knights Landing";
        case cpuinfo_uarch_knights_hill:
            return "Knights Hill";
        case cpuinfo_uarch_knights_mill:
            return "Knights Mill";
        case cpuinfo_uarch_k5:
            return "K5";
        case cpuinfo_uarch_k6:
            return "K6";
        case cpuinfo_uarch_k7:
            return "K7";
        case cpuinfo_uarch_k8:
            return "K8";
        case cpuinfo_uarch_k10:
            return "K10";
        case cpuinfo_uarch_bulldozer:
            return "Bulldozer";
        case cpuinfo_uarch_piledriver:
            return "Piledriver";
        case cpuinfo_uarch_steamroller:
            return "Steamroller";
        case cpuinfo_uarch_excavator:
            return "Excavator";
        case cpuinfo_uarch_zen:
            return "Zen";
        case cpuinfo_uarch_zen2:
            return "Zen 2";
        case cpuinfo_uarch_zen3:
            return "Zen 3";
        case cpuinfo_uarch_geode:
            return "Geode";
        case cpuinfo_uarch_bobcat:
            return "Bobcat";
        case cpuinfo_uarch_jaguar:
            return "Jaguar";
        case cpuinfo_uarch_puma:
            return "Puma";
        case cpuinfo_uarch_xscale:
            return "XScale";
        case cpuinfo_uarch_arm7:
            return "ARM7";
        case cpuinfo_uarch_arm9:
            return "ARM9";
        case cpuinfo_uarch_arm11:
            return "ARM11";
        case cpuinfo_uarch_cortex_a5:
            return "Cortex-A5";
        case cpuinfo_uarch_cortex_a7:
            return "Cortex-A7";
        case cpuinfo_uarch_cortex_a8:
            return "Cortex-A8";
        case cpuinfo_uarch_cortex_a9:
            return "Cortex-A9";
        case cpuinfo_uarch_cortex_a12:
            return "Cortex-A12";
        case cpuinfo_uarch_cortex_a15:
            return "Cortex-A15";
        case cpuinfo_uarch_cortex_a17:
            return "Cortex-A17";
        case cpuinfo_uarch_cortex_a32:
            return "Cortex-A32";
        case cpuinfo_uarch_cortex_a35:
            return "Cortex-A35";
        case cpuinfo_uarch_cortex_a53:
            return "Cortex-A53";
        case cpuinfo_uarch_cortex_a55r0:
            return "Cortex-A55r0";
        case cpuinfo_uarch_cortex_a55:
            return "Cortex-A55";
        case cpuinfo_uarch_cortex_a57:
            return "Cortex-A57";
        case cpuinfo_uarch_cortex_a65:
            return "Cortex-A65";
        case cpuinfo_uarch_cortex_a72:
            return "Cortex-A72";
        case cpuinfo_uarch_cortex_a73:
            return "Cortex-A73";
        case cpuinfo_uarch_cortex_a75:
            return "Cortex-A75";
        case cpuinfo_uarch_cortex_a76:
            return "Cortex-A76";
        case cpuinfo_uarch_cortex_a77:
            return "Cortex-A77";
        case cpuinfo_uarch_cortex_a78:
            return "Cortex-A78";
        case cpuinfo_uarch_neoverse_n1:
            return "Neoverse-N1";
        case cpuinfo_uarch_neoverse_v1:
            return "Neoverse-V1";
        case cpuinfo_uarch_neoverse_n2:
            return "Neoverse-N2";
        case cpuinfo_uarch_cortex_x1:
            return "Cortex-X1";
        case cpuinfo_uarch_scorpion:
            return "Scorpion";
        case cpuinfo_uarch_krait:
            return "Krait";
        case cpuinfo_uarch_kryo:
            return "Kryo";
        case cpuinfo_uarch_falkor:
            return "Falkor";
        case cpuinfo_uarch_saphira:
            return "Saphira";
        case cpuinfo_uarch_denver:
            return "Denver";
        case cpuinfo_uarch_denver2:
            return "Denver 2";
        case cpuinfo_uarch_carmel:
            return "Carmel";
        case cpuinfo_uarch_exynos_m1:
            return "Exynos M1";
        case cpuinfo_uarch_exynos_m2:
            return "Exynos M2";
        case cpuinfo_uarch_exynos_m3:
            return "Exynos M3";
        case cpuinfo_uarch_exynos_m4:
            return "Exynos M4";
        case cpuinfo_uarch_exynos_m5:
            return "Exynos M5";
        case cpuinfo_uarch_swift:
            return "Swift";
        case cpuinfo_uarch_cyclone:
            return "Cyclone";
        case cpuinfo_uarch_typhoon:
            return "Typhoon";
        case cpuinfo_uarch_twister:
            return "Twister";
        case cpuinfo_uarch_hurricane:
            return "Hurricane";
        case cpuinfo_uarch_monsoon:
            return "Monsoon";
        case cpuinfo_uarch_mistral:
            return "Mistral";
        case cpuinfo_uarch_vortex:
            return "Vortex";
        case cpuinfo_uarch_tempest:
            return "Tempest";
        case cpuinfo_uarch_lightning:
            return "Lightning";
        case cpuinfo_uarch_thunder:
            return "Thunder";
        case cpuinfo_uarch_firestorm:
            return "Firestorm";
        case cpuinfo_uarch_icestorm:
            return "Icestorm";
        case cpuinfo_uarch_thunderx:
            return "ThunderX";
        case cpuinfo_uarch_thunderx2:
            return "ThunderX2";
        case cpuinfo_uarch_pj4:
            return "PJ4";
        case cpuinfo_uarch_brahma_b15:
            return "Brahma B15";
        case cpuinfo_uarch_brahma_b53:
            return "Brahma B53";
        case cpuinfo_uarch_xgene:
            return "X-Gene";
        case cpuinfo_uarch_dhyana:
            return "Dhyana";
        case cpuinfo_uarch_taishan_v110:
            return "TaiShan v110";
        default:
            return NULL;
    }
}
#endif // MEGPEAK_USE_CPUINFO

void print_cpu_info(size_t dev_id, size_t cpu_count)
{
    printf("there are %zu cores, currently use core id :%zu\n", cpu_count, dev_id);
#ifdef MEGPEAK_USE_CPUINFO
    const cpuinfo_core* core = cpuinfo_get_current_core();
    const char* vendor_string = vendor_to_string(core->vendor);
    const char* uarch_string = uarch_to_string(core->uarch);

    printf("Vendor is: %s, uArch: %s, frequency: %ldHz\n", vendor_string,
           uarch_string, core->frequency);
#endif // MEGPEAK_USE_CPUINFO
    printf("\n");
}

}  // namespace

void CPUBackend::execute() {
    size_t cpu_count = get_cpu_count();
    cpu_set_affinity(m_dev_id);
    print_cpu_info(m_dev_id, cpu_count);
    bandwidth();
    aarch64();
    armv7();
    x86_avx();
    x86_sse();
}

// vim: syntax=cpp.doxygen
