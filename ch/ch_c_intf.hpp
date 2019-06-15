#pragma once

#include "ch.h"
#include "ch_cpuid.hpp"
#include "ch_init.hpp"
#include "ch_internal.hpp"
#include "ch_magic.hpp"
#include "ch_position.hpp"

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
}
void CHAPI ch_new_game()
{
    g_pos.new_game();
}
void CHAPI ch_load_fen(char const* fen)
{
    g_pos.load_fen(fen);
}
uint64_t CHAPI ch_perft(int depth, uint64_t counts[256])
{
#if CH_ENABLE_AVX
    if(ch::has_avx())
        return g_pos.root_perft<ch::ACCEL_AVX>(depth, counts);
#endif
#if CH_ENABLE_SSE
#if CH_ENABLE_UNACCEL
    if(ch::has_sse())
#endif
        return g_pos.root_perft<ch::ACCEL_SSE>(depth, counts);
#endif
#if CH_ENABLE_UNACCEL
    return g_pos.root_perft<ch::ACCEL_UNACCEL>(depth, counts);
#endif
}

}
