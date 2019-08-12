#ifndef GAME_PLAY_H
#define GAME_PLAY_H

#include "button.h"
#include "game_draw.h"
#include "game_player_move.h"
#include "game_ai_move.h"

static void game_play(void)
{
    bestmv = 0;
    prevmv = 0;
    selpos = -1;

    button_draw(&btn_undo);

    update_board();
    draw_board();
    draw_pieces();
    refresh();

    for(;;)
    {
        if(ch_is_checkmate() || ch_is_draw())
        {
            input i;
            if(ch_is_checkmate())
                text("Checkmate", 190, 10, TEXT_COLOR);
            else
                text("Stalemate", 190, 10, TEXT_COLOR);
            refresh();
            for(;;) wait_for_input(&i);
        }

        if(ishuman[ch_current_turn()])
            player_move();
        else
            ai_move();
    }
}

#endif
