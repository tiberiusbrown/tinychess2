#pragma once

#include "ch_bb.hpp"
#include "ch_bb_sse.hpp"
#include "ch_bb_avx.hpp"

#include "ch_evaluator.hpp"
#include "ch_position.hpp"


namespace ch
{

template<acceleration accel>
struct evaluator
{
    uint64_t all_pawn_attacks[2];
    uint64_t mobility_squares[2];
    uint64_t occ;
    uint64_t pawns;
    uint64_t attacked_by[2];
    int king_sq[2];

    CH_FORCEINLINE int eval_pawns(position const& p, color c)
    {
        uint64_t const mp = p.bbs[c + PAWN];
        uint64_t const ep = p.bbs[opposite(c) + PAWN];
        uint64_t t = mp;
        int x = 0;
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t mask = 1ull << sq;

            uint64_t const pawns_in_adjacent_files =
                mp & adjacent_files_of_sq(sq);
            uint64_t const threats = ep & pawn_attacks[sq][c];
            uint64_t const push_threats =
                ep & pawn_attacks[forward_sq(sq, c)][c];

            // isolated pawn penalty
            if(!pawns_in_adjacent_files && !threats)
                x += PAWN_ISOLATED;

            // doubled pawn penalty
            if(more_than_one<accel>(file_of_sq(sq) & mp))
                x += PAWN_DOUBLED;

            // backward pawn penalty
            if(push_threats && !(pawn_passed_check[sq][opposite(c)] & mp))
                x += PAWN_BACKWARD;

            // passed pawn bonus
            if(!(pawn_passed_check[sq][c] & ep))
            {
                x += passed_pawn_files[file_index_of_sq(sq)];
                if(mask & all_pawn_attacks[c])
                    x += PAWN_PASSED_PROTECTED;
            }
        }
        return x;
    }

