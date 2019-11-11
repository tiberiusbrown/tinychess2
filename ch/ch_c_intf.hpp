#pragma once

#include "ch.h"
#include "ch_cpuid.hpp"
#include "ch_evaluate.hpp"
#include "ch_hash.hpp"
#include "ch_history_heuristic.hpp"
#include "ch_init.hpp"
#include "ch_internal.hpp"
#include "ch_magic.hpp"
#include "ch_move_list.hpp"
#include "ch_position.hpp"
#include "ch_search.hpp"

#include <atomic>

extern "C"
{
struct ch_game
{
    ch::trans_table tt;
    ch::position pos;
    ch::search_data sd[CH_MAX_THREADS];
    ch::move_list moves;
    ch::history_heuristic hh;
    int redo_ply;
    std::atomic_int num_threads;
};
}

static void update_moves(ch_game* g)
{
    using namespace ch;
#if CH_ENABLE_AVX
    if(has_avx())
        g->moves.generate<ACCEL_AVX>(g->pos.current_turn, g->pos);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        g->moves.generate<ACCEL_SSE>(g->pos.current_turn, g->pos);
    else
#endif
#if CH_ENABLE_UNACCEL
        g->moves.generate<ACCEL_UNACCEL>(g->pos.current_turn, g->pos);
#else
    { }
#endif
}

template<ch::acceleration accel>
static int ch_qsearch_helper(ch_game* g)
{
    using namespace ch;
    g->sd[0].p = g->pos;
#if CH_COLOR_TEMPLATE
    if(g->pos.current_turn == WHITE)
        return ch::quiesce<WHITE, accel>(g->sd[0], 0, MATED_SCORE, MATE_SCORE, 0);
    else
        return -ch::quiesce<BLACK, accel>(g->sd[0], 0, MATED_SCORE, MATE_SCORE, 0);
#else
    int v = ch::quiesce<accel>(
        g->pos.current_turn, g->sd[0], 0, INT_MIN, INT_MAX, 0);
    return g->pos.current_turn == WHITE ? v : -v;
#endif
}

