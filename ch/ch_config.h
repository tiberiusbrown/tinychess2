#pragma once

#include "ch_arch.h"

/*

GOALS:

Mate in 7 (h5h7 g8h7 e4f6 h7h6 e5g4 ...)
Might need to order checking moves high and/or have special check quiescence?
Stockfish finds the mate in 7 at depth 16 in less than 100 ms
rn3rk1/pbppq1pp/1p2pb2/4N2Q/3PN3/3B4/PPP2PPP/R3K2R w KQ - 7 11

Mate in 8
Stockfish finds it at depth 15 in less than 20 ms
8/7K/8/8/8/8/R7/7k w - - 0 1

IMPLEMENT:

Mop-up Evaluation for better endgame
Proper PV tracking (i.e., not with hash)
Moves that give check in quiescence? (depth-limited)
Record and report statistics (number of fail-highs in PVS, etc)
Simplified ABDADA:
    http://www.tckerrigan.com/Chess/Parallel_Search/Simplified_ABDADA/simplified_abdada.html
Lazy SMP properly synchonized iterative deepening?
Move ordering by threatened piece:
    If the static evaluation is >= beta, the null move fails low,
    and the refutation of the null move is a capture, this means
    that the captured piece is in danger. So it is a good idea to
    move it away. 

*/

// color is templated instead of passed as argument
#define CH_COLOR_TEMPLATE 1

// enable SSE and AVX acceleration
#define CH_ENABLE_ACCEL 1

// disable forcing inline
#define CH_NEVER_FORCE_INLINE 0

// never request inline
#define CH_NEVER_REQUEST_INLINE 0

// magic bitboards
#define CH_ENABLE_MAGIC 1
// the following only apply at SSE+ (is hq faster for bishop?)
#define CH_ENABLE_MAGIC_BISHOP 1
#define CH_ENABLE_MAGIC_ROOK 1

// enable transposition table
#define CH_ENABLE_HASH 1
#define CH_ENABLE_HASH_PERFT 1
#define CH_HASH_BUCKETS_POW 2

#define CH_ENABLE_HISTORY_HEURISTIC 1

// seems to hinder right now (?)
#define CH_ENABLE_COUNTERMOVE_HEURISTIC 0

#define CH_MAX_THREADS 8

// number of killer moves to track at each height
#define CH_ENABLE_KILLERS 1
#define CH_NUM_KILLERS 2

// force off magic bitboards for 32-bit targets (expensive multiply)
//#if CH_ARCH_32BIT
//#undef CH_ENABLE_MAGIC
//#define CH_ENABLE_MAGIC 0
//#endif

// override for minimum size
#ifdef CH_MINIMAL_SIZE
#undef CH_COLOR_TEMPLATE
#undef CH_ENABLE_ACCEL
#undef CH_NEVER_FORCE_INLINE
#undef CH_NEVER_REQUEST_INLINE

#define CH_COLOR_TEMPLATE 0
#define CH_ENABLE_ACCEL 0
#define CH_NEVER_FORCE_INLINE 1
#define CH_NEVER_REQUEST_INLINE 1
#endif
