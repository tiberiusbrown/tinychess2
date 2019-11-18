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
    SC( 100,  116), 
    SC( 338,  292), 
    SC( 369,  305), 
    SC( 478,  529), 
    SC( 994,  986),  )
CH_PARAM(PAWN_ISOLATED, SC(-13, -6))
CH_PARAM(PAWN_DOUBLED, SC(-4, -10))
CH_PARAM(PAWN_BACKWARD, SC(-2, -3))
CH_PARAM_ARRAY(PAWN_PASSED_FILE[4],
    SC(   2,   30), SC(  -8,   29), SC(  -6,   15), SC( -13,   19),  )
CH_PARAM(PAWN_PASSED_PROTECTED, SC(10, 8))
CH_PARAM_ARRAY(KNIGHT_OUTPOST[4],
    SC(   3,   -4), SC(   9,    1), SC(  10,   19), SC(  31,    8),  )
CH_PARAM(KNIGHT_BEHIND_PAWN, SC(4, 18))
CH_PARAM_ARRAY(KNIGHT_MOBILITY[9],
    SC( -36,  -77), SC( -21,  -74), SC( -20,  -26), 
    SC(  -9,  -10), SC(  -6,   -7), SC(  -7,    5), 
    SC(   0,    3), SC(  12,    3), SC(   0,    0),  )
CH_PARAM_ARRAY(BISHOP_OUTPOST[4],
    SC(   4,    2), SC(  24,   10), SC(   8,    9), SC(  23,   -3),  )
CH_PARAM(BISHOP_BEHIND_PAWN, SC(4, 23))
CH_PARAM_ARRAY(BISHOP_MOBILITY[15],
    SC(   0,    0), SC( -44,  -40), SC( -28,  -36), SC( -10,  -20), 
    SC(  -2,  -10), SC(   5,   -5), SC(   9,    2), SC(  14,    2), 
    SC(  15,    6), SC(  15,    9), SC(  19,    6), SC(  41,    2), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(ROOK_MOBILITY[15],
    SC(   0,    0), SC( -85,  -91), SC( -23,  -62), SC( -17,  -39), 
    SC( -16,  -21), SC( -13,  -18), SC(  -7,   -4), SC(   1,   -5), 
    SC(   5,    2), SC(  15,    1), SC(  23,    5), SC(  32,    8), 
    SC(  34,   12), SC(  35,   12), SC(  40,    7),  )
CH_PARAM_ARRAY(QUEEN_MOBILITY[29],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC( -81,  -94), 
    SC( -26,  -98), SC( -29,  -97), SC( -18,  -65), SC( -16,  -54), 
    SC( -10,  -47), SC(  -6,  -44), SC(  -4,  -26), SC(  -3,  -14), 
    SC(   3,   -9), SC(   5,   -7), SC(  12,    2), SC(   7,    9), 
    SC(  13,   14), SC(  13,   23), SC(  18,   17), SC(  21,   30), 
    SC(  27,   34), SC(  47,   19), SC(  26,   36), SC(  27,   47), 
    SC(  51,   28), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(   0,    0),  )
CH_PARAM_ARRAY(KING_PAWN_TROPISM[11],
    SC(   0,    0), SC(  45,   46), SC(  17,   24), 
    SC(   8,   13), SC(   1,    6), SC(   2,   -2), 
    SC(  -3,   -9), SC( -12,  -17), SC( -16,  -29), 
    SC( -25,  -56), SC( -38,  -41),  )
CH_PARAM_ARRAY(KING_SAFETY_MOVES[9],
    SC( -91,  -67), SC( -76,   11), SC( -22,    6), 
    SC(  -3,   13), SC(   9,   12), SC(   9,    2), 
    SC(   5,    3), SC(   9,   -1), SC(  26,  -23),  )
CH_PARAM_ARRAY(INIT_TABLE_PAWN[32],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(  -5,  168), SC(  57,  147), SC(  65,  128), SC(  81,  112), 
    SC( -28,   63), SC( -11,   66), SC(  14,   51), SC(  21,   38), 
    SC( -38,   -6), SC( -18,  -12), SC( -17,  -12), SC(  -2,  -25), 
    SC( -43,  -29), SC( -28,  -31), SC( -21,  -33), SC(  -4,  -40), 
    SC( -35,  -38), SC( -16,  -39), SC( -16,  -35), SC( -13,  -31), 
    SC( -39,  -33), SC( -10,  -39), SC( -14,  -27), SC( -15,  -24), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT[32],
    SC(-146,  -32), SC( -83,  -29), SC( -74,   -8), SC(   5,  -21), 
    SC( -66,  -17), SC( -29,   -1), SC(  58,  -27), SC(   7,    0), 
    SC( -12,  -33), SC(  53,  -28), SC(  27,    2), SC(  42,    2), 
    SC(   4,  -18), SC(  12,   -9), SC(  16,    9), SC(  27,   14), 
    SC( -17,  -16), SC(  14,  -12), SC(  10,    9), SC(   9,   12), 
    SC( -11,  -19), SC(  10,  -20), SC(  12,  -10), SC(  11,    3), 
    SC( -10,  -33), SC( -23,  -17), SC(   3,  -20), SC(  16,  -12), 
    SC( -43,  -17), SC(  -9,  -35), SC( -27,  -16), SC(  -8,  -13),  )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP[32],
    SC( -37,   -7), SC( -35,   -6), SC( -96,    5), SC( -78,    5), 
    SC( -71,   -9), SC( -17,   -9), SC( -20,    1), SC( -21,   -5), 
    SC( -38,   -8), SC(  10,  -13), SC(  16,   -4), SC(   8,   -7), 
    SC( -28,  -11), SC(  -4,   -9), SC(   0,    4), SC(  19,    4), 
    SC( -21,  -18), SC(  -4,  -10), SC(  -4,    2), SC(  14,    5), 
    SC(   4,   -7), SC(   9,   -7), SC(  14,   -1), SC(   7,    5), 
    SC(  10,  -13), SC(  19,  -18), SC(  11,   -9), SC(   3,    1), 
    SC( -17,   -5), SC(  -4,    0), SC(  -5,  -10), SC(  -6,    1),  )
