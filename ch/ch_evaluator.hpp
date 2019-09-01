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

//static constexpr int const HALF_OPEN_FILE = 8;
//
//static constexpr int const PAWN_PROTECT_ANY = 4;
//static constexpr int const PAWN_PROTECT_PAWN = 4;
//static constexpr int const PAWN_THREATEN_KNIGHT = 20;
//static constexpr int const PAWN_THREATEN_BISHOP = 15;
//static constexpr int const PAWN_THREATEN_ROOK = 40;
//static constexpr int const PAWN_THREATEN_QUEEN = 25;
//static constexpr int const PASSED_PAWN_MG[8] =
//{
//    0, 0, 4, 8, 16, 24, 32, 0,
//};
//static constexpr int const PASSED_PAWN_EG[8] =
//{
//    0, 4, 16, 32, 64, 128, 196, 0,
//};
//static constexpr int const PASSED_PAWN_FREE_EG[8] =
//{
//    0, 8, 32, 64, 128, 228, 324, 0,
//};
//static constexpr int const PASSED_PAWN_KING_ESCORT = 10;


//static constexpr int const KNIGHT_PAWN_BONUS_MG = 4;
//static constexpr int const KNIGHT_PAWN_BONUS_EG = 2;
//static constexpr int const KNIGHT_MOBILITY_BONUS_MG = 8;
//static constexpr int const KNIGHT_MOBILITY_BONUS_EG = 3;
//static constexpr int const KNIGHT_THREATEN_BISHOP = 0;
//static constexpr int const KNIGHT_THREATEN_ROOK = 0;
//static constexpr int const KNIGHT_THREATEN_QUEEN = 0;
//static constexpr int const KNIGHT_THREATEN_KING = 0;
//static constexpr int const KNIGHT_OUTPOST_HALF_OPEN_FILE = 4;
//static constexpr int const KNIGHT_OUTPOST_OPEN_FILE = 8;

//static constexpr int const BISHOP_MOBILITY_BONUS_MG = 5;
//static constexpr int const BISHOP_MOBILITY_BONUS_EG = 3;
//static constexpr int const BISHOP_THREATEN_ROOK = 15;
//static constexpr int const BISHOP_THREATEN_QUEEN = 30;
//static constexpr int const BISHOP_THREATEN_KING = 0;
//
//static constexpr int const ROOK_MOBILITY_BONUS_MG = 3;
//static constexpr int const ROOK_MOBILITY_BONUS_EG = 6;
//static constexpr int const ROOK_THREATEN_QUEEN = 20;
//static constexpr int const ROOK_THREATEN_KING = 0;
//static constexpr int const ROOK_ON_OPEN_FILE = 40;
//
//static constexpr int const QUEEN_ON_OPEN_FILE = 40;

//static constexpr int const KING_DEFENDERS_MG[12] =
//{
//    -32, -8, 1, 8, 16, 28, 32, 16, 12, 12, 12, 12,
//};

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


