#pragma once

#include "ch_internal.hpp"
#include "ch_move.hpp"

namespace ch
{

struct position;

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel>
struct move_generator
{
    static int generate(move* mvs, position const& p);
};
#else
template<acceleration accel>
struct move_generator
{
    static int generate(color c, move* mvs, position const& p);
};
#endif

#if CH_COLOR_TEMPLATE
template<color c>
CH_FORCEINLINE move* add_pawn_promotions(move* m, move mv)
#else
CH_FORCEINLINE move* add_pawn_promotions(color c, move* m, move mv)
#endif
{
    m[0] = mv + move::pawn_promotion(c + QUEEN);
    m[1] = mv + move::pawn_promotion(c + KNIGHT);
    m[2] = mv + move::pawn_promotion(c + ROOK);
    m[3] = mv + move::pawn_promotion(c + BISHOP);
    return m + 4;
}

#if CH_COLOR_TEMPLATE
template<color c, bool promote>
CH_FORCEINLINE move* generate_non_pinned_pawn_moves(move* m,
#else
template<bool promote>
CH_FORCEINLINE move* generate_non_pinned_pawn_moves(color c, move* m,
#endif
    uint64_t pawns, uint64_t empty, uint64_t enemy_all)
{
    if(promote && pawns == 0) return m;

    // forward moves
    uint64_t pf;
    if(c == WHITE) pf = shift_n(pawns) & empty;
    else pf = shift_s(pawns) & empty;
    uint64_t t = pf;
    while(t)
    {
        int i = pop_lsb(t);
        if(promote)
        {
#if CH_COLOR_TEMPLATE
            if(c == WHITE) m = add_pawn_promotions<c>(m, move(i + 8, i));
            else m = add_pawn_promotions<c>(m, move(i - 8, i));
#else
            if(c == WHITE) m = add_pawn_promotions(c, m, move(i + 8, i));
            else m = add_pawn_promotions(c, m, move(i - 8, i));
#endif
        }
        else
        {
            if(c == WHITE) *m++ = (move(i + 8, i));
            else *m++ = (move(i - 8, i));
        }
    }

    if(!promote)
    {
        // double forward moves
        if(c == WHITE)
            pf = shift_n(pf & RANK3) & empty;
        else
            pf = shift_s(pf & RANK6) & empty;
        while(pf)
        {
            int i = pop_lsb(pf);
            if(c == WHITE)
                *m++ = (move::pawn_dmove(i + 16, i));
            else
                *m++ = (move::pawn_dmove(i - 16, i));
        }
    }

    // captures
    if(c == WHITE)
    {
        pf = shift_nw(pawns) & enemy_all;
        while(pf)
        {
            int i = pop_lsb(pf);
#if CH_COLOR_TEMPLATE
            if(promote) m = add_pawn_promotions<c>(m, move(i + 9, i));
#else
            if(promote) m = add_pawn_promotions(c, m, move(i + 9, i));
#endif
            else *m++ = move(i + 9, i);
        }
        pf = shift_ne(pawns) & enemy_all;
        while(pf)
        {
            int i = pop_lsb(pf);
#if CH_COLOR_TEMPLATE
            if(promote) m = add_pawn_promotions<c>(m, move(i + 7, i));
#else
            if(promote) m = add_pawn_promotions(c, m, move(i + 7, i));
#endif
            else *m++ = move(i + 7, i);
        }
    }
    else
    {
        pf = shift_sw(pawns) & enemy_all;
        while(pf)
        {
            int i = pop_lsb(pf);
#if CH_COLOR_TEMPLATE
            if(promote) m = add_pawn_promotions<c>(m, move(i - 7, i));
#else
            if(promote) m = add_pawn_promotions(c, m, move(i - 7, i));
#endif
            else *m++ = move(i - 7, i);
        }
        pf = shift_se(pawns) & enemy_all;
        while(pf)
        {
            int i = pop_lsb(pf);
#if CH_COLOR_TEMPLATE
            if(promote) m = add_pawn_promotions<c>(m, move(i - 9, i));
#else
            if(promote) m = add_pawn_promotions(c, m, move(i - 9, i));
#endif
            else *m++ = move(i - 9, i);
        }
    }

    return m;
}

#if CH_COLOR_TEMPLATE
template<color c>
CH_FORCEINLINE bool is_en_passant_legal(
#else
CH_FORCEINLINE bool is_en_passant_legal(color c,
#endif
    uint64_t pawn, uint64_t ep, int king_sq, uint64_t all_pieces,
    uint64_t enemy_orth_sliders, uint64_t enemy_diag_sliders)
{
    if(!pawn) return false;
    uint64_t mask = all_pieces ^ pawn ^ ep;
    if(c == WHITE) mask ^= shift_n(ep);
    else mask ^= shift_s(ep);
    uint64_t attacks =
        (hq_bishop_attacks(king_sq, mask) & enemy_diag_sliders) |
        (hq_rook_attacks(king_sq, mask) & enemy_orth_sliders);
    return attacks == 0;
}

#if CH_COLOR_TEMPLATE
template<color c>
CH_FORCEINLINE move* generate_en_passant_moves(
#else
CH_FORCEINLINE move* generate_en_passant_moves(color c,
#endif
    move* m, position const& p, int king_sq, uint64_t all_pieces,
    uint64_t enemy_orth_sliders, uint64_t enemy_diag_sliders)
{
    int ep_sq = p.stack().ep_sq;
    if(!ep_sq) return m;
    uint64_t ep = (1ull << ep_sq);
    int dest = (c == WHITE ? ep_sq - 8 : ep_sq + 8);
#if CH_COLOR_TEMPLATE
    if(is_en_passant_legal<c>(shift_w(ep) & p.bbs[c + PAWN], ep,
#else
    if(is_en_passant_legal(c, shift_w(ep) & p.bbs[c + PAWN], ep,
#endif
        king_sq, all_pieces, enemy_orth_sliders, enemy_diag_sliders))
    {
        *m++ = move::en_passant(ep_sq) + move(ep_sq - 1, dest);
    }
#if CH_COLOR_TEMPLATE
    if(is_en_passant_legal<c>(shift_e(ep) & p.bbs[c + PAWN], ep,
#else
    if(is_en_passant_legal(c, shift_e(ep) & p.bbs[c + PAWN], ep,
#endif
        king_sq, all_pieces, enemy_orth_sliders, enemy_diag_sliders))
    {
        *m++ = move::en_passant(ep_sq) + move(ep_sq + 1, dest);
    }
    return m;
}

}
