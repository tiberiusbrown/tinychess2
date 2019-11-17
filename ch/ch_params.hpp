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
    SC( 100,  130), 
    SC( 335,  291), 
    SC( 366,  306), 
    SC( 468,  531), 
    SC( 979,  990),  )

CH_PARAM(PAWN_ISOLATED, SC(0, 0))
CH_PARAM(PAWN_DOUBLED, SC(0, 0))

CH_PARAM_ARRAY(KNIGHT_OUTPOST[4],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), )
CH_PARAM_ARRAY(KNIGHT_MOBILITY[8],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), )

CH_PARAM_ARRAY(BISHOP_MOBILITY[14],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), )

CH_PARAM_ARRAY(ROOK_MOBILITY[14],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), )

CH_PARAM_ARRAY(QUEEN_MOBILITY[28],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), 
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0), )

CH_PARAM_ARRAY(INIT_TABLE_PAWN[32],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(  77,  154), SC(  99,  144), SC( 101,  111), SC( 100,  102), 
    SC(   5,   54), SC(  20,   63), SC(  35,   36), SC(  30,   31), 
    SC( -34,   -4), SC(   1,  -10), SC( -10,  -18), SC(   9,  -31), 
    SC( -39,  -26), SC( -14,  -24), SC( -21,  -35), SC(   0,  -41), 
    SC( -31,  -34), SC(  -1,  -29), SC( -16,  -35), SC( -18,  -29), 
    SC( -41,  -29), SC(   1,  -25), SC( -16,  -23), SC( -35,  -19), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT[32],
    SC( -65,  -70), SC( -55,  -53), SC( -73,  -20), SC(  -7,  -30), 
    SC( -60,  -38), SC( -27,  -19), SC(  39,  -24), SC(  17,  -10), 
    SC(  -7,  -38), SC(  36,  -15), SC(  45,    3), SC(  40,   10), 
    SC(   8,  -20), SC(  11,    1), SC(  32,   17), SC(  37,   22), 
    SC( -12,  -23), SC(   4,   -2), SC(  14,   13), SC(  22,   15), 
    SC( -19,  -26), SC(   8,  -16), SC(  16,   -8), SC(  13,    7), 
    SC( -21,  -51), SC( -41,  -23), SC(  -1,  -22), SC(   0,  -11), 
    SC( -58,  -57), SC( -19,  -48), SC( -37,  -28), SC( -26,  -26),  )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP[32],
    SC( -28,  -21), SC( -24,  -15), SC( -63,  -12), SC( -44,   -7), 
    SC( -40,  -12), SC(   7,   -7), SC(  -2,    1), SC( -10,   -6), 
    SC( -16,    0), SC(  29,   -7), SC(  28,    4), SC(  24,   -3), 
    SC(  -7,   -3), SC(  -3,    8), SC(  20,    9), SC(  37,   14), 
    SC( -11,   -8), SC(   2,   -4), SC(   8,   10), SC(  26,    7), 
    SC(   1,  -12), SC(  12,   -6), SC(  12,    5), SC(  11,   10), 
    SC(  -1,  -23), SC(  20,  -19), SC(  13,  -11), SC(   1,   -1), 
    SC( -36,  -24), SC( -14,  -13), SC( -17,  -26), SC( -29,  -10),  )
CH_PARAM_ARRAY(INIT_TABLE_ROOK[32],
    SC(  12,   20), SC(  41,   10), SC(   9,   23), SC(  38,   19), 
    SC(  24,   13), SC(  31,   14), SC(  43,   15), SC(  39,   14), 
    SC(   5,    7), SC(  20,   10), SC(  23,    6), SC(  21,    9), 
    SC( -21,    5), SC(  -7,    4), SC(  16,   12), SC(  30,   -1), 
    SC( -23,   -3), SC( -13,    4), SC( -10,    7), SC(   7,    0), 
    SC( -33,   -4), SC(  -6,   -4), SC(  -8,   -6), SC(  -6,   -3), 
    SC( -51,    0), SC( -11,   -4), SC(  -6,   -1), SC(  -5,    0), 
    SC( -17,  -16), SC( -18,    4), SC(   8,   -4), SC(  16,   -1),  )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN[32],
    SC(  -5,   -5), SC(  16,    3), SC(  22,   21), SC(  23,   17), 
    SC(  -7,  -15), SC( -27,    8), SC(  18,   23), SC(   4,   43), 
    SC(  18,  -12), SC(  18,  -10), SC(  12,   15), SC(  27,   40), 
    SC(  -2,    0), SC( -23,   45), SC(   6,   21), SC(   0,   35), 
    SC(  -4,  -15), SC(  -5,   21), SC(  -9,   31), SC(  -8,   41), 
    SC(  -8,   -9), SC(   8,  -13), SC(  -1,    6), SC(  -4,    3), 
    SC( -27,  -28), SC( -10,  -10), SC(  15,  -25), SC(  -1,   -5), 
    SC( -13,  -34), SC( -17,  -35), SC( -18,  -26), SC(   7,  -26),  )
CH_PARAM_ARRAY(INIT_TABLE_KING[32],
    SC( -21,  -28), SC(  43,  -13), SC(  20,    1), SC( -11,  -10), 
    SC(  -6,    9), SC(  12,   30), SC(  19,   28), SC(  17,   19), 
    SC(  13,   15), SC(  38,   33), SC(  61,   31), SC(  29,   16), 
    SC( -22,    2), SC(   3,   28), SC(   8,   31), SC(   1,   28), 
    SC( -33,  -10), SC( -12,    9), SC( -13,   26), SC( -14,   32), 
    SC( -20,   -6), SC(  -2,    7), SC( -18,   19), SC( -32,   27), 
    SC(  16,  -17), SC(   1,    2), SC( -31,   16), SC( -52,   21), 
    SC(   7,  -44), SC(  31,  -25), SC( -23,   -9), SC(   7,  -24),  )

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
