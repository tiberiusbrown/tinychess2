#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#include "config.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#define MYALIGNOF(t) offsetof(struct{char __a;t __b;},__b)

#ifdef _MSC_VER
#define ALIGN(bytes) __declspec(align(bytes))
#elif defined(__GNUC__)
#define ALIGN(bytes) __attribute__((aligned(bytes)))
#else
#error "Unsupported"
#endif

#ifdef _MSC_VER
#define FORCEINLINE __forceinline
#elif __GNUC__
#define FORCEINLINE __attribute__((always_inline)) inline
#else
#define FORCEINLINE
#endif

/* window dimensions */
static int ww;
static int wh;

/* mouse position */
static int mx;
static int my;

#if ENABLE_MS
/* current time in ms */
static uint32_t ms;
#endif

enum input_type
{
    INPUT_NONE,
    INPUT_MOUSE_MOVE,
    INPUT_MOUSE_DOWN,
    INPUT_MOUSE_UP,
#if ENABLE_RBUTTON
    INPUT_MOUSE2_DOWN,
    INPUT_MOUSE2_UP,
#endif
#if ENABLE_MS
    INPUT_TIMER,
#endif
};
typedef struct input
{
    enum input_type type;
} input;

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SIGN(x) ((x) < 0 ? -1 : (x) > 0 ? 1 : 0)
#define CLAMP(x, a, b) ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))

/* methods defined in start_*.c */
static void refresh(void);
static int wait_for_input(input* i);
#if ENABLE_MS
static int wait_for_input_timeout(input* i, int* timeout_ms);
#endif
static uint32_t get_ms(void);
static void shutdown(int code);
static void* alloc(uint32_t bytes);
static void dealloc(void* p);

/* defined in run.h */
static void flush_input(void);
static void delay_ms(int ms);

#if USE_COLORMAP && COLORMAP666
typedef uint8_t pixel;
#define PIXEL_RGB_ENCODE(r, g, b) (uint8_t)( \
    (uint8_t)(r) + \
    ((uint8_t)(g) * 6) + \
    ((uint8_t)(b) * 6 * 6) + \
    0)
#define PIXEL_RGBA(r, g, b, a) PIXEL_RGB_ENCODE( \
    ((r) + 0) / 43, \
    ((g) + 0) / 43, \
    ((b) + 0) / 43)
#define PIXEL_DECODE_R(p) ((p) % 6)
#define PIXEL_DECODE_G(p) (((p) / 6) % 6)
#define PIXEL_DECODE_B(p) ((p) / 6 / 6)
#elif USE_COLORMAP && !COLORMAP666
#ifdef _MSC_VER
#pragma warning(disable:4214)
#endif
typedef struct pixel
{
    uint8_t r : 3;
    uint8_t g : 3;
    uint8_t b : 2;
} pixel;
#define PIXEL_RGBA(r, g, b, a) { \
    ((uint8_t)(r) >> 5) & 0x7, \
    ((uint8_t)(g) >> 5) & 0x7, \
    ((uint8_t)(b) >> 6) & 0x3 }
#else
#if defined(_WIN32)
typedef struct pixel
{
    uint8_t b, g, r, a;
} pixel;
#define PIXEL_RGBA(r, g, b, a) { b, g, r, a }
#elif defined(__linux__)
typedef struct pixel
{
    uint8_t r, g, b, a;
} pixel;
#define PIXEL_RGBA(r, g, b, a) { r, g, b, a }
#endif
#endif

#if defined(_WIN32)
static pixel* pixels;
#elif defined(__linux__)
ALIGN(4)
pixel pixels[FBW * FBH];
#endif

#define PIXEL_RGB(r, g, b) PIXEL_RGBA(r, g, b, 255)

