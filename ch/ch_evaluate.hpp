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
    uint64_t const cap = ~p.bb_alls[c];
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
    uint64_t const all = p.bb_alls[c];
    if(c == WHITE)
        pawn_protectors = (shift_sw(all) | shift_se(all));
    else
        pawn_protectors = (shift_nw(all) | shift_ne(all));
    pawn_protectors &= p.bbs[c + PAWN];
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
            x += PASSED_PAWN_PENALTIES[i];
        mask <<= 1;
    }
    return x;
}

template<acceleration accel>
struct evaluator
{
    static CH_FORCEINLINE int evaluate_simple(position const& p, color c)
    {
        int x = 0;
        x += p.stack().piece_vals[WHITE];
        x -= p.stack().piece_vals[BLACK];
        x = (c == WHITE ? x : -x);
        return x;
    }

    static int evaluate(position const& p, color c)
    {
        int x = 0;

        x += p.stack().piece_vals[WHITE];
        x -= p.stack().piece_vals[BLACK];

        // phase: 0 is false, 256 is true
        int mg, eg;
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

        {
            int d = 0;

            d += eval_piece_table<accel>(p.bbs[WHITE + KNIGHT], INIT_TABLE_KNIGHT);
            d += eval_piece_table<accel>(p.bbs[WHITE + BISHOP], INIT_TABLE_BISHOP);
            d += eval_piece_table<accel>(p.bbs[WHITE + ROOK], INIT_TABLE_ROOK);
            d += eval_piece_table<accel>(p.bbs[WHITE + QUEEN], INIT_TABLE_QUEEN);

            d -= eval_piece_table_flipped<accel>(p.bbs[BLACK + KNIGHT], INIT_TABLE_KNIGHT);
            d -= eval_piece_table_flipped<accel>(p.bbs[BLACK + BISHOP], INIT_TABLE_BISHOP);
            d -= eval_piece_table_flipped<accel>(p.bbs[BLACK + ROOK], INIT_TABLE_ROOK);
            d -= eval_piece_table_flipped<accel>(p.bbs[BLACK + QUEEN], INIT_TABLE_QUEEN);

            x += d * mg / 256;

            x += eval_piece_table<accel>(p.bbs[WHITE + PAWN], INIT_TABLE_PAWN);
            x -= eval_piece_table_flipped<accel>(p.bbs[BLACK + PAWN], INIT_TABLE_PAWN);
        }

        {
            int wk = lsb<accel>(p.bbs[WHITE + KING]);
            int bk = lsb<accel>(p.bbs[BLACK + KING]) ^ 56;
            int d = 0;
            d += (TABLE_KING_MG[wk] - TABLE_KING_MG[bk]) * mg;
            d += (TABLE_KING_EG[wk] - TABLE_KING_EG[bk]) * eg;
            x += d / 256;
        }

        x += mobility_bonuses<accel>(WHITE, p);
        x -= mobility_bonuses<accel>(BLACK, p);

        x += pawn_protector_bonuses<accel>(WHITE, p);
        x -= pawn_protector_bonuses<accel>(BLACK, p);

        x += passed_pawn_penalties<accel>(WHITE, p);
        x -= passed_pawn_penalties<accel>(BLACK, p);

        x = (c == WHITE ? x : -x);

        return x;
    }
};

}
