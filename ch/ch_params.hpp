#pragma once

#include "ch_internal.hpp"

#ifndef CH_TUNABLE
#define CH_PARAM(n_, default_) static constexpr int const n_ = default_;
#define CH_PARAM_ARRAY(n_, ...) static constexpr int const n_ = { __VA_ARGS__ };
#else
#ifndef CH_PARAM
#define CH_PARAM(n_, default_) extern int n_;
#define CH_PARAM_ARRAY(n_, ...) extern int n_;
#endif
#endif

namespace ch
{

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

#if 0

//
// EVAL PARAMS
//

CH_PARAM(MATERIAL_MG, 2048 + 650)
CH_PARAM(MATERIAL_EG, 650)

CH_PARAM_ARRAY(PIECE_VALUES_MAG[5], 100, 325, 335, 500, 975, )

CH_PARAM(HALF_OPEN_FILE, 8)

CH_PARAM(PAWN_PROTECT_ANY, 4)
CH_PARAM(PAWN_PROTECT_PAWN, 4)
CH_PARAM(PAWN_THREATEN_KNIGHT, 20)
CH_PARAM(PAWN_THREATEN_BISHOP, 25)
CH_PARAM(PAWN_THREATEN_ROOK, 60)
CH_PARAM(PAWN_THREATEN_QUEEN, 35)
CH_PARAM_ARRAY(PASSED_PAWN_MG[8], 0, 0, 4, 8, 16, 24, 32, 0, )
CH_PARAM_ARRAY(PASSED_PAWN_EG[8], 0, 4, 16, 32, 64, 128, 196, 0, )
CH_PARAM_ARRAY(PASSED_PAWN_FREE_EG[8], 0, 8, 32, 64, 128, 228, 324, 0, )
CH_PARAM(PASSED_PAWN_KING_ESCORT, 10)

CH_PARAM(KNIGHT_PAWN_BONUS_MG, 4)
CH_PARAM(KNIGHT_PAWN_BONUS_EG, 2)
CH_PARAM(KNIGHT_MOBILITY_BONUS_MG, 8)
CH_PARAM(KNIGHT_MOBILITY_BONUS_EG, 3)
CH_PARAM(KNIGHT_THREATEN_BISHOP, 0)
CH_PARAM(KNIGHT_THREATEN_ROOK, 0)
CH_PARAM(KNIGHT_THREATEN_QUEEN, 0)
CH_PARAM(KNIGHT_THREATEN_KING, 0)
CH_PARAM(KNIGHT_OUTPOST, 8)
CH_PARAM(KNIGHT_OUTPOST_HALF_OPEN_FILE, 4)
CH_PARAM(KNIGHT_OUTPOST_OPEN_FILE, 8)

CH_PARAM(BISHOP_MOBILITY_BONUS_MG, 5)
CH_PARAM(BISHOP_MOBILITY_BONUS_EG, 3)
CH_PARAM(BISHOP_THREATEN_ROOK, 15)
CH_PARAM(BISHOP_THREATEN_QUEEN, 30)
CH_PARAM(BISHOP_THREATEN_KING, 0)

CH_PARAM(ROOK_MOBILITY_BONUS_MG, 3)
CH_PARAM(ROOK_MOBILITY_BONUS_EG, 6)
CH_PARAM(ROOK_THREATEN_QUEEN, 20)
CH_PARAM(ROOK_THREATEN_KING, 0)
CH_PARAM(ROOK_ON_OPEN_FILE, 40)

CH_PARAM(QUEEN_MOBILITY_BONUS_MG, 4)
CH_PARAM(QUEEN_MOBILITY_BONUS_EG, 8)
CH_PARAM(QUEEN_ON_OPEN_FILE, 40)

CH_PARAM_ARRAY(KING_DEFENDERS_MG[12], 
    -32, -8, 1, 8, 16, 28, 32, 16, 12, 12, 12, 12,
    )

CH_PARAM_ARRAY(INIT_TABLE_PAWN_MG[32],
     0,  0,   0,   0,
    70, 70,  70,  70,
    20, 20,  20,  30,
     5,  5,  10,  25,
     0,  0,   0,  20,
     5, -5, -10,   0,
     5, 10,  10, -20,
     0,  0,   0,   0,
    )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_EG[32],
     0,  0,  0,  0,
    120, 120, 120, 120,
    70, 70, 70, 70,
    20, 20, 20, 20,
     5,  5,  5,  5,
     0,  0,  0,  0,
    -5, -5, -5, -5,
     0,  0,  0,  0,
    )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG[32],
    -50, -40, -30, -30,
    -40, -20,   0,   0,
    -30,   0,  10,  15,
    -30,   5,  15,  20,
    -30,   0,  15,  20,
    -30,   5,  10,  15,
    -40, -20,   0,   5,
    -50, -40, -30, -30,
    )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG[32],
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_MG[32],
    -20, -10, -10, -10,
    -10,   0,   0,   0,
    -10,   0,   5,  10,
    -10,   5,   5,  10,
    -10,   0,  10,  10,
    -10,  10,  10,  10,
    -10,   5,   0,   0,
    -20, -10, -10, -10,
    )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_EG[32],
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_MG[32],
     0,  0,  0,  0,
     5, 10, 10, 10,
    -5,  0,  0,  0,
    -5,  0,  0,  0,
    -5,  0,  0,  0,
    -5,  0,  0,  0,
    -5,  0,  0,  0,
     0,  0,  0,  5,
    )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_EG[32],
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_MG[32],
    -20, -10, -10, -5,
    -10,   0,   0,  0,
    -10,   0,   5,  5,
     -5,   0,   5,  5,
      0,   0,   5,  5,
    -10,   5,   5,  5,
    -10,   0,   5,  0,
    -20, -10, -10, -5,
    )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_EG[32],
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    )
CH_PARAM_ARRAY(INIT_TABLE_KING_MG[32],
    -30, -40, -40, -50,
    -30, -40, -40, -50,
    -30, -40, -40, -50,
    -30, -40, -40, -50,
    -20, -30, -30, -40,
    -10, -20, -20, -20,
     20,  20,   0,   0,
     20,  30,  10,   0,
    )
