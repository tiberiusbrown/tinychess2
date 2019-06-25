#pragma once

#include "ch.h"
#include "ch_cpuid.hpp"
#include "ch_evaluate_unaccel.hpp"
#include "ch_evaluate_sse.hpp"
#include "ch_evaluate_avx.hpp"
#include "ch_hash.hpp"
#include "ch_init.hpp"
#include "ch_internal.hpp"
#include "ch_magic.hpp"
#include "ch_position.hpp"
#include "ch_search.hpp"

static ch::position g_pos;

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
    g_pos.tt.set_memory(nullptr, 0);
}

void CHAPI ch_set_hash(void* mem, int size_megabyte_log2)
{
    g_pos.tt.set_memory(mem, size_megabyte_log2);
}

void CHAPI ch_new_game()
{
    g_pos.new_game();
}

void CHAPI ch_load_fen(char const* fen)
{
    g_pos.load_fen(fen);
}

static int helper_ch_negamax(uint32_t* best, int depth, int alpha, int beta)
{
    ch::move bestmv = ch::INVALID_MOVE;
    int r = ch::MIN_SCORE;
#if CH_ENABLE_AVX
    if(ch::has_avx())
        r = ch::negamax_root<ch::ACCEL_AVX>(g_pos, bestmv, depth, alpha, beta);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        r = ch::negamax_root<ch::ACCEL_SSE>(g_pos, bestmv, depth, alpha, beta);
    else
#endif
#if CH_ENABLE_UNACCEL
        r = ch::negamax_root<ch::ACCEL_UNACCEL>(g_pos, bestmv, depth, alpha, beta);
#else
        ;
#endif
    if(best) *best = bestmv;
    return r;
}

int CHAPI ch_evaluate(void)
{
#if CH_ENABLE_AVX
    if(ch::has_avx())
        return ch::evaluator<ch::ACCEL_AVX>::evaluate(g_pos, g_pos.current_turn);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        return ch::evaluator<ch::ACCEL_SSE>::evaluate(g_pos, g_pos.current_turn);
    else
#endif
#if CH_ENABLE_UNACCEL
        return ch::evaluator<ch::ACCEL_UNACCEL>::evaluate(g_pos, g_pos.current_turn);
#else
        return CH_MIN_SCORE;
#endif
}

int CHAPI ch_negamax(uint32_t* best, int depth, int alpha, int beta)
{
    g_pos.tt.clear();
    g_pos.nodes = 0;
    return helper_ch_negamax(best, depth, alpha, beta);
}

uint32_t CHAPI ch_depth_search(int depth)
{
    uint32_t best = 0;
    ch_negamax(&best, depth, ch::MIN_SCORE, ch::MAX_SCORE);
    return best;
}

uint32_t CHAPI ch_depth_search_iterative(int depth)
{
    uint32_t best = 0;
    g_pos.tt.clear();
    g_pos.nodes = 0;
    int center = ch_evaluate();
    for(int i = 2; i <= depth; ++i)
    {
        int delta = 14;
        int alpha = center - delta;
        int beta = center + delta;
        int v;
        for(;;)
        {
            v = helper_ch_negamax(&best, i, alpha, beta);
            if(v <= alpha)
            {
                beta = (alpha + beta) / 2;
                alpha = std::max(CH_MIN_SCORE, alpha - delta);
            }
            else if(v >= beta)
                beta = std::min(CH_MAX_SCORE, beta + delta);
            else
                break;
            delta += delta / 2;
        }
        center = v;
    }
    return best;
}

uint64_t CHAPI ch_get_nodes(void)
{
    return g_pos.nodes;
}

char const* CHAPI ch_extended_algebraic(uint32_t m)
{
    return ch::move(m).extended_algebraic();
}

uint64_t CHAPI ch_perft(int depth, uint64_t counts[256])
{
#if CH_ENABLE_AVX
    if(ch::has_avx())
        return g_pos.root_perft<ch::ACCEL_AVX>(depth, counts);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        return g_pos.root_perft<ch::ACCEL_SSE>(depth, counts);
    else
#endif
#if CH_ENABLE_UNACCEL
        return g_pos.root_perft<ch::ACCEL_UNACCEL>(depth, counts);
#else
        return 0ull;
#endif
}

}
