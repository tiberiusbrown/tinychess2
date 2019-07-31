#ifndef SETTINGS_H
#define SETTINGS_H

#include "common.h"

static struct settings
{
    int ww, wh;
    int fullscreen;
} settings;

static void settings_default(void)
{
    settings.ww = FBW * 4;
    settings.wh = FBH * 4;
    settings.fullscreen = 0;
}

static void settings_init(void)
{
    settings_default();
}

#endif
