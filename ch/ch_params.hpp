#pragma once

#include <algorithm>

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
static constexpr int aspiration_next_delta(int delta)
{
    return delta + delta / ASPIRATION_C0 + ASPIRATION_C1;
}

// seems to hinder right now
#define CH_ENABLE_PROBCUT_PRUNING 0
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
#define CH_ENABLE_FUTILITY_PRUNING 1
CH_PARAM(FUTILITY_PRUNING_MAX_DEPTH, 7)
CH_PARAM(FUTILITY_PRUNING_C0, 64)
CH_PARAM(FUTILITY_PRUNING_C1, 64)
static constexpr int reverse_futility_margin(int depth, int beta)
{
    return beta + FUTILITY_PRUNING_C0 * depth;
}
static constexpr int futility_margin(int depth, int alpha)
{
    return alpha - FUTILITY_PRUNING_C1 * depth;
}

// null move pruning
#define CH_ENABLE_NULL_MOVE 1
CH_PARAM(NULL_MOVE_MIN_DEPTH, 5)
static constexpr int null_move_depth(int depth, int eval, int beta)
{
    (void)eval;
    (void)beta;
    return std::max(1, std::min(depth / 2, depth - 4));
}

// internal iterative deepening
#define CH_ENABLE_IID 1
CH_PARAM(IID_MIN_DEPTH, 6)
CH_PARAM(IID_C0, 3)
static constexpr int iid_depth(int depth)
{
    (void)depth;
    return IID_C0;
}

// late move pruning
#define CH_ENABLE_LATE_MOVE_PRUNING 0
CH_PARAM(LMP_MAX_DEPTH, 4)
CH_PARAM(LMP_C0, 7)
CH_PARAM(LMP_C1, 0)
static constexpr int lmp_min_n(int depth)
{
    return depth * LMP_C0 + LMP_C1;
}

// late move reduction
#define CH_ENABLE_LATE_MOVE_REDUCTION 1
CH_PARAM(LMR_MIN_DEPTH, 3)
CH_PARAM(LMR_C0, 4)
static constexpr int lmr_min_n(int depth)
{
    (void)depth;
    return LMR_C0;
}
CH_PARAM(LMR_C1, 6)
CH_PARAM(LMR_C2, 3)
static int lmr_reduction(int depth, int n)
{
    int reduction = 2;
    if(n >= LMR_C1) reduction += depth / LMR_C2;
    reduction = std::min(depth - 2, std::max(1, reduction));
    return reduction;
}

//
// EVAL PARAMS
//

#if 0

CH_PARAM(HALF_OPEN_FILE, 8)

CH_PARAM(PAWN_PROTECT_ANY, 4)
CH_PARAM(PAWN_PROTECT_PAWN, 4)
CH_PARAM(PAWN_THREATEN_KNIGHT, 20)
CH_PARAM(PAWN_THREATEN_BISHOP, 15)
CH_PARAM(PAWN_THREATEN_ROOK, 40)
CH_PARAM(PAWN_THREATEN_QUEEN, 25)
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

CH_PARAM(QUEEN_ON_OPEN_FILE, 40)

CH_PARAM_ARRAY(KING_DEFENDERS_MG[12], 
    -32, -8, 1, 8, 16, 28, 32, 16, 12, 12, 12, 12,
    )

CH_PARAM_ARRAY(INIT_TABLE_PAWN_MG[32],
     0,  0,   0,   0,
    50, 50,  50,  50,
    10, 10,  20,  30,
     5,  5,  10,  25,
     0,  0,   0,  20,
     5, -5, -10,   0,
     5, 10,  10, -20,
     0,  0,   0,   0,
    )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_EG[32],
     0,  0,  0,  0,
    90, 90, 90, 90,
    50, 50, 50, 50,
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

