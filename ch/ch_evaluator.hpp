#pragma once

#include "ch_internal.hpp"

namespace ch
{

struct position;

static constexpr int const BISHOP_MOBILITY_BONUS = 3;
static constexpr int const ROOK_MOBILITY_BONUS = 1;

static constexpr int const PAWN_PROTECTOR_BONUS = 0;

static constexpr int const PASSED_PAWN_PENALTIES[8] =
{
    4, 1, 2, 2, 2, 2, 1, 4,
};

// piece values in centipawns
static constexpr int16_t const PIECE_VALUES[13] =
{
    100, 100,
    320, 320,
    330, 330,
    500, 500,
    900, 900,
    0, 0,
    0,
};
static_assert(PIECE_VALUES[WHITE + PAWN] == PIECE_VALUES[BLACK + PAWN], "");
static_assert(PIECE_VALUES[WHITE + KNIGHT] == PIECE_VALUES[BLACK + KNIGHT], "");
static_assert(PIECE_VALUES[WHITE + BISHOP] == PIECE_VALUES[BLACK + BISHOP], "");
static_assert(PIECE_VALUES[WHITE + ROOK] == PIECE_VALUES[BLACK + ROOK], "");
static_assert(PIECE_VALUES[WHITE + QUEEN] == PIECE_VALUES[BLACK + QUEEN], "");

static constexpr int8_t const INIT_TABLE_PAWN[64] =
{
     0,  0,   0,   0,   0,   0,  0,  0,
    50, 50,  50,  50,  50,  50, 50, 50,
    10, 10,  20,  30,  30,  20, 10, 10,
     5,  5,  10,  25,  25,  10,  5,  5,
     0,  0,   0,  20,  20,   0,  0,  0,
     5, -5, -10,   0,   0, -10, -5,  5,
     5, 10,  10, -20, -20,  10, 10,  5,
     0,  0,   0,   0,   0,   0,  0,  0,
};

static constexpr int8_t const INIT_TABLE_KNIGHT[64] =
{
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50,
};

static constexpr int8_t const INIT_TABLE_BISHOP[64] =
{
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20,
};

static constexpr int8_t const INIT_TABLE_ROOK[64] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0,
};

static constexpr int8_t const INIT_TABLE_QUEEN[64] =
{
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10,   0,   0,  0,  0,   0,   0, -10,
    -10,   0,   5,  5,  5,   5,   0, -10,
     -5,   0,   5,  5,  5,   5,   0,  -5,
      0,   0,   5,  5,  5,   5,   0,  -5,
    -10,   5,   5,  5,  5,   5,   0, -10,
    -10,   0,   5,  0,  0,   0,   0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20,
};

static constexpr int8_t const TABLE_KING_MG[64] =
{
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20,
};

static constexpr int8_t const TABLE_KING_EG[64] =
{
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10,   0,   0, -10, -20, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -30,   0,   0,   0,   0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50,
};

int16_t table_pawn   [64];
int16_t table_knight [64];
int16_t table_bishop [64];
int16_t table_rook   [64];
int16_t table_queen  [64];

static void evaluator_init_table(
    int16_t* dst, int8_t const* src, int16_t val)
{
    for(int sq = 0; sq < 64; ++sq)
        dst[sq] = src[sq] + val;
}

static void init_evaluator()
{

    evaluator_init_table(table_pawn, INIT_TABLE_PAWN, PIECE_VALUES[PAWN]);
    evaluator_init_table(table_knight, INIT_TABLE_KNIGHT, PIECE_VALUES[KNIGHT]);
    evaluator_init_table(table_bishop, INIT_TABLE_BISHOP, PIECE_VALUES[BISHOP]);
    evaluator_init_table(table_rook, INIT_TABLE_ROOK, PIECE_VALUES[ROOK]);
    evaluator_init_table(table_queen, INIT_TABLE_QUEEN, PIECE_VALUES[QUEEN]);
}


}
