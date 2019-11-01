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
CH_PARAM(ASPIRATION_MIN_DEPTH, 3)
CH_PARAM(ASPIRATION_C0, 5)
CH_PARAM(ASPIRATION_C1, 3)
static CH_FORCEINLINE int aspiration_next_delta(int delta)
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
CH_PARAM(FUTILITY_PRUNING_C1, 56)
static CH_FORCEINLINE int reverse_futility_margin(int depth, int beta)
{
    return beta + FUTILITY_PRUNING_C0 * depth;
}
static CH_FORCEINLINE int futility_margin(int depth, int alpha)
{
    return alpha - FUTILITY_PRUNING_C1 * depth;
}

// null move pruning
#define CH_ENABLE_NULL_MOVE 1
CH_PARAM(NULL_MOVE_MIN_DEPTH, 5)
static CH_FORCEINLINE int null_move_depth(int depth, int eval, int beta)
{
    (void)eval;
    (void)beta;
    return max(1, min(depth / 2, depth - 4));
}

// internal iterative deepening
#define CH_ENABLE_IID 1
CH_PARAM(IID_MIN_DEPTH, 6)
CH_PARAM(IID_C0, 3)
static CH_FORCEINLINE int iid_depth(int depth)
{
    (void)depth;
    return IID_C0;
}

// late move pruning
#define CH_ENABLE_LATE_MOVE_PRUNING 0
CH_PARAM(LMP_MAX_DEPTH, 4)
CH_PARAM(LMP_C0, 7)
CH_PARAM(LMP_C1, 0)
static CH_FORCEINLINE int lmp_min_n(int depth)
{
    return depth * LMP_C0 + LMP_C1;
}

// late move reduction
#define CH_ENABLE_LATE_MOVE_REDUCTION 1
CH_PARAM(LMR_MIN_DEPTH, 3)
CH_PARAM(LMR_C0, 4)
static CH_FORCEINLINE int lmr_min_n(int depth)
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
       0,    0,    0,    0, 
      61,   50,   47,   52, 
      10,    5,   20,   31, 
       5,   -1,   10,   25, 
       1,    0,    5,   22, 
       7,   -5,  -14,   -1, 
       3,    1,   16,  -20, 
       0,    0,   -3,   -1,     )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_EG[32],
      -1,    2,    0,    0, 
      90,   84,   94,   93, 
      50,   40,   50,   50, 
      27,   20,   23,   18, 
      -6,    3,   10,    8, 
       0,    1,    0,  -12, 
      -5,   -5,   -5,  -10, 
       0,    0,   -1,    3,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG[32],
     -60,  -40,  -45,  -32, 
     -38,   14,   11,    1, 
     -27,   13,   13,   12, 
     -20,    3,    5,   15, 
     -56,  -20,    3,   19, 
     -27,    5,   10,   16, 
     -32,  -19,  -10,    2, 
     -45,  -40,  -31,  -26,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG[32],
       0,    2,   26,  -10, 
      -2,   -6,  -12,   -8, 
       2,    3,   -1,   -6, 
       1,   -9,    9,    0, 
      -1,  -13,    0,    0, 
       5,   -1,  -13,    3, 
       9,    8,    0,    0, 
       0,    6,    9,   -6,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_MG[32],
     -20,  -15,  -15,  -14, 
     -14,    3,   -9,    1, 
     -15,   -5,    8,   10, 
     -17,    6,    4,   18, 
      -4,   -2,   15,   10, 
     -10,   10,   10,   10, 
     -11,   13,    8,   -3, 
     -28,   -9,   -3,  -11,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_EG[32],
       7,    7,    2,   -3, 
       2,   39,    0,   -5, 
     -17,  -17,   -4,    2, 
       0,   -7,    9,    0, 
       7,   14,   15,   -7, 
      11,   -1,    0,   -6, 
      26,    0,   -1,    2, 
       3,   -3,  -20,   -2,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_MG[32],
      -3,   -6,    3,   -5, 
       8,   11,    9,   13, 
      -3,    3,    2,    0, 
      -6,    8,   -2,  -13, 
     -15,   -1,    0,   -2, 
       5,    1,   -6,  -11, 
      -4,    0,    8,    7, 
      -1,    0,    0,    5,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_EG[32],
       1,  -27,   -2,   -3, 
      -1,   -1,    4,   -1, 
       8,    2,    2,   14, 
       0,  -10,   11,    1, 
     -13,    7,   -5,   -2, 
      -1,    4,    0,    1, 
      -9,    6,   -1,   -2, 
       0,    8,   -1,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_MG[32],
     -21,  -14,  -15,  -25, 
      -1,    3,    7,    1, 
      -5,    2,    0,    4, 
      -5,   -1,    5,   14, 
     -10,   -1,    5,    4, 
     -22,    5,    9,    1, 
     -16,  -25,    6,    0, 
     -23,  -12,  -11,   -5,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_EG[32],
      -5,   -1,    2,    3, 
      -9,   -7,   -1,    3, 
       8,    6,   -5,   -7, 
      -2,   -9,   -8,   -4, 
      10,    7,    1,    1, 
       1,   -2,    3,    6, 
      12,    2,    0,   -2, 
      -1,    4,    9,    6,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_MG[32],
     -34,  -40,  -42,  -47, 
     -34,  -40,  -42,  -50, 
     -28,  -42,  -43,  -49, 
     -35,  -44,  -39,  -50, 
     -20,  -26,  -33,  -40, 
     -10,  -18,  -18,  -20, 
      20,   20,   -7,   -5, 
      20,   41,   11,    2,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_EG[32],
     -43,  -40,  -34,  -20, 
     -31,  -20,  -10,    0, 
     -30,  -10,   19,   34, 
     -30,   -7,   39,   49, 
     -30,  -10,   32,   33, 
     -33,   -1,   20,   35, 
     -30,  -30,    0,    4, 
     -59,  -26,  -38,  -28,     )
