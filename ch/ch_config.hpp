#pragma once

// color is templated instead of passed as argument
#define CH_COLOR_TEMPLATE 1

// enable SSE and AVX acceleration
#define CH_ENABLE_ACCEL 1

// magic bitboards (bishop might be slower than HQ at SSE+)
#define CH_ENABLE_MAGIC 1
#define CH_ENABLE_MAGIC_BISHOP 0
#define CH_ENABLE_MAGIC_ROOK 1
