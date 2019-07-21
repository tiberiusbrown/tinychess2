#ifndef FORMAT_H
#define FORMAT_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* dead-simple printf-style format. supports only %d, %c, and %s */

#define FORMAT_NUM_BUFS 4
static char format_buf[FORMAT_NUM_BUFS][1 * 1024];
static int format_buf_index = 0;
#define MAX_LENGTH (sizeof(format_buf) - 1) 

char* capitalize(char* s)
{
    if(*s >= 'a' && *s <= 'z') *s += ('A' - 'a');
    return s;
}

static FORCEINLINE void vsnprintf_out(char** buf, char* endbuf, char c)
{
    if(endbuf > *buf)
    {
        **buf = c;
        ++(*buf);
    }
}

typedef struct vsnprintf_mods
{
    int plus;        /* always show plus sign (plus sign) */
#if 0
    int space;       /* include space for missing minus sign */
    int align_left;  /* left justify (minus sign) */
    int zero_pad;    /* left-pad with zeros (leading zero) */
    int width;       /* total width (negative for unconstrained) */
    int prec;        /* width of fractional part */
#endif
} vsnprintf_mods;

static void vsnprintf_get_mods(char const** fmt, vsnprintf_mods* mods)
{
    /* default values */
    mods->plus = 0;
#if 0
    mods->space = 0;
    mods->align_left = 0;
    mods->zero_pad = 0;
    mods->width = -1;
    mods->prec = -1;
#endif

    while(1)
    {
        char c = **fmt;

        if(0)
            ;
        else if(c == '+')
            mods->plus = 1;
#if 0
        else if(c == ' ')
            mods->space = 1;
        else if(c == '-')
            mods->align_left = 1;
        else if(c == '0')
            mods->zero_pad = 1;
        else if(c >= '1' && c <= '9')
        {
            mods->width = get_numeric_mod(fmt);
            continue;
        }
        else if(c == '.')
        {
            ++(*fmt);
            mods->prec = get_numeric_mod(fmt);
            continue;
        }
#endif
        else
            break;

        ++(*fmt);
    }
}

static void vsnprintf_out_pad(
    char** buf,
    char* endbuf,
    char const* str,
    vsnprintf_mods const* mods)
{
    int length = 0;
    char c;
#if 0
    if(mods->width >= 0)
    {
        char const* cp = str;
        while(*cp++ != '\0')
            ++length;
        length = mini(mods->width, mods->width - length);
        if(!mods->align_left)
        {
            if(mods->zero_pad)
            {
                if(*str == '+' || *str == '-')
                    vsnprintf_out(buf, endbuf, *str++);
            }
            while(length-- > 0)
                vsnprintf_out(buf, endbuf, mods->zero_pad ? '0' : ' ');
        }
    }
#else
    (void)mods;
#endif
    while((c = *str++) != '\0')
        vsnprintf_out(buf, endbuf, c);
    while(length-- > 0)
        vsnprintf_out(buf, endbuf, ' ');
}

#define VSNPRINTF_OUT(c_) vsnprintf_out(&buf, endbuf, c_)
static int vsnprintf(char* b, int n, char const* fmt, va_list va)
{
    char* buf = b;
    char* endbuf = b + n - 1;
    char c;

    char temp1[64];
    char temp2[64];

    vsnprintf_mods mods;

    while(buf < endbuf && (c = *fmt++) != '\0')
    {
        if(c != '%')
        {
            VSNPRINTF_OUT(c);
            continue;
        }

        vsnprintf_get_mods(&fmt, &mods);
        c = *fmt++;
        if(c == '%')
            VSNPRINTF_OUT('%');
        else if(c == 'c')
        {
            char xc = (char)va_arg(va, int);
            VSNPRINTF_OUT(xc);
        }
        else if(c == 's')
        {
            char const* xs = va_arg(va, char const*);
            vsnprintf_out_pad(&buf, endbuf, xs, &mods);
        }
        else if(c == 'd')
        {
            int xi = va_arg(va, int);
            int xp = (xi < 0 ? -xi : xi);
            char* t1 = temp1;
            char* t2 = temp2;
            do
            {
                *t1++ = '0' + (xp % 10);
                xp /= 10;
            } while(xp != 0);
            if(xi < 0) *t2++ = '-';
            else if(mods.plus) *t2++ = '+';
#if 0
            else if(mods.space) *t2++ = ' ';
#endif
            while(t1-- > temp1)
                *t2++ = *t1;
            *t2++ = '\0';
            vsnprintf_out_pad(&buf, endbuf, temp2, &mods);
        }
    }

    *buf = '\0';

    return (int)(ptrdiff_t)(buf - b);
}
#undef VSNPRINTF_OUT

static char* vformat(char const* fmt, va_list v)
{
    char* b = format_buf[format_buf_index];
    vsnprintf(b, sizeof(format_buf[0]), fmt, v);
    format_buf_index = (format_buf_index + 1) % FORMAT_NUM_BUFS;
    return b;
}

static char* format(char const* fmt, ...)
{
    va_list v;
    char* b;
    va_start(v, fmt);
    b = vformat(fmt, v);
    va_end(v);
    return b;
}

static int snprintf(char* buf, int n, char const* fmt, ...)
{
    int r;
    va_list v;
    va_start(v, fmt);
    r = vsnprintf(buf, n, fmt, v);
    va_end(v);
    return r;
}

#endif
