#pragma once

#include "ch_internal.hpp"

#ifndef CH_TUNABLE
#define CH_PARAM(n_, default_) static constexpr int32_t const n_ = default_;
#define CH_PARAM_ARRAY(n_, ...) static constexpr int32_t const n_ = { __VA_ARGS__ };
#else
#ifndef CH_PARAM
#define CH_PARAM(n_, default_) extern int32_t n_;
#define CH_PARAM_ARRAY(n_, ...) extern int32_t n_;
#endif
#endif

namespace ch
{

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4307)
#endif

static constexpr int32_t SC(int16_t mg, int16_t eg)
{
    return int32_t((uint32_t(mg) << 16) + uint32_t(int32_t(eg)));
}
static constexpr int SC_MG(int32_t x)
{
    return int(int16_t(uint16_t(uint32_t(x + 0x8000) >> 16)));
}
static constexpr int SC_EG(int32_t x)
{
    return int(int16_t(uint16_t(x)));
}

//
// SEARCH PARAMS
//

// aspiration window
CH_PARAM(ASPIRATION_BASE_DELTA, 10)
CH_PARAM(ASPIRATION_MIN_DEPTH, 4)
CH_PARAM(ASPIRATION_C0, 4)
CH_PARAM(ASPIRATION_C1, 5)

// seems to hinder right now
CH_PARAM(CH_ENABLE_PROBCUT_PRUNING, 0)
CH_PARAM(PROBCUT_MIN_DEPTH, 5)
CH_PARAM(PROBCUT_MARGIN, 100)

// [deep] razoring
#define CH_ENABLE_RAZORING 1
#define CH_ENABLE_DEEP_RAZORING 0
// razoring drops into qsearch instead of reducing depth
#define CH_ENABLE_RAZOR_QSEARCH 1
CH_PARAM(RAZOR_MARGIN, 100)
CH_PARAM(RAZOR_MAX_DEPTH, 2)
CH_PARAM(DEEP_RAZOR_MARGIN, 300)
CH_PARAM(DEEP_RAZOR_MAX_DEPTH, 4)

// [reverse] futility pruning
CH_PARAM(CH_ENABLE_FUTILITY_PRUNING, 1)
CH_PARAM(FUTILITY_PRUNING_MAX_DEPTH, 7)
CH_PARAM(FUTILITY_PRUNING_C0, 64)
CH_PARAM(FUTILITY_PRUNING_C1, 64)

// null move pruning
CH_PARAM(CH_ENABLE_NULL_MOVE, 1)
CH_PARAM(NULL_MOVE_MIN_DEPTH, 5)

// internal iterative deepening
CH_PARAM(CH_ENABLE_IID, 1)
CH_PARAM(IID_MIN_DEPTH, 6)
CH_PARAM(IID_C0, 3)

// late move pruning
CH_PARAM(CH_ENABLE_LATE_MOVE_PRUNING, 0)
CH_PARAM(LMP_MAX_DEPTH, 4)
CH_PARAM(LMP_C0, 7)
CH_PARAM(LMP_C1, 0)

// late move reduction
CH_PARAM(CH_ENABLE_LATE_MOVE_REDUCTION, 1)
CH_PARAM(LMR_MIN_DEPTH, 3)
CH_PARAM(LMR_C0, 4)
CH_PARAM(LMR_C1, 6)
CH_PARAM(LMR_C2, 3)

//
// EVAL PARAMS
//

CH_PARAM_ARRAY(PIECE_VALUES_MAG[5],
    SC( 100,  129), 
    SC( 335,  291), 
    SC( 366,  306), 
    SC( 468,  531), 
    SC( 980,  988),  )
CH_PARAM(PAWN_ISOLATED, SC(-9, -4))
CH_PARAM(PAWN_DOUBLED, SC(-6, -11))
CH_PARAM_ARRAY(KNIGHT_OUTPOST[4],
    SC(  -2,   -7), SC(   2,    2), SC(   6,   17), SC(  21,    8),  )
