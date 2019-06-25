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
    move best = INVALID_MOVE;
    int alpha_orig = alpha;

    if(depth == 0)
        return evaluator<accel>::evaluate(p, c);

    // transposition table lookup
    move hash_move = INVALID_MOVE;
    if(depth > 1)
    {
        hash_info i;
        if(p.tt.get(p.stack().hash, i) && i.depth >= depth)
        {
            if(i.flag == hash_info::EXACT)
                return i.value;
            else if(i.flag == hash_info::LOWER)
                alpha = std::max<int>(alpha, i.value);
            else if(i.flag == hash_info::UPPER)
                beta = std::min<int>(beta, i.value);
            if(alpha >= beta)
                return i.value;
            hash_move = i.best;
        }
    }

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

    for(int n = 0; n < num; ++n)
    {
        if(mvs[n] == hash_move)
        {
            std::swap(mvs[0], mvs[n]);
            break;
        }
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

        if(value > alpha)
        {
            alpha = value;
            best = mvs[n];
        }
        if(alpha >= beta) break;
    }

    // transposition table store
    if(depth > 1)
    {
        hash_info i;
        i.value = int16_t(value);
        if(value <= alpha_orig)
            i.flag = hash_info::UPPER;
        else if(value >= beta)
            i.flag = hash_info::LOWER;
        else
            i.flag = hash_info::EXACT;
        i.depth = int8_t(depth);
        i.best = best;
        p.tt.put(p.stack().hash, i);
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
            -negamax<accel>(p.current_turn, p, depth - 1, -beta, -alpha));
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
