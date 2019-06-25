#pragma once

#include "ch_evaluate.hpp"
#include "ch_position.hpp"

#if CH_ENABLE_UNACCEL || 1
namespace ch
{

static CH_FORCEINLINE int eval_piece_table(uint64_t pieces, int8_t const table[64])
{
    int r = 0;
    while(pieces)
        r += table[pop_lsb(pieces)];
    return r;
}

static CH_FORCEINLINE int eval_piece_table_flipped(uint64_t pieces, int8_t const table[64])
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

        for(int i = 0; i < 10; ++i)
            x += popcnt(p.bbs[i]) * PIECE_VALUES[i];

        x += eval_piece_table(p.bbs[WHITE + PAWN  ], TABLE_PAWN  );
        x += eval_piece_table(p.bbs[WHITE + KNIGHT], TABLE_KNIGHT);
        x += eval_piece_table(p.bbs[WHITE + BISHOP], TABLE_BISHOP);
        x += eval_piece_table(p.bbs[WHITE + ROOK  ], TABLE_ROOK  );
        x += eval_piece_table(p.bbs[WHITE + QUEEN ], TABLE_QUEEN );

        x -= eval_piece_table_flipped(p.bbs[BLACK + PAWN  ], TABLE_PAWN  );
        x -= eval_piece_table_flipped(p.bbs[BLACK + KNIGHT], TABLE_KNIGHT);
        x -= eval_piece_table_flipped(p.bbs[BLACK + BISHOP], TABLE_BISHOP);
        x -= eval_piece_table_flipped(p.bbs[BLACK + ROOK  ], TABLE_ROOK  );
        x -= eval_piece_table_flipped(p.bbs[BLACK + QUEEN ], TABLE_QUEEN );

        return c == WHITE ? x : -x;
    }
};

}
#endif
