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

static constexpr int const MAX_VARIATION = 128;

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
    move best;
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
    1 * 4, 1 * 4,
    3 * 4, 3 * 4,
    3 * 4, 3 * 4,
    5 * 4, 5 * 4,
    9 * 4, 9 * 4,
};

static CH_FORCEINLINE void order_moves(
    move_list& mvs,
    search_data const& d,
    move const& hashmove,
    std::array<move, CH_NUM_KILLERS> const& killers)
{
    move parent_move = d.p.stack().prev_move;
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
#if CH_ENABLE_COUNTERMOVE_HEURISTIC
        else if(mv == countermove)
        {
            x = INT8_MAX - 2;
        }
#endif
        else if(mv.to() == parent_move.to())
        {
            x = INT8_MAX - 2 - MOVE_ORDER_PIECEVALS[d.p.pieces[mv.from()]];
        }
        else if(mv.is_promotion())
        {
            x = MOVE_ORDER_PIECEVALS[mv.promotion_piece()]
                - MOVE_ORDER_PIECEVALS[PAWN];
            if(d.p.pieces[mv.to()] != EMPTY)
                x += MOVE_ORDER_PIECEVALS[d.p.pieces[mv.to()]];
            x += 64;
        }
        else if(d.p.pieces[mv.to()] != EMPTY)
        {
            x = MOVE_ORDER_PIECEVALS[d.p.pieces[mv.to()]]
                - MOVE_ORDER_PIECEVALS[d.p.pieces[mv.from()]];
            x += 64;
        }
        else
        {
            x = 0;
#if CH_ENABLE_HISTORY_HEURISTIC
            x += d.hh->get_hh_score(d.p, mv);
#endif
        }

        mv.sort_key() = int8_t(x);
    }

    mvs.sort();
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

    int stand_pat = evaluator<accel>::evaluate(p, c);
    ++d.nodes;
    if(stand_pat >= beta)
        return beta;

    d.seldepth = std::max(d.seldepth, height);

    // TODO: optimized tactical move generation
#if CH_COLOR_TEMPLATE
    mvs.generate<c, accel>(p);
#else
    mvs.generate<accel>(c, p);
#endif

    //{
    //    bool promotion = false;
    //    for(move mv : mvs)
    //        if(mv.is_promotion()) { promotion = true; break; }
    //    int delta = 975;
    //    if(promotion) delta += 775;
    //    if(stand_pat + delta < alpha)
    //        return alpha;
    //}

    alpha = std::max(alpha, stand_pat);

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
#if CH_COLOR_TEMPLATE
        int value = -quiesce<opposite(c), accel>(
#else
        int value = -quiesce<accel>(opposite(c),
#endif
            d, depth - 1, -beta, -alpha, height + 1);
        p.undo_move<accel>(mv);

        if(value > alpha)
        {
            if(value >= beta)
                return beta;
            alpha = value;
        }
    }

    return alpha;
}

