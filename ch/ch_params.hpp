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

#if 1

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
CH_PARAM_ARRAY(PIECE_VALUES_MAG[5], 100, 325, 335, 500, 975, )
CH_PARAM(HALF_OPEN_FILE, 5)
CH_PARAM(PAWN_PROTECT_ANY, 4)
CH_PARAM(PAWN_PROTECT_PAWN, 4)
CH_PARAM(PAWN_THREATEN_KNIGHT, 20)
CH_PARAM(PAWN_THREATEN_BISHOP, 25)
CH_PARAM(PAWN_THREATEN_ROOK, 23)
CH_PARAM(PAWN_THREATEN_QUEEN, 35)
CH_PARAM_ARRAY(PASSED_PAWN_MG[8],
       0,    0,    0,    0,    0,    0,    0,    0,     )
CH_PARAM_ARRAY(PASSED_PAWN_EG[8],
       0,    0,    0,    8,   15,   39,   72,    0,     )
CH_PARAM_ARRAY(PASSED_PAWN_FREE_EG[8],
       0,    0,    2,   28,   65,  135,  192,    0,     )
CH_PARAM(PASSED_PAWN_KING_ESCORT, 8)
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
CH_PARAM(KNIGHT_OUTPOST_OPEN_FILE, 4)
CH_PARAM(BISHOP_MOBILITY_BONUS_MG, 5)
CH_PARAM(BISHOP_MOBILITY_BONUS_EG, 3)
CH_PARAM(BISHOP_THREATEN_ROOK, 15)
CH_PARAM(BISHOP_THREATEN_QUEEN, 30)
CH_PARAM(BISHOP_THREATEN_KING, 0)
CH_PARAM(ROOK_MOBILITY_BONUS_MG, 3)
CH_PARAM(ROOK_MOBILITY_BONUS_EG, 6)
CH_PARAM(ROOK_THREATEN_QUEEN, 20)
CH_PARAM(ROOK_THREATEN_KING, 0)
CH_PARAM(ROOK_ON_OPEN_FILE, 18)
CH_PARAM(QUEEN_MOBILITY_BONUS_MG, 4)
CH_PARAM(QUEEN_MOBILITY_BONUS_EG, 8)
CH_PARAM(QUEEN_ON_OPEN_FILE, 7)
CH_PARAM_ARRAY(KING_DEFENDERS_MG[12],
     -32,   -8,    1,    8,   16,   28, 
      32,   16,   12,   12,   12,   12,     )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_MG[32],
      -1,    0,    0,    0, 
       9,   31,   36,    6, 
      -8,   13,    3,    1, 
      -6,   -1,    6,    1, 
      -8,  -21,   -6,   -5, 
      -3,  -18,  -17,  -19, 
       5,   -6,  -11,  -23, 
      -9,    0,    0,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_PAWN_EG[32],
       0,    0,    0,    0, 
      20,    3,    7,    1, 
      11,    6,    2,   -5, 
      -2,   -2,   -8,  -16, 
      -7,   -7,   -8,  -18, 
      -9,  -11,   -4,  -14, 
      -9,   -5,   -5,   -5, 
     -12,    0,    0,    0,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_MG[32],
     -92, -127,  -45,  -69, 
     -40,  -27,  -31,    0, 
     -30,    0,   10,   15, 
     -30,    5,   10,   20, 
     -36,  -13,   15,    7, 
     -30,   -6,  -13,   -3, 
     -56,  -25,  -15,  -13, 
     -53,  -58,  -41,  -61,     )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT_EG[32],
     -81,  -59,  -36,  -26, 
     -45,  -31,  -12,   -9, 
     -21,  -13,   -3,    0, 
     -10,    0,    0,    0, 
      -4,    0,    0,    0, 
     -29,  -12,    0,    0, 
     -40,  -38,  -17,  -20, 
     -53,  -44,  -52,  -29,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_MG[32],
     -62,  -64,  -41,  -79, 
     -10,  -21,  -17,  -38, 
     -10,  -11,    4,    7, 
     -11,    1,   -4,   10, 
     -13,  -14,    7,    0, 
     -10,   -5,   -5,   -3, 
     -10,   -6,   -2,   -8, 
     -20,  -26,  -20,  -16,     )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP_EG[32],
      -1,  -12,  -15,  -15, 
     -16,  -12,   -8,  -11, 
     -10,   -6,  -16,  -16, 
     -15,   -8,   -3,   -9, 
     -16,   -6,  -12,    0, 
     -19,  -16,   -3,   -5, 
     -21,  -25,  -17,  -19, 
     -22,  -25,  -20,  -22,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_MG[32],
       0,    0,    0,    0, 
       5,   10,   10,   10, 
      -5,    0,    0,    0, 
     -12,    0,   -5,    0, 
     -13,  -23,  -14,  -15, 
     -26,  -25,  -21,  -22, 
     -12,  -25,  -15,  -16, 
      -1,   -6,   -1,    5,     )
CH_PARAM_ARRAY(INIT_TABLE_ROOK_EG[32],
       0,    0,    0,    0, 
       0,    0,    0,    0, 
       0,    0,    0,    0, 
      -1,    0,   -1,    0, 
      -4,    0,   -1,   -1, 
      -6,   -8,   -9,  -10, 
     -15,   -7,  -11,   -9, 
      -3,   -9,  -10,  -10,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_MG[32],
     -20,  -45,  -10,   -9, 
     -10,  -38,  -68,  -27, 
     -10,  -18,  -24,  -30, 
      -5,  -29,  -42,  -47, 
     -16,  -44,  -36,  -45, 
     -18,  -31,  -34,  -35, 
     -39,  -36,  -18,  -17, 
     -20,  -41,  -39,   -5,     )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN_EG[32],
       0,    0,    0,    0, 
       0,    0,   -4,    0, 
       0,   -3,    0,   -7, 
       0,    0,   -1,  -15, 
       0,    0,   -5,  -10, 
       0,   -4,  -17,  -17, 
     -32,  -10,  -40,  -36, 
     -23,  -20,  -19,  -62,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_MG[32],
     -52,  -88,  -64,  -56, 
     -46,  -44,  -40,  -50, 
     -30,  -40,  -42,  -52, 
     -38,  -43,  -56,  -68, 
     -42,  -50,  -57,  -77, 
     -10,  -40,  -44,  -49, 
      -3,  -23,  -45,  -55, 
      20,  -47,  -19,  -26,     )
CH_PARAM_ARRAY(INIT_TABLE_KING_EG[32],
     -57,  -59,  -64,  -55, 
     -34,  -20,  -24,  -22, 
     -30,  -10,  -15,  -16, 
     -30,  -10,   -6,   -3, 
     -30,  -10,  -10,   -5, 
     -30,  -15,  -13,  -14, 
     -37,  -30,  -17,  -20, 
     -50,  -51,  -57,  -52,     )

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
