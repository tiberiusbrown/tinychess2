#pragma once

#include "ch_internal.hpp"
#include "ch_bb.hpp"

namespace ch
{

CH_OPT_SIZE void init()
{
    static int8_t const NDI[8] = { -2, -2, 1, -1, 2, 2, -1, 1 };
    static int8_t const NDJ[8] = { -1, 1, 2, 2, 1, -1, -2, -2 };
    static int8_t const KDI[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    static int8_t const KDJ[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    for(auto& i : masks)
    {
        i.knight_attacks = 0;
        i.king_attacks = 0;
        i.rook_pseudo_attacks = 0;
        i.bishop_pseudo_attacks = 0;
        i.singleton = 0;
        i.vertical = 0;
        i.diag_anti[0] = 0;
        i.diag_anti[1] = 0;
    }
    for(auto& i : betweens)
        for(auto& j : i)
            j = 0;
    for(auto& i : lines)
        for(auto& j : i)
            j = 0;

    for(int i = 0; i < 64; ++i)
    {
        uint64_t s = (1ull << i);
        uint64_t v = s, d = s, a = s;
        masks[i].singleton = s;
        for(int j = 0; j < 7; ++j)
        {
            v |= shift_n(v) | shift_s(v);
            d |= shift_nw(d) | shift_se(d);
            a |= shift_ne(a) | shift_sw(a);
        }
        masks[i].vertical = v ^ s;
        masks[i].diag_anti[0] = d ^ s;
        masks[i].diag_anti[1] = a ^ s;

        masks[i].white_pawn_attacks = shift_nw(s) | shift_ne(s);
        masks[i].black_pawn_attacks = shift_sw(s) | shift_se(s);
    }

    // simple first rank attacks by occupancy
    for(int i = 0; i < 128; i += 2)
    {
        for(int j = 0; j < 8; ++j)
        {
            auto& a = first_rank_attacks[i * 4 + j];
            uint8_t m = 0;
            for(int k = j - 1; k >= 0; --k)
            {
                m |= (1 << k);
                if((1 << k) & i) break;
            }
            for(int k = j + 1; k < 8; ++k)
            {
                m |= (1 << k);
                if((1 << k) & i) break;
            }
            a = m;
        }
    }

#if 0
    for(int i = 0; i < 64; ++i)
        for(auto& n : sbamg_masks[i].low)
            n = (i == 0 ? 1ull : (1ull << i) - 1);
    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < 8; ++j)
        {
            int k = i * 8 + j;
            auto& m = sbamg_masks[k];
            uint64_t const rank = 0xFFull << (i * 8);
            uint64_t const file = 0x0101010101010101ull << j;
            uint64_t const singleton = (1ull << k);
            uint64_t diag = singleton, anti = singleton;
            for(int n = 0; n < 7; ++n)
            {
                diag |= shift_nw(diag) | shift_se(diag);
                anti |= shift_ne(anti) | shift_sw(anti);
            }
            m.lin[SBAMG_RANK] = rank ^ singleton;
            m.lin[SBAMG_FILE] = file ^ singleton;
            m.lin[SBAMG_DIAG] = diag ^ singleton;
            m.lin[SBAMG_ANTI] = anti ^ singleton;
            uint64_t const edges = 0xFF818181818181FFull;
            uint64_t const rank_edges = 0x8181818181818181ull;
            uint64_t const file_edges = 0xFF000000000000FFull;
            m.out[SBAMG_DIAG] = (m.lin[SBAMG_DIAG] & edges) | 1;
            m.out[SBAMG_ANTI] = (m.lin[SBAMG_ANTI] & edges) | 1;
            m.out[SBAMG_RANK] = (m.lin[SBAMG_RANK] & rank_edges) | 1;
            m.out[SBAMG_FILE] = (m.lin[SBAMG_FILE] & file_edges) | 1;
        }
    }
    sbamg_masks[SBAMG_RANK].out[0] = 0x81ull;
#endif

    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < 8; ++j)
        {
            int k = i * 8 + j;
            for(int d = 0; d < 8; ++d)
            {
                int di, dj, dk;

                di = i + NDI[d];
                dj = j + NDJ[d];
                dk = di * 8 + dj;
                if(unsigned(di) < 8 && unsigned(dj) < 8)
                    masks[k].knight_attacks |= (1ull << dk);

                di = i + KDI[d];
                dj = j + KDJ[d];
                dk = di * 8 + dj;
                if(unsigned(di) < 8 && unsigned(dj) < 8)
                    masks[k].king_attacks |= (1ull << dk);

                di = i;
                dj = j;
                if(d % 2 == 0)
                {
                    for(int n = 0; n < 7; ++n)
                    {
                        di += KDI[d];
                        dj += KDJ[d];
                        dk = di * 8 + dj;
                        if(unsigned(di) < 8 && unsigned(dj) < 8)
                            masks[k].bishop_pseudo_attacks |= (1ull << dk);
                    }
                }
                else
                {
                    for(int n = 0; n < 7; ++n)
                    {
                        di += KDI[d];
                        dj += KDJ[d];
                        dk = di * 8 + dj;
                        if(unsigned(di) < 8 && unsigned(dj) < 8)
                            masks[k].rook_pseudo_attacks |= (1ull << dk);
                    }
                }
            }
        }
    }

    for(int i = 0; i < 64; ++i)
    {
        for(int j = 0; j < 64; ++j)
        {
            uint64_t m = (i < j ?
                (((1ull << (j - i)) - 1) << i) :
                (((1ull << (i - j)) - 1) << j));
            if(masks[i].bishop_pseudo_attacks & (1ull << j))
            {
                betweens[i][j] = m &
                    masks[i].bishop_pseudo_attacks &
                    masks[j].bishop_pseudo_attacks;
            }
            else if(masks[i].rook_pseudo_attacks & (1ull << j))
            {
                betweens[i][j] = m &
                    masks[i].rook_pseudo_attacks &
                    masks[j].rook_pseudo_attacks;
            }
        }
    }

    for(int i = 0; i < 64; ++i)
    {
        uint64_t const file = (FILEA << (i & 0x7));
        uint64_t const rank = (0xFFull << (i & 0x38));
        uint64_t const diag = masks[i].diag_anti[0];
        uint64_t const anti = masks[i].diag_anti[1];

        for(int j = i - 8; j >= 0; j -= 8)
            lines[i][j] = file;
        for(int j = i + 8; j < 64; j += 8)
            lines[i][j] = file;

        for(int j = i - 1; (j & 0x7) != 7; j -= 1)
            lines[i][j] = rank;
        for(int j = i + 1; (j & 0x7) != 0; j += 1)
            lines[i][j] = rank;

        for(int j = i - 9; j >= 0; j -= 9)
            lines[i][j] = diag;
        for(int j = i + 9; j < 64; j += 9)
            lines[i][j] = diag;

        for(int j = i - 7; j >= 0; j -= 7)
            lines[i][j] = anti;
        for(int j = i + 7; j < 64; j += 7)
            lines[i][j] = anti;
    }
}

}
