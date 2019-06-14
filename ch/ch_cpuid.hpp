#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "ch_internal.hpp"

namespace ch
{

static bool has_sse_;
static bool has_avx_;

CH_OPT_SIZE static void init_cpuid()
{
#ifdef _MSC_VER
    int n, data[4];
    has_sse_ = false;
    has_avx_ = false;
    __cpuid(data, 0);
    n = data[0];
    if(n < 1) return;
    __cpuid(data, 1);
    has_sse_ = (data[3] & (1 << 26)) != 0; // SSE2
    //has_sse_ &= (data[2] & (1 << 9)) != 0; // SSSE3
    has_avx_ = (data[2] & (1 << 23)) != 0; // popcnt
    if(n < 7) return;
    __cpuid(data, 7);
    has_avx_ &= (data[1] & (1 << 7)) != 0; // AVX2
#else
    has_sse_ = __builtin_cpu_supports("sse2");// &&
        //__builtin_cpu_supports("ssse3");
    has_avx_ = __builtin_cpu_supports("popcnt") &&
        __builtin_cpu_supports("avx2");
#endif
}

#if CH_ENABLE_SSE && CH_ARCH_32BIT
bool has_sse() { return has_sse_; }
#endif
bool has_avx() { return has_avx_; }

}

