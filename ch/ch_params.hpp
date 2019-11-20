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

CH_PARAM(TEMPO_BONUS, 17)
CH_PARAM_ARRAY(PIECE_VALUES_MAG[5],
    SC( 100,  113), 
    SC( 338,  291), 
    SC( 370,  305), 
    SC( 480,  529), 
    SC(1000,  981),  )
CH_PARAM(PAWN_ISOLATED, SC(-14, -6))
CH_PARAM(PAWN_DOUBLED, SC(-2, -9))
CH_PARAM(PAWN_BACKWARD, SC(-2, -3))
CH_PARAM_ARRAY(PAWN_PASSED_FILE[4],
    SC(   2,   31), SC(  -9,   31), SC(  -6,   16), SC( -12,   20),  )
CH_PARAM(PAWN_PASSED_PROTECTED, SC(11, 8))
CH_PARAM_ARRAY(KNIGHT_OUTPOST[4],
    SC(   5,   -3), SC(  14,    3), SC(  11,   21), SC(  35,   10),  )
CH_PARAM(KNIGHT_BEHIND_PAWN, SC(3, 18))
CH_PARAM_ARRAY(KNIGHT_MOBILITY[9],
    SC( -37,  -82), SC( -24,  -68), SC( -21,  -25), 
    SC( -10,   -9), SC(  -6,   -8), SC(  -7,    6), 
    SC(   1,    2), SC(  13,    2), SC(   0,    0),  )
CH_PARAM_ARRAY(BISHOP_OUTPOST[4],
    SC(   9,    7), SC(  28,   11), SC(  12,   13), SC(  31,   -4),  )
CH_PARAM(BISHOP_BEHIND_PAWN, SC(4, 23))
CH_PARAM(BISHOP_BOTH_COLORS, SC(0, 13))
CH_PARAM_ARRAY(BISHOP_MOBILITY[15],
    SC(   0,    0), SC( -46,  -38), SC( -29,  -34), SC( -10,  -18), 
    SC(  -2,  -11), SC(   5,   -6), SC(  10,    1), SC(  14,    3), 
    SC(  15,    6), SC(  15,    8), SC(  18,    2), SC(  37,   -1), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(ROOK_MOBILITY[15],
    SC(   0,    0), SC( -83,  -87), SC( -24,  -62), SC( -18,  -38), 
    SC( -16,  -21), SC( -13,  -17), SC(  -7,   -2), SC(   1,   -3), 
    SC(   5,    3), SC(  16,    1), SC(  24,    4), SC(  34,    5), 
    SC(  35,   10), SC(  33,    9), SC(  42,    2),  )
CH_PARAM_ARRAY(QUEEN_MOBILITY[29],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC( -80, -101), 
    SC( -29, -112), SC( -29,  -97), SC( -20,  -72), SC( -17,  -57), 
    SC( -10,  -52), SC(  -6,  -45), SC(  -4,  -23), SC(  -2,  -15), 
    SC(   3,   -7), SC(   5,   -6), SC(  13,    4), SC(   8,   13), 
    SC(  14,   14), SC(  13,   25), SC(  13,   20), SC(  20,   27), 
    SC(  27,   30), SC(  49,   17), SC(  23,   36), SC(  25,   42), 
    SC(  49,   19), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(   0,    0),  )
CH_PARAM_ARRAY(KING_PAWN_TROPISM[11],
    SC(   0,    0), SC(  42,   51), SC(  17,   30), 
    SC(  10,   16), SC(   1,    9), SC(   2,   -2), 
    SC(  -5,  -12), SC( -15,  -21), SC(  -6,  -38), 
    SC( -11,  -60), SC( -41,  -43),  )
CH_PARAM_ARRAY(KING_SAFETY_MOVES[9],
    SC(-129,  -29), SC(-104,   34), SC( -29,   12), 
    SC(  -4,   14), SC(   9,   13), SC(   9,    2), 
    SC(   7,    3), SC(  10,   -2), SC(  29,  -24),  )
CH_PARAM_ARRAY(INIT_TABLE_PAWN[32],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(  -8,  177), SC(  66,  152), SC(  63,  136), SC(  83,  119), 
    SC( -29,   62), SC( -10,   64), SC(  12,   51), SC(  21,   39), 
    SC( -37,   -7), SC( -19,  -10), SC( -17,  -11), SC(  -2,  -25), 
    SC( -43,  -29), SC( -29,  -31), SC( -20,  -33), SC(  -4,  -40), 
    SC( -35,  -39), SC( -17,  -40), SC( -17,  -35), SC( -13,  -31), 
    SC( -39,  -35), SC( -10,  -40), SC( -14,  -28), SC( -15,  -26), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT[32],
    SC(-140,  -35), SC( -83,  -29), SC( -77,   -8), SC(   4,  -19), 
    SC( -60,  -21), SC( -25,    0), SC(  59,  -25), SC(   4,    2), 
    SC( -16,  -32), SC(  51,  -28), SC(  31,    2), SC(  42,    3), 
    SC(   2,  -19), SC(  12,   -6), SC(  19,   12), SC(  26,   15), 
    SC( -19,  -16), SC(  15,  -10), SC(  11,    8), SC(   8,   13), 
    SC( -11,  -19), SC(  12,  -16), SC(  12,   -9), SC(  13,    5), 
    SC(  -9,  -30), SC( -18,  -16), SC(   6,  -20), SC(  18,  -12), 
    SC( -39,  -18), SC(  -8,  -33), SC( -21,  -17), SC(  -7,  -12),  )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP[32],
    SC( -35,  -12), SC( -42,   -7), SC(-100,    5), SC( -84,    5), 
    SC( -84,   -9), SC(  -7,  -11), SC( -23,   -2), SC( -25,  -11), 
    SC( -42,  -10), SC(   5,  -17), SC(  13,   -7), SC(   6,  -10), 
    SC( -32,  -13), SC(  -8,  -11), SC(  -3,    1), SC(  13,    3), 
    SC( -25,  -19), SC(  -7,  -14), SC(  -7,    1), SC(   8,    1), 
    SC(   3,   -7), SC(   8,   -6), SC(  15,    0), SC(   6,    6), 
    SC(  11,  -15), SC(  20,  -17), SC(  11,   -9), SC(   4,    0), 
    SC( -16,   -4), SC(  -2,   -2), SC(  -5,  -10), SC(  -7,    1),  )
