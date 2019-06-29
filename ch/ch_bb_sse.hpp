#pragma once

#include "ch_internal.hpp"

#if CH_ENABLE_SSE

#include "ch_bb.hpp"

#include <emmintrin.h>

// TODO: remove once magic bitboards are implemented
//       and dependency on _mm_shuffle_spi8 is removed
#include <tmmintrin.h>

namespace ch
{

static CH_FORCEINLINE __m128i shift_n(__m128i x)
{
    return _mm_srli_epi64(x, 8);
}
static CH_FORCEINLINE __m128i shift_s(__m128i x)
{
    return _mm_slli_epi64(x, 8);
}
static CH_FORCEINLINE __m128i shift_w(__m128i x)
{
    __m128i const M = _mm_set1_epi64x(~FILEH);
    return _mm_and_si128(M, _mm_slli_epi64(x, 1));
}
static CH_FORCEINLINE __m128i shift_e(__m128i x)
{
    __m128i const M = _mm_set1_epi64x(~FILEA);
    return _mm_and_si128(M, _mm_srli_epi64(x, 1));
}
static CH_FORCEINLINE __m128i shift_nw(__m128i x)
{
    __m128i const M = _mm_set1_epi64x(~FILEH);
    return _mm_and_si128(M, _mm_srli_epi64(x, 9));
}
static CH_FORCEINLINE __m128i shift_ne(__m128i x)
{
    __m128i const M = _mm_set1_epi64x(~FILEA);
    return _mm_and_si128(M, _mm_srli_epi64(x, 7));
}
static CH_FORCEINLINE __m128i shift_sw(__m128i x)
{
    __m128i const M = _mm_set1_epi64x(~FILEH);
    return _mm_and_si128(M, _mm_slli_epi64(x, 7));
}
static CH_FORCEINLINE __m128i shift_se(__m128i x)
{
    __m128i const M = _mm_set1_epi64x(~FILEA);
    return _mm_and_si128(M, _mm_slli_epi64(x, 9));
}

static CH_FORCEINLINE __m128i slide_fill_n(__m128i x, __m128i empty)
{
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 8)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 8));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 16)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 16));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 32)));
    return x;
}
static CH_FORCEINLINE __m128i slide_fill_s(__m128i x, __m128i empty)
{
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 8)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 8));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 16)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 16));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 32)));
    return x;
}
static CH_FORCEINLINE __m128i slide_fill_w(__m128i x, __m128i empty)
{
    empty = _mm_and_si128(empty, _mm_set1_epi64x(~FILEH));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 1)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 1));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 2)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 2));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 4)));
    return x;
}
static CH_FORCEINLINE __m128i slide_fill_e(__m128i x, __m128i empty)
{
    empty = _mm_and_si128(empty, _mm_set1_epi64x(~FILEA));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 1)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 1));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 2)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 2));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 4)));
    return x;
}
static CH_FORCEINLINE __m128i slide_fill_nw(__m128i x, __m128i empty)
{
    empty = _mm_and_si128(empty, _mm_set1_epi64x(~FILEH));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 9)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 9));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 18)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 18));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 36)));
    return x;
}
static CH_FORCEINLINE __m128i slide_fill_ne(__m128i x, __m128i empty)
{
    empty = _mm_and_si128(empty, _mm_set1_epi64x(~FILEA));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 7)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 7));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 14)));
    empty = _mm_and_si128(empty, _mm_srli_epi64(empty, 14));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_srli_epi64(x, 28)));
    return x;
}
static CH_FORCEINLINE __m128i slide_fill_sw(__m128i x, __m128i empty)
{
    empty = _mm_and_si128(empty, _mm_set1_epi64x(~FILEH));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 7)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 7));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 14)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 14));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 28)));
    return x;
}
static CH_FORCEINLINE __m128i slide_fill_se(__m128i x, __m128i empty)
{
    empty = _mm_and_si128(empty, _mm_set1_epi64x(~FILEA));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 9)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 9));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 18)));
    empty = _mm_and_si128(empty, _mm_slli_epi64(empty, 18));
    x = _mm_or_si128(x, _mm_and_si128(empty, _mm_slli_epi64(x, 36)));
    return x;
}

static CH_FORCEINLINE __m128i slide_attack_n(__m128i bb, __m128i empty)
{
    return shift_n(slide_fill_n(bb, empty));
}
static CH_FORCEINLINE __m128i slide_attack_s(__m128i bb, __m128i empty)
{
    return shift_s(slide_fill_s(bb, empty));
}
static CH_FORCEINLINE __m128i slide_attack_w(__m128i bb, __m128i empty)
{
    return shift_w(slide_fill_w(bb, empty));
}
static CH_FORCEINLINE __m128i slide_attack_e(__m128i bb, __m128i empty)
{
    return shift_e(slide_fill_e(bb, empty));
}
static CH_FORCEINLINE __m128i slide_attack_nw(__m128i bb, __m128i empty)
{
    return shift_nw(slide_fill_nw(bb, empty));
}
static CH_FORCEINLINE __m128i slide_attack_ne(__m128i bb, __m128i empty)
{
    return shift_ne(slide_fill_ne(bb, empty));
}
static CH_FORCEINLINE __m128i slide_attack_sw(__m128i bb, __m128i empty)
{
    return shift_sw(slide_fill_sw(bb, empty));
}
static CH_FORCEINLINE __m128i slide_attack_se(__m128i bb, __m128i empty)
{
    return shift_se(slide_fill_se(bb, empty));
}

static CH_FORCEINLINE __m128i bswap(__m128i x)
{
    // TODO: use magic bitboards to negate this
    // WARNING: this is an SSSE3 instruction!!!!!
    CH_ALIGN(16) static uint8_t const M[16] =
    {
        7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
    };
    return _mm_shuffle_epi8(x, *(__m128i*)M);
}

static CH_FORCEINLINE __m128i hq_attacks(int sq, __m128i occ, __m128i mask)
{
    __m128i forward = _mm_and_si128(occ, mask);
    __m128i reverse = bswap(forward);
    forward = _mm_sub_epi64(forward, _mm_set1_epi64x(masks[sq].singleton));
    reverse = _mm_sub_epi64(reverse, _mm_set1_epi64x(masks[sq ^ 0x38].singleton));
    forward = _mm_xor_si128(forward, bswap(reverse));
    return _mm_and_si128(forward, mask);
}
static CH_FORCEINLINE uint64_t hq_bishop_attacks_sse(int sq, uint64_t occ)
{
    __m128i occ128 = _mm_set1_epi64x(occ);
    __m128i a = hq_attacks(sq, occ128, *(__m128i*)masks[sq].diag_anti);
#ifdef __GNUC__
    // TODO: use Agner Fog's vector class to avoid this aliasing nonsense
    using u64a = uint64_t __attribute((__may_alias__));
    return ((u64a*)&a)[0] | ((u64a*)&a)[1];
#else
    return ((uint64_t*)&a)[0] | ((uint64_t*)&a)[1];
#endif
}

}

#endif
