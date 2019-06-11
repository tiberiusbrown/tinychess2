#pragma once

#include <stdint.h>
#include <assert.h>

#include <array>

#include <emmintrin.h>

#include "ch_config.hpp"
#include "ch_move.hpp"

#ifdef _MSC_VER
#ifdef _M_IX86
#define CH_ARCH_X86
#endif
#endif

#ifdef __GNUC__
#ifdef __i386__
#define CH_ARCH_32BIT
#endif
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

struct position
{
    CH_ALIGN(64) std::array<uint64_t, BLACK + END_BB> bbs;

    std::array<uint8_t, 64> pieces;

    static constexpr int const STACK_SIZE = 64;
    struct stack_node
    {
        int cap_piece;
        int ep_sq;
        int castling_rights;
    };
    std::array<stack_node, STACK_SIZE> stack_data;
    int stack_index;
    stack_node& stack() { return stack_data[stack_index]; }
    stack_node const& stack() const { return stack_data[stack_index]; }
    stack_node& stack_push()
    {
        stack_data[stack_index + 1] = stack_data[stack_index];
        ++stack_index;
        return stack();
    }
    constexpr void stack_pop() { --stack_index; }

    color current_turn;

    void new_game();
    void load_fen(char const* fen);

    template<acceleration accel>
    void do_move(move const& mv);

    template<acceleration accel>
    void undo_move(move const& mv);

#if CH_COLOR_TEMPLATE
    template<color c, acceleration accel>
    uint64_t perft(int depth);
#else
    template<acceleration accel>
    uint64_t perft(color c, int depth);
#endif

    template<acceleration accel>
    uint64_t root_perft(int depth, uint64_t* counts);
};

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel>
struct move_generator
{
    static int generate(move* mvs, position const& p);
};
#else
template<acceleration accel>
struct move_generator
{
    static int generate(color c, move* mvs, position const& p);
};
#endif

inline int msb(uint64_t x)
{
    unsigned long i;
    assert(x != 0);
#ifdef CH_ARCH_X86
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
}

inline int lsb(uint64_t x)
{
    unsigned long i;
    assert(x != 0);
#ifdef CH_ARCH_X86
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

void print_position(position const& p);

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
std::array<precomputed_mask_data, 64> masks;

// for simple rank attacks: indexed by: occupancy (center 6 bits), col
std::array<uint8_t, 64 * 8> first_rank_attacks;

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

std::array<std::array<uint64_t, 64>, 64> betweens;
std::array<std::array<uint64_t, 64>, 64> lines;

void init();

#if CH_ENABLE_ACCEL
void init_cpuid();
bool has_sse();
bool has_popcnt();
#else
static inline void init_cpuid() {}
static inline bool has_sse() { return false; }
static inline bool has_avx() { return false; }
#endif

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

}
