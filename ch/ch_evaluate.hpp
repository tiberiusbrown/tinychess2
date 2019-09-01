#pragma once

#include "ch_bb.hpp"
#include "ch_bb_sse.hpp"
#include "ch_bb_avx.hpp"

#include "ch_evaluator.hpp"
#include "ch_position.hpp"


namespace ch
{

template<acceleration accel>
static CH_FORCEINLINE int eval_piece_table(uint64_t pieces, int8_t const table[64])
{
    int r = 0;
    while(pieces)
        r += table[pop_lsb<accel>(pieces)];
    return r;
}

template<acceleration accel>
static CH_FORCEINLINE int eval_piece_table_flipped(uint64_t pieces, int8_t const table[64])
{
    int r = 0;
    while(pieces)
        r += table[pop_lsb<accel>(pieces) ^ 56];
    return r;
}

template<acceleration accel>
struct evaluator
{
    uint64_t pawn_attacks[2];
    uint64_t half_open_files[2];
    uint64_t open_files;
    int mg, eg, v;

    CH_FORCEINLINE int king_safety_bonuses(position const& p, color c)
    {
        uint64_t t = masks[lsb<accel>(p.bbs[c + KING])].king_areas[c];
        t &= (p.bbs[c + PAWN] | p.bbs[c + KNIGHT] | p.bbs[c + BISHOP]);
        return KING_DEFENDERS_MG[popcnt<accel>(t)] * mg / 256;
    }

    CH_FORCEINLINE int eval_passed_pawns(position const& p, color c)
    {
        uint64_t t = p.bbs[opposite(c) + PAWN];
        t |= shift_w(t) | shift_e(t);
        t = slide_fill_forward(opposite(c), t, ~0ull);
        t = p.bbs[c + PAWN] & ~t;

        uint64_t const alle = p.bb_alls[opposite(c)];
        int mk = lsb<accel>(p.bbs[c + KING]);
        int ek = lsb<accel>(p.bbs[opposite(c) + KING]);

        int x = 0, xm = 0, xe = 0;
        while(t)
        {
            uint64_t m = lsb_mask<accel>(t);
            int sq = pop_lsb<accel>(t);
            int rank = (c == WHITE ? 7 - (sq >> 3) : sq >> 3);
            xm += PASSED_PAWN_MG[rank];
            xe += PASSED_PAWN_EG[rank];
            xe += (cheby_dist[sq][ek] - cheby_dist[sq][mk]) *
                PASSED_PAWN_KING_ESCORT;
            if(!(slide_fill_forward(c, m, ~0ull) & alle))
                xe += PASSED_PAWN_FREE_EG[rank];
        }

        return x + (xm * mg + xe * eg) / 256;
    }

    CH_FORCEINLINE int eval_pawns(position const& p, color c)
    {
        int x = 0;
        uint64_t const mp = p.bbs[c + PAWN];
        uint64_t const a = pawn_attacks[c];
        x += popcnt<accel>(a & p.bb_alls[c]) * PAWN_PROTECT_ANY;
        x += popcnt<accel>(a & mp) * PAWN_PROTECT_PAWN;
        x += popcnt<accel>(a & p.bbs[opposite(c) + KNIGHT]) * PAWN_THREATEN_KNIGHT;
        x += popcnt<accel>(a & p.bbs[opposite(c) + BISHOP]) * PAWN_THREATEN_BISHOP;
        x += popcnt<accel>(a & p.bbs[opposite(c) + ROOK]) * PAWN_THREATEN_ROOK;
        x += popcnt<accel>(a & p.bbs[opposite(c) + QUEEN]) * PAWN_THREATEN_QUEEN;
        return x;
    }

