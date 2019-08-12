#ifndef DRAW_GAME_H
#define DRAW_GAME_H

#include "common.h"
#include "draw_helpers.h"
#include "format.h"
#include "img.h"
#include "text.h"

#include "game_common.h"

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

#define ANIMATE_NUM_STEPS 10
#define ANIMATE_STEP_MS 25

static void draw_board(void)
{
    int n;
    char rf[4] = { 'A', '\0', '1', '\0' };

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
            int r, c;
            ch_move mv = ch_get_move(n);
            if(ch_move_fr_sq(mv) != selpos) continue;
            r = ch_move_to_sq(mv) / 8;
            c = ch_move_to_sq(mv) % 8;
            r = r * 20 + 10;
            c = c * 20 + 10;
            draw_box(c + 1, r + 1, c + 18, r + 18, COLOR_THINKING);
        }
    }

    /* previous move */
    if(prevmv != 0)
    {
        drawsqbox(ch_move_fr_sq(prevmv), COLOR_MOVE);
        drawsqbox(ch_move_to_sq(prevmv), COLOR_MOVE);
    }
}

static void draw_piece(int pc, int x, int y)
{
    img_data const* img = &IMG_DATA[pc >> 1];
    int r, c;
    int col = pc & 1;
    x = CLAMP(x, 0, 180 - 16);
    y = CLAMP(y, 0, 180 - 16);
    for(r = 0; r < 16; ++r)
    {
        uint32_t d = img->d[r];
        for(c = 0; c < 16; ++c)
        {
            uint32_t di = (d >> (c * 2)) & 0x3;
            if(!di) continue;
            set_pixel_unsafe(
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
            if(y * 8 + x == selpos)
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

static void animate_move(int a, int b)
{
    /* animate piece moving */
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
}

#endif
