#pragma once

#include "ch_internal.hpp"

namespace ch
{

struct position;

template<acceleration accel>
struct evaluator
{
    static int evaluate(position const& p, color c);
};

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

static constexpr int8_t const TABLE_KING_MIDDLEGAME[64] =
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

static constexpr int8_t const TABLE_KING_ENDGAME[64] =
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
    // piece values in centipawns
    static constexpr int16_t const PIECE_VALUES[10] =
    {
        100, 0,
        320, 0,
        330, 0,
        500, 0,
        900, 0,
    };

    evaluator_init_table(table_pawn, INIT_TABLE_PAWN, PIECE_VALUES[PAWN]);
    evaluator_init_table(table_knight, INIT_TABLE_KNIGHT, PIECE_VALUES[KNIGHT]);
    evaluator_init_table(table_bishop, INIT_TABLE_BISHOP, PIECE_VALUES[BISHOP]);
    evaluator_init_table(table_rook, INIT_TABLE_ROOK, PIECE_VALUES[ROOK]);
    evaluator_init_table(table_queen, INIT_TABLE_QUEEN, PIECE_VALUES[QUEEN]);
}


}
