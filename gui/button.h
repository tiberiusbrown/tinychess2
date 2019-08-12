#ifndef BUTTON_H
#define BUTTON_H

#include "common.h"
#include "draw_helpers.h"
#include "text.h"

typedef struct button
{
    int16_t x1, y1, x2, y2;
    char const* txt;
    int pressed, hovered;
    pixel ctext, coutline, cpressed, chovered, cunpressed;
} button;

static int button_hovered(button const* b)
{
    return
        mx > b->x1 &&
        mx < b->x2 &&
        my > b->y1 &&
        my < b->y2;
}

static void button_init(button* b, char const* txt, int x1, int y1, int x2, int y2)
{
    b->x1 = (int16_t)x1;
    b->x2 = (int16_t)x2;
    b->y1 = (int16_t)y1;
    b->y2 = (int16_t)y2;
    b->txt = txt;
    b->hovered = b->pressed = 0;
    b->ctext = BUTTON_TEXT_COLOR;
    b->coutline = BUTTON_OUTLINE_COLOR;
    b->cpressed = BUTTON_PRESSED_COLOR;
    b->chovered = BUTTON_HOVERED_COLOR;
    b->cunpressed = BUTTON_UNPRESSED_COLOR;
}

static void button_draw_pressed(button const* b)
{
    draw_filled_box(
        b->x1, b->y1, b->x2, b->y2,
        b->coutline, b->cpressed);
    text_centered(
        b->txt,
        (b->x1 + b->x2) / 2 + 1, (b->y1 + b->y2) / 2 + 2,
        b->ctext);
}

static void button_draw_unpressed(button const* b)
{
    draw_filled_box(
        b->x1, b->y1, b->x2, b->y2,
        b->coutline,
        button_hovered(b) ? b->chovered : b->cunpressed);
    text_centered(
        b->txt,
        (b->x1 + b->x2) / 2, (b->y1 + b->y2) / 2 + 1,
        b->ctext);
}

static void button_draw(button const* b)
{
    if(b->pressed && b->hovered)
        button_draw_pressed(b);
    else
        button_draw_unpressed(b);
}

/*
returns 1 if button was just released after being pressed
sets 'refresh_needed' to 1 or doesn't change it
*/
static int button_input(button* b, input const* i, int* refresh_needed)
{
    int released = 0;
    int pressed = b->pressed;
    int hovered = button_hovered(b);
    if(pressed)
    {
        if(i->type == INPUT_MOUSE_UP)
        {
            pressed = 0;
            if(hovered)
                released = 1;
        }
    }
    else
    {
        if(hovered && i->type == INPUT_MOUSE_DOWN)
            pressed = 1;
    }
    if(b->hovered != hovered || b->pressed != pressed)
    {
        b->hovered = hovered;
        b->pressed = pressed;
        button_draw(b);
        *refresh_needed = 1;
    }
    return released;
}

#endif
