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

template<acceleration accel>
int generate_moves_color(color c, move* mvs, position& p)
{
#if CH_COLOR_TEMPLATE
    if(c == WHITE)
        return move_generator<WHITE, accel>::generate(mvs, p);
    else
        return move_generator<BLACK, accel>::generate(mvs, p);
#else
    return move_generator<accel>::generate(c, mvs, p);
#endif
}

}