#define PIXEL_HEX(v) PIXEL_RGB( \
    ((0x##v) >> 16) & 0xff, \
    ((0x##v) >>  8) & 0xff, \
    ((0x##v) >>  0) & 0xff)

#define PIXEL_SHADE(x) PIXEL_RGB(x, x, x)

static pixel const PIXEL_BLACK  = PIXEL_RGB(  0,  0,  0);
static pixel const PIXEL_WHITE  = PIXEL_RGB(255,255,255);
static pixel const PIXEL_RED    = PIXEL_RGB(255,  0,  0);
static pixel const PIXEL_GREEN  = PIXEL_RGB(  0,255,  0);
static pixel const PIXEL_BLUE   = PIXEL_RGB(  0,  0,255);
static pixel const PIXEL_YELLOW = PIXEL_RGB(255,255,  0);

static pixel const OUTLINE_COLOR = PIXEL_RGB(50, 50, 70);
static pixel const TEXT_COLOR = PIXEL_SHADE(220);
static pixel const BACKGROUND_COLOR = PIXEL_SHADE(30);
static pixel const BACKGROUND2_COLOR = PIXEL_SHADE(60);
static pixel const BACKGROUND3_COLOR = PIXEL_SHADE(100);

static pixel const BUTTON_OUTLINE_COLOR = PIXEL_RGB(100, 100, 130);
static pixel const BUTTON_HOVERED_COLOR = PIXEL_SHADE(170);
static pixel const BUTTON_PRESSED_COLOR = PIXEL_SHADE(130);
static pixel const BUTTON_UNPRESSED_COLOR = PIXEL_SHADE(200);
static pixel const BUTTON_TEXT_COLOR = PIXEL_SHADE(30);

#if USE_COLORMAP && COLORMAP666
#define INVERT_PIXEL(p) do { p = PIXEL_RGB_ENCODE( \
    5 - PIXEL_DECODE_R(p), \
    5 - PIXEL_DECODE_G(p), \
    5 - PIXEL_DECODE_B(p)); } while(0)
#elif USE_COLORMAP && !COLORMAP666
#define INVERT_PIXEL(p) do { \
    p.r = 7 - p.r; p.g = 7 - p.g; p.b = 3 - p.b; \
    } while(0)
#else
#define INVERT_PIXEL(p) do { \
    p.r = 255 - p.r; p.g = 255 - p.g; p.b = 255 - p.b; \
    } while(0)
#endif

#if USE_COLORMAP && COLORMAP666
#define DARKEN_PIXEL(p) do { p = PIXEL_RGB_ENCODE( \
    PIXEL_DECODE_R(p) / 2, \
    PIXEL_DECODE_G(p) / 2, \
    PIXEL_DECODE_B(p) / 2); } while(0)
#else
#define DARKEN_PIXEL(p) do { p.r /= 2; p.g /= 2; p.b /= 2; } while(0)
#endif

#if USE_COLORMAP
#if COLORMAP666
#define LIGHTEN_PIXEL(p) do { \
    uint8_t _r = PIXEL_DECODE_R(p); \
    uint8_t _g = PIXEL_DECODE_G(p); \
    uint8_t _b = PIXEL_DECODE_B(p); \
    p = PIXEL_RGB_ENCODE( \
        _r + (5 - _r) / 2, \
        _g + (5 - _g) / 2, \
        _b + (5 - _b) / 2); \
    } while(0)
#else
#define LIGHTEN_PIXEL(p) do { \
    p.r += (7 - p.r) / 2; \
    p.g += (7 - p.g) / 2; \
    p.b += (3 - p.b) / 2; \
    } while(0)
#endif
#else
#define LIGHTEN_PIXEL(p) do { \
    p.r += (255 - p.r) / 2; \
    p.g += (255 - p.g) / 2; \
    p.b += (255 - p.b) / 2; \
    } while(0)
#endif

static void my_strncpy(char* dst, char const* src, int n)
{
    do *dst++ = *src;
    while(*src++ && --n > 0);
    if(n > 0) *dst = '\0';
}

static int my_strcmp(char const* a, char const* b)
{
    while(*a)
    {
        if(*a != *b)
            return (int)(*a - *b);
        ++a;
        ++b;
    }
    return *b ? -1 : 0;
}

#if defined(_MSC_VER)
#pragma intrinsic(__stosb)
static void my_memset32(void* p, uint32_t x, int n32)
{
    __stosd((unsigned long*)p, x, n32);
}
static FORCEINLINE void my_memset(void* p, uint8_t x, int n)
{
    __stosb((unsigned char*)p, x, n);
}
#else
static void my_memset32(void* p, uint32_t  x, int n32)
{
    uint32_t* i = (uint32_t*)p;
    while(n32-- > 0) *i++ = x;
}
static void my_memset(void* p, uint8_t x, int n)
{
    uint8_t* i = (uint8_t*)p;
    if(!((intptr_t)p & 0x3) && !(n & 0x3))
        return my_memset32(p, 0, n / 4);
    while(n-- > 0) *i++ = x;
}
#endif

