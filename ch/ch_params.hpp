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

CH_PARAM(TEMPO_BONUS, 18)
CH_PARAM_ARRAY(PIECE_VALUES_MAG[5],
    SC( 100,  112), 
    SC( 340,  290), 
    SC( 371,  305), 
    SC( 479,  527), 
    SC(1011,  971),  )
CH_PARAM(PAWN_ISOLATED, SC(-14, -6))
CH_PARAM(PAWN_DOUBLED, SC(-2, -9))
CH_PARAM(PAWN_BACKWARD, SC(-2, -3))
CH_PARAM_ARRAY(PAWN_PASSED_FILE[4],
    SC(   2,   32), SC(  -9,   32), SC(  -7,   17), SC( -12,   20),  )
CH_PARAM(PAWN_PASSED_PROTECTED, SC(12, 8))
CH_PARAM_ARRAY(KNIGHT_OUTPOST[4],
    SC(   6,   -2), SC(  18,    6), SC(  12,   22), SC(  39,   14),  )
CH_PARAM(KNIGHT_BEHIND_PAWN, SC(3, 17))
CH_PARAM_ARRAY(KNIGHT_MOBILITY[9],
    SC( -28,  -70), SC( -18,  -72), SC( -21,  -25), 
    SC( -11,  -10), SC(  -6,   -9), SC(  -6,    3), 
    SC(   1,    2), SC(  13,    2), SC(   0,    0),  )
CH_PARAM_ARRAY(KNIGHT_MOBILITY_MOVE[9],
    SC( -12,  -24), SC(  -7,   -1), SC(  -1,   -4), 
    SC(  -2,   -2), SC(   1,   -2), SC(   1,    1), 
    SC(   1,    0), SC(   2,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(KNIGHT_NUM_PAWNS[9],
    SC( -98,  -82), SC(  41,  -14), SC(  10,   -8), 
    SC(   4,   -6), SC(   3,   -2), SC(   0,    0), 
    SC(  -1,    5), SC(   0,   14), SC(   5,   12),  )
CH_PARAM_ARRAY(KNIGHT_NUM_ENEMY_PAWNS[9],
    SC(-106,  -80), SC(  -5,   -7), SC(  -6,    2), 
    SC(  -5,    5), SC(  -3,    8), SC(  -1,    6), 
    SC(   2,    1), SC(   1,    3), SC(   1,    0),  )
CH_PARAM_ARRAY(BISHOP_OUTPOST[4],
    SC(  15,    8), SC(  29,   11), SC(  19,   16), SC(  33,   -3),  )
CH_PARAM(BISHOP_BEHIND_PAWN, SC(3, 22))
CH_PARAM(BISHOP_BOTH_COLORS, SC(-1, 25))
CH_PARAM_ARRAY(BISHOP_MOBILITY[15],
    SC(   0,    0), SC( -45,  -38), SC( -30,  -33), SC( -11,  -20), 
    SC(  -1,  -11), SC(   6,   -5), SC(  11,    0), SC(  14,    2), 
    SC(  14,    5), SC(  15,    5), SC(  17,    2), SC(  36,   -5), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(ROOK_MOBILITY[15],
    SC(   0,    0), SC( -82,  -90), SC( -22,  -62), SC( -16,  -38), 
    SC( -13,  -24), SC( -11,  -15), SC(  -9,   -2), SC(  -3,   -3), 
    SC(   3,    4), SC(  15,    1), SC(  23,    3), SC(  37,    4), 
    SC(  36,   10), SC(  38,    8), SC(  46,    1),  )
CH_PARAM_ARRAY(QUEEN_MOBILITY[29],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(-109, -123), 
    SC( -52, -125), SC( -29, -120), SC( -22,  -67), SC( -18,  -57), 
    SC( -11,  -55), SC(  -7,  -44), SC(  -3,  -27), SC(  -1,  -16), 
    SC(   3,   -8), SC(   5,   -5), SC(  13,    5), SC(  11,   10), 
    SC(  16,   13), SC(  13,   25), SC(  17,   20), SC(  23,   26), 
    SC(  23,   27), SC(  50,   15), SC(  19,   38), SC(  29,   40), 
    SC(  49,   14), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(   0,    0),  )
CH_PARAM_ARRAY(KING_PAWN_TROPISM[11],
    SC(   0,    0), SC(  33,   50), SC(  18,   30), 
    SC(  12,   16), SC(   1,   10), SC(   3,   -3), 
    SC(  -7,  -13), SC( -17,  -20), SC(  -2,  -36), 
    SC(  -5,  -56), SC( -40,  -40),  )
CH_PARAM_ARRAY(KING_SAFETY_MOVES[9],
    SC(-179,   16), SC(-109,   37), SC( -40,   16), 
    SC(  -4,   14), SC(  10,   12), SC(  10,    1), 
    SC(  13,    1), SC(  13,   -4), SC(  31,  -24),  )
CH_PARAM(KING_ON_HALF_OPEN_FILE, SC(-14, 14))
CH_PARAM(KING_ON_OPEN_FILE, SC(-32, 4))
CH_PARAM(KING_NEAR_HALF_OPEN_FILE, SC(-29, 15))
CH_PARAM(KING_NEAR_OPEN_FILE, SC(-9, -4))
CH_PARAM_ARRAY(INIT_TABLE_PAWN[32],
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0), 
    SC(  -8,  175), SC(  66,  152), SC(  57,  137), SC(  88,  120), 
    SC( -30,   63), SC( -14,   66), SC(  13,   52), SC(  12,   44), 
    SC( -38,   -7), SC( -19,  -11), SC( -19,   -9), SC(  -3,  -22), 
    SC( -44,  -29), SC( -30,  -31), SC( -20,  -32), SC(  -4,  -39), 
    SC( -36,  -39), SC( -17,  -40), SC( -17,  -35), SC( -13,  -31), 
    SC( -40,  -37), SC( -10,  -40), SC( -14,  -28), SC( -16,  -26), 
    SC(   0,    0), SC(   0,    0), SC(   0,    0), SC(   0,    0),  )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT[32],
    SC(-141,  -27), SC( -77,  -20), SC( -82,    1), SC(   5,  -13), 
    SC( -67,  -20), SC( -28,    4), SC(  55,  -23), SC(   5,    2), 
    SC( -25,  -28), SC(  44,  -24), SC(  27,    2), SC(  40,    4), 
    SC(   1,  -18), SC(  10,   -8), SC(  16,   10), SC(  25,   15), 
    SC( -22,  -15), SC(  12,   -6), SC(   9,    7), SC(  10,   11), 
    SC(  -9,  -12), SC(  13,  -17), SC(  12,  -10), SC(  12,    6), 
    SC(  -5,  -30), SC( -16,  -12), SC(   6,  -18), SC(  20,  -14), 
    SC( -37,  -10), SC(  -5,  -30), SC( -20,  -14), SC(   0,  -11),  )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP[32],
    SC( -35,  -15), SC( -41,  -10), SC(-102,    3), SC( -88,    3), 
    SC( -90,  -10), SC( -11,  -19), SC( -27,   -8), SC( -30,  -14), 
    SC( -48,  -10), SC(  -4,  -18), SC(   9,  -10), SC(   0,  -13), 
    SC( -33,  -14), SC( -11,  -14), SC(  -8,   -2), SC(  10,    2), 
    SC( -27,  -21), SC( -11,  -19), SC( -11,   -5), SC(   6,   -2), 
    SC(   7,  -11), SC(  11,   -6), SC(  15,    1), SC(   6,    6), 
    SC(  14,  -18), SC(  22,  -18), SC(  12,  -10), SC(   2,   -1), 
    SC( -12,   -8), SC(   3,   -4), SC(  -4,  -10), SC(  -1,   -3),  )
