#ifndef RUN_H
#define RUN_H

#include <ch.h>

#include "common.h"
#include "draw_helpers.h"
#include "format.h"
#include "img.h"
#include "text.h"
#include "thread.h"

#define ANIMATE_NUM_STEPS 10
#define ANIMATE_STEP_MS 30
    
static pixel const COLOR_BORDER = PIXEL_SHADE(50);

/* white, black */
static pixel const COLOR_SQUARES[2] = { PIXEL_SHADE(140), PIXEL_SHADE(90) };
static pixel const COLOR_LEGAL[2] = { PIXEL_SHADE(200), PIXEL_SHADE(170) };
static pixel const COLOR_PIECES[2][3] = {
    { PIXEL_SHADE(45), PIXEL_SHADE(175), PIXEL_SHADE(255), },
    { PIXEL_SHADE(30), PIXEL_SHADE(60), PIXEL_SHADE(90), },
};
static pixel const COLOR_THINKING = PIXEL_SHADE(220);
static pixel const COLOR_MOVE = PIXEL_SHADE(30);

static int selpos;
static ch_move prevmv;
static ch_move bestmv;

static int ishuman[2];
static int board[8][8];

static thread thrd;
static int thinking;
static int cur_depth;
static int cur_score;
static uint64_t cur_nodes;

static uint64_t hash_mem[(1 << 17) * 64];

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
            board[r][c] = ch_get_piece_at(r * 8 + c);
}

static void drawsqbox(int sq, pixel p)
{
    int r = sq / 8;
    int c = sq % 8;
    if(r >= 0 && c >= 0 && r < 8 && c < 8)
    {
        r = r * 20 + 10;
        c = c * 20 + 10;
        draw_box(c, r, c + 19, r + 19, p);
    }
}

static void draw_board(void)
{
    int n;
    char rf[4] = { 'A', '\0', '1', '\0' };

    draw_rect_unsafe(180, 0, FBW - 1, FBH - 1, PIXEL_BLACK);

    /* border */
    draw_rect_unsafe(0, 0, 179, 9, COLOR_BORDER);
    draw_rect_unsafe(0, 170, 179, 179, COLOR_BORDER);
    draw_rect_unsafe(0, 10, 9, 169, COLOR_BORDER);
    draw_rect_unsafe(170, 10, 179, 169, COLOR_BORDER);

    /* rank and file text */
    for(n = 0; n < 8; ++n)
    {
        text(&rf[0], n * 20 + 18, 1, TEXT_COLOR);
        text(&rf[0], n * 20 + 18, 172, TEXT_COLOR);
        text(&rf[2], 172, n * -20 + 157, TEXT_COLOR);
        text(&rf[2], 4, n * -20 + 157, TEXT_COLOR);
        ++rf[0];
        ++rf[2];
    }

    /* squares */
    for(n = 0; n < 32; ++n)
    {
        int r = n / 4;
        int b = r % 2;
        int c = n % 4 * 40 + 10;
        r = r * 20 + 10;
        draw_rect_unsafe(c, r, c + 19, r + 19, COLOR_SQUARES[b]);
        c += 20;
        b = !b;
        draw_rect_unsafe(c, r, c + 19, r + 19, COLOR_SQUARES[b]);
    }

    /* legal moves */
    if(selpos)
    {
        for(n = 0; n < ch_num_moves(); ++n)
        {
            int r, c, b;
            ch_move mv = ch_get_move(n);
            if(ch_move_fr_sq(mv) != selpos) continue;
            r = ch_move_to_sq(mv) / 8;
            c = ch_move_to_sq(mv) % 8;
            b = (r + c) % 2;
            r = r * 20 + 10;
            c = c * 20 + 10;
            draw_box(c + 1, r + 1, c + 18, r + 18, COLOR_THINKING);
            //draw_rect_unsafe(c, r, c + 19, r + 19, COLOR_LEGAL[b]);
        }
    }

    /* previous move */
    if(prevmv != 0)
    {
        drawsqbox(ch_move_fr_sq(prevmv), COLOR_MOVE);
        drawsqbox(ch_move_to_sq(prevmv), COLOR_MOVE);
    }

    /* best move while ai is thinking */
    //if(thinking)
    //{
    //    drawsqbox(ch_move_fr_sq(bestmv), COLOR_THINKING);
    //    drawsqbox(ch_move_to_sq(bestmv), COLOR_THINKING);
    //}
}

static void draw_piece(int pc, int x, int y)
{
    img_data const* img = &IMG_DATA[pc >> 1];
    int r, c;
    int col = pc & 1;
    for(r = 0; r < 16; ++r)
    {
        uint32_t d = img->d[r];
        for(c = 0; c < 16; ++c)
        {
            uint32_t di = (d >> (c * 2)) & 0x3;
            if(!di) continue;
            set_pixel_safe_bounds(
                c + x,
                r + y,
                COLOR_PIECES[col][di - 1]);
        }
    }
}

static void draw_pieces_without_selected(void)
{
    int x, y;

    /* pieces */
    for(y = 0; y < 8; ++y)
    {
        for(x = 0; x < 8; ++x)
        {
            int pc = board[y][x];
            if(pc == CH_EMPTY) continue;
            if(y == (int)selpos / 8 && x == (int)selpos % 8)
                continue;
            draw_piece(pc, x * 20 + 12, y * 20 + 12);
        }
    }
}

