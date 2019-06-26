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

    typedef uint32_t ch_move;

    typedef struct ch_system_info
    {
        // return monotonic millisecond clock
        uint32_t(*get_ms)(void);

        // sleep briefly
        void(*thread_yield)(void);

        // info is a UCI string like
        // "info depth 2 score cp 214 time 1242 nodes 2124 nps 34928 pv e2e4 e7e5 g1f3"
        void(*uci_info)(char const* info);
    } ch_init_info;

    void CHAPI ch_init(ch_system_info const* info);

    // mem must be at least 8-byte aligned
    void CHAPI ch_set_hash(void* mem, int size_megabyte_log2);

    void CHAPI ch_new_game(void);
    void CHAPI ch_load_fen(char const* fen);

    int CHAPI ch_evaluate(void);

    ch_move CHAPI ch_depth_search(int depth);

    // get node count from lasst search
    uint64_t CHAPI ch_get_nodes(void);

    // convert move to string
    char const* CHAPI ch_extended_algebraic(ch_move m);

    uint64_t CHAPI ch_perft(int depth, uint64_t counts[256]);

#ifdef __cplusplus
}
#endif
