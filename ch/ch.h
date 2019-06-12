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
        void(*mutex_lock)(void* m);
        void(*mutex_unlock)(void* m);
        uint32_t(*get_ms)(void);
    } ch_init_info;

    void CHAPI ch_init(ch_system_info const* info);
    void CHAPI ch_new_game(void);
    void CHAPI ch_load_fen(char const* fen);

    uint64_t CHAPI ch_perft(int depth, uint64_t counts[256]);

#ifdef __cplusplus
}
#endif