CH_PARAM_ARRAY(INIT_TABLE_KING_EG[32],
    -50, -40, -30, -20,
    -30, -20, -10,   0,
    -30, -10,  20,  30,
    -30, -10,  30,  40,
    -30, -10,  30,  40,
    -30, -10,  20,  30,
    -30, -30,   0,   0,
    -50, -30, -30, -30,
    )

#else

CH_PARAM(MATERIAL_MG, 2048 + 650)
CH_PARAM(MATERIAL_EG, 650)

CH_PARAM_ARRAY(KING_DEFENDERS_MG[12], 
    -32, -8, 1, 8, 16, 28, 32, 16, 12, 12, 12, 12,
    )
CH_PARAM_ARRAY(PIECE_VALUES_MAG[5],
     100,  361,  397,  593, 1242,     )
CH_PARAM(HALF_OPEN_FILE, 0)
CH_PARAM(PAWN_PROTECT_ANY, 9)
CH_PARAM(PAWN_PROTECT_PAWN, 11)
CH_PARAM(PAWN_THREATEN_KNIGHT, 34)
CH_PARAM(PAWN_THREATEN_BISHOP, 47)
CH_PARAM(PAWN_THREATEN_ROOK, 25)
CH_PARAM(PAWN_THREATEN_QUEEN, 42)
CH_PARAM_ARRAY(PASSED_PAWN_MG[8],
       0,    0,    1,   35,   57,   95,  153,    0,     )
CH_PARAM_ARRAY(PASSED_PAWN_EG[8],
       0,    0,    0,    0,    0,   30,   80,    0,     )
CH_PARAM_ARRAY(PASSED_PAWN_FREE_EG[8],
       0,    0,    0,    0,   56,  137,  237,    0,     )
