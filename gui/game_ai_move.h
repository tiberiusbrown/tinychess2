#ifndef GAME_AI_MOVE_H
#define GAME_AI_MOVE_H

#include "game_draw.h"

static void ai_move(void)
{
    ch_search_limits limits =
    {
        0, 0, 300, 0, 0
    };

    cur_turn = ch_current_turn();
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
    animate_move(ch_move_fr_sq(prevmv), ch_move_to_sq(prevmv));
    ch_do_move(prevmv);
    update_board();
    draw_board();
    draw_pieces();
    refresh();
}

#endif
