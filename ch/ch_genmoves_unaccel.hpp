#pragma once

#include "ch_internal.hpp"

#include "ch_bb.hpp"
#include "ch_genmoves.hpp"
#include "ch_position.hpp"

namespace ch
{

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

#if CH_ARCH_32BIT

#if CH_COLOR_TEMPLATE
template<color c>
struct move_generator<c, ACCEL_UNACCEL>
{
    static int generate(move* mvs, position& p)
#else
template<>
struct move_generator<ACCEL_UNACCEL>
{
    static int generate(color c, move* mvs, position& p)
#endif
    {

#if CH_ARCH_64BIT
#if CH_COLOR_TEMPLATE
        return move_generator<c, ACCEL_SSE>::generate(mvs, p);
#else
        return move_generator<ACCEL_SSE>::generate(c, mvs, p);
#endif
#else

        move* m = mvs;

        color const enemy_color = opposite(c);

        // setup bitboards

        uint64_t const my_diag_sliders = p.bbs[c + QUEEN] | p.bbs[c + BISHOP];
        uint64_t const my_orth_sliders = p.bbs[c + QUEEN] | p.bbs[c + ROOK];
        uint64_t const enemy_diag_sliders =
            p.bbs[enemy_color + QUEEN] | p.bbs[enemy_color + BISHOP];
        uint64_t const enemy_orth_sliders =
            p.bbs[enemy_color + QUEEN] | p.bbs[enemy_color + ROOK];
        uint64_t const my_all =
            my_diag_sliders | my_orth_sliders |
            p.bbs[c + PAWN] | p.bbs[c + KNIGHT] | p.bbs[c + KING];
        uint64_t const enemy_all =
            enemy_diag_sliders | enemy_orth_sliders |
            p.bbs[enemy_color + PAWN] | p.bbs[enemy_color + KNIGHT] |
            p.bbs[enemy_color + KING];
        uint64_t const all_pieces = my_all | enemy_all;
        uint64_t const my_pro_pawns = p.bbs[c + PAWN] &
            (c == WHITE ? RANK7 : RANK2);
        uint64_t const my_nonpro_pawns = p.bbs[c + PAWN] ^ my_pro_pawns;

        uint64_t const capture = ~my_all;
        uint64_t const empty = ~all_pieces;
        uint64_t const king = p.bbs[c + KING];

        uint64_t pin_mask = 0;
        uint64_t attacked_nonking = 0;
        int const king_sq = lsb(king);

        // find squares attacked by enemy
        // these are used to test for both check and castling legality
        {
            uint64_t const empty_nonking = empty ^ king;
            uint64_t e;
            e = enemy_diag_sliders;
            attacked_nonking |= slide_attack_nw(e, empty_nonking);
            attacked_nonking |= slide_attack_ne(e, empty_nonking);
            attacked_nonking |= slide_attack_sw(e, empty_nonking);
            attacked_nonking |= slide_attack_se(e, empty_nonking);
            e = enemy_orth_sliders;
            attacked_nonking |= slide_attack_n(e, empty_nonking);
            attacked_nonking |= slide_attack_s(e, empty_nonking);
            attacked_nonking |= slide_attack_w(e, empty_nonking);
            attacked_nonking |= slide_attack_e(e, empty_nonking);
            e = p.bbs[enemy_color + KING];
            assert(e != 0);
            attacked_nonking |= masks[lsb(e)].king_attacks;
            e = p.bbs[enemy_color + KNIGHT];
            while(e) attacked_nonking |= masks[pop_lsb(e)].knight_attacks;
            e = p.bbs[enemy_color + PAWN];
            if(enemy_color == WHITE)
                attacked_nonking |= shift_nw(e) | shift_ne(e);
            else
                attacked_nonking |= shift_sw(e) | shift_se(e);
            p.attacked_nonking = attacked_nonking;
        }

        // find pinned pieces
        {
            // first find enemy pieces that could be pinners
            uint64_t potential_pinners =
                (masks[king_sq].rook_pseudo_attacks & enemy_orth_sliders) |
                (masks[king_sq].bishop_pseudo_attacks & enemy_diag_sliders);

            // see if exactly one piece is between any pinners and our king
            while(potential_pinners)
            {
                int sq = pop_lsb(potential_pinners);
                uint64_t between = betweens[king_sq][sq] & all_pieces;
                if(between && !(between & (between - 1)))
                    pin_mask |= between;
            }

            p.pinned_pieces = pin_mask;
        }

        // generate legal king moves
        {
            uint64_t a = masks[king_sq].king_attacks;
            a &= capture & ~attacked_nonking;
            while(a) *m++ = move(king_sq, pop_lsb(a));
        }
        
        p.in_check = ((attacked_nonking & king) != 0);
        if(p.in_check)
        {
            
            /*
            we are in check. specialized move generator:
                if there are any knights or pawns attacking our king:
                    if there are no slider threats to our king:
                        generate threat capture moves by non-pinned pieces
                        add (with full verification) en passant capture(s)
                else:
                    find slider threats to our king and ray attack masks
                    if there is not more than one slider threat:
                        generate threat capture / block ray attack moves
                            (non-pinned pieces only -- pinned pieces can
                            never move to block or capture sliding attackers)
            */

            uint64_t slider_threats =
                (hq_bishop_attacks(king_sq, all_pieces) & enemy_diag_sliders) |
                (hq_rook_attacks(king_sq, all_pieces) & enemy_orth_sliders);
            uint64_t knight_pawn_threats =
                masks[king_sq].knight_attacks & p.bbs[enemy_color + KNIGHT];
            if(c == WHITE)
            {
                knight_pawn_threats |= (
                    masks[king_sq].white_pawn_attacks &
                    p.bbs[enemy_color + PAWN]);
            }
            else
            {
                knight_pawn_threats |= (
                    masks[king_sq].black_pawn_attacks &
                    p.bbs[enemy_color + PAWN]);
            }

            if(knight_pawn_threats)
            {
                assert(!more_than_one(knight_pawn_threats));
                if(slider_threats == 0)
                {
                    // generate non-pinned moves that capture threat
                    int i = lsb(knight_pawn_threats);
                    move mv = move::to(i);
                    uint64_t pcs =
                        (masks[i].knight_attacks & p.bbs[c + KNIGHT]) |
                        (hq_bishop_attacks(i, all_pieces) & my_diag_sliders) |
                        (hq_rook_attacks(i, all_pieces) & my_orth_sliders);
                    if(c == WHITE)
                        pcs |= (masks[i].black_pawn_attacks & my_nonpro_pawns);
                    else
                        pcs |= (masks[i].white_pawn_attacks & my_nonpro_pawns);
                    pcs &= ~pin_mask;
                    while(pcs) *m++ = mv + move::from(pop_lsb(pcs));

                    // TODO: promotions

#if CH_COLOR_TEMPLATE
                    m = generate_en_passant_moves<c>(m, p, king_sq,
#else
                    m = generate_en_passant_moves(c, m, p, king_sq,
#endif
                        all_pieces, enemy_orth_sliders, enemy_diag_sliders);
                }
            }
            else if(!more_than_one(slider_threats))
            {
                uint64_t ray = betweens[king_sq][lsb(slider_threats)];

                // generate non-pinned non-pawn moves that resolve threat
                uint64_t ray_and_threat = ray | slider_threats;
                while(ray_and_threat)
                {
                    int i = pop_lsb(ray_and_threat);
                    move mv = move::to(i);
                    uint64_t pcs = (
                        (masks[i].knight_attacks & p.bbs[c + KNIGHT]) |
                        (hq_bishop_attacks(i, all_pieces) & my_diag_sliders) |
                        (hq_rook_attacks(i, all_pieces) & my_orth_sliders)
                        ) & ~pin_mask;
                    while(pcs) *m++ = mv + move::from(pop_lsb(pcs));
                }

                // generate non-pinned pawn forward moves that block threat
                uint64_t non_pinned_pawns = p.bbs[c + PAWN] & ~pin_mask;
                if(c == WHITE)
                {
                    uint64_t b = shift_n(non_pinned_pawns) & empty;
                    uint64_t t = b & ray;
                    while(t)
                    {
                        int i = pop_lsb(t);
                        if(t & my_pro_pawns)
#if CH_COLOR_TEMPLATE
                            m = add_pawn_promotions<c>(m, move(i + 8, i));
#else
                            m = add_pawn_promotions(c, m, move(i + 8, i));
#endif
                        else
                            *m++ = move(i + 8, i);
                    }
                    t = shift_n(b & RANK3) & empty & ray;
                    while(t)
                    {
                        int i = pop_lsb(t);
                        *m++ = move::pawn_dmove(i + 16, i);
                    }
                }
                else
                {
                    uint64_t b = shift_s(non_pinned_pawns) & empty;
                    uint64_t t = b & ray;
                    while(t)
                    {
                        int i = pop_lsb(t);
                        if(t & my_pro_pawns)
#if CH_COLOR_TEMPLATE
                            m = add_pawn_promotions<c>(m, move(i - 8, i));
#else
                            m = add_pawn_promotions(c, m, move(i - 8, i));
#endif
                        else
                            *m++ = move(i - 8, i);
                    }
                    t = shift_s(b & RANK6) & empty & ray;
                    while(t)
                    {
                        int i = pop_lsb(t);
                        *m++ = move::pawn_dmove(i - 16, i);
                    }
                }

                // generate non-pinned pawn moves that capture threat
                // TODO: promotions
                if(c == WHITE)
                {
                    int i = lsb(slider_threats);
                    move mv = move::to(i);
                    uint64_t t = masks[i].black_pawn_attacks &
                        non_pinned_pawns & my_nonpro_pawns;
                    while(t) *m++ = mv + move::from(pop_lsb(t));
                    t = masks[i].black_pawn_attacks &
                        non_pinned_pawns & my_pro_pawns;
#if CH_COLOR_TEMPLATE
                    while(t) m = add_pawn_promotions<c>(m, mv + move::from(pop_lsb(t)));
#else
                    while(t) m = add_pawn_promotions(c, m, mv + move::from(pop_lsb(t)));
#endif
                }
                else
                {
                    int i = lsb(slider_threats);
                    move mv = move::to(i);
                    uint64_t t = masks[i].white_pawn_attacks &
                        non_pinned_pawns & my_nonpro_pawns;
                    while(t) *m++ = mv + move::from(pop_lsb(t));
                    t = masks[i].white_pawn_attacks &
                        non_pinned_pawns & my_pro_pawns;
#if CH_COLOR_TEMPLATE
                    while(t) m = add_pawn_promotions<c>(m, mv + move::from(pop_lsb(t)));
#else
                    while(t) m = add_pawn_promotions(c, m, mv + move::from(pop_lsb(t)));
#endif
                }
            }

            return int(m - mvs);
        }

        // generate castlings
        {
            int castling_rights = p.stack().castling_rights;
            if(c == WHITE)
            {
                if((castling_rights & CASTLE_WQ_MASK) == 0 &&
                    (attacked_nonking & (0x0Cull << 56)) == 0 &&
                    (all_pieces & (0x0Eull << 56)) == 0)
                {
                    *m++ = move::castle_q(king_sq);
                }
                if((castling_rights & CASTLE_WK_MASK) == 0 &&
                    ((attacked_nonking | all_pieces) & (0x60ull << 56)) == 0)
                {
                    *m++ = move::castle_k(king_sq);
                }
            }
            else
            {
                if((castling_rights & CASTLE_BQ_MASK) == 0 &&
                    (attacked_nonking & (0x0Cull << 0)) == 0 &&
                    (all_pieces & (0x0Eull << 0)) == 0)
                {
                    *m++ = move::castle_q(king_sq);
                }
                if((castling_rights & CASTLE_BK_MASK) == 0 &&
                    ((attacked_nonking | all_pieces) & (0x60ull << 0)) == 0)
                {
                    *m++ = move::castle_k(king_sq);
                }
            }
        }

        // pinned pawns
        {
            uint64_t pawns = p.bbs[c + PAWN] & pin_mask;
            while(pawns)
            {
                uint64_t pawn = pop_lsb_mask(pawns);
                int sq = lsb(pawn);
                uint64_t pf;
                uint64_t ray = lines[king_sq][sq];

                // forward moves (cannot promote when pinned)
                if(c == WHITE)
                {
                    pf = shift_n(pawn) & empty & ray;
                    if(pf)
                    {
                        *m++ = move(sq, sq - 8);
                        pf = shift_n(pf & RANK3) & empty;
                        if(pf) *m++ = move::pawn_dmove(sq, sq - 16);
                    }
                }
                else
                {
                    pf = shift_s(pawn) & empty & ray;
                    if(pf)
                    {
                        *m++ = move(sq, sq + 8);
                        pf = shift_s(pf & RANK6) & empty;
                        if(pf) *m++ = move::pawn_dmove(sq, sq + 16);
                    }
                }

                // captures (can promote)
                if(c == WHITE) pf = masks[sq].white_pawn_attacks;
                else pf = masks[sq].black_pawn_attacks;
                pf &= enemy_all & ray;
                // at most one capture can be generated for a pinned pawn
                if(pf)
                {
                    if(pawn & my_pro_pawns)
#if CH_COLOR_TEMPLATE
                        m = add_pawn_promotions<c>(m, move(sq, lsb(pf)));
#else
                        m = add_pawn_promotions(c, m, move(sq, lsb(pf)));
#endif
                    else
                        *m++ = move(sq, lsb(pf));
                }
            }
        }

        // non-pinned pawns
#if CH_COLOR_TEMPLATE
        m = generate_non_pinned_pawn_moves<c, false>(
            m, my_nonpro_pawns & ~pin_mask, empty, enemy_all);
        m = generate_non_pinned_pawn_moves<c, true>(
            m, my_pro_pawns & ~pin_mask, empty, enemy_all);
#else
        m = generate_non_pinned_pawn_moves<false>(c,
            m, my_nonpro_pawns & ~pin_mask, empty, enemy_all);
        m = generate_non_pinned_pawn_moves<true>(c,
            m, my_pro_pawns & ~pin_mask, empty, enemy_all);
#endif

#if CH_COLOR_TEMPLATE
        m = generate_en_passant_moves<c>(m, p, king_sq,
#else
        m = generate_en_passant_moves(c, m, p, king_sq,
#endif
            all_pieces, enemy_orth_sliders, enemy_diag_sliders);

        // non-pinned knights (pinned knights cannot move)
        {
            uint64_t knights = p.bbs[c + KNIGHT] & ~pin_mask;
            while(knights)
            {
                int i = pop_lsb(knights);
                uint64_t a = masks[i].knight_attacks & capture;
                move mv = move::from(i);
                while(a)
                    *m++ = (mv + move::to(pop_lsb(a)));
            }
        }
        
        // pinned diagonal sliders
        {
            uint64_t pcs = my_diag_sliders & pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb(pcs);
                move mv = move::from(i);
                bb_from ^= pcs;
                uint64_t a = hq_bishop_attacks(i, all_pieces) & capture;
                a &= lines[king_sq][i];
                while(a) *m++ = (mv + move::to(pop_lsb(a)));
            }
        }

        // non-pinned diagonal sliders
        {
            uint64_t pcs = my_diag_sliders & ~pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb(pcs);
                move mv = move::from(i);
                bb_from ^= pcs;
                uint64_t a = hq_bishop_attacks(i, all_pieces) & capture;
                while(a) *m++ = (mv + move::to(pop_lsb(a)));
            }
        }

        // pinned orthogonal sliders
        {
            uint64_t pcs = my_orth_sliders & pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb(pcs);
                move mv = move::from(i);
                bb_from ^= pcs;
                uint64_t a = hq_rook_attacks(i, all_pieces) & capture;
                a &= lines[king_sq][i];
                while(a) *m++ = (mv + move::to(pop_lsb(a)));
            }
        }

        // non-pinned orthogonal sliders
        {
            uint64_t pcs = my_orth_sliders & ~pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb(pcs);
                move mv = move::from(i);
                bb_from ^= pcs;
                uint64_t a = hq_rook_attacks(i, all_pieces) & capture;
                while(a) *m++ = (mv + move::to(pop_lsb(a)));
            }
        }

        return int(m - mvs);

#endif
    }
};

#endif

}

#undef CH_PUSH_MOVE
