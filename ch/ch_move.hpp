#pragma once

#include <stdint.h>

#include <array>

#include "ch_internal.hpp"

// TODO: sort key broken on endiannesses that don't have MSB come last

namespace ch
{

struct move
{
    uint32_t d;
    move() = default;
    constexpr move(uint32_t d_) : d(d_) {}
    constexpr move(int a, int b) : d((a << 8) + b) {}
    constexpr operator uint32_t() const { return d; }
    move& operator+=(move const& m) { d += m.d; return *this; }
    constexpr bool operator<(move const& m) { return d < m.d; }
    constexpr uint8_t& sort_key() { return *((uint8_t*)&d + 3); }
    static constexpr move from(int sq) { return sq << 8; }
    static constexpr move to(int sq) { return sq; }
    static constexpr move pawn_dmove(int a, int b) { return move(a, b) + pawn_dmove(); }
    constexpr int from() const { return (d >> 8) & 0xFF; }
    constexpr int to() const { return d & 0xFF; }
    static constexpr move castle_q(int k) { return MOVE_CASTLEQ + move(k, k - 2); }
    static constexpr move castle_k(int k) { return MOVE_CASTLEK + move(k, k + 2); }
    static constexpr move en_passant() { return MOVE_EN_PASSANT; }
    constexpr int en_passant_sq() const { return ((d >> 8) & 0x38) | (d & 7); }
    static constexpr move pawn_dmove() { return MOVE_PAWN_DMOVE; }
    static constexpr move pawn_promotion(int t) { return MOVE_PROMOTION + ((t - 2) << 16); }
    constexpr int promotion_piece() const { return ((d >> 16) & 7) + 2; }
    constexpr bool is_special() const { return (d & 0x00FF0000) != 0; }
    constexpr bool is_castleq() const { return (d & MOVE_CASTLEQ) != 0; }
    constexpr bool is_castlek() const { return (d & MOVE_CASTLEK) != 0; }
    constexpr bool is_castle() const { return (d & (MOVE_CASTLEK | MOVE_CASTLEQ)) != 0; }
    constexpr bool is_en_passant() const { return (d & MOVE_EN_PASSANT) != 0; }
    constexpr bool is_pawn_dmove() const { return (d & MOVE_PAWN_DMOVE) != 0; }
    constexpr bool is_promotion() const { return (d & MOVE_PROMOTION) != 0; }
    char const* extended_algebraic() const;
    constexpr bool is_similar_to(move const& m) const { return (d & 0xffff) == (m.d & 0xffff); }


    CH_FORCEINLINE bool is_killer(
        std::array<move, CH_NUM_KILLERS> const& killers)
    {
        for(int i = 0; i < CH_NUM_KILLERS; ++i)
            if(is_similar_to(killers[i]))
                return true;
        return false;
    }


private:
    static constexpr uint32_t const MOVE_PAWN_DMOVE = 0x00800000;
    static constexpr uint32_t const MOVE_EN_PASSANT = 0x00400000;
    static constexpr uint32_t const MOVE_CASTLEQ    = 0x00200000;
    static constexpr uint32_t const MOVE_CASTLEK    = 0x00100000;
    static constexpr uint32_t const MOVE_PROMOTION  = 0x00080000;
};

static constexpr move const NULL_MOVE = 0;

char const* move::extended_algebraic() const
{
    static char buf[16];
    char* bp = buf;

    int a = from();
    int b = to();

    *bp++ = char('a' + (a % 8));
    *bp++ = char('8' - (a / 8));
    *bp++ = char('a' + (b % 8));
    *bp++ = char('8' - (b / 8));

    if(is_promotion())
    {
        switch((d >> 16) & 0xff)
        {
        case WHITE + KNIGHT: case BLACK + KNIGHT: *bp++ = 'n'; break;
        case WHITE + BISHOP: case BLACK + BISHOP: *bp++ = 'b'; break;
        case WHITE + ROOK  : case BLACK + ROOK  : *bp++ = 'r'; break;
        case WHITE + QUEEN : case BLACK + QUEEN : *bp++ = 'q'; break;
        }
    }

    *bp = '\0';
    return buf;
}

}
