#pragma once

#include "ch_config.hpp"
#if CH_ENABLE_MAGIC

#include "ch_bb.hpp"
#include "ch_internal.hpp"

namespace ch
{

struct magic_info
{
    uint64_t magic;
    uint32_t offset;
};

extern std::array<uint64_t, 97264> magic_lookup;

extern std::array<uint64_t, 64> magic_bishop_rays;
extern std::array<uint64_t, 64> magic_rook_rays;

static constexpr int const MAGIC_BISHOP_SHIFT = 9;
static constexpr int const MAGIC_ROOK_SHIFT = 12;

extern magic_info const BISHOP_MAGICS[64];
extern magic_info const ROOK_MAGICS[64];

static CH_FORCEINLINE uint64_t& magic_bishop_attacks(int sq, uint64_t occ)
{
    magic_info const& m = BISHOP_MAGICS[sq];
    uint64_t i = magic_bishop_rays[sq] & occ;
    i *= m.magic;
    i >>= (64 - MAGIC_BISHOP_SHIFT);
    return magic_lookup[m.offset + uint32_t(i)];
}

static CH_FORCEINLINE uint64_t& magic_rook_attacks(int sq, uint64_t occ)
{
    magic_info const& m = ROOK_MAGICS[sq];
    uint64_t i = magic_rook_rays[sq] & occ;
    i *= m.magic;
    i >>= (64 - MAGIC_ROOK_SHIFT);
    return magic_lookup[m.offset + uint32_t(i)];
}

static CH_FORCEINLINE uint64_t sw_pdep(uint64_t val, uint64_t mask)
{
    uint64_t res = 0;
    for(uint64_t bb = 1; mask; bb += bb)
    {
        if(val & bb)
            res |= mask & uint64_t(-int64_t(mask));
        mask &= mask - 1;
    }
    return res;
}

// note: must be called after ch::init
void init_magic();

}

#endif