CH_PARAM(HALF_OPEN_FILE, 8)
CH_PARAM(PAWN_PROTECT_ANY, 1)
CH_PARAM(PAWN_PROTECT_PAWN, 18)
CH_PARAM(PAWN_THREATEN_KNIGHT, 25)
CH_PARAM(PAWN_THREATEN_BISHOP, 22)
CH_PARAM(PAWN_THREATEN_ROOK, 63)
CH_PARAM(PAWN_THREATEN_QUEEN, 25)
CH_PARAM_ARRAY(PASSED_PAWN_MG[8],
       0,    0,    1,    0,   16,   17,   22,    0,     )
CH_PARAM_ARRAY(PASSED_PAWN_EG[8],
      0,    0,   57,   27,   72,   89,  195,    0,     )
CH_PARAM_ARRAY(PASSED_PAWN_FREE_EG[8],
      0,    0,    0,    1,  161,  162,  258,   0,     )
CH_PARAM(PASSED_PAWN_KING_ESCORT, 12)
CH_PARAM(KNIGHT_PAWN_BONUS_MG, 4)
CH_PARAM(KNIGHT_PAWN_BONUS_EG, 0)
CH_PARAM(KNIGHT_MOBILITY_BONUS_MG, 8)
CH_PARAM(KNIGHT_MOBILITY_BONUS_EG, 10)
CH_PARAM(KNIGHT_THREATEN_BISHOP, 0)
CH_PARAM(KNIGHT_THREATEN_ROOK, 12)
CH_PARAM(KNIGHT_THREATEN_QUEEN, 11)
CH_PARAM(KNIGHT_THREATEN_KING, 0)
CH_PARAM(KNIGHT_OUTPOST, 10)
CH_PARAM(KNIGHT_OUTPOST_HALF_OPEN_FILE, 1)
CH_PARAM(KNIGHT_OUTPOST_OPEN_FILE, 9)
CH_PARAM(BISHOP_MOBILITY_BONUS_MG, 7)
CH_PARAM(BISHOP_MOBILITY_BONUS_EG, 2)
CH_PARAM(BISHOP_THREATEN_ROOK, 21)
CH_PARAM(BISHOP_THREATEN_QUEEN, 32)
CH_PARAM(BISHOP_THREATEN_KING, 28)
CH_PARAM(ROOK_MOBILITY_BONUS_MG, 3)
CH_PARAM(ROOK_MOBILITY_BONUS_EG, 11)
CH_PARAM(ROOK_THREATEN_QUEEN, 15)
CH_PARAM(ROOK_THREATEN_KING, 10)
CH_PARAM(ROOK_ON_OPEN_FILE, 42)
CH_PARAM(QUEEN_ON_OPEN_FILE, 24)
CH_PARAM_ARRAY(KING_DEFENDERS_MG[12],
     -32,   -9,    1,    7,   23,   24, 
      32,   12,   24,    9,   17,   14,     )


#endif

}
