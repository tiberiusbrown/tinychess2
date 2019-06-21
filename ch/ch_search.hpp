#pragma once

#include "ch_internal.hpp"
#include "ch_evaluate.hpp"
#include "ch_genmoves.hpp"
#include "ch_position.hpp"

#include <algorithm>

namespace ch
{

static constexpr int const MIN_SCORE = -32000;
static constexpr int const MAX_SCORE = +32000;


#if CH_COLOR_TEMPLATE
template<color c, acceleration accel>
int negamax(position& p, int depth, int alpha, int beta)
#else
template<acceleration accel>
int negamax(color c, position& p, int depth, int alpha, int beta)
#endif
{
    // transposition table lookup
    {
        // TODO
        /*
        alphaOrig := a
        ttEntry := transpositionTableLookup(node)
        if ttEntry is valid and ttEntry.depth >= depth then
            if ttEntry.flag == EXACT then
                return ttEntry.value
            else if ttEntry.flag == LOWERBOUND then
                a := max(a, ttEntry.value)
            else if ttEntry.flag == UPPERBOUND then
                b := min(b, ttEntry.value)

            if a >= b then
                return ttEntry.value
        */
    }

    if(depth == 0)
        return evaluator<accel>::evaluate(p, c);

    move mvs[256];
#if CH_COLOR_TEMPLATE
    int num = move_generator<c, accel>::generate(mvs, p);
#else
    int num = move_generator<accel>::generate(c, mvs, p);
#endif

    if(num == 0)
    {
        // stalemate?
        if(!p.in_check)
            return 0; // TODO: stalemate eval
        else
            return MIN_SCORE;
    }

    int value = MIN_SCORE;

    for(int n = 0; n < num; ++n)
    {
        p.do_move<accel>(mvs[n]);
        value = std::max(value,
#if CH_COLOR_TEMPLATE
            -negamax<opposite(c), accel>(p, depth - 1, -beta, -alpha));
#else
            -negamax<accel>(opposite(c), p, depth - 1, -beta, -alpha));
#endif
        p.undo_move<accel>(mvs[n]);

        alpha = std::max(alpha, value);
        if(alpha >= beta) break;
    }

    // transposition table store
    {
        // TODO
        /*
        ttEntry.value := value
        if value <= alphaOrig then
            ttEntry.flag := UPPERBOUND
        else if value >= b then
            ttEntry.flag := LOWERBOUND
        else
            ttEntry.flag := EXACT
        ttEntry.depth := depth	
        transpositionTableStore(node, ttEntry)
        */
    }

    return value;
}

template<acceleration accel>
int negamax_root(position& p, move& best, int depth, int alpha, int beta)
{
    move mvs[256];
    int num;
#if CH_COLOR_TEMPLATE
    if(p.current_turn == WHITE)
        num = move_generator<WHITE, accel>::generate(mvs, p);
    else
        num = move_generator<BLACK, accel>::generate(mvs, p);
#else
    num = move_generator<accel>::generate(p.current_turn, mvs, p);
#endif

    int value = MIN_SCORE;
    best = INVALID_MOVE;

    for(int n = 0; n < num; ++n)
    {
        p.do_move<accel>(mvs[n]);
        value = std::max(value,
#if CH_COLOR_TEMPLATE
            p.current_turn == WHITE ?
            -negamax<WHITE, accel>(p, depth - 1, -beta, -alpha) :
            -negamax<BLACK, accel>(p, depth - 1, -beta, -alpha));
#else
            -negamax<accel>(opposite(p.current_turn), p, depth - 1, -beta, -alpha));
#endif
        p.undo_move<accel>(mvs[n]);

        if(value > alpha)
        {
            alpha = value;
            best = mvs[n];
        }
        if(alpha >= beta) break;
    }

    return value;
}

}
