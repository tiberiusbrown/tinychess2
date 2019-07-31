#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#define CH_CPUID(d_, n_) __cpuid(d_, n_)
#else
#include <cpuid.h>
#define CH_CPUID(d_, n_) __cpuid(n_, d_[0], d_[1], d_[2], d_[3])
#endif

#include "ch_internal.hpp"

namespace ch
{

static bool has_sse_;
static bool has_avx_;

CH_OPT_SIZE static void init_cpuid()
{
    int n, data[4];
    has_sse_ = false;
    has_avx_ = false;
    CH_CPUID(data, 0);
    n = data[0];
    if(n < 1) return;
    CH_CPUID(data, 1);
    has_sse_ = (data[3] & (1 << 26)) != 0; // SSE2
    has_sse_ &= (data[2] & (1 << 9)) != 0; // SSSE3
    has_sse_ &= (data[2] & (1 << 20)) != 0; // SSE4.2
    has_sse_ &= (data[2] & (1 << 23)) != 0; // popcnt
    has_avx_ = (data[2] & (1 << 23)) != 0; // popcnt
    if(n < 7)
    {
        has_avx_ = false;
        return;
    }
    CH_CPUID(data, 7);
    has_avx_ &= (data[1] & (1 << 5)) != 0; // AVX2
    has_avx_ &= (data[1] & (1 << 3)) != 0; // BMI1
}

#if CH_ENABLE_SSE && CH_ARCH_32BIT
bool has_sse() { return has_sse_; }
#endif
bool has_avx() { return has_avx_; }

}