static void draw_pieces(void)
{
    draw_pieces_without_selected();

    /* selected piece */
    if(selpos >= 0 && selpos < 64)
        draw_piece(board[selpos / 8][selpos % 8], mx - 8, my - 8);
}

static int get_board_pos(void)
{
    int r = (my + 10) / 20 - 1;
    int c = (mx + 10) / 20 - 1;
    if(r < 0 || c < 0 || r >= 8 || c >= 8)
        return -1;
    return r * 8 + c;
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
    cur_score = score;
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
    ch_thread_start();
}

static void animate_move(ch_move mv)
{
    /* animate piece moving */
    int a = ch_move_fr_sq(mv);
    int b = ch_move_to_sq(mv);
    int n;
    int pc = board[a / 8][a % 8];
    int x1 = (a % 8) * 20 + 12;
    int y1 = (a / 8) * 20 + 12;
    int dx = (b % 8) * 20 + 12 - x1;
    int dy = (b / 8) * 20 + 12 - y1;
    for(n = 0; n < ANIMATE_NUM_STEPS; ++n)
    {
        delay_ms(ANIMATE_STEP_MS);
        selpos = -1;
        draw_board();
        selpos = a;
        draw_pieces_without_selected();
        draw_piece(pc,
            x1 + dx * n / ANIMATE_NUM_STEPS,
            y1 + dy * n / ANIMATE_NUM_STEPS);
        refresh();
    }
    delay_ms(ANIMATE_STEP_MS);
    selpos = -1;
    ch_do_move(mv);
}

static FORCEINLINE void run(void)
{
    ch_system_info info =
    {
        get_ms,
        thread_yield,
        search_info,
        best_move
    };

    ch_search_limits limits =
    {
        0, 100, 0, 0
    };

    ch_init(&info);
    ch_set_hash(hash_mem, 6);
    ch_new_game();

    thread_create(&thrd, thread_func, NULL);

    ishuman[0] = 1;
    ishuman[1] = 0;
    bestmv = 0;
    prevmv = 0;
    selpos = -1;

    update_board();
    draw_board();
    draw_pieces();
    refresh();

    for(;;)
    {
        if(ch_is_checkmate(NULL) || ch_is_draw())
        {
            input i;
            if(ch_is_checkmate(NULL))
                text("Checkmate", 190, 10, TEXT_COLOR);
            else
                text("Stalemate", 190, 10, TEXT_COLOR);
            for(;;) wait_for_input(&i);
        }

        if(ishuman[ch_current_turn()])
        {
            int n;
            int movepos;
            input i = { INPUT_NONE };
            selpos = movepos = -1;
            while(selpos < 0)
            {
                int p, pos = 0;
                wait_for_input(&i);
                p = get_board_pos();
                if(p && i.type == INPUT_MOUSE_DOWN)
                {
                    int pc = board[p / 8][p % 8];
                    if(pc != CH_EMPTY && (pc & 1) == ch_current_turn())
                        selpos = p;
                }
                if(p != pos)
                {
                    draw_board();
                    /* drawsqbox(p, COLOR_THINKING); */
                    draw_pieces();
                    pos = p;
                    refresh();
                }
            }
            while(movepos < 0)
            {
                int p = get_board_pos();
                draw_board();
                drawsqbox(selpos, COLOR_THINKING);
                for(n = 0; n < ch_num_moves(); ++n)
                {
                    ch_move mv = ch_get_move(n);
                    if(ch_move_fr_sq(mv) == selpos && ch_move_to_sq(mv) == p)
                    {
                        drawsqbox(p, COLOR_THINKING);
                        break;
                    }
                }
                draw_pieces();
                refresh();
                wait_for_input(&i);
                if(i.type == INPUT_MOUSE_UP)
                    movepos = get_board_pos();
            }
            for(n = 0; n < ch_num_moves(); ++n)
            {
                ch_move mv = ch_get_move(n);
                if(ch_move_fr_sq(mv) == selpos && ch_move_to_sq(mv) == movepos)
                {
                    prevmv = mv;
                    ch_do_move(mv);
                    update_board();
                    break;
                }
            }
            selpos = -1;
            draw_board();
            draw_pieces();
            refresh();
        }
        else
        {
            thinking = 1;
            ch_search(&limits);
            while(thinking)
            {
                delay_ms(1);
                //delay_ms(20);
                //draw_board();
                //draw_pieces();
                //text(format("Nodes: %d", game.nodes + game.qnodes), 190, 20, TEXT_COLOR);
                //text(format("Depth: %d", cur_depth), 190, 30, TEXT_COLOR);
                //text(format("Score: %d", cur_score), 190, 40, TEXT_COLOR);
                //refresh();
            }
            prevmv = bestmv;
            animate_move(bestmv);
            update_board();
            draw_board();
            draw_pieces();
            refresh();
        }
    }
}

static FORCEINLINE void postrun(void)
{
    ch_kill_threads();
    thread_join(&thrd);
}

#endif
