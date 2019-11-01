#pragma once

#include "ch_internal.hpp"

// xoshiro256** 1.0

namespace ch
{

static CH_FORCEINLINE uint64_t random_rotl(uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static uint64_t random(uint64_t seed[4])
{
    uint64_t const r = random_rotl(seed[1] * 5, 7) * 9;
    uint64_t const t = seed[1] << 17;
    seed[2] ^= seed[0];
    seed[3] ^= seed[1];
    seed[1] ^= seed[2];
    seed[0] ^= seed[3];
    seed[2] ^= t;
    seed[3] = random_rotl(seed[3], 45);
    return r;
}

}
