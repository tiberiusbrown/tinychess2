#pragma once

#include "ch_arch.h"

/*

GOALS:

Mate in 7 (h5h7 g8h7 e4f6 h7h6 e5g4 ...)
Might need to order checking moves high and/or have special check quiescence?
Stockfish finds the mate in 7 at depth 16 in less than 100 ms
rn3rk1/pbppq1pp/1p2pb2/4N2Q/3PN3/3B4/PPP2PPP/R3K2R w KQ - 7 11

IMPLEMENT:

Move picker class (like Stockfish) that can score/pick in stages
Incrementally updated piece-square position value
Record and report statistics (number of fail-highs in PVS, etc)

*/

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

// no zw protection right now
#define CH_ENABLE_NULL_MOVE 0

#define CH_ENABLE_QUIESCENCE 1

#define CH_ENABLE_PVS 1

// causes delays in finding mates right now
#define CH_ENABLE_LATE_MOVE_REDUCTION 0

#define CH_MAX_THREADS 8

// number of killer moves to track at each height
#define CH_ENABLE_KILLERS 1
#define CH_NUM_KILLERS 2

// force off magic bitboards for 32-bit targets (expensive multiply)
#if CH_ARCH_32BIT
#undef CH_ENABLE_MAGIC
#define CH_ENABLE_MAGIC 0
#endif
