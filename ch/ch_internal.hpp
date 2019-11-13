#pragma once

#include <stdint.h>
#include <assert.h>

#include <array>

#include <emmintrin.h>
#include <immintrin.h>

#include "ch.h"
#include "ch_config.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if defined(_MSC_VER) && !defined(NDEBUG) && !CH_TUNABLE
extern char const* const CH_FILESTRS[8];
extern char const* const CH_RANKSTRS[8];
// natvis help
char const* const CH_FILESTRS[8] =
{
    "a", "b", "c", "d", "e", "f", "g", "h"
};
char const* const CH_RANKSTRS[8] =
{
    "8", "7", "6", "5", "4", "3", "2", "1"
};
#endif

#define CH_ENABLE_UNACCEL 1
#if CH_ENABLE_ACCEL
#define CH_ENABLE_SSE 1
#define CH_ENABLE_AVX 1
#else
#define CH_ENABLE_SSE 0
#define CH_ENABLE_AVX 0
#endif

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

#if !defined(NDEBUG)
#define CH_FORCEINLINE
#define CH_FORCENOINLINE
#define CH_OPT_SIZE
#else

#if CH_NEVER_FORCE_INLINE
#if CH_NEVER_REQUEST_INLINE
#define CH_FORCEINLINE
#else
#define CH_FORCEINLINE inline
#endif
#define CH_FORCENOINLINE 
#else
#if defined(_MSC_VER)
#define CH_FORCEINLINE __forceinline
#define CH_FORCENOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define CH_FORCEINLINE __attribute__((always_inline)) inline
#define CH_FORCENOINLINE __attribute__((noinline))
#endif
#endif

#if defined(_MSC_VER)
#define CH_OPT_SIZE __pragma(optimize("ts", on))
#elif defined(__GNUC__)
#define CH_OPT_SIZE __attribute__((optimize("-Os")))
#else
#define CH_OPT_SIZE
#endif
#endif

#ifndef CH_FORCEINLINE
#define CH_FORCEINLINE
#endif
#ifndef CH_FORCENOINLINE
#define CH_FORCENOINLINE
#endif
#ifndef CH_OPT_SIZE
#define CH_OPT_SIZE
#endif