CH_PARAM_ARRAY(INIT_TABLE_ROOK[32],
    SC(   5,   14), SC(  30,    5), SC(  -6,   20), SC(  38,    8), 
    SC(  18,    4), SC(  18,    7), SC(  46,    0), SC(  46,    0), 
    SC(  -4,    6), SC(  32,   -1), SC(  22,    4), SC(  14,    8), 
    SC( -27,    9), SC(  -8,    1), SC(   8,   11), SC(  14,    1), 
    SC( -38,    5), SC( -13,    2), SC( -17,    7), SC(  -2,    1), 
    SC( -39,    0), SC( -17,   -3), SC( -11,   -7), SC( -11,   -4), 
    SC( -50,    4), SC( -10,   -8), SC( -10,   -3), SC(  -6,   -3), 
    SC( -15,  -12), SC( -14,    1), SC(   4,   -4), SC(  16,   -4),  )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN[32],
    SC(   1,   -1), SC(   7,    9), SC(   7,   23), SC(  23,   21), 
    SC( -12,   -6), SC( -54,   34), SC( -13,   25), SC( -42,   59), 
    SC(   8,  -14), SC(   5,   -5), SC(  -7,   14), SC(  -6,   51), 
    SC( -13,   28), SC( -27,   44), SC( -17,   27), SC( -34,   53), 
    SC(  -7,    2), SC( -17,   36), SC( -17,   30), SC( -26,   42), 
    SC( -16,   11), SC(   4,  -22), SC( -12,   14), SC( -10,    2), 
    SC( -14,  -21), SC( -12,  -25), SC(  12,  -40), SC(   4,  -12), 
    SC(  -3,  -33), SC(  -2,  -38), SC(  -4,  -26), SC(  14,  -36),  )
CH_PARAM_ARRAY(INIT_TABLE_KING[32],
    SC( -18,  -26), SC(  81,  -20), SC(  34,    0), SC(   3,  -11), 
    SC( -10,    8), SC(   3,   31), SC(  19,   34), SC(  40,   17), 
    SC(  25,    2), SC(  60,   30), SC(  80,   24), SC(  20,   15), 
    SC( -33,   -8), SC(  -5,   22), SC( -10,   25), SC( -18,   23), 
    SC( -66,  -10), SC( -33,    5), SC( -62,   24), SC( -70,   29), 
    SC( -30,  -10), SC( -25,    7), SC( -49,   18), SC( -73,   31), 
    SC(  16,  -22), SC(   1,    3), SC( -50,   23), SC( -61,   27), 
    SC(  28,  -44), SC(  33,  -27), SC( -18,   -6), SC(   1,  -17),  )

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
