#pragma once

#include "ch.h"
#include "ch_cpuid.hpp"
#include "ch_evaluate_unaccel.hpp"
#include "ch_evaluate_sse.hpp"
#include "ch_evaluate_avx.hpp"
#include "ch_hash.hpp"
#include "ch_history_heuristic.hpp"
#include "ch_init.hpp"
#include "ch_internal.hpp"
#include "ch_magic.hpp"
#include "ch_move_list.hpp"
#include "ch_position.hpp"
#include "ch_search.hpp"


static ch::trans_table g_tt;
static ch::position g_pos;
static ch::search_data g_sd[CH_MAX_THREADS];
static ch::move_list g_moves;
static ch::history_heuristic g_hh;

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
        ;
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
    g_tt.set_memory(nullptr, 0);
    for(int n = 0; n < CH_MAX_THREADS; ++n)
    {
        g_sd[n].tt = &g_tt;
        g_sd[n].hh = &g_hh;
        g_sd[n].nodes = 0;
    }
    g_pos.new_game();
    update_moves();
}

void CHAPI ch_set_hash(void* mem, int size_megabyte_log2)
{
    g_tt.set_memory(mem, size_megabyte_log2);
}

void CHAPI ch_clear_caches(void)
{
    g_tt.clear();
    g_hh.clear();
}

void CHAPI ch_new_game()
{
    g_pos.new_game();
    update_moves();
}

void CHAPI ch_load_fen(char const* fen)
{
    g_pos.load_fen(fen);
    update_moves();
}

void CHAPI ch_do_move(ch_move m)
{
    using namespace ch;

    // validate move
    {
        int valid = 0;
        for(move const& tm : g_moves)
            if(m == tm)
                valid = 1;
        if(!valid)
        {
            printf("uci info string invalid move: %u %s\n",
                m, move(m).extended_algebraic());
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
        ;
#endif

    g_pos.stack_reset();
    update_moves();
}

void CHAPI ch_do_move_str(char const* str)
{
    using namespace ch;
    move m = ch::INVALID_MOVE;

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
                return;
            if((m & 0xFFFF) == (g_moves[n] & 0xFFFF))
            {
                m = g_moves[n];
                break;
            }
        }
        break;
    }

    ch_do_move(m);
}

int CHAPI ch_evaluate(void)
{
    using namespace ch;
#if CH_ENABLE_AVX
    if(has_avx())
        return evaluator<ACCEL_AVX>::evaluate(g_pos, g_pos.current_turn);
    else
#endif
#if CH_ENABLE_SSE
    if(has_sse())
        return evaluator<ACCEL_SSE>::evaluate(g_pos, g_pos.current_turn);
    else
#endif
#if CH_ENABLE_UNACCEL
        return evaluator<ACCEL_UNACCEL>::evaluate(g_pos, g_pos.current_turn);
#else
        return 0;
#endif
}

ch_move CHAPI ch_search(ch_search_limits const* limits)
{
    uint32_t start_time = ch::get_ms();
#if CH_ENABLE_HISTORY_HEURISTIC
    g_hh.clear();
#endif
    ch_search_limits newlimits = *limits;
    if(newlimits.depth <= 0) newlimits.depth = INT_MAX;
    if(newlimits.mstime <= 0) newlimits.mstime = INT_MAX;
    if(newlimits.remtime <= 0) newlimits.remtime = INT_MAX;
    if(newlimits.inctime <= 0) newlimits.inctime = INT_MAX;
    for(int n = 0; n < CH_MAX_THREADS; ++n)
    {
        g_sd[n].start_time = start_time;
        g_sd[n].limits = newlimits;
    }
#if CH_ENABLE_AVX
    if(ch::has_avx())
        return ch::iterative_deepening<ch::ACCEL_AVX>(g_sd[0], g_pos);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        return ch::iterative_deepening<ch::ACCEL_SSE>(g_sd[0], g_pos);
    else
#endif
#if CH_ENABLE_UNACCEL
        return ch::iterative_deepening<ch::ACCEL_UNACCEL>(g_sd[0], g_pos);
#else
        return 0;
#endif
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

}
