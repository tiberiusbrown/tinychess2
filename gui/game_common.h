#ifndef GAME_COMMON_H
#define GAME_COMMON_H

#include <ch.h>

#include "button.h"
#include "system.h"
#include "thread.h"

static button btn_undo;
//static button btn_redo;

static ch_game* game;

static int selpos;
static ch_move prevmv;
static ch_move bestmv;

static int ishuman[2];
static int board[8][8];

static thread thrd;
static int cur_turn;
static int thinking;
static int cur_depth;
static int cur_score[2];
static uint64_t cur_nodes;

// 8 MB hash
#define HASH_MEM_MB_LOG2 3

static uint64_t hash_mem[1 << (20 + HASH_MEM_MB_LOG2 - 3)];

static void flush_input(void)
{
    input i;
    int t;
    for(;;)
    {
        t = 1;
        i.type = INPUT_NONE;
        wait_for_input_timeout(&i, &t);
        if(i.type == INPUT_TIMER)
            break;
    }
}

static void wait_ignoring_input(int* timeout_ms)
{
    input dummy;
    while(*timeout_ms > 0)
        wait_for_input_timeout(&dummy, timeout_ms);
}

static void delay_ms(int timeout_ms)
{
    wait_ignoring_input(&timeout_ms);
}

static void update_board(void)
{
    int r, c;
    for(r = 0; r < 8; ++r)
        for(c = 0; c < 8; ++c)
            board[r][c] = ch_get_piece_at(game, r * 8 + c);
}

static int get_board_pos(void)
{
    int r = (my + 10) / 20 - 1;
    int c = (mx + 10) / 20 - 1;
    if(r < 0 || c < 0 || r >= 8 || c >= 8)
        return -1;
    return r * 8 + c;
}

static void* alloc(uint32_t bytes)
{
    return FN_malloc((size_t)bytes);
}
static void dealloc(void* p)
{
    FN_free(p);
}

static void search_info(
    int depth,
    int seldepth,
    uint64_t nodes,
    int mstime,
    int score,
    uint64_t nps,
    ch_move* pv,
    int pvlen
)
{
    (void)seldepth;
    (void)mstime;
    (void)nps;
    (void)pvlen;
    cur_depth = depth;
    cur_score[cur_turn] = score;
    cur_nodes = nodes;
    bestmv = pv[0];
}

static void best_move(ch_move mv)
{
    bestmv = mv;
    thinking = 0;
}

static void thread_func(void* user)
{
    (void)user;
    ch_thread_start(game);
}

#endif