extern "C"
{

void CHAPI ch_init(ch_system_info const* info)
{
    if(info)
        ch::system = *info;
    else
        ch::memzero(&ch::system, sizeof(ch::system));
    ch::init();
    ch::init_cpuid();
#if CH_ENABLE_MAGIC
    ch::init_magic();
#endif
    ch::init_hashes();
    ch::init_evaluator();
}

ch_game* CHAPI ch_create()
{
    using namespace ch;
    ch_game* g = (ch_game*)aligned_alloc(sizeof(ch_game), alignof(ch_game));
    g->tt.set_memory(nullptr, 0);
    for(int n = 0; n < CH_MAX_THREADS; ++n)
    {
        g->sd[n].tt = &g->tt;
        g->sd[n].hh = &g->hh;
        g->sd[n].nodes = 0;
        g->sd[n].stop = true;
        g->sd[n].stopped = false;
        g->sd[n].kill = false;
        g->sd[n].killed = false;
        g->sd[n].data_index = n;
        g->sd[n].num_threads = 1;
    }
    g->num_threads = 0;
    g->pos.new_game();
    update_moves(g);
    ch_clear_caches(g);
    return g;
}

void ch_destroy(ch_game* g)
{
    ch::aligned_dealloc(g);
}

static void helper_start_search(ch_game* g, ch::search_data& d)
{
    ch::move mv;
    d.stopped = false;
#if CH_ENABLE_AVX
    if(ch::has_avx())
        mv = ch::iterative_deepening<ch::ACCEL_AVX>(d, g->pos);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        mv = ch::iterative_deepening<ch::ACCEL_SSE>(d, g->pos);
    else
#endif
#if CH_ENABLE_UNACCEL
        mv = ch::iterative_deepening<ch::ACCEL_UNACCEL>(d, g->pos);
#else
        mv = ch::NULL_MOVE;
#endif
    if(d.data_index == 0 && ch::system.best_move)
        ch::system.best_move(mv);
}

void CHAPI ch_thread_start(ch_game* g)
{
    int index = g->num_threads.fetch_add(1, std::memory_order_relaxed);
    if(index >= CH_MAX_THREADS)
    {
        g->num_threads -= 1;
        return;
    }
    ch::search_data& d = g->sd[index];

    for(;;)
    {
        d.stop = true;
        d.stopped = true;
        while(d.stop && !d.kill)
            ch::thread_yield();
        if(d.kill) break;
        d.stopped = false;
        helper_start_search(g, d);
    }
    d.killed = true;
}

static void helper_stop_threads(ch_game* g)
{
    int n = g->num_threads;
    for(int i = n - 1; i >= 0; --i)
        g->sd[i].stop = true;
    int num_stopped = 0;
    do
    {
        for(int i = n - 1; i >= 0; --i)
        {
            if(g->sd[i].stopped)
            {
                ++num_stopped;
                break;
            }
        }
    } while(num_stopped < n);
}

static void helper_start_threads(ch_game* g)
{
    int n = g->num_threads;
    for(int i = 0; i < n; ++i)
    {
        g->sd[i].num_threads = n;
        g->sd[i].depth = 0;
    }
    while(n-- > 0)
        g->sd[n].stop = false;
}

void CHAPI ch_kill_threads(ch_game* g)
{
    int n = g->num_threads;
    for(int i = n - 1; i >= 0; --i)
        g->sd[i].kill = g->sd[i].stop = true;
    int num_killed = 0;
    do
    {
        for(int i = n - 1; i >= 0; --i)
        {
            if(g->sd[i].killed)
            {
                g->sd[i].killed = g->sd[i].kill = false;
                ++num_killed;
                break;
            }
        }
    } while(num_killed < n);
    g->num_threads = 0;
}

void CHAPI ch_set_hash(ch_game* g, void* mem, int size_megabyte_log2)
{
    g->tt.set_memory(mem, size_megabyte_log2);
}

void CHAPI ch_clear_caches(ch_game* g)
{
    g->tt.clear();
    g->hh.clear();
    for(int n = 0; n < CH_MAX_THREADS; ++n)
        memzero32(&g->sd[n].killers[0][0], sizeof(g->sd[n].killers) / 4);
}

void CHAPI ch_new_game(ch_game* g)
{
    g->pos.new_game();
    g->redo_ply = 0;
    update_moves(g);
}

void CHAPI ch_load_fen(ch_game* g, char const* fen)
{
    g->pos.load_fen(fen);
    g->redo_ply = 0;
    update_moves(g);
}

void CHAPI ch_do_move(ch_game* g, ch_move m)
{
    using namespace ch;

    // validate move
    {
        int valid = 0;
        move mv(m);
        mv.sort_key() = 0;
        for(move const& tm : g->moves)
            if(mv == tm)
                valid = 1;
        if(!valid)
        {
            //printf("uci info string invalid move: %u %s\n",
            //    m, move(m).extended_algebraic());
            return;
        }
    }

#if CH_ENABLE_AVX
    if(has_avx())
        g->pos.do_move<ACCEL_AVX>(m);
    else
#endif
#if CH_ENABLE_SSE
    if(has_sse())
        g->pos.do_move<ACCEL_SSE>(m);
    else
#endif
#if CH_ENABLE_UNACCEL
        g->pos.do_move<ACCEL_UNACCEL>(m);
#else
    { }
#endif

    g->pos.stack_reset();
    g->pos.age += 1;
    g->redo_ply = g->pos.ply;
    update_moves(g);
}

static ch_move convert_move(ch_game* g, char const* str)
{
    using namespace ch;
    move m = NULL_MOVE;

    m += move::from(('8' - str[1]) * 8 + str[0] - 'a');
    m += move::to(('8' - str[3]) * 8 + str[2] - 'a');
    switch(str[4])
    {
    case 'n':
        m += move::pawn_promotion(g->pos.current_turn + KNIGHT);
        break;
    case 'b':
        m += move::pawn_promotion(g->pos.current_turn + BISHOP);
        break;
    case 'r':
        m += move::pawn_promotion(g->pos.current_turn + ROOK);
        break;
    case 'q':
        m += move::pawn_promotion(g->pos.current_turn + QUEEN);
        break;
    default:
        // check for castling or en passant moves
        for(int n = 0; ; ++n)
        {
            if(n >= g->moves.size())
                return 0;
            if((m & 0xFFFF) == (g->moves[n] & 0xFFFF))
            {
                m = g->moves[n];
                break;
            }
        }
        break;
    }

    return m;
}

void CHAPI ch_do_move_str(ch_game* g, char const* str)
{
    ch_do_move(g, convert_move(g, str));
}

ch_move CHAPI ch_last_move(ch_game* g)
{
    if(g->pos.ply == 0) return ch::NULL_MOVE;
    return g->pos.move_history[g->pos.ply - 1];
}

ch_move CHAPI ch_undo_move(ch_game* g)
{
    int p = g->pos.ply - 1;
    if(p < 0) return ch::NULL_MOVE;
    ch_new_game(g);
    for(int n = 0; n < p; ++n)
        ch_do_move(g, g->pos.move_history[n]);
    return g->pos.move_history[p];
}

ch_move CHAPI ch_redo_move(ch_game* g)
{
    if(g->pos.ply >= g->redo_ply) return ch::NULL_MOVE;
    ch::move mv = g->pos.move_history[g->pos.ply];
    ch_do_move(g, mv);
    return mv;
}

int CHAPI ch_move_fr_sq(ch_move mv)
{
    return ch::move(mv).from();
}

int CHAPI ch_move_to_sq(ch_move mv)
{
    return ch::move(mv).to();
}

int CHAPI ch_num_moves(ch_game* g)
{
    return g->moves.size();
}

ch_move CHAPI ch_get_move(ch_game* g, int n)
{
    if(n < 0 || n >= g->moves.size()) return ch::NULL_MOVE;
    return g->moves[n];
}

int CHAPI ch_evaluate(ch_game* g)
{
    using namespace ch;
    evaluator<ACCEL_UNACCEL> e;
    return e.evaluate(g->pos, g->pos.current_turn);
}

int CHAPI ch_evaluate_white(ch_game* g)
{
    using namespace ch;
    evaluator<ACCEL_UNACCEL> e;
    return e.evaluate(g->pos, WHITE);
}

int CHAPI ch_qsearch(ch_game* g)
{
    using namespace ch;
#if CH_ENABLE_AVX
    if(has_avx())
        return ch_qsearch_helper<ACCEL_AVX>(g);
    else
#endif
#if CH_ENABLE_SSE
    if(has_sse())
        return ch_qsearch_helper<ACCEL_SSE>(g);
    else
#endif
#if CH_ENABLE_UNACCEL
        return ch_qsearch_helper<ACCEL_UNACCEL>(g);
#else
        return 0ull;
#endif
}

void CHAPI ch_search(ch_game* g, ch_search_limits const* limits)
{
    using namespace ch;
    uint32_t start_time = get_ms();
    helper_stop_threads(g);
    g->redo_ply = g->pos.ply;
#if CH_ENABLE_HISTORY_HEURISTIC
    g->hh.clear();
#endif
    ch_search_limits newlimits = *limits;
    if(newlimits.depth <= 0) newlimits.depth = INT_MAX;
    //if(newlimits.mstime <= 0) newlimits.mstime = INT_MAX;
    update_limits(g->pos, newlimits);
    for(int n = 0; n < CH_MAX_THREADS; ++n)
    {
        g->sd[n].start_time = start_time;
        g->sd[n].limits = newlimits;
    }
    if(g->num_threads == 0)
    {
        g->sd[0].stop = false;
        helper_start_search(g, g->sd[0]);
    }
    else
        helper_start_threads(g);
}

void CHAPI ch_stop(ch_game* g)
{
    helper_stop_threads(g);
}

uint64_t CHAPI ch_get_nodes(ch_game* g)
{
    uint64_t total = 0;
    for(int n = 0; n < CH_MAX_THREADS; ++n)
        total += g->sd[n].nodes;
    return total;
}

char const* CHAPI ch_extended_algebraic(uint32_t m)
{
    return ch::move(m).extended_algebraic();
}

int CHAPI ch_get_piece_at(ch_game* g, int sq)
{
    if(sq < 0 || sq >= 64) return ch::EMPTY;
    return int(g->pos.pieces[sq]);
}

uint64_t CHAPI ch_perft(ch_game* g, int depth, uint64_t counts[256])
{
#if CH_ENABLE_HASH_PERFT
    g->tt.clear();
#endif
#if CH_ENABLE_AVX
    if(ch::has_avx())
        return g->pos.root_perft<ch::ACCEL_AVX>(g->tt, depth, counts);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        return g->pos.root_perft<ch::ACCEL_SSE>(g->tt, depth, counts);
    else
#endif
#if CH_ENABLE_UNACCEL
        return g->pos.root_perft<ch::ACCEL_UNACCEL>(g->tt, depth, counts);
#else
        return 0ull;
#endif
}

int CHAPI ch_is_draw(ch_game* g)
{
    if(g->pos.is_draw_by_insufficient_material()) return CH_DRAW_MATERIAL;
    if(g->pos.repetition_count() >= 2) return CH_DRAW_REPETITION;
    if(!g->pos.in_check && g->moves.empty()) return CH_DRAW_STALEMATE;
    return CH_DRAW_NONE;
}

int CHAPI ch_is_check(ch_game* g)
{
    return g->pos.in_check ? 1 : 0;
}

int CHAPI ch_is_checkmate(ch_game* g)
{
    if(g->pos.in_check && g->moves.empty())
        return 1;
    return 0;
}

int CHAPI ch_current_turn(ch_game* g)
{
    return g->pos.current_turn;
}

int CHAPI ch_see(ch_game* g, char const* mvstr)
{
    ch_move m = convert_move(g, mvstr);
    if(m == 0 || !g->pos.move_is_promotion_or_capture(m)) return INT_MIN;
    return ch::see<ch::ACCEL_UNACCEL>(m, g->pos);
}

}
