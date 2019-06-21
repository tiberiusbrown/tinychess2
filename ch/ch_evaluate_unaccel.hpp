#pragma once

#include "ch_evaluate.hpp"

#if CH_ENABLE_UNACCEL || 1
namespace ch
{

template<>
struct evaluator<ACCEL_UNACCEL>
{
    static int evaluate(position const& p, color c)
    {
        int x = 0;

        for(int i = 0; i < 10; ++i)
            x += popcnt(p.bbs[i]) * PIECE_VALUES[i];

        return c == WHITE ? x : -x;
    }
};

}
#endif