namespace ch
{

template<class T> constexpr T const& min(T const& a, T const& b)
{
    return a < b ? a : b;
}
template<class T> constexpr T const& max(T const& a, T const& b)
{
    return a < b ? b : a;
}
template<class T> constexpr T abs(T const& x)
{
    return x < T(0) ? -x : x;
}

template<class T> struct less
{
    constexpr bool operator()(T const& a, T const& b) const
    {
        return a < b;
    }
};
template<class RandomIt, class Compare>
void insertion_sort(
    RandomIt const& a, RandomIt const& b,
    Compare comp = less<typename RandomIt::value_type>())
{
    for(RandomIt i = a + 1; i < b; ++i)
        for(RandomIt j = i; j > a && comp(*j, *(j - 1)); --j)
            std::swap(*j, *(j - 1));
}

enum acceleration
{
    ACCEL_UNACCEL, // no acceleration
    ACCEL_SSE,     // SSE2
    ACCEL_AVX,     // AVX2, popcnt, TODO: BMI2
    ACCEL_AVX512   // TODO: AVX512F
};

static constexpr int CASTLE_WQ_MASK = 0x1;
static constexpr int CASTLE_BQ_MASK = 0x2;
static constexpr int CASTLE_WK_MASK = 0x4;
static constexpr int CASTLE_BK_MASK = 0x8;

enum ptype
{
    PAWN = 0,
    KNIGHT = 2,
    BISHOP = 4,
    ROOK = 6,
    QUEEN = 8,
    KING = 10,
    EMPTY = 12,
};

enum color
{
    WHITE = 0,
    BLACK = 1,
};

template<class T>
static constexpr bool is_white(T p) { return (p & 1) == 0; }
template<class T>
static constexpr bool is_black(T p) { return (p & 1) != 0; }

static constexpr int const NUM_BB = 13;

static constexpr color opposite(color c)
{
    return color(c ^ BLACK);
};

template<acceleration accel>
static CH_FORCEINLINE int lsb(uint64_t x)
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

template<acceleration accel>
static CH_FORCEINLINE int pop_lsb(uint64_t& x)
{
    int i = lsb<accel>(x);
    x &= x - 1;
    return i;
}
template<acceleration accel>
static CH_FORCEINLINE uint64_t lsb_mask(uint64_t x)
{
    return x & uint64_t(-int64_t(x));
}
template<acceleration accel>
static CH_FORCEINLINE uint64_t pop_lsb_mask(uint64_t& x)
{
    uint64_t r = lsb_mask<accel>(x);
    x ^= r;
    return r;
}

template<>
CH_FORCEINLINE int pop_lsb<ACCEL_AVX>(uint64_t& x)
{
#if CH_ARCH_32BIT
    return pop_lsb<ACCEL_UNACCEL>(x);
#else
    int i = lsb<ACCEL_AVX>(x);
    x = _blsr_u64(x);
    return i;
#endif
}
template<>
CH_FORCEINLINE uint64_t lsb_mask<ACCEL_AVX>(uint64_t x)
{
#if CH_ARCH_32BIT
    return lsb_mask<ACCEL_UNACCEL>(x);
#else
    return _blsi_u64(x);
#endif
}
template<>
CH_FORCEINLINE uint64_t pop_lsb_mask<ACCEL_AVX>(uint64_t& x)
{
    uint64_t r = lsb_mask<ACCEL_AVX>(x);
    x ^= r;
    return r;
}

template<acceleration accel>
static CH_FORCEINLINE int popcnt(uint64_t x)
{
    constexpr uint64_t  m1 = 0x5555555555555555ull;
    constexpr uint64_t  m2 = 0x3333333333333333ull;
    constexpr uint64_t  m4 = 0x0f0f0f0f0f0f0f0full;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >> 8;
    x += x >> 16;
    x += x >> 32;
    return int(x & 0x7f);
}

#if CH_ENABLE_SSE
template<>
CH_FORCEINLINE int popcnt<ACCEL_SSE>(uint64_t x)
{
#ifdef _MSC_VER
#if CH_ARCH_32BIT
    return int(__popcnt(uint32_t(x))) + int(__popcnt(uint32_t(x >> 32)));
#else
    return int(__popcnt64(x));
#endif
#else
    return __builtin_popcountll(x);
#endif
}
#endif
#if CH_ENABLE_AVX
template<>
CH_FORCEINLINE int popcnt<ACCEL_AVX>(uint64_t x)
{
    return popcnt<ACCEL_SSE>(x);
}
#endif

template<acceleration accel>
static CH_FORCEINLINE bool more_than_one(uint64_t x)
{
    return (x & (x - 1)) != 0;
}
#if CH_ENABLE_AVX
template<>
CH_FORCEINLINE bool more_than_one<ACCEL_AVX>(uint64_t x)
{
#if CH_ARCH_32BIT
    return more_than_one<ACCEL_UNACCEL>(x);
#else
    return _blsr_u64(x) != 0;
#endif
}
#endif

void print_bbs(uint64_t bbs[], int n);
inline void print_bb(uint64_t bb) { print_bbs(&bb, 1); }
inline void print_bb(__m128i bb) { print_bbs((uint64_t*)&bb, 2); }

struct precomputed_mask_data
{
    uint64_t knight_attacks;
    uint64_t king_attacks;
    uint64_t rook_pseudo_attacks;
    uint64_t bishop_pseudo_attacks;
    uint64_t pawn_attacks[2]; // white, black

    // for king defenders
    uint64_t king_areas[2];

