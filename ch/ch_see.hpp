#pragma once

#include "ch_magic.hpp"
#include "ch_move.hpp"
#include "ch_position.hpp"

namespace ch
{

static constexpr int16_t const SEE_PIECE_VALUES[] =
{
     4,  4, // p
    13, 13, // n
    14, 14, // b
    20, 20, // r
    36, 36, // q
    256, 256, // k
     0,  0, // empty
};

#define CH_CHECK_SEE_A(pc_) do { \
    if(a) { pc = pc_; return lsb_mask<accel>(a); } \
    } while(0)

template<acceleration accel>
static CH_FORCEINLINE uint64_t see_least_valuable_attacker(
    position& p, uint64_t occ, color c, int sq, int& pc)
{
    uint64_t a;
    auto const& d = masks[sq];

    a = d.pawn_attacks[opposite(c)] & p.bbs[c + PAWN] & occ;
    CH_CHECK_SEE_A(PAWN);

    a = d.knight_attacks & p.bbs[c + KNIGHT] & occ;
    CH_CHECK_SEE_A(KNIGHT);

    uint64_t bishop_attacks = magic_bishop_attacks(sq, occ) & occ;
    a = bishop_attacks & p.bbs[c + BISHOP];
    CH_CHECK_SEE_A(BISHOP);

    uint64_t rook_attacks = magic_rook_attacks(sq, occ) & occ;
    a = rook_attacks & p.bbs[c + ROOK];
    CH_CHECK_SEE_A(ROOK);

    uint64_t queen_attacks = rook_attacks | bishop_attacks;
    a = queen_attacks & p.bbs[c + QUEEN];
    CH_CHECK_SEE_A(QUEEN);

    a = d.king_attacks & p.bbs[c + KING] & occ;
    if(a)
    {
        pc = KING;
        return a;
    }

    return 0ull;
}

#undef CH_CHECK_SEE_A

// TODO: template by color?
template<acceleration accel>
static int see(move mv, position& p)
{
    int to = mv.to();
    // only legal moves generated so king captures are always free
    if(p.pieces[mv.from()] >= KING)
        return SEE_PIECE_VALUES[p.pieces[to]];

    int gain[32];
    int d = 0;
    int fr = mv.from();
    int frompc = p.pieces[fr];
    uint64_t occ = p.bb_alls[WHITE] | p.bb_alls[BLACK];
    uint64_t fromset = 1ull << fr;
    color c = (frompc & 1) == 0 ? WHITE : BLACK;
    gain[0] = SEE_PIECE_VALUES[p.pieces[to]];
    if(mv.is_promotion())
        gain[0] += SEE_PIECE_VALUES[mv.promotion_piece()] - SEE_PIECE_VALUES[PAWN];

    do
    {
        ++d;
        gain[d] = SEE_PIECE_VALUES[frompc] - gain[d - 1];
        if(std::max(-gain[d - 1], gain[d]) < 0) break;

        // find least valuable attacker
        occ ^= fromset;
        c = opposite(c);
        fromset = see_least_valuable_attacker<accel>(p, occ, c, to, frompc);
    } while(fromset != 0);

    while(--d)
        gain[d - 1] = -std::max(-gain[d - 1], gain[d]);

    return gain[0];
}

}
