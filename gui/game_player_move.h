#ifndef GAME_PLAYER_MOVE_H
#define GAME_PLAYER_MOVE_H

#include "game_draw.h"

static void player_move(void)
{
    int n;
    int movepos;
    input i = { INPUT_NONE };
    selpos = movepos = -1;
    while(selpos < 0)
    {
        int p, pos = 0, refresh_needed = 0;
        wait_for_input(&i);
        if(button_input(&btn_undo, &i, &refresh_needed))
        {
            ch_move mv = ch_last_move();
            if(mv != 0)
            {
                animate_move(ch_move_to_sq(mv), ch_move_fr_sq(mv));
                ch_undo_move();
                prevmv = ch_last_move();
                update_board();
            }
        }
        if(refresh_needed) refresh();
        p = get_board_pos();
        if(p >= 0 && i.type == INPUT_MOUSE_DOWN)
        {
            int pc = board[p / 8][p % 8];
            if(pc != CH_EMPTY && (pc & 1) == ch_current_turn())
                selpos = p;
        }
        if(p != pos)
        {
            draw_board();
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

#endif