    CH_FORCEINLINE int eval_knights(position const& p, color c)
    {
        uint64_t t = p.bbs[c + KNIGHT];
        if(!t) return 0;
        uint64_t const OPPOSING_HALF = (c == WHITE ?
            (RANK5 | RANK6 | RANK7 | RANK8) :
            (RANK1 | RANK2 | RANK3 | RANK4));
        int x = 0, xm = 0, xe = 0, xtb = 0, xtr = 0, xtq = 0, xtk = 0;
        {
            int n = popcnt<accel>(t) * popcnt<accel>(p.bbs[c + PAWN]);
            xm += n * KNIGHT_PAWN_BONUS_MG;
            xe += n * KNIGHT_PAWN_BONUS_EG;
        }
        {
            uint64_t op = t;
            op &= pawn_attacks[c];
            op &= ~pawn_attacks[opposite(c)];
            op &= OPPOSING_HALF;
            x += popcnt<accel>(op) * KNIGHT_OUTPOST;
            x += popcnt<accel>(op & half_open_files[c]) * KNIGHT_OUTPOST_HALF_OPEN_FILE;
            x += popcnt<accel>(op & open_files) * KNIGHT_OUTPOST_OPEN_FILE;
        }
        while(t)
        {
            int const sq = pop_lsb<accel>(t);
            uint64_t const a = masks[sq].knight_attacks;
            uint64_t const tb = a & p.bbs[opposite(c) + BISHOP];
            uint64_t const tr = a & p.bbs[opposite(c) + ROOK];
            uint64_t const tq = a & p.bbs[opposite(c) + QUEEN];
            uint64_t const tk = a & p.bbs[opposite(c) + KING];
            xtb += popcnt<accel>(tb);
            xtr += popcnt<accel>(tr);
            xtq += popcnt<accel>(tq);
            xtk += popcnt<accel>(tk);
        }
        return
            x +
            xtb * KNIGHT_THREATEN_BISHOP +
            xtr * KNIGHT_THREATEN_ROOK +
            xtq * KNIGHT_THREATEN_QUEEN +
            xtk * KNIGHT_THREATEN_KING +
            (xm * mg + xe * eg) / 256;
    }

    CH_FORCEINLINE int eval_bishops(position const& p, color c)
    {
        uint64_t t = p.bbs[c + BISHOP];
        if(!t) return 0;
        uint64_t const occ = p.bb_alls[WHITE] | p.bb_alls[BLACK];
        int x = 0, xtr = 0, xtq = 0, xtk = 0;
        while(t)
        {
            int const sq = pop_lsb<accel>(t);
            uint64_t const a = magic_bishop_attacks(sq, occ);
            uint64_t const tr = a & p.bbs[opposite(c) + ROOK];
            uint64_t const tq = a & p.bbs[opposite(c) + QUEEN];
            uint64_t const tk = a & p.bbs[opposite(c) + KING];
            x += popcnt<accel>(a);
            xtr += popcnt<accel>(tr);
            xtq += popcnt<accel>(tq);
            xtk += popcnt<accel>(tk);
        }
        return (
            x * (mg * BISHOP_MOBILITY_BONUS_MG + eg * BISHOP_MOBILITY_BONUS_EG) / 256 +
            xtr * BISHOP_THREATEN_ROOK +
            xtq * BISHOP_THREATEN_QUEEN +
            xtk * BISHOP_THREATEN_KING +
            0);
    }

    CH_FORCEINLINE int eval_rooks(position const& p, color c)
    {
        uint64_t t = p.bbs[c + ROOK];
        if(!t) return 0;
        uint64_t const occ = p.bb_alls[WHITE] | p.bb_alls[BLACK];
        int x = 0, xm = 0, xtq = 0, xtk = 0;
        x += popcnt<accel>(t & open_files) * ROOK_ON_OPEN_FILE;
        while(t)
        {
            int const sq = pop_lsb<accel>(t);
            uint64_t const a = magic_rook_attacks(sq, occ);
            uint64_t const tq = a & p.bbs[opposite(c) + QUEEN];
            uint64_t const tk = a & p.bbs[opposite(c) + KING];
            xm += popcnt<accel>(a);
            xtq += popcnt<accel>(tq);
            xtk += popcnt<accel>(tk);
        }
        return (
            x +
            xm * (mg * ROOK_MOBILITY_BONUS_MG + eg * ROOK_MOBILITY_BONUS_EG) / 256 +
            xtq * ROOK_THREATEN_QUEEN +
            xtk * ROOK_THREATEN_KING +
            0);
    }

