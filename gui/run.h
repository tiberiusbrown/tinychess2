#ifndef RUN_H
#define RUN_H

#include <ch.h>

#include "common.h"
#include "thread.h"

#include "game_play.h"

static FORCEINLINE void run(void)
{
    ch_system_info info =
    {
        &alloc,
        &dealloc,
        &get_ms,
        &thread_yield,
        &search_info,
        &best_move,
    };

    ch_init(&info);
    game = ch_create();
    ch_set_hash(game, hash_mem, HASH_MEM_MB_LOG2);
    ch_new_game(game);

    thread_create(&thrd, thread_func, NULL);

    button_init(&btn_undo, TEXT_ARROW_LEFT, 200, 50, 220, 60);

    ishuman[0] = 1;
    ishuman[1] = 0;
    game_play();
}

static FORCEINLINE void postrun(void)
{
    ch_kill_threads(game);
    thread_join(&thrd);
    ch_destroy(game);
    game = NULL;
}

#endif
