#pragma once

#include "ch_internal.hpp"
#include "ch_evaluate.hpp"
#include "ch_genmoves.hpp"
#include "ch_position.hpp"

#include <algorithm>
#include <array>

namespace ch
{

static constexpr int const MIN_SCORE = -32000;
static constexpr int const MAX_SCORE = +32000;

static constexpr int const MAX_VARIATION = 256;

struct principal_variation
{
    int pvlen;
    std::array<move, MAX_VARIATION> pv;
};

struct search_data
{
    uint64_t nodes;
    trans_table* tt;
    position p;
};

namespace search_flags
{
    static constexpr int const ROOT = (1 << 0);
}

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel, int flags = 0> int negamax(
#else
template<acceleration accel, int flags = 0> int negamax(color c,
#endif
    search_data& d, principal_variation& pv,
    int depth, int alpha, int beta)
{
    move hash_move = INVALID_MOVE;
    move best_move = INVALID_MOVE;
    int alpha_orig = alpha;
    int best = MIN_SCORE;
    position& p = d.p;

    principal_variation child_pv;

    pv.pvlen = 0;
    ++d.nodes;

    if(!(flags & search_flags::ROOT))
    {
        if(depth == 0)
            return evaluator<accel>::evaluate(p, c);

        // transposition table lookup
        if(depth > 1)
        {
            hash_info i;
            if(d.tt->get(p.stack().hash, i) && i.depth >= depth)
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
            return 0;
        else
            return MIN_SCORE; // TODO: incorporate depth
    }

    // TODO: integrate killer move heuristic here
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
            -negamax<opposite(c), accel>(d, child_pv, depth - 1, -beta, -alpha));
#else
            -negamax<accel>(opposite(c), d, child_pv, depth - 1, -beta, -alpha));
#endif
        p.undo_move<accel>(mvs[n]);

        if(value > best)
        {
            best = value;
            best_move = mvs[n];
            if(value > alpha)
            {
                alpha = value;
                pv.pvlen = child_pv.pvlen + 1;
                pv.pv[0] = best_move;
                memcpy32(&pv.pv[1], &child_pv.pv[0], child_pv.pvlen);
                if(alpha >= beta) break;
            }
        }
    }

    if(!(flags & search_flags::ROOT))
    {
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
            d.tt->put(p.stack().hash, i);
        }
    }

    return value;
}

template<acceleration accel>
int negamax_root(
    search_data& d, principal_variation& pv,
    int depth, int alpha, int beta)
{
#if CH_COLOR_TEMPLATE
    if(d.p.current_turn == WHITE)
        return negamax<WHITE, accel, search_flags::ROOT>(d, pv, depth, alpha, beta);
    else
        return negamax<BLACK, accel, search_flags::ROOT>(d, pv, depth, alpha, beta);
#else
    return negamax<accel, search_flags::ROOT>(p.current_turn,
        d, pv, depth, alpha, beta);
#endif
}

}
