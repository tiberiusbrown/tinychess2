#ifndef ALLDATA_H
#define ALLDATA_H

#include <stdint.h>

#define FONT_DEFINE_DATA
#include "font.h"

#define IMG_DEFINE_DATA
#include "img.h"

#include "config.h"

typedef enum type_
{
    END, STR, BIN_FILE, TXT_FILE
} type_;

static inline int type_is_file(type_ t)
{
    switch(t)
    {
    case BIN_FILE:
    case TXT_FILE:
        return 1;
    default:
        return 0;
    }
}

#define STR_DATA(x, t) { STR, #x, (uint8_t const*)x, sizeof(x), 0, #t }
#define STR_DATA_NAME(n, t, x) { STR, #n, (uint8_t const*)x, sizeof(x), 0, #t }
#define BIN_FILE_DATA(n) { BIN_FILE, n, NULL, 0, 0, NULL }
#define TXT_FILE_DATA(n) { TXT_FILE, n, NULL, 0, 0, NULL }
#define END_DATA { END, NULL, NULL, 0, 0, NULL }

#define STRING_NAME(n, x) STR_DATA_NAME(n, char, x)

struct datadecl
{
    type_ type;
    char const* name; /* or filename */
    uint8_t const* data;
    size_t size;
    int img_req_comp;
    char const* type_str;
};

#if defined(_WIN32)
static char const PROCNAMES_KERNEL32[] =
    "FreeLibrary"          "\0"
    "ExitProcess"          "\0"
    "GetModuleHandleA"     "\0"
    "Sleep"                "\0"
    "GetProcessHeap"       "\0"
    "HeapAlloc"            "\0"
    "HeapFree"             "\0"
#if ENABLE_MS
    "QueryPerformanceFrequency" "\0"
    "QueryPerformanceCounter" "\0"
#endif
;
#endif

