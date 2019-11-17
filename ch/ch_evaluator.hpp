#pragma once

#include "ch_internal.hpp"

#include "ch_params.hpp"

namespace ch
{

#ifdef CH_TUNABLE
static int32_t PIECE_VALUES[13];
static int32_t PIECE_VALUES_WHITE[13];
static int32_t PIECE_VALUES_ESTIMATE[13];
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
    for(int i = 10; i < 13; ++i)
        PIECE_VALUES_ESTIMATE[i] = (SC_MG(PIECE_VALUES[i]) + SC_EG(PIECE_VALUES[i])) / 2;
}
#else
// piece values in centipawns
static constexpr int32_t const PIECE_VALUES[13] =
{
    PIECE_VALUES_MAG[0], PIECE_VALUES_MAG[0],
    PIECE_VALUES_MAG[1], PIECE_VALUES_MAG[1],
    PIECE_VALUES_MAG[2], PIECE_VALUES_MAG[2],
    PIECE_VALUES_MAG[3], PIECE_VALUES_MAG[3],
    PIECE_VALUES_MAG[4], PIECE_VALUES_MAG[4],
    0, 0,
    0,
};
static constexpr int32_t const PIECE_VALUES_ESTIMATE[13] =
{
    (SC_MG(PIECE_VALUES[ 0]) + SC_EG(PIECE_VALUES[ 0])) / 2,
    (SC_MG(PIECE_VALUES[ 1]) + SC_EG(PIECE_VALUES[ 1])) / 2,
    (SC_MG(PIECE_VALUES[ 2]) + SC_EG(PIECE_VALUES[ 2])) / 2,
    (SC_MG(PIECE_VALUES[ 3]) + SC_EG(PIECE_VALUES[ 3])) / 2,
    (SC_MG(PIECE_VALUES[ 4]) + SC_EG(PIECE_VALUES[ 4])) / 2,
    (SC_MG(PIECE_VALUES[ 5]) + SC_EG(PIECE_VALUES[ 5])) / 2,
    (SC_MG(PIECE_VALUES[ 6]) + SC_EG(PIECE_VALUES[ 6])) / 2,
    (SC_MG(PIECE_VALUES[ 7]) + SC_EG(PIECE_VALUES[ 7])) / 2,
    (SC_MG(PIECE_VALUES[ 8]) + SC_EG(PIECE_VALUES[ 8])) / 2,
    (SC_MG(PIECE_VALUES[ 9]) + SC_EG(PIECE_VALUES[ 9])) / 2,
    (SC_MG(PIECE_VALUES[10]) + SC_EG(PIECE_VALUES[10])) / 2,
    (SC_MG(PIECE_VALUES[11]) + SC_EG(PIECE_VALUES[11])) / 2,
    (SC_MG(PIECE_VALUES[12]) + SC_EG(PIECE_VALUES[12])) / 2,
};
#endif

static constexpr int32_t const INIT_TABLE_ZERO[32] =
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

int32_t piece_tables[13][64];

template<bool flipped>
static void evaluator_init_table(int32_t dst[64], int32_t const src[32], int32_t pval)
{
    for(int r = 0; r < 8; ++r)
    {
        int nr = flipped ? 7 - r : r;
        for(int c = 0; c < 8; ++c)
        {
            int sq = nr * 4 + (c >= 4 ? 7 - c : c);
            int32_t srcsq = src[sq] + pval;
            dst[r * 8 + c] = (flipped ? -srcsq : srcsq);
        }
    }
}

static void init_evaluator()
{
#ifdef CH_TUNABLE
    evaluator_init_piece_values();
#endif

    // middle game
    evaluator_init_table<false>(piece_tables[WHITE + PAWN  ], INIT_TABLE_PAWN  , PIECE_VALUES_MAG[0]);
    evaluator_init_table<false>(piece_tables[WHITE + KNIGHT], INIT_TABLE_KNIGHT, PIECE_VALUES_MAG[1]);
    evaluator_init_table<false>(piece_tables[WHITE + BISHOP], INIT_TABLE_BISHOP, PIECE_VALUES_MAG[2]);
    evaluator_init_table<false>(piece_tables[WHITE + ROOK  ], INIT_TABLE_ROOK  , PIECE_VALUES_MAG[3]);
    evaluator_init_table<false>(piece_tables[WHITE + QUEEN ], INIT_TABLE_QUEEN , PIECE_VALUES_MAG[4]);
    evaluator_init_table<false>(piece_tables[WHITE + KING  ], INIT_TABLE_KING  , 0);
    evaluator_init_table<true >(piece_tables[BLACK + PAWN  ], INIT_TABLE_PAWN  , PIECE_VALUES_MAG[0]);
    evaluator_init_table<true >(piece_tables[BLACK + KNIGHT], INIT_TABLE_KNIGHT, PIECE_VALUES_MAG[1]);
    evaluator_init_table<true >(piece_tables[BLACK + BISHOP], INIT_TABLE_BISHOP, PIECE_VALUES_MAG[2]);
    evaluator_init_table<true >(piece_tables[BLACK + ROOK  ], INIT_TABLE_ROOK  , PIECE_VALUES_MAG[3]);
    evaluator_init_table<true >(piece_tables[BLACK + QUEEN ], INIT_TABLE_QUEEN , PIECE_VALUES_MAG[4]);
    evaluator_init_table<true >(piece_tables[BLACK + KING  ], INIT_TABLE_KING  , 0);
    evaluator_init_table<false>(piece_tables[EMPTY         ], INIT_TABLE_ZERO  , 0);
}


}
