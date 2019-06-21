#pragma once

#include "ch_internal.hpp"
#include "ch_move.hpp"

namespace ch
{

struct position;

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel>
struct move_generator
{
    static int generate(move* mvs, position& p);
};
#else
template<acceleration accel>
struct move_generator
{
    static int generate(color c, move* mvs, position& p);
};
#endif

}
