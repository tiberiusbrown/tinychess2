#pragma once

#include "ch_bb.hpp"
#include "ch_bb_sse.hpp"
#include "ch_bb_avx.hpp"

#include "ch_evaluator.hpp"
#include "ch_position.hpp"


namespace ch
{

template<acceleration accel>
struct evaluator
{
    CH_FORCEINLINE int evaluate(position const& p, color c)
    {
        int phase;
        {
            // zurichess-style phase
            int total = 4*1 + 4*1 + 4*3 + 2*6;
            int curr = total;
            curr -= popcnt<accel>(p.bbs[WHITE+KNIGHT] | p.bbs[BLACK+KNIGHT]) * 1;
            curr -= popcnt<accel>(p.bbs[WHITE+BISHOP] | p.bbs[BLACK+BISHOP]) * 1;
            curr -= popcnt<accel>(p.bbs[WHITE+ROOK] | p.bbs[BLACK+ROOK]) * 3;
            curr -= popcnt<accel>(p.bbs[WHITE+QUEEN] | p.bbs[BLACK+QUEEN]) * 6;
            curr = std::max(curr, 0);
            phase = (curr * 256 + total / 2) / total;
        }

        int x = 0;
        x += p.stack().piece_sq;
        x = (SC_MG(x) * (256 - phase) + SC_EG(x) * phase) / 256;
        x = (c == WHITE ? x : -x);

        return x;
    }
};

}