CH_PARAM_ARRAY(INIT_TABLE_ROOK[32],
    SC(   5,   14), SC(  31,    3), SC( -12,   20), SC(  37,    7), 
    SC(  16,    9), SC(  14,   11), SC(  44,    5), SC(  46,    2), 
    SC(  -1,    8), SC(  32,    2), SC(  23,    5), SC(  15,    8), 
    SC( -24,   10), SC(  -8,    3), SC(  10,   13), SC(  13,    5), 
    SC( -38,    7), SC( -13,    3), SC( -16,    8), SC(  -1,    2), 
    SC( -40,    0), SC( -18,   -1), SC( -11,   -5), SC( -11,   -4), 
    SC( -49,    3), SC(  -9,   -9), SC( -10,   -3), SC(  -4,   -4), 
    SC( -15,  -12), SC( -14,    1), SC(   4,   -4), SC(  16,   -5),  )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN[32],
    SC(   0,   -2), SC(   2,   12), SC(   0,   26), SC(  16,   20), 
    SC(  -9,   -6), SC( -53,   36), SC( -21,   28), SC( -49,   66), 
    SC(  -1,   -8), SC(  -3,    1), SC(  -5,   18), SC( -13,   56), 
    SC( -15,   29), SC( -26,   42), SC( -21,   30), SC( -33,   55), 
    SC(  -7,    2), SC( -19,   38), SC( -17,   30), SC( -23,   44), 
    SC( -15,   14), SC(   5,  -23), SC( -13,   15), SC(  -9,    3), 
    SC( -14,  -17), SC(  -9,  -26), SC(  12,  -41), SC(   4,  -14), 
    SC(   0,  -33), SC(  -2,  -36), SC(  -2,  -27), SC(  13,  -38),  )
CH_PARAM_ARRAY(INIT_TABLE_KING[32],
    SC( -20,  -31), SC(  83,  -21), SC(  38,   -4), SC(   7,  -13), 
    SC(  -3,    6), SC(   0,   32), SC(  15,   34), SC(  38,   17), 
    SC(  22,    1), SC(  58,   29), SC(  79,   24), SC(  13,   15), 
    SC( -34,   -9), SC(  -6,   22), SC( -12,   23), SC( -22,   21), 
    SC( -63,  -13), SC( -36,    4), SC( -67,   21), SC( -75,   28), 
    SC( -30,  -11), SC( -28,    7), SC( -55,   18), SC( -77,   29), 
    SC(  16,  -22), SC(  -2,    3), SC( -54,   23), SC( -64,   26), 
    SC(  42,  -45), SC(  34,  -26), SC( -17,   -6), SC(  -1,  -17),  )

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