    // for hyperbola quintessence
    uint64_t singleton;
    uint64_t vertical;
    CH_ALIGN(16) uint64_t diag_anti[2];
};
extern std::array<precomputed_mask_data, 64> masks;

// for simple rank attacks: indexed by: occupancy (center 6 bits), col
extern std::array<uint8_t, 64 * 8> first_rank_attacks;

extern std::array<std::array<uint64_t, 64>, 64> betweens;
extern std::array<std::array<uint64_t, 64>, 64> lines;

extern std::array<std::array<uint8_t, 64>, 64> cheby_dist;

#if CH_ENABLE_SSE
#if CH_ARCH_64BIT
static inline constexpr bool has_sse() { return true; }
#else
bool has_sse();
#endif
#else
static inline constexpr bool has_sse() { return false; }
#endif
bool has_avx();

static ch_system_info system;
static uint32_t get_ms()
{
    return system.get_ms ? system.get_ms() : 0;
}
static void thread_yield()
{
    if(system.thread_yield) system.thread_yield();
}
static void* alloc(uint32_t bytes)
{
    return system.alloc ? system.alloc(bytes) : nullptr;
}
static void dealloc(void* p)
{
    if(system.dealloc) system.dealloc(p);
}

// https://stackoverflow.com/questions/38088732/explanation-to-aligned-malloc-implementation
static void* alloc_aligned(uint32_t bytes, uint32_t alignment)
{
    void* p1; // original block
    void** p2; // aligned block
    int offset = alignment - 1 + sizeof(void*);
    if((p1 = (void*)alloc(bytes + offset)) == NULL)
    {
        return NULL;
    }
    p2 = (void**)(((uintptr_t)(p1)+offset) & ~((uintptr_t)alignment - 1));
    p2[-1] = p1;
    return p2;
}

static void dealloc_aligned(void* p)
{
    dealloc(((void**)p)[-1]);
}

static CH_FORCEINLINE void search_info(
    int depth,
    int seldepth,
    uint64_t nodes,
    int mstime,
    int score,
    uint64_t nps,
    ch_move* pv,
    int pvlen
)
{
    if(!system.search_info) return;
    system.search_info(
        depth, seldepth, nodes, mstime, score, nps, pv, pvlen);
}

#ifdef _MSC_VER
static CH_FORCEINLINE void memzero(void* p, int n)
{
    __stosb((unsigned char*)p, 0, size_t(n));
}
static CH_FORCEINLINE void memzero32(void* p, int n)
{
    __stosd((unsigned long*)p, 0, size_t(n));
}
static CH_FORCEINLINE void memcpy32(void* dst, void const* src, int n)
{
    __movsd(
        (unsigned long*)dst,
        (unsigned long const*)src,
        size_t(n));
}
#else
static CH_FORCEINLINE void memzero(void* p, int n)
{
    uint8_t* i = (uint8_t*)p;
    while(n-- > 0) *i++ = 0;
}
static CH_FORCEINLINE void memzero32(void* p, int n)
{
    uint32_t* i = (uint32_t*)p;
    while(n-- > 0) *i++ = 0;
}
static CH_FORCEINLINE void memcpy32(void* dst, void const* src, int n)
{
    while(n-- > 0)
        ((uint32_t*)dst)[n] = ((uint32_t const*)src)[n];
}
#endif

static CH_FORCEINLINE int div_nps_mstime(uint64_t a, int ms)
{
    int r;
#if CH_ARCH_32BIT
    static double const C1000 = 1000;
#ifdef _MSC_VER
    __asm
    {
        fild   QWORD PTR a
        fmul   QWORD PTR C1000
        fild   DWORD PTR ms
        fdiv
        fisttp DWORD PTR r
    }
#else
    /* assume gcc-style (mingw) */
    /* warning: not tested!! might not work... */
    asm(
        "fild   %[input0]\n\t"
        "fmul   %[input1]\n\t"
        "fild   %[input2]\n\t"
        "fdivp           \n\t"
        "fisttp %[output0]\n\t"
        : [output0] "=m" (r)
        : [input0] "m" (a)
        , [input1] "m" (C1000)
        : );
#endif
#else
    r = int(a * 1000 / ms);
#endif
    return r;
}

}
