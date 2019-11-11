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

//CH_PARAM_ARRAY(PIECE_VALUES_MAG[5], 100, 325, 335, 500, 975, )
CH_PARAM_ARRAY(PIECE_VALUES_MAG[5],
     100,  359,  379,  605, 1181,     )
//CH_PARAM_ARRAY(PIECE_VALUES_MAG[5],
//    0, 300, 315, 500, 1000, )

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
       0,    0,    0,    0, 
      28,   63,   65,   64, 
      -8,   25,   33,   42, 
     -31,  -36,  -21,  -30, 
     -41,  -56,  -50,  -37, 
     -28,  -55,  -57,  -61, 
      -9,  -33,  -36,  -58, 
       0,    0,    0,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_EG[32],
       0,    0,    0,    0, 
      62,   39,   58,   43, 
      47,   65,   53,   33, 
      26,   49,   34,   30, 
      17,   24,   23,    3, 
      10,   19,   21,   18, 
      15,   29,   26,   25, 
       0,    0,    0,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG[32],
     -65,  -65,  -65,  -65, 
     -65,  -65,  -65,  -56, 
       0,  -65,  -13,   -6, 
       5,   44,    7,   -1, 
       2,   36,   29,   33, 
      23,   55,   37,   56, 
     -24,   10,   49,   45, 
      11,  -24,   12,  -47,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG[32],
     -63,  -64,  -20,  -22, 
     -61,  -45,   13,   23, 
     -33,    5,   15,   45, 
     -22,    5,   21,   41, 
       3,   36,   59,   62, 
     -12,   11,   43,   49, 
      -7,   -7,   10,   10, 
     -63,  -46,  -54,   -3,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_MG[32],
     -65,  -65,  -65,  -65, 
     -65,  -65,  -52,  -65, 
     -52,  -65,  -20,  -45, 
     -11,  -18,    1,    5, 
      -9,   30,    6,   28, 
      27,   38,   26,   46, 
      65,   31,   52,   31, 
      26,   22,  -22,   31,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_EG[32],
       4,  -19,   12,   15, 
      -2,   10,   38,   32, 
       7,   31,   33,   45, 
       4,   29,   31,   28, 
       6,   15,   31,   43, 
      12,   21,   46,   37, 
      -5,   19,   13,   24, 
      -6,  -10,   37,    2,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_MG[32],
     -61,  -65,  -65,  -65, 
     -65,    2,   26,   43, 
     -33,  -19,  -22,   -3, 
     -22,  -23,  -17,  -20, 
     -25,  -26,  -37,  -26, 
     -28,  -44,  -32,  -26, 
      17,  -29,  -22,  -18, 
      -5,   15,   18,   30,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_EG[32],
      62,   65,   53,   38, 
      53,   65,   63,   58, 
      48,   65,   51,   51, 
      37,   65,   56,   57, 
      34,   52,   45,   54, 
      19,   40,   27,   39, 
      -8,   23,   19,   32, 
      14,   31,   37,   47,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_MG[32],
     -14,  -65,  -65,  -65, 
       5,  -48,   -4,  -55, 
       5,  -55,  -17,  -45, 
      39,  -30,    5,  -23, 
      17,    6,    3,   14, 
      44,   37,   38,   28, 
      14,   14,   50,   53, 
     -13,  -25,  -18,   60,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_EG[32],
     -31,   19,   56,   37, 
     -25,   64,   63,   65, 
     -26,   46,   30,   44, 
      -5,   43,   53,   55, 
      10,   51,   31,   49, 
      -9,   -1,  -10,    4, 
     -54,  -23,  -50,  -44, 
      -3,  -56,  -58,  -65,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_MG[32],
      -6,    3,   40,   39, 
      38,   26,   61,   64, 
       3,   65,   63,   64, 
     -55,   20,   30,   39, 
     -26,   -9,   29,   10, 
      21,    5,    3,  -13, 
      28,   -3,  -32,  -48, 
      51,  -64,    5,  -13,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_EG[32],
       0,  -23,  -16,  -39, 
      45,   43,   47,   25, 
      62,   65,   60,   55, 
      54,   60,   58,   53, 
      26,   56,   54,   51, 
      16,   50,   58,   59, 
      16,   45,   56,   42, 
     -17,    2,  -26,  -23,     )

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