static struct datadecl const ALLDATA[] =
{
    STR_DATA(IMG_DATA, img_data),
    STR_DATA_NAME(FONT_DATA, font_character, FONT),

#if defined(_WIN32)
    STR_DATA_NAME(PROCNAMES_KERNEL32, char, PROCNAMES_KERNEL32),
    STR_DATA_NAME(PROCNAMES_USER32, char,
        "RegisterClassExA"   "\0"
        "LoadCursorA"        "\0"
        "CreateWindowExA"    "\0"
        "DestroyWindow"      "\0"
        "ShowWindow"         "\0"
        "UpdateWindow"       "\0"
        "DefWindowProcA"     "\0"
        "AdjustWindowRectEx" "\0"
        "SetWindowPlacement" "\0"
        "GetWindowPlacement" "\0"
        "SetWindowLongA"     "\0"
        "SetWindowPos"       "\0"
        "GetMonitorInfoA"    "\0"
        "MonitorFromWindow"  "\0"
        "PostQuitMessage"    "\0"
        "TranslateMessage"   "\0"
        "DispatchMessageA"   "\0"
        "PeekMessageA"       "\0"
        "GetDC"              "\0"
        "ReleaseDC"          "\0"
        "BeginPaint"         "\0"
        "EndPaint"           "\0"
        "InvalidateRect"     "\0"
        "GetCursorPos"       "\0"
        "ScreenToClient"     "\0"
		"FillRect"           "\0"
    ),
    STR_DATA_NAME(PROCNAMES_GDI32, char,
        "CreateCompatibleDC" "\0"
        "CreateDIBSection"   "\0"
		"CreateSolidBrush"   "\0"
        "DeleteDC"           "\0"
        "DeleteObject"       "\0"
        "SelectObject"       "\0"
        "BitBlt"             "\0"
        "StretchBlt"         "\0"
        "SetStretchBltMode"  "\0"
    ),
#if ENABLE_THREAD
    STR_DATA_NAME(PROCNAMES_THREAD, char,
        "InitializeCriticalSection" "\0"
        "DeleteCriticalSection"     "\0"
        "EnterCriticalSection"      "\0"
        "LeaveCriticalSection"      "\0"
        "CreateThread"              "\0"
        "CloseHandle"               "\0"
        "WaitForSingleObject"       "\0"
        "WaitForMultipleObjects"    "\0"
        "CreateEventA"              "\0"
        "SetEvent"                  "\0"
        "ResetEvent"                "\0"
    ),
#endif
    STR_DATA_NAME(KERNEL32, char, "kernel32"),
    STR_DATA_NAME(USER32, char, "user32"),
    STR_DATA_NAME(GDI32, char, "gdi32"),
#elif defined(__linux)
    STR_DATA_NAME(PROCNAMES_X11, char,
        "XOpenDisplay"        "\0"
        "XCloseDisplay"       "\0"
        "XCreateGC"           "\0"
        "XCreateColormap"     "\0"
        "XCreateWindow"       "\0"
        "XDestroyWindow"      "\0"
        "XCreateImage"        "\0"
        "XPutImage"           "\0"
        "XGetImage"           "\0"
        "XDestroyImage"       "\0"
        "XSelectInput"        "\0"
        "XInternAtom"         "\0"
        "XSetWMProtocols"     "\0"
        "XMapWindow"          "\0"
        "XStoreName"          "\0"
        "XNextEvent"          "\0"
        "XSync"               "\0"
        "XQueryPointer"       "\0"
        "XMatchVisualInfo"    "\0"
        "XGetGeometry"        "\0"
        "XPending"            "\0"
    ),
    STR_DATA_NAME(PROCNAMES_C, char,
        "exit"                "\0"
        "usleep"              "\0"
        "gettimeofday"        "\0"
        "malloc"              "\0"
        "free"                "\0"
    ),
#if ENABLE_XSHM
    STR_DATA_NAME(PROCNAMES_C_SHM, char,
        "shmget"              "\0"
        "shmat"               "\0"
        "shmdt"               "\0"
        "shmctl"              "\0"
    ),
    STR_DATA_NAME(PROCNAMES_XEXT, char,
        "XShmAttach"          "\0"
        "XShmDetach"          "\0"
        "XShmCreateImage"     "\0"
        "XShmPutImage"        "\0"
    ),
    STR_DATA_NAME(LIBXEXT_SO, char, "libXext.so.6"),
#endif
    STR_DATA_NAME(LIBX11_SO, char, "libX11.so.6"),
    STR_DATA_NAME(LIBC_SO, char, "libc.so.6"),
#if ENABLE_THREAD
    STR_DATA_NAME(LIBPTHREAD_SO, char, "libpthread.so.0"),
#endif
#if ENABLE_OPENGL
    STR_DATA_NAME(LIBGL_SO, char, "libGL.so.1"),
    STR_DATA_NAME(PROCNAME_GLXGETPROCADDRESSARB, char, "glXGetProcAddressARB"),
    STR_DATA_NAME(PROCNAME_GLXSWAPINTERVALEXT, char, "glXSwapIntervalEXT"),
    STR_DATA_NAME(PROCNAME_GLXSWAPINTERVALMESA, char, "glXSwapIntervalMESA"),
    STR_DATA_NAME(PROCNAME_GLXSWAPINTERVALSGI, char, "glXSwapIntervalSGI"),
    STR_DATA_NAME(PROCNAMES_GL, char,
        "glXChooseVisual"     "\0"
        "glXCreateContext"    "\0"
        "glXDestroyContext"   "\0"
        "glXMakeCurrent"      "\0"
        "glXSwapBuffers"      "\0"
        "glGenTextures"       "\0"
        "glDeleteTextures"    "\0"
        "glBindTexture"       "\0"
        "glTexImage2D"        "\0"
        "glTexSubImage2D"     "\0"
        "glTexParameteri"     "\0"
        "glDrawArrays"        "\0"
        "glVertexPointer"     "\0"
        "glTexCoordPointer"   "\0"
        "glEnableClientState" "\0"
        "glEnable"            "\0"
        "glViewport"          "\0"
        "glClear"             "\0"
    ),
#endif
#if ENABLE_THREAD
    STR_DATA_NAME(PROCNAMES_THREAD, char,
        "pthread_mutex_init" "\0"
        "pthread_mutex_destroy" "\0"
        "pthread_mutex_lock" "\0"
        "pthread_mutex_unlock" "\0"
        "pthread_create" "\0"
        "pthread_join" "\0"
    ),
#endif
    STR_DATA_NAME(ATOM_WM_PROTOCOLS, char, "WM_PROTOCOLS"),
    STR_DATA_NAME(ATOM_WM_DELETE_WINDOW, char, "WM_DELETE_WINDOW"),
    STR_DATA_NAME(ATOM__NEW_WM_PING, char, "_NET_WM_PING"),
#endif
    END_DATA
};
static size_t const NUMDATA = sizeof(ALLDATA) / sizeof(ALLDATA[0]) - 1;

static struct datadecl const ALLDATA_DONT_COMPRESS[] =
{

    STRING_NAME(WINDOW_TITLE, "tinychess"),

#if defined(_WIN32)
    STR_DATA_NAME(CLASS_NAME, char, "MYW"),
#endif

    END_DATA
};
static size_t const NUMDATA_DONT_COMPRESS =
    sizeof(ALLDATA_DONT_COMPRESS) / sizeof(ALLDATA_DONT_COMPRESS[0]) - 1;

#endif
