#pragma once

#include <stdint.h>

#ifdef _MSC_VER
#define CHAPI __cdecl
#else
#define CHAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct ch_system_info
    {
        uint32_t(*get_ms)(void);
    } ch_init_info;

    void CHAPI ch_init(ch_system_info const* info);

    // mem must be at least 8-byte aligned
    void CHAPI ch_set_hash(void* mem, int size_megabyte_log2);

    void CHAPI ch_new_game(void);
    void CHAPI ch_load_fen(char const* fen);

    uint32_t CHAPI ch_depth_search(int depth);

    // get node count from lasst search
    uint64_t CHAPI ch_get_nodes(void);

    // convert move to string
    char const* CHAPI ch_extended_algebraic(uint32_t m);

    uint64_t CHAPI ch_perft(int depth, uint64_t counts[256]);

#ifdef __cplusplus
}
#endif