#if 1
#define memzero(p__, n__) my_memset(p__, 0, n__)
#define memzero32(p__, n__) my_memset32(p__, 0, n__)
#else
#if defined(_MSC_VER)
#pragma intrinsic(__stosb)
static void memzero32(void* p, int n32)
{
    __stosd((unsigned long*)p, 0, n32);
}
static FORCEINLINE void memzero(void* p, int n)
{
    __stosb((unsigned char*)p, 0, n);
}
#else
static void memzero32(void* p, int n32)
{
    uint32_t* i = (uint32_t*)p;
    while(n32-- > 0) *i++ = 0;
}
static void memzero(void* p, int n)
{
    uint8_t* i = (uint8_t*)p;
    if(!((intptr_t)p & 0x3) && !(n & 0x3))
        return memzero32(p, n / 4);
    while(n-- > 0) *i++ = 0;
}
#endif
#endif

static int memequal32(void const* a, void const* b, int n32)
{
    uint32_t const* ca = (uint32_t const*)a;
    uint32_t const* cb = (uint32_t const*)b;
    while(n32-- > 0)
        if(*ca++ != *cb++) return 0;
    return 1;
}

static int memequal(void const* a, void const* b, int n)
{
    char const* ca;
    char const* cb;
    if(!((intptr_t)a & 0x3) && !((intptr_t)b & 0x3) && !(n & 0x3))
        return memequal32(a, b, n / 4);
    ca = (char const*)a;
    cb = (char const*)b;
    while(n-- > 0)
        if(*ca++ != *cb++) return 0;
    return 1;
}

#if defined(_MSC_VER)
#pragma intrinsic(__movsd)
static FORCEINLINE void my_memcpy32(void* dst, void const* src, int n)
{
    __movsd((unsigned long*)dst, (unsigned long const*)src, n);
}
#else
static void my_memcpy32(void* dst, void const* src, int n32)
{
    uint32_t* cd = (uint32_t*)dst;
    uint32_t const* cs = (uint32_t const*)src;
    while(n32-- > 0)
        *cd++ = *cs++;
}
#endif

#if defined(_MSC_VER)
#pragma intrinsic(__movsd)
static FORCEINLINE void my_memcpy(void* dst, void const* src, int n)
{
    __movsb((unsigned char*)dst, (unsigned char const*)src, n);
}
#else
static void my_memcpy(void* dst, void const* src, int n)
{
    char* cd;
    char const* cs;
    if(!((intptr_t)dst & 0x3) && !((intptr_t)src & 0x3) && !(n & 0x3))
    {
        my_memcpy32(dst, src, n / 4);
        return;
    }
    cd = (char*)dst;
    cs = (char const*)src;
    while(n-- > 0)
        *cd++ = *cs++;
}
#endif

static void set_pixels(pixel* base, pixel p, int n)
{
#if !USE_COLORMAP
    my_memset32(base, *((uint32_t*)(&p)), n);
#else
    my_memset(base, *((uint8_t*)(&p)), n);
#endif
}

static void clear_pixels(pixel p)
{
    set_pixels(pixels, p, FBW * FBH);
}

static void fill_rect_pixels_unsafe(int x, int y, int w, int h, pixel p)
{
    pixel* pp = &pixels[y * FBW + x];
    while(h-- > 0)
    {
        set_pixels(pp, p, w);
        pp += FBW;
    }
}

static void fill_rect_pixels_safe(int x, int y, int w, int h, pixel p)
{
    if(x < 0)
    {
        w += x;
        x = 0;
    }
    if(y < 0)
    {
        h += y;
        y = 0;
    }
    if(w <= 0 || h <= 0) return;
    w = MIN(w, FBW - x);
    h = MIN(h, FBH - y);
    fill_rect_pixels_unsafe(x, y, w, h, p);
}

static void copy_pixels(pixel* dst, pixel const* src, int n)
{
    while(n-- > 0)
        *dst++ = *src++;
}

static pixel get_pixel_unsafe(int x, int y)
{
    return pixels[y * FBW + x];
}

static pixel get_pixel_safe(int x, int y)
{
    static pixel const DUMMY = { 0 };
    int i = y * FBW + x;
    if((unsigned)i < (FBW * FBH))
        return pixels[i];
    return DUMMY;
}

static void set_pixel_unsafe(int x, int y, pixel p)
{
    pixels[y * FBW + x] = p;
}

static void set_pixel_safe(int x, int y, pixel p)
{
    int i = y * FBW + x;
    if((unsigned)i < (FBW * FBH))
        pixels[i] = p;
}

static void set_pixel_safe_bounds(int x, int y, pixel p)
{
    if(x < 0 || y < 0 || x >= FBW || y >= FBH)
        return;
    pixels[y * FBW + x] = p;
}

#define FORMAT_CONCAT3 (FORMAT_CONCAT4 + 2)
#define FORMAT_CONCAT2 (FORMAT_CONCAT4 + 4)
#define FORMAT_CONCAT1 (FORMAT_CONCAT4 + 6)

#endif
