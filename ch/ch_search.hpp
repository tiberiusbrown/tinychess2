#pragma once

#include "ch.h"
#include "ch_internal.hpp"
#include "ch_evaluate.hpp"
#include "ch_genmoves.hpp"
#include "ch_history_heuristic.hpp"
#include "ch_move_picker.hpp"
#include "ch_position.hpp"
#include "ch_params.hpp"

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
            time_limit_reached() ||
            0;
    }
};

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
    {
        evaluator<accel> e;
        stand_pat = e.evaluate(p, c);
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

    bool const endgame = (
        p.stack().piece_vals[WHITE] + p.stack().piece_vals[BLACK] <
        position::ST_PIECE_VALUES[PAWN] * 12);
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
            stand_pat + PIECE_VALUES_ESTIMATE[p.pieces[mv.to()]] +
            PIECE_VALUES_ESTIMATE[PAWN] < alpha
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

        // TODO: crazy idea: try removing this block to see if stuff improves
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

    bool tthit = false;
    hash_info ttinfo = { 0 };
    int ttvalue = 0;
#if CH_ENABLE_HASH
    // transposition table lookup
    if(depth > 1)
    {
        if(d.tt->get(d.p.hash(), ttinfo))
        {
            tthit = true;
            hash_move = ttinfo.best;
            if(ttinfo.depth >= depth)
            {
                ttvalue = ttinfo.value;
                if(ttvalue >= MATE_SCORE - 256)
                    ttvalue -= height;
                if(ttvalue <= MATED_SCORE + 256)
                    ttvalue += height;
#if 1
                if(ttinfo.flag == hash_info::EXACT ||
                    (ttinfo.flag == hash_info::LOWER && ttvalue >= beta) ||
                    (ttinfo.flag == hash_info::UPPER && ttvalue <= alpha))
                {
                    d.best[height] = hash_move;
                    return ttvalue;
                }
#else
                if(ttinfo.flag == hash_info::EXACT)
                {
                    d.best[height] = hash_move;
                    return ttvalue;
                }
                else if(ttinfo.flag == hash_info::LOWER)
                    alpha = std::max<int>(alpha, ttvalue);
                else if(ttinfo.flag == hash_info::UPPER)
                    beta = std::min<int>(beta, ttvalue);
                if(alpha >= beta)
                {
                    d.best[height] = hash_move;
                    return ttvalue;
                }
#endif
            }
        }
    }
#endif

    int static_eval;
    {
        evaluator<accel> e;
        static_eval = e.evaluate(d.p, c);
    }

    if(height >= MAX_VARIATION)
        return static_eval;

    move_list mvs;

    if(CH_ENABLE_PROBCUT_PRUNING)
    {
        // Ethereal-style Probcut
        if(
            alpha + 1 < beta &&
            height > 0 &&
            depth >= PROBCUT_MIN_DEPTH &&
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
                    return v;
            }
        }
    }

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
        if(!in_check &&
            ((depth <= RAZOR_MAX_DEPTH && static_eval + RAZOR_MARGIN <= alpha) ||
#if CH_ENABLE_DEEP_RAZORING
            (depth <= DEEP_RAZOR_MAX_DEPTH && static_eval + DEEP_RAZOR_MARGIN <= alpha) ||
#endif
            0))
        {
#if CH_ENABLE_RAZOR_QSEARCH
#if CH_COLOR_TEMPLATE
            return quiesce<c, accel>(d, depth, alpha, beta, height);
#else
            return quiesce<accel>(c, d, depth, alpha, beta, height);
#endif
#else
            --depth;
#endif
        }
    }
#endif

    // futility pruning
    bool fprune = false;
    if(
        CH_ENABLE_FUTILITY_PRUNING &&
        height > 0 &&
        !in_check &&
        depth <= FUTILITY_PRUNING_MAX_DEPTH &&
        beta < MATE_SCORE - 256 &&
        beta > MATED_SCORE + 256 &&
        alpha < MATE_SCORE - 256 &&
        alpha > MATED_SCORE + 256 &&
        1)
    {
        if(static_eval >= reverse_futility_margin(depth, beta))
        {
            return static_eval;
        }
        if(static_eval <= futility_margin(depth, alpha))
            fprune = true;
    }

    // null move pruning
    bool mate_threat = false;
    if(
        CH_ENABLE_NULL_MOVE &&
        height > 0 &&
        !in_check &&
        depth >= NULL_MOVE_MIN_DEPTH &&
        (height < 1 || d.mvstack[height - 1] != NULL_MOVE) &&
        //(height < 2 || d.mvstack[height - 2] != NULL_MOVE) &&
        d.p.has_piece_better_than_pawn(c) &&
        (!tthit || !(ttinfo.flag == TTFLAG_UPPER) || ttvalue >= beta) &&
        1)
    {
        d.mvstack[height] = NULL_MOVE;
        d.p.do_null_move();
        int newd = null_move_depth(depth, static_eval, beta);
#if CH_COLOR_TEMPLATE
        int value = -negamax<opposite(c), accel>(
#else
        int value = -negamax<accel>(opposite(c),
#endif
            d, newd, -beta, -beta + 1, height + 1);
        d.p.undo_null_move();
        if(value >= beta)
        {
            if(value >= MATE_SCORE - 256)
                value = beta;
            return value;
        }
        else if(value <= CH_MATED_SCORE + 256)
            mate_threat = true;
    }

    // internal iterative deepening
    if(CH_ENABLE_IID && hash_move == NULL_MOVE && depth >= IID_MIN_DEPTH)
    {
#if CH_COLOR_TEMPLATE
        negamax<c, accel>(
#else
        negamax<accel>(c,
#endif
            d, iid_depth(depth), alpha, beta, height);
        hash_move = d.best[height];
    }

    move_picker<accel> picker(
        mvs, d.p, hash_move,
        d.killers[height], d.hh, depth);

    bool const tail = (depth <= 1);
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

    if(mate_threat || mvs.size() <= 2)
        ++depth;

    for(int n = 0;; ++n)
    {
        int stage;
        move mv = picker.get(stage);
#if 0
        bool const quiet_or_losing = (
            stage >= move_picker<accel>::STAGE_QUIETS);
        bool const losing = (
            stage >= move_picker<accel>::STAGE_LOSING_CAPTURES);
#endif

        if(mv == NULL_MOVE)
            break;
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

        if(CH_ENABLE_FUTILITY_PRUNING && !dont_reduce && fprune && n > 0)
            continue;

        if(CH_ENABLE_LATE_MOVE_PRUNING &&
            !dont_reduce &&
            newdepth <= LMP_MAX_DEPTH &&
            n >= lmp_min_n(newdepth))
            continue;

        d.mvstack[height] = mv;
        d.p.do_move<accel>(mv);

        int v = alpha + 1;
        if(CH_ENABLE_LATE_MOVE_REDUCTION &&
            !dont_reduce &&
            newdepth >= LMR_MIN_DEPTH &&
            n >= lmr_min_n(newdepth))
        {
            int const R = lmr_reduction(newdepth, n);
#if CH_COLOR_TEMPLATE
            v = -negamax<opposite(c), accel>(
#else
            v = -negamax<accel>(opposite(c),
#endif
                d, newdepth - R, -alpha - 1, -alpha, height + 1);

        }

        if(newdepth <= 1)
        {
#if CH_COLOR_TEMPLATE
            v = -quiesce<opposite(c), accel>(
#else
            v = -quiesce<accel>(opposite(c),
#endif
                d, newdepth - 1, -beta, -alpha, height + 1);
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

        delta = aspiration_next_delta(delta);
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
