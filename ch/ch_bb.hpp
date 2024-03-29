#pragma once

#include <stdint.h>

#ifdef _MSC_VER
// for bswap on msvc
#include <stdlib.h>
#endif

namespace ch
{

static constexpr uint64_t const RANK1 = 0xFF00000000000000ull;
static constexpr uint64_t const RANK2 = 0x00FF000000000000ull;
static constexpr uint64_t const RANK3 = 0x0000FF0000000000ull;
static constexpr uint64_t const RANK4 = 0x000000FF00000000ull;
static constexpr uint64_t const RANK5 = 0x00000000FF000000ull;
static constexpr uint64_t const RANK6 = 0x0000000000FF0000ull;
static constexpr uint64_t const RANK7 = 0x000000000000FF00ull;
static constexpr uint64_t const RANK8 = 0x00000000000000FFull;

static constexpr uint64_t const OUTPOST_RANKS[2] =
{
    RANK4 | RANK5 | RANK6 | RANK7,
    RANK5 | RANK4 | RANK3 | RANK2,
};

static constexpr uint64_t const FILEA = (0x0101010101010101ull << 0);
static constexpr uint64_t const FILEB = (0x0101010101010101ull << 1);
static constexpr uint64_t const FILEC = (0x0101010101010101ull << 2);
static constexpr uint64_t const FILED = (0x0101010101010101ull << 3);
static constexpr uint64_t const FILEE = (0x0101010101010101ull << 4);
static constexpr uint64_t const FILEF = (0x0101010101010101ull << 5);
static constexpr uint64_t const FILEG = (0x0101010101010101ull << 6);
static constexpr uint64_t const FILEH = (0x0101010101010101ull << 7);

static constexpr uint64_t const WHITE_SQUARES = 0x5555555555555555ull;
static constexpr uint64_t const BLACK_SQUARES = ~WHITE_SQUARES;

static constexpr uint64_t file_n(int n) { return FILEA << n; }

static constexpr int file_index_of_sq(int sq) { return sq & 0x7; }
static constexpr uint64_t file_of_sq(int sq) { return file_n(file_index_of_sq(sq)); }

static constexpr uint64_t const ADJACENT_FILES[8] =
{
    FILEB,
    FILEA | FILEC,
    FILEB | FILED,
    FILEC | FILEE,
    FILED | FILEF,
    FILEE | FILEG,
    FILEF | FILEH,
    FILEG,
};

static constexpr uint64_t adjacent_files_of_sq(int sq)
{
    return ADJACENT_FILES[file_index_of_sq(sq)];
}

static CH_FORCEINLINE constexpr uint64_t shift_n(uint64_t bb)
{
    return bb >> 8;
}
static CH_FORCEINLINE constexpr uint64_t shift_s(uint64_t bb)
{
    return bb << 8;
}
static CH_FORCEINLINE constexpr uint64_t shift_w(uint64_t bb)
{
    return (bb >> 1) & ~FILEH;
}
static CH_FORCEINLINE constexpr uint64_t shift_e(uint64_t bb)
{
    return (bb << 1) & ~FILEA;
}
static CH_FORCEINLINE constexpr uint64_t shift_nw(uint64_t bb)
{
    return (bb >> 9) & ~FILEH;
}
static CH_FORCEINLINE constexpr uint64_t shift_ne(uint64_t bb)
{
    return (bb >> 7) & ~FILEA;
}
static CH_FORCEINLINE constexpr uint64_t shift_sw(uint64_t bb)
{
    return (bb << 7) & ~FILEH;
}
static CH_FORCEINLINE constexpr uint64_t shift_se(uint64_t bb)
{
    return (bb << 9) & ~FILEA;
}

static CH_FORCEINLINE constexpr int forward_sq(int sq, color c)
{
    return c == WHITE ? sq - 8 : sq + 8;
}

template<color c>
static CH_FORCEINLINE constexpr uint64_t shift_pawn_attack(uint64_t bb);
template<>
CH_FORCEINLINE constexpr uint64_t shift_pawn_attack<WHITE>(uint64_t bb)
{
    return shift_nw(bb) | shift_ne(bb);
}
template<>
CH_FORCEINLINE constexpr uint64_t shift_pawn_attack<BLACK>(uint64_t bb)
{
    return shift_sw(bb) | shift_se(bb);
}
static CH_FORCEINLINE constexpr uint64_t shift_pawn_attack(color c, uint64_t bb)
{
    return c == WHITE ? shift_pawn_attack<WHITE>(bb) : shift_pawn_attack<BLACK>(bb);
}

template<color c>
static CH_FORCEINLINE constexpr uint64_t shift_forward(uint64_t bb);
template<>
CH_FORCEINLINE constexpr uint64_t shift_forward<WHITE>(uint64_t bb)
{
    return shift_n(bb);
}
template<>
CH_FORCEINLINE constexpr uint64_t shift_forward<BLACK>(uint64_t bb)
{
    return shift_s(bb);
}
static CH_FORCEINLINE constexpr uint64_t shift_forward(color c, uint64_t bb)
{
    return c == WHITE ? shift_forward<WHITE>(bb) : shift_forward<BLACK>(bb);
}

static CH_FORCEINLINE uint64_t slide_fill_n(uint64_t bb, uint64_t empty)
{
    bb |= empty & (bb >> 8);
    empty &= (empty >> 8);
    bb |= empty & (bb >> 16);
    empty &= (empty >> 16);
    bb |= empty & (bb >> 32);
    return bb;
}
static CH_FORCEINLINE uint64_t slide_fill_s(uint64_t bb, uint64_t empty)
{
    bb |= empty & (bb << 8);
    empty &= (empty << 8);
    bb |= empty & (bb << 16);
    empty &= (empty << 16);
    bb |= empty & (bb << 32);
    return bb;
}
static CH_FORCEINLINE uint64_t slide_fill_w(uint64_t bb, uint64_t empty)
{
    empty &= ~FILEH;
    bb |= empty & (bb >> 1);
    empty &= (empty >> 1);
    bb |= empty & (bb >> 2);
    empty &= (empty >> 2);
    bb |= empty & (bb >> 4);
    return bb;
}
static CH_FORCEINLINE uint64_t slide_fill_e(uint64_t bb, uint64_t empty)
{
    empty &= ~FILEA;
    bb |= empty & (bb << 1);
    empty &= (empty << 1);
    bb |= empty & (bb << 2);
    empty &= (empty << 2);
    bb |= empty & (bb << 4);
    return bb;
}
static CH_FORCEINLINE uint64_t slide_fill_nw(uint64_t bb, uint64_t empty)
{
    empty &= ~FILEH;
    bb |= empty & (bb >> 9);
    empty &= (empty >> 9);
    bb |= empty & (bb >> 18);
    empty &= (empty >> 18);
    bb |= empty & (bb >> 36);
    return bb;
}
static CH_FORCEINLINE uint64_t slide_fill_ne(uint64_t bb, uint64_t empty)
{
    empty &= ~FILEA;
    bb |= empty & (bb >> 7);
    empty &= (empty >> 7);
    bb |= empty & (bb >> 14);
    empty &= (empty >> 14);
    bb |= empty & (bb >> 28);
    return bb;
}
static CH_FORCEINLINE uint64_t slide_fill_sw(uint64_t bb, uint64_t empty)
{
    empty &= ~FILEH;
    bb |= empty & (bb << 7);
    empty &= (empty << 7);
    bb |= empty & (bb << 14);
    empty &= (empty << 14);
    bb |= empty & (bb << 28);
    return bb;
}
static CH_FORCEINLINE uint64_t slide_fill_se(uint64_t bb, uint64_t empty)
{
    empty &= ~FILEA;
    bb |= empty & (bb << 9);
    empty &= (empty << 9);
    bb |= empty & (bb << 18);
    empty &= (empty << 18);
    bb |= empty & (bb << 36);
    return bb;
}

template<color c>
static CH_FORCEINLINE uint64_t slide_fill_forward(uint64_t bb, uint64_t empty)
{
    return c == WHITE ? slide_fill_n(bb, empty) : slide_fill_s(bb, empty);
}
static CH_FORCEINLINE uint64_t slide_fill_forward(color c, uint64_t bb, uint64_t empty)
{
    return c == WHITE ? slide_fill_n(bb, empty) : slide_fill_s(bb, empty);
}

static CH_FORCEINLINE uint64_t slide_attack_n(uint64_t bb, uint64_t empty)
{
    return shift_n(slide_fill_n(bb, empty));
}
static CH_FORCEINLINE uint64_t slide_attack_s(uint64_t bb, uint64_t empty)
{
    return shift_s(slide_fill_s(bb, empty));
}
static CH_FORCEINLINE uint64_t slide_attack_w(uint64_t bb, uint64_t empty)
{
    return shift_w(slide_fill_w(bb, empty));
}
static CH_FORCEINLINE uint64_t slide_attack_e(uint64_t bb, uint64_t empty)
{
    return shift_e(slide_fill_e(bb, empty));
}
static CH_FORCEINLINE uint64_t slide_attack_nw(uint64_t bb, uint64_t empty)
{
    return shift_nw(slide_fill_nw(bb, empty));
}
static CH_FORCEINLINE uint64_t slide_attack_ne(uint64_t bb, uint64_t empty)
{
    return shift_ne(slide_fill_ne(bb, empty));
}
static CH_FORCEINLINE uint64_t slide_attack_sw(uint64_t bb, uint64_t empty)
{
    return shift_sw(slide_fill_sw(bb, empty));
}
static CH_FORCEINLINE uint64_t slide_attack_se(uint64_t bb, uint64_t empty)
{
    return shift_se(slide_fill_se(bb, empty));
}

static CH_FORCEINLINE uint64_t bswap(uint64_t x)
{
#if 1
#ifdef _MSC_VER
    return _byteswap_uint64(x);
#else
    return __builtin_bswap64(x);
#endif
#else
#ifdef _MSC_VER
    return
        ((x << 56)) |
        ((x << 40) & 0x00FF000000000000ull) |
        ((x << 24) & 0x0000FF0000000000ull) |
        ((x <<  8) & 0x000000FF00000000ull) |
        ((x >>  8) & 0x00000000FF000000ull) |
        ((x >> 24) & 0x0000000000FF0000ull) |
        ((x >> 40) & 0x000000000000FF00ull) |
        ((x >> 56));
#else
    return __builtin_bswap64(x);
#endif
#endif
}

}