enum { NODE_PV = 0, NODE_CUT = 1, NODE_ALL = -1 };

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel> static int negamax(
#else
template<acceleration accel> static int negamax(color c,
#endif
    search_data& d,
    int depth, int alpha, int beta, int height, int node_type)
{
    move hash_move = NULL_MOVE;

#if CH_ENABLE_HASH
    // transposition table lookup
    if(depth > 1)
    {
        hash_info i;
        if(d.tt->get(d.p.hash(), i) && i.depth >= depth)
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
#endif

    move_list mvs;
#if CH_COLOR_TEMPLATE
    mvs.generate<c, accel>(d.p);
#else
    mvs.generate<accel>(c, d.p);
#endif
    if(mvs.empty())
        return d.p.in_check ? MATED_SCORE + height : 0;

#if CH_ENABLE_NULL_MOVE
    if(!d.p.in_check && depth > 5 &&
        (height < 1 || d.mvstack[height - 1] != NULL_MOVE) &&
        (height < 2 || d.mvstack[height - 2] != NULL_MOVE))
    {
        // null move pruning
        d.mvstack[height] = NULL_MOVE;
        d.p.do_null_move();
#if CH_COLOR_TEMPLATE
        int value = -negamax<opposite(c), accel>(
#else
        int value = -negamax<accel>(opposite(c),
#endif
            d, depth - 4, -beta, -beta + 1, height + 1, -node_type);
        d.p.undo_null_move();
        if(value >= beta)
            return beta;

        // VERY EXPERIMENTAL
        alpha = std::max(alpha, value);
    }
#endif

    bool const tail = (depth <= 1);

    if(tail)
    {
        for(int i = 0, n = 0; n < mvs.size(); ++n)
        {
            if(mvs[n] == hash_move
#if CH_ENABLE_KILLERS
                || std::find(
                    d.killers[height].begin(),
                    d.killers[height].end(),
                    mvs[n]) != d.killers[height].end()
#endif
                )
            {
                std::swap(mvs[i++], mvs[n]);
            }
        }
    }
    else
    {
        order_moves(mvs, d, hash_move, d.killers[height]);
        mvs.sort();
    }

    int value = MIN_SCORE;
    int best = MIN_SCORE;
    move best_move = NULL_MOVE;
    int hh_increment = depth * depth;
    int alpha_orig = alpha;

#if CH_ENABLE_HISTORY_HEURISTIC
    if(!tail)
        for(move mv : mvs)
            if(!d.p.move_is_tactical(mv))
                d.hh->increment_bf(d.p, mv, hh_increment);
#endif

    bool const can_reduce = (node_type != NODE_PV && depth > 2);
    for(int n = 0; n < mvs.size(); ++n)
    {
        move mv = mvs[n];
        bool const quiet = d.p.move_is_tactical(mv);

        d.mvstack[height] = mv;
        d.p.do_move<accel>(mv);

        int reduction = 1;
#if CH_ENABLE_LATE_MOVE_REDUCTION
        if(node_type != NODE_PV)
        {
            // stupid simple late move reduction
            if(can_reduce && quiet && n >= 6)
                reduction = depth / 3;
        }
#endif

        if(tail)
        {
            int v;
#if CH_ENABLE_QUIESCENCE
            if(!quiet)
            {
#if CH_COLOR_TEMPLATE
                v = -quiesce<opposite(c), accel>(
#else
                v = -quiesce<accel>(opposite(c),
#endif
                    d, depth - reduction, -beta, -alpha, height + 1);
            }
            else
#endif
            {
                ++d.nodes;
                v = evaluator<accel>::evaluate(d.p, opposite(c));
            }
            value = std::max(value, v);
        }
#if CH_ENABLE_PVS
        else if(n > 0)
        {
            int v;
#if CH_COLOR_TEMPLATE
            v = -negamax<opposite(c), accel>(
#else
            v = -negamax<accel>(opposite(c),
#endif
                d, depth - reduction, -alpha - 1, -alpha, height + 1,
                node_type == NODE_CUT ? NODE_ALL : NODE_CUT);
            if((v > alpha && v < beta) ||
                (node_type == NODE_PV && v == beta && beta == alpha + 1))
            {
                if(v == alpha + 1)
                    v = alpha;
#if CH_COLOR_TEMPLATE
                v = -negamax<opposite(c), accel>(
#else
                v = -negamax<accel>(opposite(c),
#endif
                    d, depth - reduction, -beta, -v, height + 1, node_type);
            }
            value = std::max(value, v);
        }
#endif
        else
        {
#if CH_COLOR_TEMPLATE
            value = std::max(value, -negamax<opposite(c), accel>(
#else
            value = std::max(value, -negamax<accel>(opposite(c),
#endif
                d, depth - reduction, -beta, -alpha, height + 1, -node_type));
        }

        d.p.undo_move<accel>(mv);

        if(value > best)
        {
            best = value;
            best_move = mv;
            if(height == 0)
                d.best = mv;
            if(value > alpha)
            {
                alpha = value;
                if(alpha >= beta)
                {
                    // history updates
#if CH_ENABLE_KILLERS
                    {
                        auto& k = d.killers[height];
                        if(mv != hash_move && std::find(
                            k.begin(), k.end(), mv) == k.end())
                        {
                            for(int m = 0; m < CH_NUM_KILLERS - 1; ++m)
                                k[m + 1] = k[m];
                            k[0] = mv;
                        }
                    }
#endif
#if CH_ENABLE_HISTORY_HEURISTIC
                    if(quiet)
                        d.hh->increment_hh(d.p, mv, hh_increment);
#endif
#if CH_ENABLE_COUNTERMOVE_HEURISTIC
                    if(height > 0)
                        d.hh->set_countermove(d.p.stack().prev_move, mv);
#endif
                    break;
                }
            }
        }
    }

#if CH_ENABLE_HASH
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
        d.tt->put(d.p.hash(), i);
    }
#endif

    return value;
}

template<acceleration accel>
static int negamax_root(
    search_data& d, int depth, int alpha, int beta)
{
#if CH_COLOR_TEMPLATE
    if(d.p.current_turn == WHITE)
    {
        return negamax<WHITE, accel>(
            d, depth, alpha, beta, 0, NODE_PV);
    }
    else
    {
        return negamax<BLACK, accel>(
            d, depth, alpha, beta, 0, NODE_PV);
    }
#else
    return negamax<accel>(d.p.current_turn,
        d, depth, alpha, beta, 0, NODE_PV);
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
    return d.best;
}

}