    CH_FORCEINLINE int eval_queens(position const& p, color c)
    {
        uint64_t t = p.bbs[c + QUEEN];
        int x = 0;
        x += popcnt<accel>(t & open_files) * QUEEN_ON_OPEN_FILE;
        return x;
    }

    CH_FORCEINLINE int eval_pieces(position const& p, color c)
    {
        int x = 0;

        x += eval_pawns  (p, c);
        x += eval_knights(p, c);
        x += eval_bishops(p, c);
        x += eval_rooks  (p, c);
        x += eval_queens (p, c);

        return x;
    }

    CH_FORCEINLINE int evaluate_first(position const& p, color c)
    {
        int x = 0;
        {
            static constexpr int const EG_MAT = 650;
            static constexpr int const MG_MAT = EG_MAT + 2048;
            mg = std::max(
                p.stack().piece_vals[WHITE],
                p.stack().piece_vals[BLACK]);
            mg = std::min(mg, MG_MAT);
            mg -= EG_MAT;
            mg = std::max(mg, 0);
            mg *= 256;
            mg /= (MG_MAT - EG_MAT);
            eg = 256 - mg;
        }
        x += p.stack().piece_vals[WHITE];
        x -= p.stack().piece_vals[BLACK];
        x += (
            p.stack().piece_sq[0] * mg +
            p.stack().piece_sq[1] * eg) / 256;
        x = (c == WHITE ? x : -x);
        return v = x;
    }

    CH_FORCEINLINE int evaluate_second_side(position const& p, color c)
    {
        int x = 0;

        // pawn attacks
        pawn_attacks[WHITE] = shift_pawn_attack<WHITE>(p.bbs[WHITE + PAWN]);
        pawn_attacks[BLACK] = shift_pawn_attack<BLACK>(p.bbs[BLACK + PAWN]);

        // find open and half-open files
        half_open_files[WHITE] = half_open_files[BLACK] = 0;
        open_files = 0;
        {
            uint64_t wp = p.bbs[WHITE + PAWN];
            uint64_t bp = p.bbs[BLACK + PAWN];
            uint64_t pp = wp | bp;
            for(int i = 0; i < 8; ++i)
            {
                uint64_t f = file(i);
                bool wf = (f & wp) != 0;
                bool bf = (f & bp) != 0;
                if(bf && !wf)
                {
                    half_open_files[WHITE] |= f;
                    x += HALF_OPEN_FILE;
                }
                else if(wf && !bf)
                {
                    half_open_files[BLACK] |= f;
                    x -= HALF_OPEN_FILE;
                }
                else if(!(f & pp))
                    open_files |= f;
            }
        }

        x += eval_pieces(p, c);
        x += eval_passed_pawns(p, c);
        //x += king_safety_bonuses<accel>(p, c);

        if(!p.bbs[c + PAWN])
        {
            // cannot mate with own material
            int pv = p.stack().piece_vals[c];
            if(pv <= PIECE_VALUES[BISHOP])
                x /= 2;
            // if endgame and ahead in material, give a bonus if our
            // king is close to enemy king (assisting in mate)
            else if(mg == 0 && !p.bbs[opposite(c) + PAWN] &&
                pv >= p.stack().piece_vals[opposite(c)] + PIECE_VALUES[KNIGHT])
            {
                // find chebyshev distance to enemy king
                int mk = lsb<ACCEL_UNACCEL>(p.bbs[c + KING]);
                int ek = lsb<ACCEL_UNACCEL>(p.bbs[opposite(c) + KING]);
                x += (7 - cheby_dist[mk][ek]) * 4;
            }
        }
        return x;
    }

    int evaluate_second(position const& p, color c)
    {
        int x = 0;
        
        x += evaluate_second_side(p, WHITE);
        x -= evaluate_second_side(p, BLACK);
        
        x = (c == WHITE ? x : -x);
        
        return v += x;
    }

    int evaluate(position const& p, color c)
    {
        evaluate_first(p, c);
        return evaluate_second(p, c);
    }
};

}
