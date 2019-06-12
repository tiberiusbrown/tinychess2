#pragma once

#include "ch_internal.hpp"

namespace ch
{

struct position;

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel>
struct move_generator
{
    static int generate(move* mvs, position const& p);
};
#else
template<acceleration accel>
struct move_generator
{
    static int generate(color c, move* mvs, position const& p);
};
#endif

}
