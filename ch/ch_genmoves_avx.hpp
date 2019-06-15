#pragma once

#include "ch_internal.hpp"

#if CH_ENABLE_AVX

#include "ch_genmoves.hpp"
#include "ch_position.hpp"

#include <immintrin.h>

namespace ch
{

#if CH_COLOR_TEMPLATE
template<color c>
struct move_generator<c, ACCEL_AVX>
{
    static int generate(move* mvs, position const& p)
    {
#else
template<>
struct move_generator<ACCEL_AVX>
{
    static int generate(color c, move* mvs, position const& p)
    {
#endif
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
            uint64_t e;
            uint64_t const empty_nonking = empty ^ king;
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
            while(e) attacked_nonking |= masks[pop_lsb_avx(e)].knight_attacks;
            e = p.bbs[enemy_color + PAWN];
            if(enemy_color == WHITE)
                attacked_nonking |= shift_nw(e) | shift_ne(e);
            else
                attacked_nonking |= shift_sw(e) | shift_se(e);
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
                int sq = pop_lsb_avx(potential_pinners);
                uint64_t between = betweens[king_sq][sq] & all_pieces;
                if(between && !(between & (between - 1)))
                    pin_mask |= between;
            }
        }

        // generate legal king moves
        {
            uint64_t a = masks[king_sq].king_attacks;
            a &= capture & ~attacked_nonking;
            while(a) *m++ = move(king_sq, pop_lsb_avx(a));
        }

        if(attacked_nonking & king)
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
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_BISHOP
                (magic_bishop_attacks(king_sq, all_pieces) & enemy_diag_sliders) |
#else
                (hq_bishop_attacks_sse(king_sq, all_pieces) & enemy_diag_sliders) |
#endif
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_ROOK
                (magic_rook_attacks(king_sq, all_pieces) & enemy_orth_sliders);
#else
                (hq_rook_attacks(king_sq, all_pieces) & enemy_orth_sliders);
#endif
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
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_BISHOP
                        (magic_bishop_attacks(i, all_pieces) & my_diag_sliders) |
#else
                        (hq_bishop_attacks_sse(i, all_pieces) & my_diag_sliders) |
#endif
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_ROOK
                        (magic_rook_attacks(i, all_pieces) & my_orth_sliders);
#else
                        (hq_rook_attacks(i, all_pieces) & my_orth_sliders);
#endif
                    if(c == WHITE)
                        pcs |= (masks[i].black_pawn_attacks & my_nonpro_pawns);
                    else
                        pcs |= (masks[i].white_pawn_attacks & my_nonpro_pawns);
                    pcs &= ~pin_mask;
                    while(pcs) *m++ = mv + move::from(pop_lsb_avx(pcs));

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
                    int i = pop_lsb_avx(ray_and_threat);
                    move mv = move::to(i);
                    uint64_t pcs = (
                        (masks[i].knight_attacks & p.bbs[c + KNIGHT]) |

#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_BISHOP
                        (magic_bishop_attacks(i, all_pieces) & my_diag_sliders) |
#else
                        (hq_bishop_attacks_sse(i, all_pieces) & my_diag_sliders) |
#endif
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_ROOK
                        (magic_rook_attacks(i, all_pieces) & my_orth_sliders)
#else
                        (hq_rook_attacks(i, all_pieces) & my_orth_sliders)
#endif
                        ) & ~pin_mask;
                    while(pcs) *m++ = mv + move::from(pop_lsb_avx(pcs));
                }

