#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "button.h"
#include "common.h"
#include "draw_helpers.h"
#include "text.h"

#define MSGBOX_DEFAULT_W 200
#define MSGBOX_MIN_W 90

#define MSGBOX_X1 ((FBW - MSGBOX_W) / 2)
#define MSGBOX_X2 (MSGBOX_X1 + MSGBOX_W - 1)
#define MSGBOX_Y1 ((FBH - MSGBOX_H) / 2)
#define MSGBOX_Y2 (MSGBOX_Y1 + MSGBOX_H - 1)

#define MSGBOX_BUTTON_W 80
#define MSGBOX_BUTTON_H 12

#define MSGBOX_MAX_LINES \
    ((FBH - MSGBOX_BUTTON_H - 8) / TEXT7X8_ROW_HEIGHT)

void messagebox(char const* msg)
{
    int w = MSGBOX_DEFAULT_W, h = 0;
    int x1, y1, x2, y2;
    button b;

    /* calculate message box dimensions */
    while(w < FBW)
    {
        int lines = text_wrapped_lines(msg, w - 6);
        if(lines <= MSGBOX_MAX_LINES)
        {
            if(lines == 1)
            {
                w = text_width(msg) + 6;
                w = MAX(w, MSGBOX_MIN_W);
            }
            h = lines * TEXT_ROW_HEIGHT + MSGBOX_BUTTON_H + 8;
            break;
        }
        ++w;
    }

    x1 = (FBW - w) / 2;
    y1 = (FBH - h) / 2;
    x2 = x1 + w;
    y2 = y1 + h;

    button_init(
        &b, STR_CONTINUE,
        (FBW - MSGBOX_BUTTON_W) / 2, y2 - 14,
        (FBW + MSGBOX_BUTTON_W) / 2, y2 - 2);

    {
        int n;
        for(n = 0; n < FBW * FBH; ++n)
            LIGHTEN_PIXEL(pixels[n]);
    }
    draw_filled_box(
        x1, y1, x2, y2,
        OUTLINE_COLOR, BACKGROUND_COLOR);
    text_wrapped(
        msg,
        x1 + 3, y1 + 3,
        TEXT_COLOR,
        w - 6);
    button_draw(&b);
    refresh();

    for(;;)
    {
        input i;
        int refresh_needed = 0;
        wait_for_input(&i);
        if(button_input(&b, &i, &refresh_needed))
            break;
        if(refresh_needed)
            refresh();
    }
}

#endif
