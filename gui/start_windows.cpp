#ifdef _MSC_VER

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

/*
Reduce namespace pollution by windows.h
*/
#define WIN32_LEAN_AND_MEAN
//#define NOGDICAPMASKS
//#define NOSYSMETRICS
//#define NOMENUS
//#define NOKEYSTATES
//#define NOSYSCOMMANDS
//#define OEMRESOURCE
//#define NOATOM
//#define NOCLIPBOARD
//#define NOCTLMGR
//#define NODRAWTEXT
//#define NONLS
//#define NOMB
//#define NOMEMMGR
//#define NOMETAFILE
#define NOMINMAX
//#define NOOPENFILE
//#define NOSCROLL
//#define NOSERVICE
//#define NOSOUND
//#define NOTEXTMETRIC
//#define NOWH
//#define NOWINOFFSETS
//#define NOCOMM
//#define NOKANJI
//#define NOHELP
//#define NOPROFILER
//#define NODEFERWINDOWPOS
//#define NOMCX
#include <windows.h>
#include <windowsx.h>

#include <ch.h>

void shutdown(int code)
{
    ExitProcess(code);
}

void start(void)
{
    ch_system_info info = { 0 };

    ch_init(&info);

    ch_new_game();
    ch_depth_search(6);

    shutdown((int)ch_get_nodes());    
}

// I hate MSVC sometimes
#ifdef _MSC_VER
extern "C"
{
#pragma function(memset)
    void *memset(void *dest, int c, size_t count)
    {
        char *bytes = (char *)dest;
        while(count--)
        {
            *bytes++ = (char)c;
        }
        return dest;
    }

#pragma function(memcpy)
    void *memcpy(void *dest, const void *src, size_t count)
    {
        char *dest8 = (char *)dest;
        const char *src8 = (const char *)src;
        while(count--)
        {
            *dest8++ = *src8++;
        }
        return dest;
    }
}
#endif

#endif

