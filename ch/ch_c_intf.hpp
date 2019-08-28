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

static ch::trans_table g_tt;
static ch::position g_pos;
static ch::search_data g_sd[CH_MAX_THREADS];
static ch::move_list g_moves;
static ch::history_heuristic g_hh;
static int g_redo_ply;

// thread management
static std::atomic_int g_num_threads;

static void update_moves(void)
{
    using namespace ch;
#if CH_ENABLE_AVX
    if(has_avx())
        g_moves.generate<ACCEL_AVX>(g_pos.current_turn, g_pos);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        g_moves.generate<ACCEL_SSE>(g_pos.current_turn, g_pos);
    else
#endif
#if CH_ENABLE_UNACCEL
        g_moves.generate<ACCEL_UNACCEL>(g_pos.current_turn, g_pos);
#else
    { }
#endif
}

extern "C"
{

void CHAPI ch_init(ch_system_info const* info)
{
    ch::system = *info;
    ch::init();
    ch::init_cpuid();
#if CH_ENABLE_MAGIC
    ch::init_magic();
#endif
    ch::init_hashes();
    ch::init_evaluator();
    g_tt.set_memory(nullptr, 0);
    for(int n = 0; n < CH_MAX_THREADS; ++n)
    {
        g_sd[n].tt = &g_tt;
        g_sd[n].hh = &g_hh;
        g_sd[n].nodes = 0;
        g_sd[n].stop = true;
        g_sd[n].stopped = false;
        g_sd[n].kill = false;
        g_sd[n].killed = false;
        g_sd[n].data_index = n;
        g_sd[n].num_threads = 1;
    }
    g_num_threads = 0;
    g_pos.new_game();
    update_moves();
    ch_clear_caches();
}

static void helper_start_search(ch::search_data& d)
{
    ch::move mv;
    d.stopped = false;
#if CH_ENABLE_AVX
    if(ch::has_avx())
        mv = ch::iterative_deepening<ch::ACCEL_AVX>(d, g_pos);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        mv = ch::iterative_deepening<ch::ACCEL_SSE>(d, g_pos);
    else
#endif
#if CH_ENABLE_UNACCEL
        mv = ch::iterative_deepening<ch::ACCEL_UNACCEL>(d, g_pos);
#else
        mv = ch::NULL_MOVE;
#endif
    if(d.data_index == 0 && ch::system.best_move)
        ch::system.best_move(mv);
}

void CHAPI ch_thread_start(void)
{
    int index = g_num_threads.fetch_add(1, std::memory_order_relaxed);
    if(index >= CH_MAX_THREADS)
    {
        g_num_threads -= 1;
        return;
    }
    ch::search_data& d = g_sd[index];

    for(;;)
    {
        d.stop = true;
        d.stopped = true;
        while(d.stop && !d.kill)
            ch::thread_yield();
        if(d.kill) break;
        d.stopped = false;
        helper_start_search(d);
    }
    d.killed = true;
}

static void helper_stop_threads()
{
    int n = g_num_threads;
    for(int i = n - 1; i >= 0; --i)
        g_sd[i].stop = true;
    int num_stopped = 0;
    do
    {
        for(int i = n - 1; i >= 0; --i)
        {
            if(g_sd[i].stopped)
            {
                ++num_stopped;
                break;
            }
        }
    } while(num_stopped < n);
}

static void helper_start_threads()
{
    int n = g_num_threads;
    for(int i = 0; i < n; ++i)
    {
        g_sd[i].num_threads = n;
        g_sd[i].depth = 0;
    }
    while(n-- > 0)
        g_sd[n].stop = false;
}

void CHAPI ch_kill_threads(void)
{
    int n = g_num_threads;
    for(int i = n - 1; i >= 0; --i)
        g_sd[i].kill = g_sd[i].stop = true;
    int num_killed = 0;
    do
    {
        for(int i = n - 1; i >= 0; --i)
        {
            if(g_sd[i].killed)
            {
                g_sd[i].killed = g_sd[i].kill = false;
                ++num_killed;
                break;
            }
        }
    } while(num_killed < n);
    g_num_threads = 0;
}

void CHAPI ch_set_hash(void* mem, int size_megabyte_log2)
{
    g_tt.set_memory(mem, size_megabyte_log2);
}

void CHAPI ch_clear_caches(void)
{
    g_tt.clear();
    g_hh.clear();
    for(int n = 0; n < CH_MAX_THREADS; ++n)
        memzero32(&g_sd[n].killers[0][0], sizeof(g_sd[n].killers) / 4);
}

void CHAPI ch_new_game()
{
    g_pos.new_game();
    g_redo_ply = 0;
    update_moves();
}

void CHAPI ch_load_fen(char const* fen)
{
    g_pos.load_fen(fen);
    g_redo_ply = 0;
    update_moves();
}

void CHAPI ch_do_move(ch_move m)
{
    using namespace ch;

    // validate move
    {
        int valid = 0;
        move mv(m);
        mv.sort_key() = 0;
        for(move const& tm : g_moves)
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
        g_pos.do_move<ACCEL_AVX>(m);
    else
#endif
#if CH_ENABLE_SSE
    if(has_sse())
        g_pos.do_move<ACCEL_SSE>(m);
    else
#endif
#if CH_ENABLE_UNACCEL
        g_pos.do_move<ACCEL_UNACCEL>(m);
#else
    { }
#endif

    g_pos.stack_reset();
    g_pos.age += 1;
    g_redo_ply = g_pos.ply;
    update_moves();
}

static ch_move convert_move(char const* str)
{
    using namespace ch;
    move m = NULL_MOVE;

    m += move::from(('8' - str[1]) * 8 + str[0] - 'a');
    m += move::to(('8' - str[3]) * 8 + str[2] - 'a');
    switch(str[4])
    {
    case 'n':
        m += move::pawn_promotion(g_pos.current_turn + KNIGHT);
        break;
    case 'b':
        m += move::pawn_promotion(g_pos.current_turn + BISHOP);
        break;
    case 'r':
        m += move::pawn_promotion(g_pos.current_turn + ROOK);
        break;
    case 'q':
        m += move::pawn_promotion(g_pos.current_turn + QUEEN);
        break;
    default:
        // check for castling or en passant moves
        for(int n = 0; ; ++n)
        {
            if(n >= g_moves.size())
                return 0;
            if((m & 0xFFFF) == (g_moves[n] & 0xFFFF))
            {
                m = g_moves[n];
                break;
            }
        }
        break;
    }

    return m;
}

void CHAPI ch_do_move_str(char const* str)
{
    ch_do_move(convert_move(str));
}

ch_move CHAPI ch_last_move(void)
{
    if(g_pos.ply == 0) return ch::NULL_MOVE;
    return g_pos.move_history[g_pos.ply - 1];
}

ch_move CHAPI ch_undo_move(void)
{
    int p = g_pos.ply - 1;
    if(p < 0) return ch::NULL_MOVE;
    ch_new_game();
    for(int n = 0; n < p; ++n)
        ch_do_move(g_pos.move_history[n]);
    return g_pos.move_history[p];
}

ch_move CHAPI ch_redo_move(void)
{
    if(g_pos.ply >= g_redo_ply) return ch::NULL_MOVE;
    ch::move mv = g_pos.move_history[g_pos.ply];
    ch_do_move(mv);
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

int CHAPI ch_num_moves()
{
    return g_moves.size();
}

ch_move CHAPI ch_get_move(int n)
{
    if(n < 0 || n >= g_moves.size()) return ch::NULL_MOVE;
    return g_moves[n];
}

int CHAPI ch_evaluate(void)
{
    using namespace ch;
    evaluator<ACCEL_UNACCEL> e;
    return e.evaluate(g_pos, g_pos.current_turn);
}

void CHAPI ch_search(ch_search_limits const* limits)
{
    using namespace ch;
    uint32_t start_time = get_ms();
    helper_stop_threads();
    g_redo_ply = g_pos.ply;
#if CH_ENABLE_HISTORY_HEURISTIC
    g_hh.clear();
#endif
    ch_search_limits newlimits = *limits;
    if(newlimits.depth <= 0) newlimits.depth = INT_MAX;
    if(newlimits.mstime <= 0) newlimits.mstime = INT_MAX;
    update_limits(g_pos, newlimits);
    for(int n = 0; n < CH_MAX_THREADS; ++n)
    {
        g_sd[n].start_time = start_time;
        g_sd[n].limits = newlimits;
    }
    if(g_num_threads == 0)
        helper_start_search(g_sd[0]);
    else
        helper_start_threads();
}

void CHAPI ch_stop(void)
{
    helper_stop_threads();
}

uint64_t CHAPI ch_get_nodes(void)
{
    uint64_t total = 0;
    for(int n = 0; n < CH_MAX_THREADS; ++n)
        total += g_sd[n].nodes;
    return total;
}

char const* CHAPI ch_extended_algebraic(uint32_t m)
{
    return ch::move(m).extended_algebraic();
}

int CHAPI ch_get_piece_at(int sq)
{
    if(sq < 0 || sq >= 64) return ch::EMPTY;
    return int(g_pos.pieces[sq]);
}

uint64_t CHAPI ch_perft(int depth, uint64_t counts[256])
{
#if CH_ENABLE_HASH_PERFT
    g_tt.clear();
#endif
#if CH_ENABLE_AVX
    if(ch::has_avx())
        return g_pos.root_perft<ch::ACCEL_AVX>(g_tt, depth, counts);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        return g_pos.root_perft<ch::ACCEL_SSE>(g_tt, depth, counts);
    else
#endif
#if CH_ENABLE_UNACCEL
        return g_pos.root_perft<ch::ACCEL_UNACCEL>(g_tt, depth, counts);
#else
        return 0ull;
#endif
}

int CHAPI ch_is_draw(void)
{
    if(g_pos.is_draw_by_insufficient_material()) return CH_DRAW_MATERIAL;
    if(g_pos.repetition_count() >= 2) return CH_DRAW_REPETITION;
    if(!g_pos.in_check && g_moves.empty()) return CH_DRAW_STALEMATE;
    return CH_DRAW_NONE;
}

int CHAPI ch_is_check(void)
{
    return g_pos.in_check ? 1 : 0;
}

int CHAPI ch_is_checkmate()
{
    if(g_pos.in_check && g_moves.empty())
        return 1;
    return 0;
}

int CHAPI ch_current_turn(void)
{
    return g_pos.current_turn;
}

int CHAPI ch_see(char const* mvstr)
{
    ch_move m = convert_move(mvstr);
    if(m == 0 || !g_pos.move_is_promotion_or_capture(m)) return INT_MIN;
    return ch::see<ch::ACCEL_UNACCEL>(m, g_pos);
}

}