CH_PARAM(KNIGHT_BEHIND_PAWN, SC(5, 11))
CH_PARAM_ARRAY(KNIGHT_MOBILITY[9],
    SC( -22,  -41), SC( -16,  -65), SC( -14,  -19), 
    SC(  -5,   -6), SC(  -3,   -2), SC(  -5,    2), 
    SC(  -1,    2), SC(  11,    3), SC(   0,    0),  )
CH_PARAM_ARRAY(BISHOP_OUTPOST[4],
    SC(   1,   -1), SC(   5,    4), SC(   1,    3), SC(   6,   -5),  )
CH_PARAM(BISHOP_BEHIND_PAWN, SC(5, 15))
CH_PARAM_ARRAY(BISHOP_MOBILITY[15],
    SC(   0,    0), SC( -37,  -37), SC( -22,  -30), SC(  -8,  -16), 
    SC(  -1,   -6), SC(   5,   -3), SC(   7,    4), SC(   9,    6), 
    SC(  13,    8), SC(   9,   11), SC(  10,    7), SC(  31,    2), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(ROOK_MOBILITY[15],
    SC(   0,    0), SC( -79,  -87), SC( -19,  -51), SC( -14,  -23), 
    SC( -11,  -19), SC( -10,  -15), SC(  -5,   -7), SC(   1,   -8), 
    SC(   5,    0), SC(  11,    0), SC(  15,    7), SC(  20,   11), 
    SC(  24,   15), SC(  24,   15), SC(  24,   12),  )
CH_PARAM_ARRAY(QUEEN_MOBILITY[29],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC( -76,  -73), 
    SC( -20,  -49), SC( -20,  -69), SC( -14,  -38), SC( -12,  -39), 
    SC(  -8,  -17), SC(  -3,  -26), SC(  -1,  -12), SC(  -1,  -12), 
    SC(   3,   -7), SC(   3,   -5), SC(   4,    3), SC(   4,    8), 
    SC(   7,   13), SC(   8,   19), SC(  11,   10), SC(  12,   19), 
    SC(  22,   25), SC(  31,   23), SC(  19,   30), SC(  29,   31), 
    SC(  34,   22), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(   0,    0),  )
CH_PARAM_ARRAY(INIT_TABLE_PAWN[32],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(  25,  172), SC(  69,  154), SC(  77,  120), SC(  90,  104), 
    SC( -16,   68), SC(  -2,   71), SC(  25,   43), SC(  24,   36), 
    SC( -34,    0), SC(  -4,  -11), SC( -12,  -17), SC(   3,  -30), 
    SC( -40,  -22), SC( -21,  -26), SC( -20,  -35), SC(  -2,  -41), 
    SC( -33,  -31), SC(  -7,  -34), SC( -15,  -35), SC( -15,  -31), 
    SC( -38,  -25), SC(  -3,  -29), SC( -13,  -23), SC( -21,  -17), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT[32],
    SC(-131,  -50), SC( -73,  -38), SC( -72,  -18), SC(   1,  -28), 
    SC( -58,  -28), SC( -27,   -4), SC(  51,  -20), SC(  16,   -1), 
    SC(  -5,  -37), SC(  65,  -24), SC(  43,    2), SC(  56,    1), 
    SC(  12,  -21), SC(  17,   -2), SC(  30,   12), SC(  34,   14), 
    SC( -13,  -19), SC(   9,   -5), SC(  14,    9), SC(  18,   15), 
    SC( -15,  -21), SC(   9,  -17), SC(  13,  -12), SC(  11,    5), 
    SC( -13,  -36), SC( -29,  -18), SC(   2,  -18), SC(   4,   -9), 
    SC( -43,  -26), SC( -15,  -40), SC( -30,  -20), SC( -14,  -16),  )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP[32],
    SC( -29,  -15), SC( -24,  -14), SC( -77,   -5), SC( -59,   -5), 
    SC( -53,   -7), SC(  -1,   -8), SC(  -9,   -3), SC( -11,  -12), 
    SC( -26,   -2), SC(  22,  -11), SC(  25,   -3), SC(  23,   -9), 
    SC( -12,   -6), SC(  -2,   -1), SC(  11,    3), SC(  35,    7), 
    SC(  -9,  -12), SC(   4,  -10), SC(   1,    5), SC(  17,    6), 
    SC(   4,  -13), SC(  12,  -10), SC(  14,    1), SC(   8,    5), 
    SC(  11,  -18), SC(  19,  -19), SC(  13,  -11), SC(   1,   -1), 
    SC( -18,  -10), SC(  -6,   -4), SC(  -9,  -12), SC( -12,   -1),  )