CH_PARAM_ARRAY(INIT_TABLE_ROOK[32],
    SC(   7,   13), SC(  24,    6), SC( -13,   21), SC(  37,    8), 
    SC(  13,   10), SC(  14,   12), SC(  42,    5), SC(  47,    2), 
    SC(   1,    7), SC(  35,    2), SC(  21,    7), SC(  16,    9), 
    SC( -21,   10), SC(  -7,    4), SC(  15,   12), SC(  15,    4), 
    SC( -33,    6), SC( -10,    3), SC( -10,    6), SC(   1,    2), 
    SC( -36,    2), SC( -13,   -2), SC(  -7,   -6), SC(  -7,   -6), 
    SC( -46,    2), SC(  -7,  -10), SC( -10,   -2), SC(  -1,   -3), 
    SC( -14,  -11), SC( -13,    0), SC(   0,   -4), SC(  14,   -8),  )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN[32],
    SC(   2,   -1), SC(   3,   11), SC(  -8,   35), SC(  15,   24), 
    SC(  -7,   -5), SC( -52,   37), SC( -20,   29), SC( -53,   72), 
    SC(  -1,    0), SC(  -2,    3), SC(  -3,   19), SC( -14,   62), 
    SC( -14,   34), SC( -24,   41), SC( -23,   35), SC( -34,   59), 
    SC(  -3,    1), SC( -14,   36), SC( -13,   31), SC( -24,   48), 
    SC( -10,   10), SC(   9,  -24), SC( -11,   16), SC(  -9,    3), 
    SC( -11,  -21), SC(  -1,  -32), SC(  15,  -43), SC(   7,  -19), 
    SC(   7,  -35), SC(  -6,  -35), SC(  -6,  -29), SC(   8,  -44),  )
CH_PARAM_ARRAY(INIT_TABLE_KING[32],
    SC(  -3,  -38), SC(  85,  -22), SC(  45,   -8), SC(  10,  -14), 
    SC(  -2,    8), SC(   1,   31), SC(  10,   35), SC(  40,   17), 
    SC(  24,    1), SC(  54,   30), SC(  72,   23), SC(  16,   13), 
    SC( -32,   -7), SC( -16,   22), SC( -14,   23), SC( -22,   22), 
    SC( -66,  -13), SC( -46,    4), SC( -69,   20), SC( -81,   28), 
    SC( -28,  -13), SC( -34,    6), SC( -58,   16), SC( -83,   29), 
    SC(  16,  -24), SC(  -6,    2), SC( -62,   22), SC( -70,   26), 
    SC(  48,  -49), SC(  38,  -29), SC( -16,   -7), SC(   1,  -17),  )

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
