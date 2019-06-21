#pragma once

#include "ch_evaluate.hpp"

#include "ch_evaluate_unaccel.hpp"

#if CH_ENABLE_SSE
namespace ch
{

    template<>
    struct evaluator<ACCEL_SSE>
    {
        static int evaluate(position const& p, color c)
        {
            return evaluator<ACCEL_UNACCEL>::evaluate(p, c);
        }
    };

}
#endif
