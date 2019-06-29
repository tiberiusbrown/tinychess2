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

#define CH_MAX_THREADS 8

// number of killer moves to track at each height
#define CH_NUM_KILLERS 2

// force off magic bitboards for 32-bit targets (expensive multiply)
#if CH_ARCH_32BIT
#undef CH_ENABLE_MAGIC
#define CH_ENABLE_MAGIC 0
#endif