CH_PARAM_ARRAY(INIT_TABLE_ROOK[32],
    SC(  10,   13), SC(  41,    1), SC(   4,   15), SC(  46,    5), 
    SC(  25,    8), SC(  26,    8), SC(  50,    3), SC(  44,    4), 
    SC(  -1,    7), SC(  28,    5), SC(  24,    3), SC(  20,    5), 
    SC( -25,   11), SC(  -2,    4), SC(  13,    9), SC(  25,    0), 
    SC( -34,    5), SC( -12,    5), SC( -16,    6), SC(  -1,    1), 
    SC( -40,    1), SC( -10,    0), SC( -13,   -5), SC( -12,   -4), 
    SC( -51,    4), SC( -13,   -6), SC(  -7,   -2), SC(  -7,   -2), 
    SC( -17,  -13), SC( -16,    5), SC(   6,   -5), SC(  16,   -2),  )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN[32],
    SC(  -2,   -3), SC(  12,    8), SC(  16,   24), SC(  32,   15), 
    SC(  -7,   -8), SC( -42,   28), SC(   6,   29), SC( -15,   48), 
    SC(  19,  -15), SC(  15,   -4), SC(  10,   19), SC(  12,   43), 
    SC(  -8,   17), SC( -23,   46), SC( -11,   28), SC( -10,   42), 
    SC(  -5,   -4), SC( -14,   37), SC( -14,   32), SC( -19,   42), 
    SC( -11,    2), SC(   7,  -23), SC(  -8,    7), SC(  -9,    4), 
    SC( -17,  -22), SC(  -9,  -16), SC(  14,  -36), SC(  -1,   -8), 
    SC(  -9,  -29), SC(  -4,  -36), SC(  -6,  -29), SC(  11,  -33),  )
CH_PARAM_ARRAY(INIT_TABLE_KING[32],
    SC( -32,  -26), SC(  68,  -21), SC(  23,    3), SC(  -5,  -11), 
    SC( -11,   10), SC(  10,   24), SC(  20,   30), SC(  34,   15), 
    SC(  18,   11), SC(  57,   32), SC(  73,   29), SC(  24,   17), 
    SC( -25,    4), SC(   5,   29), SC(   1,   35), SC(  -4,   33), 
    SC( -56,   -4), SC( -20,   13), SC( -36,   32), SC( -48,   39), 
    SC( -26,   -4), SC(  -8,    8), SC( -35,   25), SC( -50,   34), 
    SC(  17,  -19), SC(   2,    0), SC( -37,   19), SC( -55,   25), 
    SC(  14,  -52), SC(  31,  -26), SC( -20,   -6), SC(   4,  -21),  )

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static CH_FORCEINLINE int aspiration_next_delta(int delta)
{
    return delta + delta / ASPIRATION_C0 + ASPIRATION_C1;
}

static CH_FORCEINLINE int reverse_futility_margin(int depth, int beta)
{
    return beta + FUTILITY_PRUNING_C0 * depth;
}
static CH_FORCEINLINE int futility_margin(int depth, int alpha)
{
    return alpha - FUTILITY_PRUNING_C1 * depth;
}

static CH_FORCEINLINE int null_move_depth(int depth, int eval, int beta)
{
    (void)eval;
    (void)beta;
    return max(1, min(depth / 2, depth - 4));
}

static CH_FORCEINLINE int iid_depth(int depth)
{
    (void)depth;
    return IID_C0;
}

static CH_FORCEINLINE int lmp_min_n(int depth)
{
    return depth * LMP_C0 + LMP_C1;
}

static CH_FORCEINLINE int lmr_min_n(int depth)
{
    (void)depth;
    return LMR_C0;
}

static int lmr_reduction(int depth, int n)
{
    int reduction = 2;
    if(n >= LMR_C1) reduction += depth / LMR_C2;
    reduction = std::min(depth - 2, std::max(1, reduction));
    return reduction;
}

}