    CH_FORCEINLINE int eval_knights(position const& p, color c)
    {
        uint64_t t = p.bbs[c + KNIGHT];
        int x = 0;
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks = knight_attacks[sq];
            uint64_t const mask = (1ull << sq);

            attacked_by[c] |= attacks;

            // knight outpost bonus
            if((mask & OUTPOST_RANKS[c]) && !(mask & all_pawn_attacks[opposite(c)]))
            {
                int const outside = (mask & (FILEA | FILEH)) != 0;
                int const defended = (mask & all_pawn_attacks[c]) != 0;
                x += KNIGHT_OUTPOST[defended * 2 + outside];
            }

            // bonus for knights shielded by a pawn of either color
            if(shift_forward(c, mask) & pawns)
                x += KNIGHT_BEHIND_PAWN;

            // mobility bonus
            {
                int mob = popcnt<accel>(attacks & mobility_squares[c]);
                mob = clamp(mob, 0, 7);
                x += KNIGHT_MOBILITY[mob];
            }
        }
        return x;
    }

    CH_FORCEINLINE int eval_bishops(position const& p, color c)
    {
        uint64_t t = p.bbs[c + BISHOP];
        int x = 0;
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks = magic_bishop_attacks(sq, occ);
            uint64_t const mask = (1ull << sq);

            attacked_by[c] |= attacks;

            // bishop outpost bonus
            if((mask & OUTPOST_RANKS[c]) && !(mask & all_pawn_attacks[opposite(c)]))
            {
                int const outside = (mask & (FILEA | FILEH)) != 0;
                int const defended = (mask & all_pawn_attacks[c]) != 0;
                x += BISHOP_OUTPOST[defended * 2 + outside];
            }

            // bonus for bishops shielded by a pawn of either color
            if(shift_forward(c, mask) & pawns)
                x += BISHOP_BEHIND_PAWN;

            // mobility bonus
            {
                int mob = popcnt<accel>(attacks & mobility_squares[c]);
                mob = clamp(mob, 1, 11);
                x += BISHOP_MOBILITY[mob];
            }
        }
        return x;
    }

    CH_FORCEINLINE int eval_rooks(position const& p, color c)
    {
        uint64_t t = p.bbs[c + ROOK];
        int x = 0;
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks = magic_rook_attacks(sq, occ);

            attacked_by[c] |= attacks;

            // mobility bonus
            {
                int mob = popcnt<accel>(attacks & mobility_squares[c]);
                mob = clamp(mob, 1, 14);
                x += ROOK_MOBILITY[mob];
            }
        }
        return x;
    }

    CH_FORCEINLINE int eval_queens(position const& p, color c)
    {
        uint64_t t = p.bbs[c + QUEEN];
        int x = 0;
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks =
                magic_bishop_attacks(sq, occ) |
                magic_rook_attacks(sq, occ);

            attacked_by[c] |= attacks;

            // mobility bonus
            {
                int mob = popcnt<accel>(attacks & mobility_squares[c]);
                mob = clamp(mob, 3, 24);
                x += QUEEN_MOBILITY[mob];
            }
        }
        return x;
    }

    CH_FORCEINLINE int eval_king(position const& p, color c)
    {
        (void)p;
        int x = 0;
        uint64_t t = pawns;
        if(t)
        {
            uint8_t d = 0, n = 0;
            int k = king_sq[c];
            while(t)
            {
                int sq = pop_lsb<accel>(t);
                d += manhatten_dist[k][sq];
                ++n;
            }
            x += KING_PAWN_TROPISM[min(d / n, 10)];
        }
        return x;
    }

    CH_FORCEINLINE int eval_king_safety(position const& p, color c)
    {
        (void)p;
        int x = 0;
        uint64_t m = king_attacks[king_sq[c]];
        m &= ~attacked_by[opposite(c)];
        x += KING_SAFETY_MOVES[popcnt<accel>(m)];
        return x;
    }

    CH_FORCEINLINE int evaluate(position const& p, color c)
    {
        int phase;
        {
            // zurichess-style phase
            int total = 4*1 + 4*1 + 4*3 + 2*6;
            int curr = total;
            curr -= popcnt<accel>(p.bbs[WHITE+KNIGHT] | p.bbs[BLACK+KNIGHT]) * 1;
            curr -= popcnt<accel>(p.bbs[WHITE+BISHOP] | p.bbs[BLACK+BISHOP]) * 1;
            curr -= popcnt<accel>(p.bbs[WHITE+ROOK  ] | p.bbs[BLACK+ROOK  ]) * 3;
            curr -= popcnt<accel>(p.bbs[WHITE+QUEEN ] | p.bbs[BLACK+QUEEN ]) * 6;
            curr = std::max(curr, 0);
            phase = (curr * 256 + total / 2) / total;
        }

        // cached info
        all_pawn_attacks[WHITE] = shift_pawn_attack<WHITE>(p.bbs[WHITE + PAWN]);
        all_pawn_attacks[BLACK] = shift_pawn_attack<BLACK>(p.bbs[BLACK + PAWN]);
        mobility_squares[WHITE] = ~(all_pawn_attacks[BLACK] | p.bbs[WHITE + KING]);
        mobility_squares[BLACK] = ~(all_pawn_attacks[WHITE] | p.bbs[BLACK + KING]);
        occ = p.bb_alls[WHITE] | p.bb_alls[BLACK];
        pawns = p.bbs[WHITE + PAWN] | p.bbs[BLACK + PAWN];
        attacked_by[WHITE] = all_pawn_attacks[WHITE];
        attacked_by[BLACK] = all_pawn_attacks[BLACK];
        king_sq[WHITE] = lsb<accel>(p.bbs[WHITE + KING]);
        king_sq[BLACK] = lsb<accel>(p.bbs[BLACK + KING]);

        int x = 0;
        x += p.stack().piece_sq;

        x += eval_pawns  (p, WHITE) - eval_pawns  (p, BLACK);
        x += eval_knights(p, WHITE) - eval_knights(p, BLACK);
        x += eval_bishops(p, WHITE) - eval_bishops(p, BLACK);
        x += eval_rooks  (p, WHITE) - eval_rooks  (p, BLACK);
        x += eval_queens (p, WHITE) - eval_queens (p, BLACK);
        x += eval_king   (p, WHITE) - eval_king   (p, BLACK);

        x += eval_king_safety(p, WHITE) - eval_king_safety(p, BLACK);

        x = (SC_MG(x) * (256 - phase) + SC_EG(x) * phase) / 256;
        x = (c == WHITE ? x : -x);

        return x;
    }
};

}
