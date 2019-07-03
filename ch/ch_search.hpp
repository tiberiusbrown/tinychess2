#pragma once

#include "ch.h"
#include "ch_internal.hpp"
#include "ch_evaluate.hpp"
#include "ch_genmoves.hpp"
#include "ch_history_heuristic.hpp"
#include "ch_position.hpp"

#include <algorithm>
#include <array>

// test mate in 4:
// 1r2k1r1/pbppnp1p/1b3P2/8/Q7/B1PB1q2/P4PPP/3R2K1 w - - 1 0

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
    history_heuristic* hh;
    position p;
    principal_variation pv;
    std::array<move, MAX_VARIATION> mvstack;
    std::array<std::array<move, CH_NUM_KILLERS>, MAX_VARIATION> killers;
    int score;
    int depth;
    int seldepth;
    uint32_t start_time;
    uint32_t info_time;
    ch_search_limits limits;
};

static constexpr int8_t const MOVE_ORDER_PIECEVALS[10] =
{
    1, 1,
    3, 3,
    3, 3,
    5, 5,
    9, 9,
};

static CH_FORCEINLINE void order_moves(
    move_list& mvs,
    search_data const& d,
    move const& hashmove,
    move const& parent_move,
    std::array<move, CH_NUM_KILLERS> const& killers)
{
    int prev_to = d.p.stack().prev_move.to();
    move countermove = d.hh->get_countermove(parent_move);

    for(move& mv : mvs)
    {
        int x;

        if(mv == hashmove)
        {
            x = INT8_MAX;
        }
        else if(std::find(
            killers.begin(), killers.end(), mv) != killers.end())
        {
            x = INT8_MAX - 1;
        }
        else if(mv == countermove)
        {
            x = INT8_MAX - 2;
        }
        else if(mv.to() == prev_to)
        {
            x = INT8_MAX - 1 - MOVE_ORDER_PIECEVALS[d.p.pieces[mv.from()]];
        }
        else if(d.p.pieces[mv.to()] != EMPTY)
        {
            x = 64
                + MOVE_ORDER_PIECEVALS[d.p.pieces[mv.to()]]
                - MOVE_ORDER_PIECEVALS[d.p.pieces[mv.from()]];
        }
        else
        {
            // TODO?
            x = 0;
#if CH_ENABLE_HISTORY_HEURISTIC
            x += d.hh->get_hh_score(d.p, mv);
#endif
        }

        mv.sort_key() = int8_t(x);
    }

    mvs.sort();
}

namespace search_flags
{
    static constexpr int const ROOT     = (1 << 0);
    static constexpr int const NULLMOVE = (1 << 1);
}

static void send_info(search_data& d)
{
    d.info_time = get_ms();
    int mstime = int(d.info_time - d.start_time);

    search_info(
        d.depth,
        std::max(d.depth, d.seldepth),
        d.nodes,
        mstime,
        (ch_move const*)&d.pv.pv[0],
        d.pv.pvlen,
        d.score,
        mstime ? d.nodes * 1000 / mstime : 0
    );
}

#if CH_COLOR_TEMPLATE
template<color c>
CH_FORCEINLINE bool quiesce_pawn_recapture(position const& p, int sq)
#else
CH_FORCEINLINE bool quiesce_pawn_recapture(color c, position const& p, int sq)
#endif
{
    uint64_t sqbb = 1ull << sq;
    if(c == WHITE)
    {
        if(p.bbs[BLACK + PAWN] & (shift_nw(sqbb) | shift_ne(sqbb)))
            return true;
    }
    else
    {
        if(p.bbs[WHITE + PAWN] & (shift_sw(sqbb) | shift_se(sqbb)))
            return true;
    }
    return false;
}

#if CH_COLOR_TEMPLATE
template<color c>
CH_FORCEINLINE bool quiesce_bad_capture(position const& p, move mv)
#else
CH_FORCEINLINE bool quiesce_bad_capture(color c, position const& p, move mv)
#endif
{
    int pc = p.pieces[mv.from()];
    int cap = p.pieces[mv.to()];
    static constexpr int const VALS[10] = { 1, 1, 3, 3, 3, 3, 5, 5, 9, 9 };
    if(VALS[cap] - VALS[pc] >= 0) return false;
    if(VALS[cap] - VALS[pc] < -2 &&
#if CH_COLOR_TEMPLATE
        quiesce_pawn_recapture<c>(p, mv.to()))
#else
        quiesce_pawn_recapture(c, p, mv.to()))
