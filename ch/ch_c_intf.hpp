#pragma once

#include "ch.h"
#include "ch_internal.hpp"
#include "ch_position.hpp"

static ch::position g_pos;

extern "C"
{

void CHAPI ch_init(ch_system_info const* info)
{
    ch::system = *info;
    ch::init();
    ch::init_cpuid();
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
#if CH_ARCH_64BIT
    return g_pos.root_perft<ch::ACCEL_SSE>(depth, counts);
#elif CH_ENABLE_SSE
    if(ch::has_sse())
        return g_pos.root_perft<ch::ACCEL_SSE>(depth, counts);
#endif
#if CH_ARCH_32BIT
    return g_pos.root_perft<ch::ACCEL_UNACCEL>(depth, counts);
#endif
}

}
