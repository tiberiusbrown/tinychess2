#pragma once

#include "ch_internal.hpp"

#include "ch_params.hpp"

namespace ch
{

#ifdef CH_TUNABLE
static int16_t PIECE_VALUES[13];
static int16_t PIECE_VALUES_WHITE[13];
static void evaluator_init_piece_values()
{
    for(int i = 0; i < 5; ++i)
    {
        PIECE_VALUES[i * 2 + 0] = (int16_t)PIECE_VALUES_MAG[i];
        PIECE_VALUES[i * 2 + 1] = (int16_t)PIECE_VALUES_MAG[i];
        PIECE_VALUES_WHITE[i * 2 + 1] = (int16_t)PIECE_VALUES_MAG[i];
        PIECE_VALUES_WHITE[i * 2 + 1] = -(int16_t)PIECE_VALUES_MAG[i];
    }
    for(int i = 10; i < 13; ++i)
        PIECE_VALUES[i] = PIECE_VALUES_WHITE[i] = 0;
}
#else
// piece values in centipawns
static constexpr int16_t const PIECE_VALUES[13] =
{
    (int16_t)PIECE_VALUES_MAG[0], (int16_t)PIECE_VALUES_MAG[0],
    (int16_t)PIECE_VALUES_MAG[1], (int16_t)PIECE_VALUES_MAG[1],
    (int16_t)PIECE_VALUES_MAG[2], (int16_t)PIECE_VALUES_MAG[2],
    (int16_t)PIECE_VALUES_MAG[3], (int16_t)PIECE_VALUES_MAG[3],
    (int16_t)PIECE_VALUES_MAG[4], (int16_t)PIECE_VALUES_MAG[4],
    0, 0,
    0,
};
// piece values from white's perspective
static constexpr int16_t const PIECE_VALUES_WHITE[13] =
{
    (int16_t)PIECE_VALUES_MAG[0], -(int16_t)PIECE_VALUES_MAG[0],
    (int16_t)PIECE_VALUES_MAG[1], -(int16_t)PIECE_VALUES_MAG[1],
    (int16_t)PIECE_VALUES_MAG[2], -(int16_t)PIECE_VALUES_MAG[2],
    (int16_t)PIECE_VALUES_MAG[3], -(int16_t)PIECE_VALUES_MAG[3],
    (int16_t)PIECE_VALUES_MAG[4], -(int16_t)PIECE_VALUES_MAG[4],
    0, 0,
    0,
};
#endif

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

int16_t piece_tables[2][13][64];

template<bool flipped>
static void evaluator_init_table(int16_t* dst, int const* src)
{
    for(int r = 0; r < 8; ++r)
    {
        int nr = flipped ? 7 - r : r;
        for(int c = 0; c < 8; ++c)
        {
            int sq = nr * 4 + (c >= 4 ? 7 - c : c);
            dst[r * 8 + c] = int16_t(flipped ? -src[sq] : src[sq]);
        }
    }
}

static void init_evaluator()
{
#ifdef CH_TUNABLE
    evaluator_init_piece_values();
#endif

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
