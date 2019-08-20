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
static CH_FORCEINLINE int mobility_bonuses(color c, position const& p)
{
    int x = 0;
    uint64_t const diag = p.bbs[c + QUEEN] | p.bbs[c + BISHOP];
    uint64_t const orth = p.bbs[c + QUEEN] | p.bbs[c + ROOK];
    //uint64_t const cap = ~p.bb_alls[c];
    uint64_t const empty = ~(p.bb_alls[WHITE] | p.bb_alls[BLACK]);

    uint64_t diag_moves =
        slide_attack_nw(diag, empty) |
        slide_attack_ne(diag, empty) |
        slide_attack_sw(diag, empty) |
        slide_attack_se(diag, empty);
    uint64_t orth_moves =
        slide_attack_n(orth, empty) |
        slide_attack_s(orth, empty) |
        slide_attack_w(orth, empty) |
        slide_attack_e(orth, empty);

    //diag_moves &= cap;
    //orth_moves &= cap;

    x += popcnt<accel>(diag_moves) * BISHOP_MOBILITY_BONUS;
    x += popcnt<accel>(orth_moves) * ROOK_MOBILITY_BONUS;

    return x;
}

// TODO: this is slightly slower...
#if 0
#if CH_ENABLE_AVX
template<>
CH_FORCEINLINE int mobility_bonuses<ACCEL_AVX>(color c, position const& p)
{
    int x = 0;
    uint64_t const diag = p.bbs[c + QUEEN] | p.bbs[c + BISHOP];
    uint64_t const orth = p.bbs[c + QUEEN] | p.bbs[c + ROOK];
    __m256i const cap = _mm256_set1_epi64x(~p.bb_alls[c]);
    __m256i const sliders = _mm256_setr_epi64x(orth, orth, diag, diag);
    __m256i const empty = _mm256_set1_epi64x(
        ~(p.bb_alls[WHITE] | p.bb_alls[BLACK]));

    union
    {
        __m256i u256;
        uint64_t u64[4];
    } u;
    u.u256 = _mm256_or_si256(
        slide_attack_n_w_nw_ne(sliders, empty),
        slide_attack_s_e_sw_se(sliders, empty));
    //u.u256 = _mm256_and_si256(cap, u.u256);
    
    uint64_t orth_moves = u.u64[0] | u.u64[1];
    uint64_t diag_moves = u.u64[2] | u.u64[3];

    x += popcnt<ACCEL_AVX>(diag_moves) * BISHOP_MOBILITY_BONUS;
    x += popcnt<ACCEL_AVX>(orth_moves) * ROOK_MOBILITY_BONUS;

    return x;
}
#endif
#endif

template<acceleration accel>
static CH_FORCEINLINE int pawn_protector_bonuses(color c, position const& p)
{
    uint64_t pawn_protectors;
    uint64_t const pawns = p.bbs[c + PAWN];
    if(c == WHITE)
        pawn_protectors = (shift_sw(pawns) | shift_se(pawns));
    else
        pawn_protectors = (shift_nw(pawns) | shift_ne(pawns));
    pawn_protectors &= pawns;
    return popcnt<accel>(pawn_protectors) * PAWN_PROTECTOR_BONUS;
}

template<acceleration accel>
static CH_FORCEINLINE int passed_pawn_penalties(color c, position const& p)
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
static CH_FORCEINLINE int king_safety_bonuses(color c, position const& p)
{
    uint64_t t = masks[lsb<accel>(p.bbs[c + KING])].king_attacks;
    t &= p.bb_alls[c];
    return popcnt<accel>(t) * KING_SAFETY_BONUS;
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
        x += mobility_bonuses<accel>(c, p);
        x += pawn_protector_bonuses<accel>(c, p);
        x += passed_pawn_penalties<accel>(c, p);
        //x += king_safety_bonuses<accel>(c, p);
        if(!p.bbs[c + PAWN])
        {
            int pv = p.stack().piece_vals[c];
            if(pv <= PIECE_VALUES[BISHOP] ||
                //pv == PIECE_VALUES[KNIGHT] + PIECE_VALUES[BISHOP] ||
                0)
                x /= 2;
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