#endif
        return true;
    return false;
}

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel> static int quiesce(
#else
template<acceleration accel> static int quiesce(color c,
#endif
    search_data& d, int depth, int alpha, int beta, int height)
{
    position& p = d.p;
    move_list mvs;
    int value = MIN_SCORE;

    int stand_pat = evaluator<accel>::evaluate(p, c);
    if(stand_pat >= beta)
        return beta;
    alpha = std::max(alpha, stand_pat);

    d.seldepth = std::max(d.seldepth, height);

    // TODO: optimized tactical move generation
#if CH_COLOR_TEMPLATE
    mvs.generate<c, accel>(p);
#else
    mvs.generate<accel>(c, p);
#endif

    for(move mv : mvs)
    {
        if(!p.move_is_tactical(mv))
            continue;

#if CH_COLOR_TEMPLATE
        if(quiesce_bad_capture<c>(p, mv) && !mv.is_promotion())
#else
        if(quiesce_bad_capture(c, p, mv) && !mv.is_promotion())
#endif
            continue;

        p.do_move<accel>(mv);
        value = std::max(value,
#if CH_COLOR_TEMPLATE
            -quiesce<opposite(c), accel>(
#else
            -quiesce<accel>(opposite(c),
#endif
                d, depth - 1, -beta, -alpha, height + 1));
        p.undo_move<accel>(mv);

        if(value > alpha)
        {
            alpha = value;

            if(alpha >= beta)
                return value;
        }
    }

    return alpha;
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
    move_list mvs;
    int value;
    move best_move = INVALID_MOVE;
    move parent_move = pv.pv[0];
    int hh_increment = depth * depth;
    bool tail = (depth <= 1 || height >= MAX_VARIATION - 1);

    principal_variation child_pv;
    child_pv.pvlen = 0;

    pv.pvlen = 0;

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
    mvs.generate<c, accel>(p);
#else
    mvs.generate<accel>(c, p);
#endif

    if(mvs.empty())
    {
        // stalemate or checkmate?
        if(!p.in_check)
            return 0;
        else
            return MATED_SCORE + height;
    }

#if CH_ENABLE_NULL_MOVE
    if(!(flags & search_flags::NULLMOVE) &&
        !p.in_check && depth > 5 &&
        d.mvstack[height - 2] != INVALID_MOVE)
    {
        // null move pruning
        child_pv.pv[0] = INVALID_MOVE;
        d.mvstack[height] = INVALID_MOVE;
        p.do_null_move();
        value =
#if CH_COLOR_TEMPLATE
            -negamax<opposite(c), accel, search_flags::NULLMOVE>(
#else
            -negamax<accel, search_flags::NULLMOVE>(opposite(c),
#endif
                d, child_pv, depth - 4, -beta, -beta + 1, height + 1);
        p.undo_null_move();
        if(value >= beta)
            return beta;
    }
#endif
    value = MIN_SCORE;

    if(depth > 1)
    {
        order_moves(mvs, d, hash_move, parent_move, d.killers[height]);
    }
    else
    {
        for(int i = 0, n = 0; n < mvs.size(); ++n)
        {
            if(mvs[n] == hash_move ||
                std::find(
                    d.killers[height].begin(),
                    d.killers[height].end(),
                    mvs[n]) != d.killers[height].end())
            {
                std::swap(mvs[i++], mvs[n]);
            }
        }
    }

    if(!(flags & search_flags::NULLMOVE))
    {
        for(int n = 0; n < CH_NUM_KILLERS; ++n)
            d.killers[height + 1][n] = INVALID_MOVE;
    }

    constexpr int child_flags =
        flags & ~search_flags::ROOT;
    for(int n = 0; n < mvs.size(); ++n)
    {
        move mv = mvs[n];
        bool quiet = !p.move_is_tactical(mv);

        int reduction = 1;
#if CH_ENABLE_LATE_MOVE_REDUCTION
        if(!(flags & search_flags::NULLMOVE))
        {
            // stupid simple late move reduction
            if(n >= 6 && depth > 2 && !p.move_is_tactical(mv))
                reduction = depth / 3;
        }
#endif
        
        p.do_move<accel>(mv);
        d.mvstack[height] = mv;
        child_pv.pv[0] = mv;
        if(tail)
        {
#if CH_ENABLE_QUIESCENCE
            if(!quiet)
            {
                // TODO: quiesce
#if CH_COLOR_TEMPLATE
                value = std::max(value, -quiesce<opposite(c), accel>(
#else
                value = std::max(value, -quiesce<accel>(opposite(c),
#endif
                    d, depth - 1, -beta, -alpha, height + 1));
            }
            else
#endif
            {
                value = std::max(value, evaluator<accel>::evaluate(p, c));
                ++d.nodes;
            }
        }
        else
        {        
#if CH_COLOR_TEMPLATE
            value = std::max(value, -negamax<opposite(c), accel, child_flags>(
#else
            value = std::max(value, -negamax<accel, child_flags>(opposite(c),
#endif
                d, child_pv, depth - reduction, -beta, -alpha, height + 1));
        }
        p.undo_move<accel>(mv);

#if CH_ENABLE_HISTORY_HEURISTIC
        if(!(flags & search_flags::NULLMOVE) && quiet)
            d.hh->increment_bf(p, mv, hh_increment);
#endif

        if(value > best)
        {
            best = value;
            best_move = mv;
            if(value > alpha)
            {
                alpha = value;
                pv.pvlen = child_pv.pvlen + 1;
                pv.pv[0] = mv;
                memcpy32(&pv.pv[1], &child_pv.pv[0], child_pv.pvlen);
                if(alpha >= beta)
                {
                    if((flags & search_flags::NULLMOVE) != 0)
                        break;

                    // history updates
                    if(mv != hash_move && mv != d.killers[height][0])
                    {
                        // store killer move
                        for(int m = 0; m < CH_NUM_KILLERS - 1; ++m)
                            d.killers[height][m + 1] = d.killers[height][m];
                        d.killers[height][0] = mv;
                    }
#if CH_ENABLE_HISTORY_HEURISTIC
                    if(quiet)
                        d.hh->increment_hh(p, mv, hh_increment);
                    if(height > 0)
                        d.hh->set_countermove(parent_move, mv);
#endif
                    break;
                }
            }
        }
    }

    if(!(flags & search_flags::NULLMOVE))
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
            i.best = best_move;
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
    // TODO: move constants to a common location
    constexpr int ASPIRATION_BASE_DELTA = 20;
    constexpr int ASPIRATION_MIN_DEPTH = 4;

    int delta = ASPIRATION_BASE_DELTA;
    int alpha = depth < ASPIRATION_MIN_DEPTH ? MIN_SCORE :
        std::max(MIN_SCORE, prev_score - delta);
    int beta = depth < ASPIRATION_MIN_DEPTH ? MAX_SCORE :
        std::min(MAX_SCORE, prev_score + delta);

    for(;;)
    {

        int value = negamax_root<accel>(d, depth, alpha, beta);

        if(value > alpha && value < beta)
            return value;

        if(value <= alpha)
        {
            beta = (alpha + beta) / 2;
            alpha = std::max(MIN_SCORE, alpha - delta);
        }

        if(value >= beta)
        {
            beta = std::min(MAX_SCORE, beta + delta);
        }

#if 0
        // ethereal
        delta += delta / 2;
#else
        // stockfish
        delta += delta / 4 + 5;
#endif
    }
}

template<acceleration accel>
static move iterative_deepening(
    search_data& d, position const& p)
{
    int depth = 1;
    int prev_score;
    d.p = p;
    d.nodes = 0;
    prev_score = ch::evaluator<accel>::evaluate(p, p.current_turn);
    for(;;)
    {
        d.seldepth = 0;
        prev_score = aspiration_window<accel>(d, depth, prev_score);
        d.depth = depth;
        d.score = prev_score;
        if(depth >= d.limits.depth) break;
        send_info(d);
        ++depth;
    }
    send_info(d);
    return d.pv.pv[0];
}

}
