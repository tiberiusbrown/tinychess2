#pragma once

#include "ch_evaluate.hpp"

#include "ch_evaluate_sse.hpp"

#if CH_ENABLE_AVX
namespace ch
{

    template<>
    struct evaluator<ACCEL_AVX>
    {
        static int evaluate(position const& p, color c)
        {
            return evaluator<ACCEL_SSE>::evaluate(p, c);
        }
    };

}
#endif
