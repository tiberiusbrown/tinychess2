#pragma once

// color is templated instead of passed as argument
#define CH_COLOR_TEMPLATE 1

// enable SSE and AVX acceleration
#define CH_ENABLE_ACCEL 1

// magic bitboards (slower than HQ at SSE and above?)
#define CH_ENABLE_MAGIC 0