CH_PARAM(PASSED_PAWN_KING_ESCORT, 14)
CH_PARAM(KNIGHT_PAWN_BONUS_MG, 6)
CH_PARAM(KNIGHT_PAWN_BONUS_EG, 12)
CH_PARAM(KNIGHT_MOBILITY_BONUS_MG, 8)
CH_PARAM(KNIGHT_MOBILITY_BONUS_EG, 3)
CH_PARAM(KNIGHT_THREATEN_BISHOP, 27)
CH_PARAM(KNIGHT_THREATEN_ROOK, 29)
CH_PARAM(KNIGHT_THREATEN_QUEEN, 29)
CH_PARAM(KNIGHT_THREATEN_KING, 61)
CH_PARAM(KNIGHT_OUTPOST, 10)
CH_PARAM(KNIGHT_OUTPOST_HALF_OPEN_FILE, 14)
CH_PARAM(KNIGHT_OUTPOST_OPEN_FILE, 0)
CH_PARAM(BISHOP_MOBILITY_BONUS_MG, 2)
CH_PARAM(BISHOP_MOBILITY_BONUS_EG, 12)
CH_PARAM(BISHOP_THREATEN_ROOK, 24)
CH_PARAM(BISHOP_THREATEN_QUEEN, 39)
CH_PARAM(BISHOP_THREATEN_KING, 95)
CH_PARAM(ROOK_MOBILITY_BONUS_MG, 4)
CH_PARAM(ROOK_MOBILITY_BONUS_EG, 9)
CH_PARAM(ROOK_THREATEN_QUEEN, 57)
CH_PARAM(ROOK_THREATEN_KING, 55)
CH_PARAM(ROOK_ON_OPEN_FILE, 12)
CH_PARAM(QUEEN_MOBILITY_BONUS_MG, 13)
CH_PARAM(QUEEN_MOBILITY_BONUS_EG, 1)
CH_PARAM(QUEEN_ON_OPEN_FILE, 0)
CH_PARAM_ARRAY(INIT_TABLE_PAWN_MG[32],
       0,    0,    0,    0, 
      76,   79,   79,   48, 
      39,   61,   58,   42, 
       9,   20,    4,   -1, 
      -1,    0,   -1,  -14, 
      -4,   -3,    0,   -5, 
      -4,    7,    2,    8, 
       0,    0,    0,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_EG[32],
       0,    0,    0,    0, 
      -6,    6,   31,  -45, 
     -18,    6,   -8,  -14, 
     -16,  -14,    7,   -1, 
     -15,  -23,   -5,   -4, 
      -6,  -22,  -22,  -26, 
      13,   -3,   -9,  -27, 
       0,    0,    0,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG[32],
     -82,  -62,  -41,  -34, 
     -45,  -35,  -10,  -20, 
     -38,  -22,  -12,    8, 
     -14,   -5,   12,   15, 
       0,    8,    7,   26, 
     -31,   -8,    8,   -1, 
     -39,  -39,  -20,  -22, 
     -48,  -35,  -50,  -30,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG[32],
     -70, -157,  -28,  -28, 
      -1,  -12,  -35,   46, 
      66,   55,   62,   48, 
      19,   58,   33,   51, 
     -12,   12,   50,   24, 
      18,   23,    9,   28, 
     -24,   17,   26,   25, 
     -11,  -24,   -3,  -26,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_MG[32],
     -10,  -23,  -28,  -23, 
     -30,  -17,  -17,  -16, 
     -18,  -13,  -28,  -26, 
     -17,  -13,  -12,  -16, 
     -27,  -12,  -19,   -6, 
     -28,  -26,   -9,   -7, 
     -36,  -32,  -20,  -18, 
     -20,  -35,  -27,  -31,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_EG[32],
     -62,  -71,  -24,  -83, 
      33,  -25,  -20,  -53, 
      17,   11,   21,   18, 
      11,    5,   15,   23, 
      12,  -11,   19,    8, 
      19,   19,    2,    3, 
      47,   26,   21,    8, 
       4,   18,   20,   24,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_MG[32],
      28,   33,   27,   34, 
      25,   29,   27,   26, 
      18,   28,   25,   24, 
      14,   23,   21,   21, 
       7,   18,   15,   23, 
       7,    7,    6,    7, 
     -17,    4,    2,   10, 
       3,   -5,    0,   -2,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_EG[32],
      24,    7,   25,    0, 
      21,   31,   32,   50, 
      24,   -1,    3,    6, 
      -1,   -3,   -9,    5, 
      -5,  -21,   -6,  -17, 
     -17,  -16,  -10,  -10, 
      12,   -6,    2,   -3, 
      -5,   16,   23,   32,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_MG[32],
     -24,   16,   12,   14, 
      25,   26,   25,   35, 
      35,   12,   -2,    3, 
      50,   13,   18,   -4, 
      30,    0,   11,    2, 
      24,    4,  -26,  -22, 
     -24,  -10,  -52,  -32, 
     -35,  -22,  -39,  -33,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_EG[32],
      59,  -43,   23,    1, 
      53,  -41,  -63,  -28, 
      20,   -6,    1,  -13, 
      14,  -13,  -25,  -31, 
       4,  -15,  -20,  -19, 
       3,   -7,   -6,   -4, 
     -14,  -13,   22,   13, 
      26,  -15,    3,   19,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_MG[32],
      20,   -6,  -16,  -22, 
      25,   37,   35,   20, 
      41,   56,   34,   30, 
      30,   45,   42,   44, 
      18,   43,   42,   52, 
       9,   37,   43,   45, 
       1,   29,   36,   40, 
     -12,    7,  -12,   -4,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_EG[32],
     -77,  -36,   -5,   51, 
      -6,   54,   13,   58, 
      11,   60,   26,   29, 
     -25,   -3,    0,   10, 
     -35,  -46,  -39,  -63, 
       6,  -41,  -56,  -64, 
      17,  -40,  -56,  -76, 
      42,  -40,    1,  -13,     )

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
