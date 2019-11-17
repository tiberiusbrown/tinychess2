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

static constexpr int32_t SC(int16_t mg, int16_t eg)
{
    return int32_t((uint32_t(mg) << 16) + int32_t(eg));
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
    SC(100, 100), SC(325, 325), SC(335, 335), SC(500, 500), SC(975, 975), )

CH_PARAM_ARRAY(INIT_TABLE_PAWN[32],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    )
CH_PARAM_ARRAY(INIT_TABLE_KNIGHT[32],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    )
CH_PARAM_ARRAY(INIT_TABLE_BISHOP[32],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    )
CH_PARAM_ARRAY(INIT_TABLE_ROOK[32],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    )
CH_PARAM_ARRAY(INIT_TABLE_QUEEN[32],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    )
CH_PARAM_ARRAY(INIT_TABLE_KING[32],
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    SC(0, 0), SC(0, 0), SC(0, 0), SC(0, 0),
    )

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