CH_PARAM_ARRAY(INIT_TABLE_PAWN_MG[32],
       0,    9,    1,   -8, 
      37,   47,   56,   47, 
     -33,   13,   50,   14, 
      27,    9,   12,   30, 
       0,   -2,  -15,    1, 
       5,   -5,  -30,   -3, 
      20,   18,   15,  -23, 
      -2,    0,    1,    3,     )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_EG[32],
       0,  -25,  -30,  -14, 
     105,   89,   89,  114, 
       5,   48,   60,   48, 
      43,   11,  -15,   17, 
       5,    3,    7,    5, 
      -8,    4,   20,    0, 
      -5,   -5,   -3,   -6, 
       0,   17,    0,   18,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG[32],
     -51,  -55,   -6,  -14, 
     -29,   11,  -19,  -20, 
     -44,   15,   45,    7, 
     -28,    6,   32,   19, 
     -27,   26,   11,   20, 
     -30,    0,   -4,   15, 
     -33,  -29,  -13,   -1, 
     -50,  -40,  -27,  -22,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG[32],
      -2,   -2,   -7,    0, 
       6,   -1,  -10,   -6, 
     -47,    3,  -36,   49, 
      -6,   15,  -39,   -5, 
       0,   -2,    1,   -6, 
     -34,  -21,   13,    7, 
     -15,    3,   20,   13, 
     -11,    2,   -1,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_MG[32],
     -37,  -24,  -10,   -1, 
     -44,  -62,    8,    8, 
       4,  -10,   21,   15, 
      -2,   10,    7,   -3, 
     -10,   -5,   10,   15, 
      -7,  -13,   10,    7, 
       3,    5,   -3,    1, 
      12,    8,  -11,   -9,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_EG[32],
      21,  -24,   44,    7, 
     -20,    3,   -2,   20, 
      26,  -37,   -4,  -23, 
      38,   10,  -19,    5, 
       6,    6,   11,   -2, 
       2,  -13,    0,    1, 
       1,   26,    2,   -1, 
      -2,  -10,   -7,   -1,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_MG[32],
      29,  -11,    6,  -20, 
      34,   28,   71,   45, 
     -32,  -34,   -9,  -20, 
     -13,   19,    0,    9, 
       9,   -3,   -3,   -5, 
      -7,  -13,   -2,    5, 
      -2,  -14,  -19,    2, 
      -3,   -2,    0,    5,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_EG[32],
      16,   42,    0,   -1, 
      66,   67,    7,   20, 
      26,    4,    0,   20, 
      17,  -17,    0,  -16, 
     -35,    2,  -22,    0, 
      14,    2,  -11,   -3, 
      20,   14,   24,    0, 
      -8,   -5,    2,   -4,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_MG[32],
     -48,  -56,   -7,   -6, 
     101,    0,   59,   71, 
       7,   21,  -17,   20, 
     -10,  -26,  -18,   34, 
      -3,   -8,    5,   15, 
      -7,  -11,    3,    5, 
     -14,    2,    8,    0, 
     -21,  -13,    1,   -5,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_EG[32],
     -12,   -1,  -29,   -5, 
       0,   -1,   -1,   19, 
      13,    4,  -19,  -22, 
      -2,   21,    7,  -17, 
      -1,   24,  -13,    8, 
      20,   19,   -7,   -1, 
      -1,   10,  -15,  -12, 
      18,  -13,  -31,    1,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_MG[32],
      20,  -64,  -12,  -40, 
      -3,  -45,  -25,  -64, 
     -45,  -38,  -57,  -52, 
     -46,  -20,  -41,  -27, 
     -32,  -25,  -16,  -42, 
      11,  -19,  -11,   10, 
       8,   19,   -1,   -7, 
      18,   15,   11,   16,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_EG[32],
     -50,  -35,  -33,   -2, 
     -19,  -17,   -4,   16, 
     -27,   23,   42,   23, 
     -58,  -18,   28,   38, 
       0,  -19,   88,   26, 
     -29,  -13,   20,   26, 
     -21,  -18,    9,   42, 
     -50,  -50,  -57,    1,     )
CH_PARAM(HALF_OPEN_FILE, 6)
CH_PARAM(PAWN_PROTECT_ANY, 5)
CH_PARAM(PAWN_PROTECT_PAWN, 11)
CH_PARAM(PAWN_THREATEN_KNIGHT, 21)
CH_PARAM(PAWN_THREATEN_BISHOP, 11)
CH_PARAM(PAWN_THREATEN_ROOK, 36)
CH_PARAM(PAWN_THREATEN_QUEEN, 64)
CH_PARAM_ARRAY(PASSED_PAWN_MG[8],
       0,    0,    2,   12,   34,   31,   33,    3,     )
CH_PARAM_ARRAY(PASSED_PAWN_EG[8],
      14,   38,   39,   43,   40,  158,  173,   18,     )
CH_PARAM_ARRAY(PASSED_PAWN_FREE_EG[8],
       0,   33,   25,   13,   45,  188,  439,   90,     )
CH_PARAM(PASSED_PAWN_KING_ESCORT, 11)
CH_PARAM(KNIGHT_PAWN_BONUS_MG, 4)
CH_PARAM(KNIGHT_PAWN_BONUS_EG, 1)
CH_PARAM(KNIGHT_MOBILITY_BONUS_MG, 10)
CH_PARAM(KNIGHT_MOBILITY_BONUS_EG, 5)
CH_PARAM(KNIGHT_THREATEN_BISHOP, 0)
CH_PARAM(KNIGHT_THREATEN_ROOK, 1)
CH_PARAM(KNIGHT_THREATEN_QUEEN, 3)
CH_PARAM(KNIGHT_THREATEN_KING, 2)
CH_PARAM(KNIGHT_OUTPOST, 9)
CH_PARAM(KNIGHT_OUTPOST_HALF_OPEN_FILE, 7)
CH_PARAM(KNIGHT_OUTPOST_OPEN_FILE, 7)
CH_PARAM(BISHOP_MOBILITY_BONUS_MG, 5)
CH_PARAM(BISHOP_MOBILITY_BONUS_EG, 6)
CH_PARAM(BISHOP_THREATEN_ROOK, 17)
CH_PARAM(BISHOP_THREATEN_QUEEN, 37)
CH_PARAM(BISHOP_THREATEN_KING, 0)
CH_PARAM(ROOK_MOBILITY_BONUS_MG, 2)
CH_PARAM(ROOK_MOBILITY_BONUS_EG, 6)
CH_PARAM(ROOK_THREATEN_QUEEN, 21)
CH_PARAM(ROOK_THREATEN_KING, 6)
CH_PARAM(ROOK_ON_OPEN_FILE, 27)
CH_PARAM(QUEEN_ON_OPEN_FILE, 4)
CH_PARAM_ARRAY(KING_DEFENDERS_MG[12],
     -33,  -20,  -22,   19,   30,   21, 
      23,    3,   18,   24,    1,   11,     )

#endif

}
