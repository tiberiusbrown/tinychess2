#pragma once

#include <stdint.h>

#include "ch_internal.hpp"

namespace ch
{

struct move
{
    uint32_t d;
    move() = default;
    move(move const&) = default;
    constexpr move(uint32_t d_) : d(d_) {}
    constexpr move(int a, int b) : d((a << 8) + b) {}
    constexpr operator uint32_t() const { return d; }
    static constexpr move from(int sq) { return sq << 8; }
    static constexpr move to(int sq) { return sq; }
    static constexpr move pawn_dmove(int a, int b) { return move(a, b) + pawn_dmove(); }
    constexpr int from() const { return (d >> 8) & 0xFF; }
    constexpr int to() const { return d & 0xFF; }
    static constexpr move castle_q(int k) { return MOVE_CASTLEQ + move(k, k - 2); }
    static constexpr move castle_k(int k) { return MOVE_CASTLEK + move(k, k + 2); }
    static constexpr move en_passant() { return MOVE_EN_PASSANT; }
    static constexpr move en_passant(int cap) { return MOVE_EN_PASSANT + (cap << 16); }
    static constexpr move pawn_dmove() { return MOVE_PAWN_DMOVE; }
    static constexpr move pawn_promotion(int t) { return MOVE_PROMOTION + (t << 16); }
    constexpr bool is_special() const { return (d & 0xFF000000) != 0; }
    constexpr bool is_castleq() const { return (d & MOVE_CASTLEQ) != 0; }
    constexpr bool is_castlek() const { return (d & MOVE_CASTLEK) != 0; }
    constexpr bool is_en_passant() const { return (d & MOVE_EN_PASSANT) != 0; }
    constexpr bool is_pawn_dmove() const { return (d & MOVE_PAWN_DMOVE) != 0; }
    constexpr bool is_promotion() const { return (d & MOVE_PROMOTION) != 0; }
    char const* extended_algebraic() const;
private:
    static constexpr uint32_t const MOVE_PAWN_DMOVE = 0x80000000;
    static constexpr uint32_t const MOVE_EN_PASSANT = 0x40000000;
    static constexpr uint32_t const MOVE_CASTLEQ = 0x20000000;
    static constexpr uint32_t const MOVE_CASTLEK = 0x10000000;
    static constexpr uint32_t const MOVE_PROMOTION = 0x08000000;
};

}
