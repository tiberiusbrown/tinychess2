#pragma once

#include "ch_arch.h"

// color is templated instead of passed as argument
#define CH_COLOR_TEMPLATE 1

// enable SSE and AVX acceleration
#define CH_ENABLE_ACCEL 1

// magic bitboards
#define CH_ENABLE_MAGIC 1
// the following only apply at SSE+ (is hq faster for bishop?)
#define CH_ENABLE_MAGIC_BISHOP 1
#define CH_ENABLE_MAGIC_ROOK 1

// enable transposition table
#define CH_ENABLE_HASH 1
#define CH_ENABLE_HASH_PERFT 1

#define CH_ENABLE_HISTORY_HEURISTIC 1
#define CH_ENABLE_COUNTERMOVE_HEURISTIC 1

#define CH_ENABLE_NULL_MOVE 1

#define CH_ENABLE_QUIESCENCE 1

#define CH_ENABLE_PVS 1

// causes failure to find mate in 4 (at depth 8) for:
// 1r2k1r1/pbppnp1p/1b3P2/8/Q7/B1PB1q2/P4PPP/3R2K1 w - - 1 0
// (finds it at depth 9 instead)
#define CH_ENABLE_LATE_MOVE_REDUCTION 1

#define CH_MAX_THREADS 8

// number of killer moves to track at each height
#define CH_ENABLE_KILLERS 1
#define CH_NUM_KILLERS 2

// force off magic bitboards for 32-bit targets (expensive multiply)
#if CH_ARCH_32BIT
#undef CH_ENABLE_MAGIC
#define CH_ENABLE_MAGIC 0
#endif