//static constexpr int8_t const INIT_TABLE_PAWN_MG[64] =
//{
//     0,  0,   0,   0,   0,   0,  0,  0,
//    50, 50,  50,  50,  50,  50, 50, 50,
//    10, 10,  20,  30,  30,  20, 10, 10,
//     5,  5,  10,  25,  25,  10,  5,  5,
//     0,  0,   0,  20,  20,   0,  0,  0,
//     5, -5, -10,   0,   0, -10, -5,  5,
//     5, 10,  10, -20, -20,  10, 10,  5,
//     0,  0,   0,   0,   0,   0,  0,  0,
//};
//
//static constexpr int8_t const INIT_TABLE_PAWN_EG[64] =
//{
//     0,  0,  0,  0,  0,  0,  0,  0,
//    90, 90, 90, 90, 90, 90, 90, 90,
//    50, 50, 50, 50, 50, 50, 50, 50,
//    20, 20, 20, 20, 20, 20, 20, 20,
//     5,  5,  5,  5,  5,  5,  5,  5,
//     0,  0,  0,  0,  0,  0,  0,  0,
//    -5, -5, -5, -5, -5, -5, -5, -5,
//     0,  0,  0,  0,  0,  0,  0,  0,
//};
//
//static constexpr int8_t const INIT_TABLE_KNIGHT_MG[64] =
//{
//    -50, -40, -30, -30, -30, -30, -40, -50,
//    -40, -20,   0,   0,   0,   0, -20, -40,
//    -30,   0,  10,  15,  15,  10,   0, -30,
//    -30,   5,  15,  20,  20,  15,   5, -30,
//    -30,   0,  15,  20,  20,  15,   0, -30,
//    -30,   5,  10,  15,  15,  10,   5, -30,
//    -40, -20,   0,   5,   5,   0, -20, -40,
//    -50, -40, -30, -30, -30, -30, -40, -50,
//};
//
//static constexpr int8_t const INIT_TABLE_BISHOP_MG[64] =
//{
//    -20, -10, -10, -10, -10, -10, -10, -20,
//    -10,   0,   0,   0,   0,   0,   0, -10,
//    -10,   0,   5,  10,  10,   5,   0, -10,
//    -10,   5,   5,  10,  10,   5,   5, -10,
//    -10,   0,  10,  10,  10,  10,   0, -10,
//    -10,  10,  10,  10,  10,  10,  10, -10,
//    -10,   5,   0,   0,   0,   0,   5, -10,
//    -20, -10, -10, -10, -10, -10, -10, -20,
//};
//
//static constexpr int8_t const INIT_TABLE_ROOK_MG[64] =
//{
//     0,  0,  0,  0,  0,  0,  0,  0,
//     5, 10, 10, 10, 10, 10, 10,  5,
//    -5,  0,  0,  0,  0,  0,  0, -5,
//    -5,  0,  0,  0,  0,  0,  0, -5,
//    -5,  0,  0,  0,  0,  0,  0, -5,
//    -5,  0,  0,  0,  0,  0,  0, -5,
//    -5,  0,  0,  0,  0,  0,  0, -5,
//     0,  0,  0,  5,  5,  0,  0,  0,
//};
//
//static constexpr int8_t const INIT_TABLE_QUEEN_MG[64] =
//{
//    -20, -10, -10, -5, -5, -10, -10, -20,
//    -10,   0,   0,  0,  0,   0,   0, -10,
//    -10,   0,   5,  5,  5,   5,   0, -10,
//     -5,   0,   5,  5,  5,   5,   0,  -5,
//      0,   0,   5,  5,  5,   5,   0,  -5,
//    -10,   5,   5,  5,  5,   5,   0, -10,
//    -10,   0,   5,  0,  0,   0,   0, -10,
//    -20, -10, -10, -5, -5, -10, -10, -20,
//};
//
//static constexpr int8_t const TABLE_KING_MG[64] =
//{
//    -30, -40, -40, -50, -50, -40, -40, -30,
//    -30, -40, -40, -50, -50, -40, -40, -30,
//    -30, -40, -40, -50, -50, -40, -40, -30,
//    -30, -40, -40, -50, -50, -40, -40, -30,
//    -20, -30, -30, -40, -40, -30, -30, -20,
//    -10, -20, -20, -20, -20, -20, -20, -10,
//     20,  20,   0,   0,   0,   0,  20,  20,
//     20,  30,  10,   0,   0,  10,  30,  20,
//};
//
//static constexpr int8_t const TABLE_KING_EG[64] =
//{
//    -50, -40, -30, -20, -20, -30, -40, -50,
//    -30, -20, -10,   0,   0, -10, -20, -30,
//    -30, -10,  20,  30,  30,  20, -10, -30,
//    -30, -10,  30,  40,  40,  30, -10, -30,
//    -30, -10,  30,  40,  40,  30, -10, -30,
//    -30, -10,  20,  30,  30,  20, -10, -30,
//    -30, -30,   0,   0,   0,   0, -30, -30,
//    -50, -30, -30, -30, -30, -30, -30, -50,
//};

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
#if 1
    for(int r = 0; r < 8; ++r)
    {
        int nr = flipped ? 7 - r : r;
        for(int c = 0; c < 8; ++c)
        {
            int sq = nr * 4 + (c >= 4 ? 7 - c : c);
            dst[r * 8 + c] = int8_t(flipped ? -src[sq] : src[sq]);
        }
    }
    for(int sq = 0; sq < 64; ++sq)
    {
        if(flipped)
            dst[sq] = -src[sq ^ 56];
        else
            dst[sq] = int8_t(src[sq]);
    }
#endif
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
