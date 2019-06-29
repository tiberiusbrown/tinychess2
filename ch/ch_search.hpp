#pragma once

#include "ch.h"
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

static constexpr int const MATED_SCORE = CH_MATED_SCORE;
static constexpr int const MATE_SCORE = CH_MATE_SCORE;

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
    principal_variation pv;
    std::array<std::array<move, CH_NUM_KILLERS>, MAX_VARIATION> killers;
    int score;
    int depth;
    int seldepth;
    uint32_t start_time;
    uint32_t info_time;
    ch_search_limits limits;
};

namespace search_flags
{
    static constexpr int const ROOT = (1 << 0);
}

static void send_info(search_data& d)
{
    d.info_time = get_ms();
    int mstime = int(d.info_time - d.start_time);

    search_info(
        d.depth,
        d.depth, // d.seldepth
        d.nodes,
        mstime,
        (ch_move const*)&d.pv.pv[0],
        d.pv.pvlen,
        d.score,
        mstime ? d.nodes * 1000 / mstime : 0
    );
}

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel> static int quiesce(
#else
template<acceleration accel> static int quiesce(color c,
#endif
    search_data& d, principal_variation& pv,
    int depth, int alpha, int beta, int height)
{
    // TODO
}

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel, int flags = 0> static int negamax(
#else
template<acceleration accel, int flags = 0> static int negamax(color c,
#endif
    search_data& d, principal_variation& pv,
    int depth, int alpha, int beta, int height)
{
    move hash_move = INVALID_MOVE;
    int alpha_orig = alpha;
    int best = MIN_SCORE;
    position& p = d.p;
    move mvs[256];
    int num;
    int value;

    principal_variation child_pv;

    pv.pvlen = 0;
    ++d.nodes;

    if(!(flags & search_flags::ROOT))
    {
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

#if CH_COLOR_TEMPLATE
    num = move_generator<c, accel>::generate(mvs, p);
#else
    num = move_generator<accel>::generate(c, mvs, p);
#endif

    if(num == 0)
    {
        // stalemate or checkmate?
        if(!p.in_check)
            return 0;
        else
            return MATED_SCORE + height;
    }

    for(int i = 0, n = 0; n < num; ++n)
    {
        if(mvs[n] == hash_move ||
#if 1
            std::find(
                d.killers[height].begin(),
                d.killers[height].end(),
                mvs[n]) != d.killers[height].end())
#else
            mvs[n] = d.killers[height][0]
#endif
        {
            std::swap(mvs[i++], mvs[n]);
        }
    }

    value = MIN_SCORE;

    for(int n = 0; n < CH_NUM_KILLERS; ++n)
        d.killers[height + 1][n] = INVALID_MOVE;

    for(int n = 0; n < num; ++n)
    {
        move mv = mvs[n];
        p.do_move<accel>(mv);
        if(depth <= 1)
        {
            //if(p.stack().cap_piece != EMPTY)
            //{
            //    // TODO: quiesce
            //}
            //else
            {
                value = std::max(value, evaluator<accel>::evaluate(p, c));
                ++d.nodes;
            }
        }
        else
        {
            value = std::max(value,
#if CH_COLOR_TEMPLATE
                -negamax<opposite(c), accel>(
#else
                -negamax<accel>(opposite(c),
#endif
                    d, child_pv, depth - 1, -beta, -alpha, height + 1));
        }
        p.undo_move<accel>(mv);

        if(value > best)
        {
            best = value;
            if(value > alpha)
            {
                alpha = value;
                pv.pvlen = child_pv.pvlen + 1;
                pv.pv[0] = mv;
                memcpy32(&pv.pv[1], &child_pv.pv[0], child_pv.pvlen);
                if(alpha >= beta)
                {
                    if(mv != hash_move && mv != d.killers[height][0])
                    {
                        // store killer move
                        for(int m = 0; m < CH_NUM_KILLERS - 1; ++m)
                            d.killers[height][m + 1] = d.killers[height][m];
                        d.killers[height][0] = mv;
                    }
                    break;
                }
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
static int negamax_root(
    search_data& d, int depth, int alpha, int beta)
{
#if CH_COLOR_TEMPLATE
    if(d.p.current_turn == WHITE)
    {
        return negamax<WHITE, accel, search_flags::ROOT>(
            d, d.pv, depth, alpha, beta, 0);
    }
    else
    {
        return negamax<BLACK, accel, search_flags::ROOT>(
            d, d.pv, depth, alpha, beta, 0);
    }
#else
    return negamax<accel, search_flags::ROOT>(d.p.current_turn,
        d, d.pv, depth, alpha, beta, 0);
#endif
}

template<acceleration accel>
static int aspiration_window(
    search_data& d, int depth, int prev_score)
{
    int delta = 14;

    for(;;)
    {
        int alpha = prev_score - delta;
        int beta = prev_score + delta;
        int value = negamax_root<accel>(d, depth, alpha, beta);

        if(value > alpha && value < beta)
            return value;

        if(value <= alpha)
        {
            beta = (alpha + beta) / 2;
            alpha = std::max(-MIN_SCORE, alpha - delta);
        }

        if(value >= beta)
            beta = std::min(MAX_SCORE, beta + delta);

        delta += delta / 2;
    }
}

template<acceleration accel>
static move iterative_deepening(
    search_data& d, position const& p)
{
    int depth = 2;
    int prev_score;
    d.p = p;
    d.nodes = 0;
    prev_score = ch::evaluator<accel>::evaluate(p, p.current_turn);
    for(;;)
    {
        prev_score = aspiration_window<accel>(d, depth, prev_score);
        d.depth = depth;
        d.score = prev_score;
        if(depth >= d.limits.depth) break;
        if(depth > 4)
            send_info(d);
        ++depth;
    }
    send_info(d);
    return d.pv.pv[0];
}

}
