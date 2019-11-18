#pragma once

#include "ch_internal.hpp"
#include "ch_bb.hpp"

namespace ch
{

CH_OPT_SIZE static uint8_t get_cheby_dist(int a, int b)
{
    int dx = abs((a & 7) - (b & 7));
    int dy = abs((a >> 3) - (b >> 3));
    return uint8_t(std::max(dx, dy));
}

CH_OPT_SIZE static void init()
{
    static int8_t const NDI[8] = { -2, -2, 1, -1, 2, 2, -1, 1 };
    static int8_t const NDJ[8] = { -1, 1, 2, 2, 1, -1, -2, -2 };
    static int8_t const KDI[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    static int8_t const KDJ[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    memzero(&knight_attacks, int(sizeof(knight_attacks)));
    memzero(&king_attacks, int(sizeof(king_attacks)));
    memzero(&rook_pseudo_attacks, int(sizeof(rook_pseudo_attacks)));
    memzero(&bishop_pseudo_attacks, int(sizeof(bishop_pseudo_attacks)));
    memzero(&pawn_passed_check, int(sizeof(pawn_passed_check)));
    memzero(&pawn_attacks, int(sizeof(pawn_attacks)));
    memzero(&betweens, int(sizeof(betweens)));
    memzero(&lines, int(sizeof(lines)));
    memzero(&cheby_dist, int(sizeof(cheby_dist)));

    for(int i = 0; i < 64; ++i)
        for(int j = 0; j < 64; ++j)
            cheby_dist[i][j] = get_cheby_dist(i, j);

    for(int i = 0; i < 64; ++i)
    {
        uint64_t s = (1ull << i);
        pawn_attacks[i][WHITE] = shift_nw(s) | shift_ne(s);
        pawn_attacks[i][BLACK] = shift_sw(s) | shift_se(s);
    }

    for(int i = 0; i < 64; ++i)
    {
        uint64_t s = (1ull << i);
        s = s | shift_w(s) | shift_e(s);
        pawn_passed_check[i][WHITE] = s ^ slide_fill_forward<WHITE>(s, ~0ull);
        pawn_passed_check[i][BLACK] = s ^ slide_fill_forward<BLACK>(s, ~0ull);
    }

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
                    knight_attacks[k] |= (1ull << dk);

                di = i + KDI[d];
                dj = j + KDJ[d];
                dk = di * 8 + dj;
                if(unsigned(di) < 8 && unsigned(dj) < 8)
                    king_attacks[k] |= (1ull << dk);

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
                            bishop_pseudo_attacks[k] |= (1ull << dk);
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
                            rook_pseudo_attacks[k] |= (1ull << dk);
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
            if(bishop_pseudo_attacks[i] & (1ull << j))
            {
                betweens[i][j] = m &
                    bishop_pseudo_attacks[i] &
                    bishop_pseudo_attacks[j];
            }
            else if(rook_pseudo_attacks[i] & (1ull << j))
            {
                betweens[i][j] = m &
                    rook_pseudo_attacks[i] &
                    rook_pseudo_attacks[j];
            }
        }
    }

    for(int i = 0; i < 64; ++i)
    {
        uint64_t const file = (FILEA << (i & 0x7));
        uint64_t const rank = (0xFFull << (i & 0x38));

        uint64_t const s = (1ull << i);
        uint64_t v = s, d = s, a = s;
        for(int j = 0; j < 7; ++j)
        {
            v |= shift_n(v) | shift_s(v);
            d |= shift_nw(d) | shift_se(d);
            a |= shift_ne(a) | shift_sw(a);
        }
        uint64_t const diag = d ^ s;
        uint64_t const anti = a ^ s;

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