                // generate non-pinned pawn forward moves that block threat
                uint64_t non_pinned_pawns = p.bbs[c + PAWN] & ~pin_mask;
                if(c == WHITE)
                {
                    uint64_t b = shift_n(non_pinned_pawns) & empty;
                    uint64_t t = b & ray;
                    while(t)
                    {
                        int i = pop_lsb_avx(t);
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
                        int i = pop_lsb_avx(t);
                        *m++ = move::pawn_dmove(i + 16, i);
                    }
                }
                else
                {
                    uint64_t b = shift_s(non_pinned_pawns) & empty;
                    uint64_t t = b & ray;
                    while(t)
                    {
                        int i = pop_lsb_avx(t);
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
                        int i = pop_lsb_avx(t);
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
                    while(t) *m++ = mv + move::from(pop_lsb_avx(t));
                    t = masks[i].black_pawn_attacks &
                        non_pinned_pawns & my_pro_pawns;
#if CH_COLOR_TEMPLATE
                    while(t) m = add_pawn_promotions<c>(m, mv + move::from(pop_lsb_avx(t)));
#else
                    while(t) m = add_pawn_promotions(c, m, mv + move::from(pop_lsb_avx(t)));
#endif
                }
                else
                {
                    int i = lsb(slider_threats);
                    move mv = move::to(i);
                    uint64_t t = masks[i].white_pawn_attacks &
                        non_pinned_pawns & my_nonpro_pawns;
                    while(t) *m++ = mv + move::from(pop_lsb_avx(t));
                    t = masks[i].white_pawn_attacks &
                        non_pinned_pawns & my_pro_pawns;
#if CH_COLOR_TEMPLATE
                    while(t) m = add_pawn_promotions<c>(m, mv + move::from(pop_lsb_avx(t)));
#else
                    while(t) m = add_pawn_promotions(c, m, mv + move::from(pop_lsb_avx(t)));
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
            uint64_t pinned_pawns = p.bbs[c + PAWN] & pin_mask;
            while(pinned_pawns)
            {
                uint64_t pawn = pop_lsb_mask_avx(pinned_pawns);
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
            m, nonpro_pawns & ~pin_mask, empty, enemy_all);
        m = generate_non_pinned_pawn_moves<true>(c,
            m, pro_pawns & ~pin_mask, empty, enemy_all);
#endif

#if CH_COLOR_TEMPLATE
        m = generate_en_passant_moves<c>(m, p, king_sq,
#else
        m = generate_en_passant_moves(c, m, p, king_sq,
#endif
            all_pieces, enemy_orth_sliders, enemy_diag_sliders);

        // non-pinned knights (pinned knights cannot move)
        {
            uint64_t unpinned_knights = p.bbs[c + KNIGHT] & ~pin_mask;
            while(unpinned_knights)
            {
                int i = pop_lsb_avx(unpinned_knights);
                uint64_t a = masks[i].knight_attacks & capture;
                move mv = move::from(i);
                while(a)
                    *m++ = (mv + move::to(pop_lsb_avx(a)));
            }
        }

        // pinned diagonal sliders
        {
            uint64_t pcs = my_diag_sliders & pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb_avx(pcs);
                bb_from ^= pcs;
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_BISHOP
                uint64_t a = magic_bishop_attacks(i, all_pieces) & capture;
#else
                uint64_t a = hq_bishop_attacks_sse(i, all_pieces) & capture;
#endif
                a &= lines[king_sq][i];
                move mv = move::from(i);
                while(a) *m++ = (mv + move::to(pop_lsb_avx(a)));
            }
        }

        // non-pinned diagonal sliders
        {
            uint64_t pcs = my_diag_sliders & ~pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb_avx(pcs);
                bb_from ^= pcs;
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_BISHOP
                uint64_t a = magic_bishop_attacks(i, all_pieces) & capture;
#else
                uint64_t a = hq_bishop_attacks_sse(i, all_pieces) & capture;
#endif
                move mv = move::from(i);
                while(a) *m++ = (mv + move::to(pop_lsb_avx(a)));
            }
        }

        // pinned orthogonal sliders
        {
            uint64_t pcs = my_orth_sliders & pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb_avx(pcs);
                bb_from ^= pcs;
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_ROOK
                uint64_t a = magic_rook_attacks(i, all_pieces) & capture;
#else
                uint64_t a = hq_rook_attacks(i, all_pieces) & capture;
#endif
                a &= lines[king_sq][i];
                move mv = move::from(i);
                while(a) *m++ = (mv + move::to(pop_lsb_avx(a)));
            }
        }

        // non-pinned orthogonal sliders
        {
            uint64_t pcs = my_orth_sliders & ~pin_mask;
            while(pcs)
            {
                uint64_t bb_from = pcs;
                int i = pop_lsb_avx(pcs);
                bb_from ^= pcs;
#if CH_ENABLE_MAGIC && CH_ENABLE_MAGIC_ROOK
                uint64_t a = magic_rook_attacks(i, all_pieces) & capture;
#else
                uint64_t a = hq_rook_attacks(i, all_pieces) & capture;
#endif
                move mv = move::from(i);
                while(a) *m++ = (mv + move::to(pop_lsb_avx(a)));
            }
        }

        return int(m - mvs);
    }
};

}

#endif
