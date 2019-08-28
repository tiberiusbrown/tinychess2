#pragma once

#include "ch.h"
#include "ch_internal.hpp"
#include "ch_evaluate.hpp"
#include "ch_genmoves.hpp"
#include "ch_history_heuristic.hpp"
#include "ch_move_picker.hpp"
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

static constexpr int const MAX_VARIATION = 96;

struct principal_variation
{
    int pvlen;
    std::array<move, MAX_VARIATION> pv;
};

static CH_FORCEINLINE void update_limits(
    position const& p,
    ch_search_limits& limits)
{
    int rem = limits.remtime[p.current_turn];
    if(rem > 0)
    {
        static constexpr int const ASSUMED_LENGTH = 40;
        rem = std::max(20, std::max(
            rem / ASSUMED_LENGTH,
            rem - rem * std::max(p.ply, ASSUMED_LENGTH) / ASSUMED_LENGTH));
        limits.mstime = rem;
    }
}

struct search_data
{
    uint64_t nodes;
    trans_table* tt;
    history_heuristic* hh;
    position p;
    std::array<move, MAX_VARIATION> best;
    std::array<move, MAX_VARIATION> mvstack;
    std::array<std::array<move, CH_NUM_KILLERS>, MAX_VARIATION> killers;
    int score;
    int depth;
    int seldepth;
    uint32_t start_time;
    uint32_t info_time;
    ch_search_limits limits;
    int data_index;
    int num_threads;
    std::atomic_bool stop, stopped, kill, killed;

    bool time_limit_reached() const
    {
        if(data_index != 0) return false;
        if(limits.mstime > 0)
            if(int(get_ms() - start_time) >= limits.mstime)
                return true;
        return false;
    }

    bool node_limit_reached() const
    {
        return limits.nodes > 0 && nodes >= limits.nodes;
    }

    bool any_limit_reached() const
    {
        return
            stop ||
            depth >= limits.depth ||
            node_limit_reached() ||
            time_limit_reached();
    }
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
#if CH_ENABLE_COUNTERMOVE_HEURISTIC
    move countermove = d.hh->get_countermove(parent_move);
#endif

