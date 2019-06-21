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

// piece values in centipawns
static constexpr int16_t const PIECE_VALUES[10] =
{
    100, -100,
    300, -300,
    310, -310,
    500, -500,
    900, -900,
};

}
