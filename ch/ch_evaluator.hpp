#pragma once

#include "ch_internal.hpp"

#include "ch_params.hpp"

namespace ch
{

// piece values in centipawns
static constexpr int16_t const PIECE_VALUES[13] =
{
    100, 100,
    325, 325,
    335, 335,
    500, 500,
    975, 975,
    0, 0,
    0,
};

static_assert(PIECE_VALUES[WHITE + PAWN  ] == PIECE_VALUES[BLACK + PAWN  ], "");
static_assert(PIECE_VALUES[WHITE + KNIGHT] == PIECE_VALUES[BLACK + KNIGHT], "");
static_assert(PIECE_VALUES[WHITE + BISHOP] == PIECE_VALUES[BLACK + BISHOP], "");
static_assert(PIECE_VALUES[WHITE + ROOK  ] == PIECE_VALUES[BLACK + ROOK  ], "");
static_assert(PIECE_VALUES[WHITE + QUEEN ] == PIECE_VALUES[BLACK + QUEEN ], "");

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

static constexpr int const INIT_TABLE_ZERO[32] =
{
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
};

int8_t piece_tables[2][13][64];

template<bool flipped>
static void evaluator_init_table(int8_t* dst, int const* src)
{
    for(int r = 0; r < 8; ++r)
    {
        int nr = flipped ? 7 - r : r;
        for(int c = 0; c < 8; ++c)
        {
            int sq = nr * 4 + (c >= 4 ? 7 - c : c);
            dst[r * 8 + c] = int8_t(flipped ? -src[sq] : src[sq]);
        }
    }
}

static void init_evaluator()
{
    // middle game
    evaluator_init_table<false>(piece_tables[0][WHITE + PAWN  ], INIT_TABLE_PAWN_MG);
    evaluator_init_table<false>(piece_tables[0][WHITE + KNIGHT], INIT_TABLE_KNIGHT_MG);
    evaluator_init_table<false>(piece_tables[0][WHITE + BISHOP], INIT_TABLE_BISHOP_MG);
    evaluator_init_table<false>(piece_tables[0][WHITE + ROOK  ], INIT_TABLE_ROOK_MG);
    evaluator_init_table<false>(piece_tables[0][WHITE + QUEEN ], INIT_TABLE_QUEEN_MG);
    evaluator_init_table<false>(piece_tables[0][WHITE + KING  ], INIT_TABLE_KING_MG);
    evaluator_init_table<true >(piece_tables[0][BLACK + PAWN  ], INIT_TABLE_PAWN_MG);
    evaluator_init_table<true >(piece_tables[0][BLACK + KNIGHT], INIT_TABLE_KNIGHT_MG);
    evaluator_init_table<true >(piece_tables[0][BLACK + BISHOP], INIT_TABLE_BISHOP_MG);
    evaluator_init_table<true >(piece_tables[0][BLACK + ROOK  ], INIT_TABLE_ROOK_MG);
    evaluator_init_table<true >(piece_tables[0][BLACK + QUEEN ], INIT_TABLE_QUEEN_MG);
    evaluator_init_table<true >(piece_tables[0][BLACK + KING  ], INIT_TABLE_KING_MG);
    evaluator_init_table<false>(piece_tables[0][EMPTY         ], INIT_TABLE_ZERO);

    evaluator_init_table<false>(piece_tables[1][WHITE + PAWN  ], INIT_TABLE_PAWN_EG);
    evaluator_init_table<false>(piece_tables[1][WHITE + KNIGHT], INIT_TABLE_KNIGHT_EG);
    evaluator_init_table<false>(piece_tables[1][WHITE + BISHOP], INIT_TABLE_BISHOP_EG);
    evaluator_init_table<false>(piece_tables[1][WHITE + ROOK  ], INIT_TABLE_ROOK_EG);
    evaluator_init_table<false>(piece_tables[1][WHITE + QUEEN ], INIT_TABLE_QUEEN_EG);
    evaluator_init_table<false>(piece_tables[1][WHITE + KING  ], INIT_TABLE_KING_EG);
    evaluator_init_table<true >(piece_tables[1][BLACK + PAWN  ], INIT_TABLE_PAWN_EG);
    evaluator_init_table<true >(piece_tables[1][BLACK + KNIGHT], INIT_TABLE_KNIGHT_EG);
    evaluator_init_table<true >(piece_tables[1][BLACK + BISHOP], INIT_TABLE_BISHOP_EG);
    evaluator_init_table<true >(piece_tables[1][BLACK + ROOK  ], INIT_TABLE_ROOK_EG);
    evaluator_init_table<true >(piece_tables[1][BLACK + QUEEN ], INIT_TABLE_QUEEN_EG);
    evaluator_init_table<true >(piece_tables[1][BLACK + KING  ], INIT_TABLE_KING_EG);
    evaluator_init_table<false>(piece_tables[1][EMPTY         ], INIT_TABLE_ZERO);
}


}