    for(move& mv : mvs)
    {
        int x;

        if(mv.is_similar_to(hashmove))
        {
            x = 127;
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
        else if(mv.is_killer(killers))
        {
            x = 62;
        }
#if CH_ENABLE_COUNTERMOVE_HEURISTIC
        else if(mv.is_similar_to(countermove))
        {
            x = 61;
        }
#endif
        else if(mv.to() == parent_move.to())
        {
            x = 60 - MOVE_ORDER_PIECEVALS[d.p.pieces[mv.from()]];
        }
        else
        {
            x = 0;
#if CH_ENABLE_HISTORY_HEURISTIC
            x += d.hh->get_hh_score(mv);
#endif
        }

        mv.sort_key() = uint8_t(x);
    }

    mvs.sort();
}

template<acceleration accel>
static void send_info(search_data& d, bool force = false)
{
    {
        static constexpr uint32_t const MIN_INFO_DELAY_MS = 200;
        uint32_t ct = get_ms();
        uint32_t dt = ct - d.info_time;
        d.info_time = ct;
        if(!force && dt < MIN_INFO_DELAY_MS)
            return;
    }
    int mstime = int(d.info_time - d.start_time);

    // extract principal variation
    move pv[256];
    move cur = d.best[0];
    cur.sort_key() = 0;
    pv[0] = cur;
    int pvlen = 1;
    while(pvlen < d.depth)
    {
        move_list mvs;
        mvs.generate<accel>(d.p.current_turn, d.p);
        bool found = false;
        for(move mv : mvs)
        {
            if(mv == cur)
            {
                found = true;
                break;
            }
        }
        if(!found) break;
        d.p.do_move<accel>(cur);
        hash_info i;
        if(!d.tt->get(d.p.hash(), i))
        {
            d.p.undo_move<accel>(cur);
            break;
        }
        cur = i.best;
        pv[pvlen++] = cur;
    }
    for(int n = pvlen - 2; n >= 0; --n)
        d.p.undo_move<accel>(pv[n]);

    int nps = mstime ? div_nps_mstime(d.nodes, mstime) : 0;

    search_info(
        d.depth,
        std::max(d.depth, d.seldepth),
        d.nodes,
        mstime,
        d.score,
        nps,
        (ch_move*)pv,
        pvlen
    );
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

    ++d.nodes;

    move hash_move = NULL_MOVE;
    int stand_pat;

//#if CH_ENABLE_HASH
//    // transposition table lookup
//    {
//        hash_info i;
//        if(d.tt->get(d.p.hash(), i) && i.depth >= depth)
//        {
//            int v = i.value;
//            if(v >= MATE_SCORE - 256)
//                v -= height;
//            if(v <= MATED_SCORE + 256)
//                v += height;
//            if(i.flag == hash_info::EXACT)
//                return v;
//            else if(i.flag == hash_info::LOWER)
//                alpha = std::max<int>(alpha, v);
//            else if(i.flag == hash_info::UPPER)
//                beta = std::min<int>(beta, v);
//            if(alpha >= beta)
//                return v;
//            hash_move = i.get_best();
//        }
//    }
//#endif

    {
        evaluator<accel> e;
        stand_pat = e.evaluate_first(p, c);
        if(stand_pat >= beta + 100)
            return beta;
        stand_pat = e.evaluate_second(p, c);
    }

    if(stand_pat >= beta)
        return beta;

    d.seldepth = std::max(d.seldepth, height);

#if CH_COLOR_TEMPLATE
    mvs.generate<c, accel, MOVEGEN_QUIESCENCE>(p);
#else
    mvs.generate<accel, MOVEGEN_QUIESCENCE>(c, p);
#endif

    alpha = std::max(alpha, stand_pat);

    bool endgame = (
        p.stack().piece_vals[WHITE] + p.stack().piece_vals[BLACK] < 1200);
    for(int n = 0; n < mvs.size(); ++n)
    {
        move& mv = mvs[n];

        if(mv.is_similar_to(hash_move))
        {
            mv.sort_key() = 255;
            continue;
        }
        
        // delta pruning
        if(
            !endgame &&
            !mv.is_promotion() &&
            stand_pat + PIECE_VALUES[p.pieces[mv.to()]] + 200 < alpha
            )
        {
            std::swap(mv, mvs[mvs.size() - 1]);
            mvs.pop_back();
            --n;
            continue;
        }

        int v = see<accel>(mv, p);
        if(v < 0)
        {
            std::swap(mv, mvs[mvs.size() - 1]);
            mvs.pop_back();
            --n;
            continue;
        }
        else
            mv.sort_key() = uint8_t(v);
    }
    mvs.sort();

    //{
    //    bool promotion = false;
    //    for(move mv : mvs)
    //        if(mv.is_promotion()) { promotion = true; break; }
    //    int delta = 975;
    //    if(promotion) delta += 775;
    //    if(stand_pat + delta < alpha)
    //        return alpha;
    //}

    for(move mv : mvs)
    {
        int value;
        p.do_move<accel>(mv);

#if CH_COLOR_TEMPLATE
        value = -quiesce<opposite(c), accel>(
#else
        value = -quiesce<accel>(opposite(c),
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

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel> static int negamax(
#else
template<acceleration accel> static int negamax(color c,
#endif
    search_data& d,
    int depth, int alpha, int beta, int height)
{
    move hash_move = NULL_MOVE;
    int alpha_orig = alpha;

    d.best[height] = NULL_MOVE;

    if(height > 0 && d.stop)
        return 0;

    if(height > 0 && d.node_limit_reached())
    {
        d.stop = 1;
        return 0;
    }

    if(height > 0 && depth > 4 && d.time_limit_reached())
    {
        d.stop = true;
        return 0;
    }

    if(height > 0)
    {
        if(d.p.is_draw_by_fifty_move_rule())
            return 0;
        int count = d.p.repetition_count();
        if(count >= 1)
            return 0;
        if(d.p.is_draw_by_insufficient_material())
            return 0;
    }

    // mate distance pruning
    if(height > 0)
    {
        int m_alpha = std::max(alpha, MATED_SCORE + height);
        int m_beta = std::min(beta, MATE_SCORE - height);
        if(m_alpha >= m_beta)
            return m_alpha;
    }

#if CH_ENABLE_HASH
    // transposition table lookup
    if(depth > 1)
    {
        hash_info i;
        if(d.tt->get(d.p.hash(), i))
        {
            hash_move = i.best;
            if(i.depth >= depth)
            {
                int v = i.value;
                if(v >= MATE_SCORE - 256)
                    v -= height;
                if(v <= MATED_SCORE + 256)
                    v += height;
#if 1
                if(i.flag == hash_info::EXACT ||
                    (i.flag == hash_info::LOWER && v >= beta) ||
                    (i.flag == hash_info::UPPER && v <= alpha))
                {
                    d.best[height] = hash_move;
                    return v;
                }
#else
                if(i.flag == hash_info::EXACT)
                {
                    d.best[height] = hash_move;
                    return v;
                }
                else if(i.flag == hash_info::LOWER)
                    alpha = std::max<int>(alpha, v);
                else if(i.flag == hash_info::UPPER)
                    beta = std::min<int>(beta, v);
                if(alpha >= beta)
                {
                    d.best[height] = hash_move;
                    return v;
                }
#endif
            }
        }
    }
#endif

    int static_eval;
    {
        evaluator<accel> e;
        static_eval = e.evaluate_first(d.p, c);

        // TODO: prune here?

        static_eval = e.evaluate_second(d.p, c);
    }

    if(height >= MAX_VARIATION)
        return static_eval;

    move_list mvs;

#if CH_ENABLE_PROBCUT_PRUNING
    // Ethereal-style Probcut
    int const PROBCUT_MARGIN = 100;
    if(
        //node_type != NODE_PV &&
        height > 0 &&
        depth >= 5 &&
        beta < MATE_SCORE - 256 &&
        beta > MATED_SCORE + 256 &&
        static_eval + d.p.best_case_move_value() >= beta + PROBCUT_MARGIN)
    {
        int rbeta = std::min(beta + PROBCUT_MARGIN, MATE_SCORE - 257);
        mvs.generate<accel, MOVEGEN_QUIESCENCE>(c, d.p);
        for(move mv : mvs)
        {
            int v;
            d.mvstack[height] = mv;
            d.p.do_move<accel>(mv);
#if CH_COLOR_TEMPLATE
            v = -negamax<opposite(c), accel>(
#else
            v = -negamax<accel>(opposite(c), 
#endif
                d, depth - 4, -rbeta, -rbeta + 1, height + 1);
            d.p.undo_move<accel>(mv);
            if(v >= rbeta)
                return beta;// v;
        }
    }
#endif

#if CH_COLOR_TEMPLATE
    mvs.generate<c, accel>(d.p);
#else
    mvs.generate<accel>(c, d.p);
#endif

    bool in_check = d.p.in_check;
    if(mvs.empty())
        return in_check ? MATED_SCORE + height : 0;

#if CH_ENABLE_RAZORING
    if(height > 0)
    {
        static constexpr int RAZOR_MARGIN = 100;
        static constexpr int DEEP_RAZOR_MARGIN = 200;
        if(!in_check &&
            ((depth <= 2 && static_eval + RAZOR_MARGIN <= alpha) ||
#if CH_ENABLE_DEEP_RAZORING
            (depth <= 4 && static_eval + DEEP_RAZOR_MARGIN <= alpha) ||
#endif
            0))
        {
#if CH_COLOR_TEMPLATE
            return quiesce<c, accel>(d, depth, alpha, beta, height);
#else
            return quiesce<accel>(c, d, depth, alpha, beta, height);
#endif
            //--depth;
        }
    }
#endif

#if CH_ENABLE_FUTILITY_PRUNING
    // futility pruning
    bool fprune = false;
    if(
        //node_type != NODE_PV &&
        height > 0 &&
        !in_check &&
        depth <= 8 &&
        beta < MATE_SCORE - 256 &&
        beta > MATED_SCORE + 256 &&
        alpha < MATE_SCORE - 256 &&
        alpha > MATED_SCORE + 256 &&
        1)
    {
        if(static_eval >= beta + 64 * depth)
//#if CH_COLOR_TEMPLATE
//            return quiesce<c, accel>(d, depth, alpha, beta, height);
//#else
//            return quiesce<accel>(c, d, depth, alpha, beta, height);
//#endif
            return static_eval;
        if(static_eval + 64 * depth <= alpha)
            fprune = true;
    }
#endif

#if CH_ENABLE_NULL_MOVE
    // null move pruning
    if(height > 0 && !in_check && depth >= 5 &&
        (height < 1 || d.mvstack[height - 1] != NULL_MOVE) &&
        //(height < 2 || d.mvstack[height - 2] != NULL_MOVE) &&
        d.p.has_piece_better_than_pawn(c)
        )
    {
        d.mvstack[height] = NULL_MOVE;
        d.p.do_null_move();
#if CH_COLOR_TEMPLATE
        int value = -negamax<opposite(c), accel>(
#else
        int value = -negamax<accel>(opposite(c),
#endif
            d, depth >= 8 ? depth / 2 : depth - 4, -beta, -beta + 1, height + 1);
            //d, depth - 4, -beta, -alpha, height + 1);
        d.p.undo_null_move();
        if(value >= beta)
        {
            if(value >= MATE_SCORE - 256)
                value = beta;
            return value;
        }
    }
#endif

#if CH_ENABLE_IID
    // internal iterative deepening
    if(hash_move == NULL_MOVE && depth > 5)
    {
#if CH_COLOR_TEMPLATE
        negamax<c, accel>(
#else
        negamax<accel>(c,
#endif
            d, 3, alpha, beta, height);
        hash_move = d.best[height];
    }
#endif

    bool const tail = (depth <= 1);

#if CH_ENABLE_MOVE_PICKER
    move_picker<accel> picker(
        mvs, d.p, hash_move,
        d.killers[height], d.hh, depth);
#else
    if(tail)
    {
        for(int i = 0, n = 0; n < mvs.size(); ++n)
        {
            if(mvs[n].is_similar_to(hash_move)
#if CH_ENABLE_KILLERS
                || mvs[n].is_killer(d.killers[height])
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
#endif

    int value = MIN_SCORE;
    int best = MIN_SCORE;
    move best_move = NULL_MOVE;
    int hh_increment = depth * depth;

#if CH_ENABLE_HISTORY_HEURISTIC
    //if(!tail)
    //    for(move mv : mvs)
    //        if(!d.p.move_is_capture(mv))
    //            d.hh->increment_bf(mv, hh_increment);
#endif

    if(in_check)
        ++depth;
#if CH_ENABLE_MOVE_PICKER
    for(int n = 0;; ++n)
    {
        int stage;
        move mv = picker.get(stage);
        bool const quiet_or_losing = (
            stage >= move_picker<accel>::STAGE_QUIETS);
        bool const losing = (
            stage >= move_picker<accel>::STAGE_LOSING_CAPTURES);

        if(mv == NULL_MOVE)
            break;
#else
    for(int n = 0; n < mvs.size(); ++n)
    {
        move mv = mvs[n];
#endif
        bool const quiet = !d.p.move_is_promotion_or_capture(mv);
        bool const check = d.p.move_is_check(mv);
        bool const castle = mv.is_castle();

        bool const dont_reduce = !quiet || in_check || check || castle;

#if CH_ENABLE_HISTORY_HEURISTIC
        if(!tail && quiet)
            d.hh->increment_bf(mv, hh_increment);
#endif

        int newdepth = depth;
        if(castle)
            ++newdepth;

#if CH_ENABLE_FUTILITY_PRUNING
        if(!dont_reduce && fprune && n > 0)
            continue;
#endif

#if CH_ENABLE_LATE_MOVE_PRUNING
        if(!dont_reduce && newdepth <= 4 && n >= newdepth * 7)
            continue;
#endif

        d.mvstack[height] = mv;
        d.p.do_move<accel>(mv);

        int v = alpha + 1;
#if CH_ENABLE_LATE_MOVE_REDUCTION
        if(!dont_reduce && newdepth >= 3 && n >= 4)
        {
            int reduction = 2;
            if(n >= 6) reduction += newdepth / 3;
            reduction = std::min(newdepth - 2, std::max(1, reduction));
#if CH_COLOR_TEMPLATE
            v = -negamax<opposite(c), accel>(
#else
            v = -negamax<accel>(opposite(c),
#endif
                d, newdepth - reduction, -alpha - 1, -alpha, height + 1);

        }
#endif

        if(newdepth <= 1)
        {
#if CH_ENABLE_QUIESCENCE
            if(CH_QUIESCE_ON_QUIETS || !quiet)
            {
#if CH_COLOR_TEMPLATE
                v = -quiesce<opposite(c), accel>(
#else
                v = -quiesce<accel>(opposite(c),
#endif
                    d, newdepth - 1, -beta, -alpha, height + 1);
            }
            else
#endif
            {
                ++d.nodes;
                v = evaluator<accel>{}.evaluate(d.p, c);
            }
            value = std::max(value, v);
        }
        else if(v > alpha)
        {
#if CH_COLOR_TEMPLATE
            value = std::max(value, -negamax<opposite(c), accel>(
#else
            value = std::max(value, -negamax<accel>(opposite(c),
#endif
                d, newdepth - 1, -beta, -alpha, height + 1));
        }
        else
        {
            value = std::max(value, v);
        }

        d.p.undo_move<accel>(mv);

        if(value > best)
        {
            best = value;
            best_move = mv;
            if(value > alpha)
            {
                alpha = value;
                if(alpha >= beta)
                {
                    // history updates
#if CH_ENABLE_KILLERS
                    {
                        auto& k = d.killers[height];
                        if(quiet &&
                            !mv.is_similar_to(hash_move) &&
                            !mv.is_killer(k))
                        {
                            for(int m = 0; m < CH_NUM_KILLERS - 1; ++m)
                                k[m + 1] = k[m];
                            k[0] = mv;
                        }
                    }
#endif
#if CH_ENABLE_HISTORY_HEURISTIC
                    if(quiet)
                        d.hh->increment_hh(mv, hh_increment);
#endif
#if CH_ENABLE_COUNTERMOVE_HEURISTIC
                    if(quiet && height > 0)
                        d.hh->set_countermove(d.p.stack().prev_move, mv);
#endif
                    break;
                }
            }
        }
    }

#if CH_ENABLE_HASH
    // transposition table store
    if(!d.stop && depth > 1)
    {
        hash_info i;
        i.value = int16_t(
            value <= MATED_SCORE + 256 ? value - height :
            value >= MATE_SCORE - 256 ? value + height :
            value);
        if(value <= alpha_orig)
            i.flag = hash_info::UPPER;
        else if(value >= beta)
            i.flag = hash_info::LOWER;
        else
            i.flag = hash_info::EXACT;
        i.depth = int8_t(depth);
        i.best.set(best_move);
        i.age = uint8_t(d.p.age);
        i.pro_piece = uint8_t(best_move.is_promotion() ?
            best_move.promotion_piece() : EMPTY);

        //if(//d.p.hash() == 2840919199511080229 &&
        //    (i.value == 732 || i.value == -732))
        //    print_position(d.p), __debugbreak();

        d.tt->put(d.p.hash(), i);
    }
    if(!d.stop)
        d.best[height] = best_move;
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
            d, depth, alpha, beta, 0);
    }
    else
    {
        return negamax<BLACK, accel>(
            d, depth, alpha, beta, 0);
    }
#else
    return negamax<accel>(d.p.current_turn,
        d, depth, alpha, beta, 0);
#endif
}

template<acceleration accel>
static int aspiration_window(
    search_data& d, int depth, int prev_score)
{
    // TODO: move constants to a common location
    constexpr int ASPIRATION_BASE_DELTA = 10;
    constexpr int ASPIRATION_MIN_DEPTH = 4;

    int delta = ASPIRATION_BASE_DELTA;
    int alpha = depth < ASPIRATION_MIN_DEPTH ? MIN_SCORE :
        std::max(MIN_SCORE, prev_score - delta);
    int beta = depth < ASPIRATION_MIN_DEPTH ? MAX_SCORE :
        std::min(MAX_SCORE, prev_score + delta);

    while(!d.stop)
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

    return 0;
}

static int get_next_depth(
    search_data const& d, int default_depth)
{
    search_data const* ds = &d - d.data_index;
    int num_above_depth = 0;
    int max_reached_depth = 0;
    for(int n = 0; n < d.num_threads; ++n)
    {
        if(n == d.data_index) continue;
        if(ds[n].depth >= default_depth)
            ++num_above_depth;
        max_reached_depth = std::max(max_reached_depth, ds[n].depth);
    }
    if(num_above_depth >= (d.num_threads + 1) / 2 ||
        default_depth < max_reached_depth - 1)
        return get_next_depth(d, default_depth + 1);
    return default_depth;
}

template<acceleration accel>
static move iterative_deepening(
    search_data& d, position const& p)
{
    uint64_t prev_nodes = 0;
    move best = NULL_MOVE;
    int depth = 1;
    int prev_score;
    d.p = p;
    d.nodes = 0;
    d.info_time = 0;
    prev_score = evaluator<accel>{}.evaluate(p, p.current_turn);
    while(depth <= MAX_VARIATION)
    {
        d.seldepth = 0;
        int score = aspiration_window<accel>(d, depth, prev_score);
        bool force = (score != prev_score || d.best[0] != best);
        prev_nodes = d.nodes;
        prev_score = score;
        d.depth = depth;
        d.score = prev_score;
        if(!d.stop) best = d.best[0];
        if(d.any_limit_reached()) break;
        if(d.data_index == 0) send_info<accel>(d, force);
        depth = get_next_depth(d, depth + 1);
    }
    if(d.data_index == 0 && !d.stop)
        send_info<accel>(d, true);

    return best;
}

}
