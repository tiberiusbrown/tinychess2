#pragma once

#include "ch_internal.hpp"

#if CH_ENABLE_AVX

#include <immintrin.h>

#include "ch_bb.hpp"
#include "ch_bb_sse.hpp"

namespace ch
{

static inline __m256i horizontal_or(__m256i x)
{
    const __m256i temp = _mm256_or_si256(x,
        _mm256_permute2f128_si256(x, x, 1));
    return _mm256_or_si256(temp,
        _mm256_shuffle_epi32(temp, _MM_SHUFFLE(1, 0, 3, 2)));
}

#ifdef _MSC_VER
#define M64_CONVERT_(a_) \
    (a_ >>  0) & 0xff, (a_ >>  8) & 0xff, (a_ >> 16) & 0xff, (a_ >> 24) & 0xff, \
    (a_ >> 32) & 0xff, (a_ >> 40) & 0xff, (a_ >> 48) & 0xff, (a_ >> 56) & 0xff,
#define M128_CONSTANT(a_, b_) \
    { { M64_CONVERT_(a_) M64_CONVERT_(b_) } }
#define M128_CONSTANT_ALL(a_) M128_CONSTANT(a_, a_)
#define M256_CONSTANT(a_, b_, c_, d_) \
    { { M64_CONVERT_(a_) M64_CONVERT_(b_) M64_CONVERT_(c_) M64_CONVERT_(d_) } }
#define M256_CONSTANT_ALL(a_) M256_CONSTANT(a_, a_, a_, a_)
#else
#define M256_CONSTANT(a_, b_, c_, d_) \
    { int64_t(a_), int64_t(b_), int64_t(c_), int64_t(d_) }
#endif

static inline __m256i shift_nw_ne(__m256i bb)
{
    static constexpr __m256i const S = M256_CONSTANT(9ull, 7ull, 9ull, 7ull);
    static constexpr __m256i const M = M256_CONSTANT(FILEH, FILEA, FILEH, FILEA);
    return _mm256_andnot_si256(M, _mm256_srlv_epi64(bb, S));
}

static inline __m256i shift_sw_se(__m256i bb)
{
    static constexpr __m256i const S = M256_CONSTANT(7ull, 9ull, 7ull, 9ull);
    static constexpr __m256i const M = M256_CONSTANT(FILEH, FILEA, FILEH, FILEA);
    return _mm256_andnot_si256(M, _mm256_sllv_epi64(bb, S));
}

static inline __m256i shift_n_w_nw_ne(__m256i bb)
{
    static constexpr __m256i const S = M256_CONSTANT(8ull, 1ull, 9ull, 7ull);
    static constexpr __m256i const M = M256_CONSTANT(0ull, FILEH, FILEH, FILEA);
    return _mm256_andnot_si256(M, _mm256_srlv_epi64(bb, S));
}

static inline __m256i shift_s_e_sw_se(__m256i bb)
{
    static constexpr __m256i const S = M256_CONSTANT(8ull, 1ull, 7ull, 9ull);
    static constexpr __m256i const M = M256_CONSTANT(0ull, FILEA, FILEH, FILEA);
    return _mm256_andnot_si256(M, _mm256_sllv_epi64(bb, S));
}

static inline __m256i slide_fill_n_w_nw_ne(__m256i bb, __m256i empty)
{
    static constexpr __m256i const S0 = M256_CONSTANT(8ull, 1ull, 9ull, 7ull);
    static constexpr __m256i const S1 = M256_CONSTANT(16ull, 2ull, 18ull, 14ull);
    static constexpr __m256i const S2 = M256_CONSTANT(32ull, 4ull, 36ull, 28ull);
    static constexpr __m256i const M = M256_CONSTANT(0ull, FILEH, FILEH, FILEA);

    empty = _mm256_andnot_si256(M, empty);
    bb = _mm256_or_si256(bb, _mm256_and_si256(empty, _mm256_srlv_epi64(bb, S0)));
    empty = _mm256_and_si256(empty, _mm256_srlv_epi64(empty, S0));
    bb = _mm256_or_si256(bb, _mm256_and_si256(empty, _mm256_srlv_epi64(bb, S1)));
    empty = _mm256_and_si256(empty, _mm256_srlv_epi64(empty, S1));
    bb = _mm256_or_si256(bb, _mm256_and_si256(empty, _mm256_srlv_epi64(bb, S2)));
    return bb;
}

static inline __m256i slide_fill_s_e_sw_se(__m256i bb, __m256i empty)
{
    static constexpr __m256i const S0 = M256_CONSTANT(8ull, 1ull, 7ull, 9ull);
    static constexpr __m256i const S1 = M256_CONSTANT(16ull, 2ull, 14ull, 18ull);
    static constexpr __m256i const S2 = M256_CONSTANT(32ull, 4ull, 28ull, 36ull);
    static constexpr __m256i const M = M256_CONSTANT(0ull, FILEA, FILEH, FILEA);

    empty = _mm256_andnot_si256(M, empty);
    bb = _mm256_or_si256(bb, _mm256_and_si256(empty, _mm256_sllv_epi64(bb, S0)));
    empty = _mm256_and_si256(empty, _mm256_sllv_epi64(empty, S0));
    bb = _mm256_or_si256(bb, _mm256_and_si256(empty, _mm256_sllv_epi64(bb, S1)));
    empty = _mm256_and_si256(empty, _mm256_sllv_epi64(empty, S1));
    bb = _mm256_or_si256(bb, _mm256_and_si256(empty, _mm256_sllv_epi64(bb, S2)));
    return bb;
}

static inline __m256i slide_attack_n_w_nw_ne(__m256i bb, __m256i empty)
{
    return shift_n_w_nw_ne(slide_fill_n_w_nw_ne(bb, empty));
}

static inline __m256i slide_attack_s_e_sw_se(__m256i bb, __m256i empty)
{
    return shift_s_e_sw_se(slide_fill_s_e_sw_se(bb, empty));
}

}

#endif
