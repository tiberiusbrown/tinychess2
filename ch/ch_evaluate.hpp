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
    uint64_t pawn_attacks[2];
    uint64_t mobility_squares[2];
    uint64_t occ;
    uint64_t pawns;
    int num_pawns[2];

    CH_FORCEINLINE int eval_pawns(position const& p, color c)
    {
        uint64_t const mp = p.bbs[c + PAWN];
        uint64_t const ep = p.bbs[c + PAWN];
        uint64_t t = mp;
        int x = 0;
        while(t)
        {
            int sq = pop_lsb<accel>(t);

            uint64_t const pawns_in_adjacent_files = mp & adjacent_files_of_sq(sq);
            uint64_t const enemy_pawn_threats = ep & masks[sq].pawn_attacks[c];
            //uint64_t const enemy_pawn_push_threats = ep & masks[forward_sq(sq, c)].pawn_attacks[c];

            // isolated pawn penalty
            if(!pawns_in_adjacent_files && !enemy_pawn_threats)
                x += PAWN_ISOLATED;

            // doubled pawn penalty
            if(more_than_one<accel>(file_of_sq(sq) & mp))
                x += PAWN_DOUBLED;

            // backward pawn penalty
            //if(pawns_in_adjacent_files && enemy_pawn_push_threats && !pawns_protecting)
        }
        return x;
    }

    CH_FORCEINLINE int eval_knights(position const& p, color c)
    {
        uint64_t t = p.bbs[c + KNIGHT];
        int x = 0;
        x += (KNIGHT_PAWN_ADJUST[num_pawns[c]] +
            KNIGHT_ENEMY_PAWN_ADJUST[num_pawns[opposite(c)]]) *
            popcnt<accel>(t);
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks = masks[sq].knight_attacks;
            uint64_t const mask = (1ull << sq);

            // knight outpost bonus
            if((mask & OUTPOST_RANKS[c]) && !(mask & pawn_attacks[opposite(c)]))
            {
                int const outside = (mask & (FILEA | FILEH)) != 0;
                int const defended = (mask & pawn_attacks[c]) != 0;
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
        x += (BISHOP_PAWN_ADJUST[num_pawns[c]] +
            BISHOP_ENEMY_PAWN_ADJUST[num_pawns[opposite(c)]]) *
            popcnt<accel>(t);
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks = magic_bishop_attacks(sq, occ);
            uint64_t const mask = (1ull << sq);

            // bishop outpost bonus
            if((mask & OUTPOST_RANKS[c]) && !(mask & pawn_attacks[opposite(c)]))
            {
                int const outside = (mask & (FILEA | FILEH)) != 0;
                int const defended = (mask & pawn_attacks[c]) != 0;
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
        x += (ROOK_PAWN_ADJUST[num_pawns[c]] +
            ROOK_ENEMY_PAWN_ADJUST[num_pawns[opposite(c)]]) *
            popcnt<accel>(t);
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks = magic_rook_attacks(sq, occ);

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
        x += (QUEEN_PAWN_ADJUST[num_pawns[c]] +
            QUEEN_ENEMY_PAWN_ADJUST[num_pawns[opposite(c)]]) *
            popcnt<accel>(t);
        while(t)
        {
            int sq = pop_lsb<accel>(t);
            uint64_t const attacks =
                magic_bishop_attacks(sq, occ) |
                magic_rook_attacks(sq, occ);

            // mobility bonus
            {
                int mob = popcnt<accel>(attacks & mobility_squares[c]);
                mob = clamp(mob, 3, 24);
                x += QUEEN_MOBILITY[mob];
            }
        }
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
        pawn_attacks[WHITE] = shift_pawn_attack<WHITE>(p.bbs[WHITE + PAWN]);
        pawn_attacks[BLACK] = shift_pawn_attack<BLACK>(p.bbs[BLACK + PAWN]);
        mobility_squares[WHITE] = ~(pawn_attacks[BLACK] | p.bbs[WHITE + KING]);
        mobility_squares[BLACK] = ~(pawn_attacks[WHITE] | p.bbs[BLACK + KING]);
        occ = p.bb_alls[WHITE] | p.bb_alls[BLACK];
        pawns = p.bbs[WHITE + PAWN] | p.bbs[BLACK + PAWN];
        num_pawns[WHITE] = popcnt<accel>(p.bbs[WHITE + PAWN]);
        num_pawns[BLACK] = popcnt<accel>(p.bbs[BLACK + PAWN]);

        int x = 0;
        x += p.stack().piece_sq;

        x += eval_pawns  (p, WHITE) - eval_pawns  (p, BLACK);
        x += eval_knights(p, WHITE) - eval_knights(p, BLACK);
        x += eval_bishops(p, WHITE) - eval_bishops(p, BLACK);
        x += eval_rooks  (p, WHITE) - eval_rooks  (p, BLACK);
        x += eval_queens (p, WHITE) - eval_queens (p, BLACK);

        x = (SC_MG(x) * (256 - phase) + SC_EG(x) * phase) / 256;
        x = (c == WHITE ? x : -x);

        return x;
    }
};

}
