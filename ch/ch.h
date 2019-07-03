#pragma once

#include <stdint.h>
#include <limits.h>

#ifdef _MSC_VER
#define CHAPI __cdecl
#else
#define CHAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef uint32_t ch_move;

#define CH_MATE_SCORE 31000
#define CH_MATED_SCORE -31000

    typedef struct ch_system_info
    {
        // return monotonic millisecond clock
        uint32_t(*get_ms)(void);

        // sleep briefly
        void(*thread_yield)(void);

        // sent periodically during search
        void(*search_info)(
            int depth,
            int seldepth,
            uint64_t nodes,
            int mstime,
            ch_move const* pv,
            int pvnum,
            int score,
            uint64_t nps
            );
    } ch_system_info;

    typedef struct ch_search_limits
    {
        int depth;
        int mstime;
        int remtime;
        int inctime;
    } ch_search_limits;

    void CHAPI ch_init(ch_system_info const* info);

    void CHAPI ch_thread_func(int index);

    // mem must be at least 8-byte aligned
    void CHAPI ch_set_hash(void* mem, int size_megabyte_log2);

    // reset hash table and heuristics (ucinewgame)
    void CHAPI ch_clear_caches(void);

    void CHAPI ch_new_game(void);
    void CHAPI ch_load_fen(char const* fen);

    void CHAPI ch_do_move(ch_move m);
    // extended algebraic, e.g. e7f8q
    void CHAPI ch_do_move_str(char const* str);

    int CHAPI ch_evaluate(void);

    ch_move CHAPI ch_search(ch_search_limits const* limits);

    // get node count from last search
    uint64_t CHAPI ch_get_nodes(void);

    // convert move to string
    char const* CHAPI ch_extended_algebraic(ch_move m);

    uint64_t CHAPI ch_perft(int depth, uint64_t counts[256]);

#ifdef __cplusplus
}
#endif
