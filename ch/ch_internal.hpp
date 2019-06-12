#pragma once

#include <stdint.h>
#include <assert.h>

#include <array>

#include <emmintrin.h>

#include "ch.h"
#include "ch_config.hpp"
#include "ch_move.hpp"

#define CH_ENABLE_UNACCEL 1
#if CH_ENABLE_ACCEL
#define CH_ENABLE_SSE 1
#define CH_ENABLE_AVX 1
#else
#define CH_ENABLE_SSE 0
#define CH_ENABLE_AVX 0
#endif

#define CH_ARCH_32BIT 0

#ifdef _MSC_VER
#ifdef _M_IX86
#undef CH_ARCH_32BIT
#define CH_ARCH_32BIT 1
#endif
#endif

#ifdef __GNUC__
#ifdef __i386__
#undef CH_ARCH_32BIT
#define CH_ARCH_32BIT 1
#endif
#endif

#define CH_ARCH_64BIT (!CH_ARCH_32BIT)

#if CH_ARCH_64BIT
#undef CH_ENABLE_UNACCEL
#define CH_ENABLE_UNACCEL 0
#undef CH_ENABLE_SSE
#define CH_ENABLE_SSE 1
#endif

#if defined(_MSC_VER)
#define CH_ALIGN(bytes) __declspec(align(bytes))
#elif defined(__GNUC__)
#define CH_ALIGN(bytes) __attribute__((aligned(bytes)))
#else
#error "Unsupported"
#endif

#ifndef NDEBUG
#define CH_FORCEINLINE
#define CH_FORCENOINLINE
#define CH_OPT_SIZE
#else
#if defined(_MSC_VER)
#define CH_FORCEINLINE __forceinline
#define CH_FORCENOINLINE __declspec(noinline)
#define CH_OPT_SIZE __pragma(optimize("ts", on))
#elif defined(__GNUC__)
#define CH_FORCEINLINE __attribute__((always_inline)) inline
#define CH_FORCENOINLINE __attribute__((noinline))
#define CH_OPT_SIZE __attribute__((optimize("-Os")))
#else
#define CH_FORCEINLINE
#define CH_FORCENOINLINE
#define CH_OPT_SIZE
#endif
#endif

namespace ch
{

enum acceleration
{
    ACCEL_UNACCEL,
    ACCEL_SSE,
    ACCEL_AVX,
    ACCEL_AVX512
};

static constexpr int CASTLE_WQ_MASK = 0x1;
static constexpr int CASTLE_BQ_MASK = 0x2;
static constexpr int CASTLE_WK_MASK = 0x4;
static constexpr int CASTLE_BK_MASK = 0x8;

inline int msb(uint64_t x)
{
    assert(x != 0);
#ifdef _MSC_VER
    unsigned long i;
#if CH_ARCH_32BIT
    if(uint32_t(x >> 32) != 0)
    {
        _BitScanReverse(&i, uint32_t(x));
        return i;
    }
    _BitScanReverse(&i, uint32_t(x));
    return i + 32;
#else
    _BitScanReverse64(&i, x);
    return i;
#endif
#else
    return __builtin_clzll(x);
#endif
}

inline int lsb(uint64_t x)
{
    assert(x != 0);
#ifdef _MSC_VER
    unsigned long i;
#if CH_ARCH_32BIT
    if(uint32_t(x) != 0)
    {
        _BitScanForward(&i, uint32_t(x));
        return i;
    }
    _BitScanForward(&i, uint32_t(x >> 32));
    return i + 32;
#else
    _BitScanForward64(&i, x);
    return i;
#endif
#else
    return __builtin_ctzll(x);
#endif
}
inline int pop_lsb(uint64_t& x)
{
    int i = lsb(x);
    x &= x - 1;
    return i;
}

inline uint64_t lsb_mask(uint64_t x)
{
    return x & uint64_t(-int64_t(x));
}
inline uint64_t pop_lsb_mask(uint64_t& x)
{
    uint64_t r = lsb_mask(x);
    x ^= r;
    return r;
}

inline bool more_than_one(uint64_t x)
{
    assert(x != 0);
    return (x & (x - 1)) != 0;
}

void print_bbs(uint64_t bbs[], int n);
inline void print_bb(uint64_t bb) { print_bbs(&bb, 1); }
inline void print_bb(__m128i bb) { print_bbs((uint64_t*)&bb, 2); }

struct precomputed_mask_data
{
    uint64_t knight_attacks;
    uint64_t king_attacks;
    uint64_t rook_pseudo_attacks;
    uint64_t bishop_pseudo_attacks;
    uint64_t white_pawn_attacks;
    uint64_t black_pawn_attacks;

    // for hyperbola quintessence
    uint64_t singleton;
    uint64_t vertical;
    CH_ALIGN(16) uint64_t diag_anti[2];
};
static std::array<precomputed_mask_data, 64> masks;

// for simple rank attacks: indexed by: occupancy (center 6 bits), col
static std::array<uint8_t, 64 * 8> first_rank_attacks;

#if 0
enum
{
    SBAMG_RANK,
    SBAMG_FILE,
    SBAMG_DIAG,
    SBAMG_ANTI,
};
struct sbamg_mask_data
{
    CH_ALIGN(32) uint64_t low[4];
    CH_ALIGN(32) uint64_t lin[4];
    CH_ALIGN(32) uint64_t out[4];
};
std::array<sbamg_mask_data, 64> sbamg_masks;
#endif

static std::array<std::array<uint64_t, 64>, 64> betweens;
static std::array<std::array<uint64_t, 64>, 64> lines;

void init();

void init_cpuid();
#if CH_ENABLE_SSE
#if CH_ARCH_64BIT
static inline constexpr bool has_sse() { return true; }
#else
bool has_sse();
#endif
#else
static inline constexpr bool has_sse() { return false; }
#endif
bool has_popcnt();
bool has_avx();

#ifdef _MSC_VER
extern "C" void * __cdecl memset(void *, int, size_t);
#pragma intrinsic(memset)

#pragma function(memset)
void * __cdecl memset(void *p, int v, size_t n)
{
    unsigned char *t = static_cast<unsigned char *>(p);
    while(n-- > 0)
        *t++ = static_cast<unsigned char>(v);
    return p;
}
#endif

static ch_system_info system;

}
