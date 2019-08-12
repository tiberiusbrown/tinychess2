#pragma once

#include "ch_internal.hpp"

namespace ch
{

struct position;

static constexpr int const BISHOP_MOBILITY_BONUS = 3;
static constexpr int const ROOK_MOBILITY_BONUS = 1;

static constexpr int const PAWN_PROTECTOR_BONUS = 4;

static constexpr int const DOUBLED_PAWN_PENALTIES[8] =
{
    10, 2, 5, 5, 5, 5, 2, 10,
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

// piece values from white's perspective
static constexpr int16_t const PIECE_VALUES_WHITE[13] =
{
    PIECE_VALUES[0], -PIECE_VALUES[0],
    PIECE_VALUES[2], -PIECE_VALUES[2],
    PIECE_VALUES[4], -PIECE_VALUES[4],
    PIECE_VALUES[6], -PIECE_VALUES[6],
    PIECE_VALUES[8], -PIECE_VALUES[8],
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

static constexpr int8_t const INIT_TABLE_PAWN_EG[64] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
    90, 90, 90, 90, 90, 90, 90, 90,
    50, 50, 50, 50, 50, 50, 50, 50,
    20, 20, 20, 20, 20, 20, 20, 20,
     5,  5,  5,  5,  5,  5,  5,  5,
     0,  0,  0,  0,  0,  0,  0,  0,
    -5, -5, -5, -5, -5, -5, -5, -5,
     0,  0,  0,  0,  0,  0,  0,  0,
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

static constexpr int8_t const TABLE_ZERO[64] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int8_t piece_tables[2][13][64];

template<bool flipped>
static void evaluator_init_table(int8_t* dst, int8_t const* src)
{
    for(int sq = 0; sq < 64; ++sq)
    {
        if(flipped)
            dst[sq] = -src[sq ^ 56];
        else
            dst[sq] = src[sq];
    }
}

static void init_evaluator()
{
    // middle game
    evaluator_init_table<false>(piece_tables[0][WHITE + PAWN  ], INIT_TABLE_PAWN  );
    evaluator_init_table<false>(piece_tables[0][WHITE + KNIGHT], INIT_TABLE_KNIGHT);
    evaluator_init_table<false>(piece_tables[0][WHITE + BISHOP], INIT_TABLE_BISHOP);
    evaluator_init_table<false>(piece_tables[0][WHITE + ROOK  ], INIT_TABLE_ROOK  );
    evaluator_init_table<false>(piece_tables[0][WHITE + QUEEN ], INIT_TABLE_QUEEN );
    evaluator_init_table<false>(piece_tables[0][WHITE + KING  ], TABLE_KING_MG    );
    evaluator_init_table<true >(piece_tables[0][BLACK + PAWN  ], INIT_TABLE_PAWN  );
    evaluator_init_table<true >(piece_tables[0][BLACK + KNIGHT], INIT_TABLE_KNIGHT);
    evaluator_init_table<true >(piece_tables[0][BLACK + BISHOP], INIT_TABLE_BISHOP);
    evaluator_init_table<true >(piece_tables[0][BLACK + ROOK  ], INIT_TABLE_ROOK  );
    evaluator_init_table<true >(piece_tables[0][BLACK + QUEEN ], INIT_TABLE_QUEEN );
    evaluator_init_table<true >(piece_tables[0][BLACK + KING  ], TABLE_KING_MG    );
    evaluator_init_table<false>(piece_tables[0][EMPTY         ], TABLE_ZERO       );

    evaluator_init_table<false>(piece_tables[1][WHITE + PAWN  ], INIT_TABLE_PAWN_EG);
    evaluator_init_table<false>(piece_tables[1][WHITE + KNIGHT], TABLE_ZERO       );
    evaluator_init_table<false>(piece_tables[1][WHITE + BISHOP], TABLE_ZERO       );
    evaluator_init_table<false>(piece_tables[1][WHITE + ROOK  ], TABLE_ZERO       );
    evaluator_init_table<false>(piece_tables[1][WHITE + QUEEN ], TABLE_ZERO       );
    evaluator_init_table<false>(piece_tables[1][WHITE + KING  ], TABLE_KING_EG    );
    evaluator_init_table<true >(piece_tables[1][BLACK + PAWN  ], INIT_TABLE_PAWN_EG);
    evaluator_init_table<true >(piece_tables[1][BLACK + KNIGHT], TABLE_ZERO       );
    evaluator_init_table<true >(piece_tables[1][BLACK + BISHOP], TABLE_ZERO       );
    evaluator_init_table<true >(piece_tables[1][BLACK + ROOK  ], TABLE_ZERO       );
    evaluator_init_table<true >(piece_tables[1][BLACK + QUEEN ], TABLE_ZERO       );
    evaluator_init_table<true >(piece_tables[1][BLACK + KING  ], TABLE_KING_EG    );
    evaluator_init_table<false>(piece_tables[1][EMPTY         ], TABLE_ZERO       );
}


}
