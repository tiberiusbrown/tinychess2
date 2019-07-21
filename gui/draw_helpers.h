#ifndef DRAW_HELPERS_H
#define DRAW_HELPERS_H

#include "common.h"

static void draw_hline(int x1, int x2, int y, pixel p)
{
    if(y < 0 || y >= FBH) return;
    x1 = MAX(x1, 0);
    x2 = MIN(x2, FBW - 1);
    set_pixels(pixels + y * FBW + x1, p, x2 - x1 + 1);
}

static void draw_vline(int x, int y1, int y2, pixel p)
{
    if(x < 0 || x >= FBW) return;
    y1 = MAX(y1, 0);
    y2 = MIN(y2, FBH - 1);
    while(y1 <= y2)
        set_pixel_unsafe(x, y1++, p);
}

static void draw_rect_unsafe(
    int x1, int y1, int x2, int y2,
    pixel pb)
{
    int yi;
    int n = x2 - x1 + 1;
    pixel* p = pixels + y1 * FBW + x1;
    for(yi = y1; yi <= y2; ++yi, p += FBW)
        set_pixels(p, pb, n);
}

static void draw_box(
    int x1, int y1, int x2, int y2,
    pixel p)
{
    draw_hline(x1, x2, y1, p);
    draw_hline(x1, x2, y2, p);
    draw_vline(x1, y1, y2, p);
    draw_vline(x2, y1, y2, p);
}

static void draw_filled_box(
    int x1, int y1, int x2, int y2,
    pixel pf, pixel pb)
{

    /* foreground edge */
    draw_box(x1, y1, x2, y2, pf);

    x1 = MAX(x1 + 1, 0);
    y1 = MAX(y1 + 1, 0);
    x2 = MIN(x2 - 1, FBW - 1);
    y2 = MIN(y2 - 1, FBH - 1);

    /* background fill */
    draw_rect_unsafe(x1, y1, x2, y2, pb);
}

#endif
