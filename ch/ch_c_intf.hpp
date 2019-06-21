#pragma once

#include "ch.h"
#include "ch_cpuid.hpp"
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

uint32_t CHAPI ch_depth_search(int depth)
{
    ch::move best = ch::INVALID_MOVE;
    int alpha = ch::MIN_SCORE;
    int beta = ch::MAX_SCORE;
    g_pos.tt.clear();
    g_pos.nodes = 0;

#if CH_ENABLE_AVX
    if(ch::has_avx())
        ch::negamax_root<ch::ACCEL_AVX>(g_pos, best, depth, alpha, beta);
    else
#endif
#if CH_ENABLE_SSE
    if(ch::has_sse())
        ch::negamax_root<ch::ACCEL_SSE>(g_pos, best, depth, alpha, beta);
    else
#endif
#if CH_ENABLE_UNACCEL
        ch::negamax_root<ch::ACCEL_UNACCEL>(g_pos, best, depth, alpha, beta);
#else
        ;
#endif
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
