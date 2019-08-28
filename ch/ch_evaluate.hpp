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
static CH_FORCEINLINE int eval_bishops(
    color c, position const& p,
    int mg, int eg)
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

template<acceleration accel>
static CH_FORCEINLINE int eval_rooks(
    color c, position const& p,
    int mg, int eg)
{
    uint64_t t = p.bbs[c + ROOK];
    if(!t) return 0;
    uint64_t const occ = p.bb_alls[WHITE] | p.bb_alls[BLACK];
    int x = 0, xtq = 0, xtk = 0;
    while(t)
    {
        int const sq = pop_lsb<accel>(t);
        uint64_t const a = magic_rook_attacks(sq, occ);
        uint64_t const tq = a & p.bbs[opposite(c) + QUEEN];
        uint64_t const tk = a & p.bbs[opposite(c) + KING];
        x += popcnt<accel>(a);
        xtq += popcnt<accel>(tq);
        xtk += popcnt<accel>(tk);
    }
    return (
        x * (mg * ROOK_MOBILITY_BONUS_MG + eg * ROOK_MOBILITY_BONUS_EG) / 256 +
        xtq * ROOK_THREATEN_QUEEN +
        xtk * ROOK_THREATEN_KING +
        0);
}

// note: not included in pawn hash (TODO)
template<acceleration accel>
static CH_FORCEINLINE int eval_pawns(
    color c, position const& p,
    int mg, int eg)
{
    (void)mg;
    (void)eg;
    int x = 0;
    uint64_t const mp = p.bbs[c + PAWN];
    uint64_t const a = shift_pawn_attack(c, mp);
    x += popcnt<accel>(a & p.bb_alls[c]) * PAWN_PROTECT_ANY;
    x += popcnt<accel>(a & mp) * PAWN_PROTECT_PAWN;
    x += popcnt<accel>(a & p.bbs[opposite(c) + KNIGHT]) * PAWN_THREATEN_KNIGHT;
    x += popcnt<accel>(a & p.bbs[opposite(c) + BISHOP]) * PAWN_THREATEN_BISHOP;
    x += popcnt<accel>(a & p.bbs[opposite(c) + ROOK]) * PAWN_THREATEN_ROOK;
    x += popcnt<accel>(a & p.bbs[opposite(c) + QUEEN]) * PAWN_THREATEN_QUEEN;
    return x;
}

template<acceleration accel>
static CH_FORCEINLINE int eval_pieces(
    color c, position const& p,
    int mg, int eg)
{
    int x = 0;

    x += eval_pawns<accel>(c, p, mg, eg);
    x += eval_bishops<accel>(c, p, mg, eg);
    x += eval_rooks<accel>(c, p, mg, eg);

    return x;
}

template<acceleration accel>
static CH_FORCEINLINE int doubled_pawn_penalties(color c, position const& p)
{
    uint64_t const pawns = p.bbs[c + PAWN];
    uint64_t mask = FILEA;
    int x = 0;
    for(int i = 0; i < 8; ++i)
    {
        uint64_t file_pawns = pawns & mask;
        if(more_than_one<accel>(file_pawns))
            x += DOUBLED_PAWN_PENALTIES[i];
        mask <<= 1;
    }
    return x;
}

template<acceleration accel>
static CH_FORCEINLINE int passed_pawn_bonuses(
    color c, position const& p,
    int mg, int eg)
{
    uint64_t t = p.bbs[opposite(c) + PAWN];
    t |= shift_w(t) | shift_e(t);
    if(c == WHITE)
        t = slide_fill_s(t, ~0ull);
    else
        t = slide_fill_n(t, ~0ull);
    t = p.bbs[c + PAWN] & ~t;
    //int xm = 0, xe = 0;
    //while(t)
    //{
    //    xm += piece_tables[0][c + PASSED_PAWN][pop_lsb<accel>(t)];
    //    xe += piece_tables[1][c + PASSED_PAWN][pop_lsb<accel>(t)];
    //}
    //return (xm * mg + xe * eg) / 256;
    (void)mg;
    (void)eg;
    return popcnt<accel>(t) * (mg * 8 + eg * 64) / 256;
}

template<acceleration accel>
static CH_FORCEINLINE int king_safety_bonuses(
    color c, position const& p, int mg)
{
    uint64_t t = masks[lsb<accel>(p.bbs[c + KING])].king_attacks;
    t &= p.bb_alls[c];
    return popcnt<accel>(t) * mg * KING_SAFETY_BONUS / 256;
}

template<acceleration accel>
struct evaluator
{
    int mg, eg, v;

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
        x += eval_pieces<accel>(c, p, mg, eg);
        x -= doubled_pawn_penalties<accel>(c, p);
        //x += passed_pawn_bonuses<accel>(c, p, mg, eg);
        //x += king_safety_bonuses<accel>(c, p, mg);
        if(!p.bbs[c + PAWN])
        {
            // cannot mate with own material
            int pv = p.stack().piece_vals[c];
            if(pv <= PIECE_VALUES[BISHOP])
                x /= 2;
            // if endgame and ahead in material, give a bonus if our
            // king is close to enemy king (assisting in mate)
            else if(mg == 0 &&
                pv >= p.stack().piece_vals[opposite(c)] + PIECE_VALUES[KNIGHT])
            {
                // find manhatten distance to enemy king
                int mk = lsb<ACCEL_UNACCEL>(p.bbs[c + KING]);
                int ek = lsb<ACCEL_UNACCEL>(p.bbs[opposite(c) + KING]);
                int dx = std::abs((mk & 7) - (ek & 7));
                int dy = std::abs((mk >> 3) - (ek >> 3));
                int d = 14 - dx - dy;
                x += d * 4;
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
