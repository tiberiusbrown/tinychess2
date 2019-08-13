#ifndef TEXT_H
#define TEXT_H

#include "common.h"
#include "font.h"

#include <stddef.h>

#include <ALLDATA_COMPRESSED.h>

#include <stddef.h>

#define TEXT_ROW_HEIGHT 9

static void text_ex(
    char const* txt,
    int x,
    int y,
    pixel p,
    int len,  /* text length, or zero to determine automatically */
    int wrap, /* nonzero for word-wrap width, zero to disable, negative to draw */
    int* w,   /* output width */
    char const** wc   /* pointer to next line's char for wrap */
)
{
    char const* t = txt;
    int c;
    font_character const* f;
    uint8_t fc;
    int ix, iy, xc = x;
    if(len == 0) len = INT32_MAX;
    *wc = NULL;
    while((c = (unsigned char)(*t++)) != '\0' && len-- > 0)
    {
        c -= 32;
        if(c >= (int)(FONT_DATA_SIZE / sizeof(font_character)))
            continue;
        f = &FONT_DATA[(int)c];

        if(wrap < 0) /* draw enabled */
        {
            for(ix = 0; ix < 7; ++ix)
            {
                fc = f->cols[ix];
                for(iy = 0; iy < 8; ++iy)
                {
                    if(fc & 0x1)
                        set_pixel_safe(xc + ix, y + iy, p);
                    fc >>= 1;
                }
            }
        }

        xc += f->width;

        if(wrap > 0 && xc > x + wrap + 1)
        {
            char const* ct = t - 1;
            while(*ct != ' ' && ct > txt)
                --ct;
            if(ct == txt)
                *wc = t - 1;
            else
                *wc = ct + 1;
            break;
        }
    }

    if(wrap > 0 && !*wc)
        *wc = t - 1;
    *w = xc - x;
    if(*w > 0) *w -= 1;
}

static int text_width(char const* t)
{
    int w;
    char const* wc;
    text_ex(t, 0, 0, PIXEL_BLACK, 0, 0, &w, &wc);
    return w;
}

static void text(
    char const* t,
    int x,
    int y,
    pixel p)
{
    int w;
    char const* wc;
    text_ex(t, x, y, p, 0, -1, &w, &wc);
}

static void text_centered(
    char const* t,
    int x, int y,
    pixel p)
{
    int w2, dw;
    char const* dwc;
    w2 = text_width(t) / 2;
    text_ex(t, x - w2, y - TEXT_ROW_HEIGHT / 2, p, 0, -1, &dw, &dwc);
}

static int text_wrapped_lines(char const* t, int wrap_width)
{
    char const* wcprev = t;
    int w = 1, lines = 0;
    while(w > 0 && *wcprev)
    {
        char const* wc;
        text_ex(wcprev, 0, 0, PIXEL_BLACK, 0, wrap_width, &w, &wc);
        wcprev = wc;
        ++lines;
    }
    return lines;
}

static void text_wrapped(
    char const* t,
    int x, int y,
    pixel p,
    int wrap_width)
{
    char const* wcprev = t;
    int w = 1;
    while(w > 0 && *wcprev)
    {
        char const* wc;
        char const* dummy;
        int len;
        text_ex(wcprev, x, y, p, 0, wrap_width, &w, &wc);
        len = (int)(intptr_t)(wc - wcprev);
        text_ex(wcprev, x, y, p, len, -1, &w, &dummy);
        wcprev = wc;
        y += TEXT_ROW_HEIGHT;
    }
}


#endif
