#pragma once

#include "ch.h"
#include "ch_internal.hpp"

static ch::position p;

extern "C"
{

void CHAPI ch_init()
{
    ch::init();
    ch::init_cpuid();
}
void CHAPI ch_new_game()
{
    p.new_game();
}
void CHAPI ch_load_fen(char const* fen)
{
    p.load_fen(fen);
}
uint64_t CHAPI ch_perft(int depth, uint64_t counts[256])
{
#if CH_ENABLE_ACCEL
    if(ch::has_sse())
        return p.root_perft<ch::ACCEL_SSE>(depth, counts);
#endif
    return p.root_perft<ch::ACCEL_UNACCEL>(depth, counts);
}

}
