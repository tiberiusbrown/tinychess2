#pragma once

#include "ch_config.hpp"
#if CH_ENABLE_MAGIC

#include "ch_bb.hpp"
#include "ch_internal.hpp"
#include "ch_magic_data.hpp"

#include "ch_print_bbs.hpp"

namespace ch
{

CH_FORCEINLINE static uint64_t& magic_bishop_attacks(int sq, uint64_t occ)
{
    magic_info const& m = BISHOP_MAGICS[sq];
    uint64_t i = magic_bishop_rays[sq] & occ;
    i *= m.magic;
    i >>= (64 - MAGIC_BISHOP_SHIFT);
    return magic_lookup[m.offset + uint32_t(i)];
}

CH_FORCEINLINE static uint64_t& magic_rook_attacks(int sq, uint64_t occ)
{
    magic_info const& m = ROOK_MAGICS[sq];
    uint64_t i = magic_rook_rays[sq] & occ;
    i *= m.magic;
    i >>= (64 - MAGIC_ROOK_SHIFT);
    return magic_lookup[m.offset + uint32_t(i)];
}

CH_FORCEINLINE static uint64_t sw_pdep(uint64_t val, uint64_t mask)
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
CH_OPT_SIZE static void init_magic()
{
    memzero(
        &magic_lookup[0],
        int(magic_lookup.size() * sizeof(magic_lookup[0])));

    for(int i = 0; i < 64; ++i)
    {
        magic_bishop_rays[i] = masks[i].bishop_pseudo_attacks &
            ~(RANK1 | RANK8 | FILEA | FILEH);
        uint64_t rm = masks[i].rook_pseudo_attacks;
        if(i / 8 != 0) rm &= ~RANK8;
        if(i / 8 != 7) rm &= ~RANK1;
        if(i % 8 != 0) rm &= ~FILEA;
        if(i % 8 != 7) rm &= ~FILEH;
        magic_rook_rays[i] = rm;
    }

    // generate bishop attacks
    for(int i = 0; i < 64; ++i)
    {
        uint64_t p = (1ull << i);
        uint64_t mask = magic_bishop_rays[i];
        int n = (1 << popcnt(mask));
        for(uint64_t j = 0; j < n; ++j)
        {
            uint64_t occ = sw_pdep(j, mask);
            uint64_t& atts = magic_bishop_attacks(i, occ);
            if(atts == 0)
            {
                uint64_t empty = ~occ;
                atts =
                    slide_attack_nw(p, empty) |
                    slide_attack_ne(p, empty) |
                    slide_attack_sw(p, empty) |
                    slide_attack_se(p, empty);
            }
        }
    }

    // generate rook attacks
    for(int i = 0; i < 64; ++i)
    {
        uint64_t p = (1ull << i);
        uint64_t mask = magic_rook_rays[i];
        int n = (1 << popcnt(mask));
        for(uint64_t j = 0; j < n; ++j)
        {
            uint64_t occ = sw_pdep(j, mask);
            uint64_t& atts = magic_rook_attacks(i, occ);
            if(atts == 0)
            {
                uint64_t empty = ~occ;
                atts =
                    slide_attack_n(p, empty) |
                    slide_attack_s(p, empty) |
                    slide_attack_w(p, empty) |
                    slide_attack_e(p, empty);
            }
        }
    }
}

}

#endif
