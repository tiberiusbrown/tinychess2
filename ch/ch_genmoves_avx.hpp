#pragma once

#include "ch_config.hpp"

#if CH_ENABLE_ACCEL

#include "ch_internal.hpp"
#include "ch_genmoves.hpp"
#include "ch_position.hpp"

#include <immintrin.h>

namespace ch
{

#if CH_COLOR_TEMPLATE
template<color c>
struct move_generator<c, ACCEL_AVX>
{
    static int generate(move* mvs, position const& p)
    {
        return move_generator<c, ACCEL_SSE>::generate(mvs, p);
#else
template<>
struct move_generator<ACCEL_AVX>
{
    static int generate(color c, move* mvs, position const& p)
    {
        return move_generator<ACCEL_SSE>::generate(c, mvs, p);
#endif
    }
};

}

#endif
