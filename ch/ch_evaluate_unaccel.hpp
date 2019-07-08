#pragma once

#include "ch_evaluate.hpp"
#include "ch_position.hpp"

#if CH_ENABLE_UNACCEL || 1
namespace ch
{

static CH_FORCEINLINE int eval_piece_table(uint64_t pieces, int16_t const table[64])
{
    int r = 0;
    while(pieces)
        r += table[pop_lsb(pieces)];
    return r;
}

static CH_FORCEINLINE int eval_piece_table_flipped(uint64_t pieces, int16_t const table[64])
{
    int r = 0;
    while(pieces)
        r += table[pop_lsb(pieces) ^ 56];
    return r;
}

template<>
struct evaluator<ACCEL_UNACCEL>
{
    static int evaluate(position const& p, color c)
    {
        int x = 0;

        x += eval_piece_table(p.bbs[WHITE + PAWN  ], table_pawn  );
        x += eval_piece_table(p.bbs[WHITE + KNIGHT], table_knight);
        x += eval_piece_table(p.bbs[WHITE + BISHOP], table_bishop);
        x += eval_piece_table(p.bbs[WHITE + ROOK  ], table_rook  );
        x += eval_piece_table(p.bbs[WHITE + QUEEN ], table_queen );

        x -= eval_piece_table_flipped(p.bbs[BLACK + PAWN  ], table_pawn  );
        x -= eval_piece_table_flipped(p.bbs[BLACK + KNIGHT], table_knight);
        x -= eval_piece_table_flipped(p.bbs[BLACK + BISHOP], table_bishop);
        x -= eval_piece_table_flipped(p.bbs[BLACK + ROOK  ], table_rook  );
        x -= eval_piece_table_flipped(p.bbs[BLACK + QUEEN ], table_queen );

        return c == WHITE ? x : -x;
    }
};

}
#endif
