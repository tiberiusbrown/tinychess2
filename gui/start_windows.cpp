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
    ch_init();

    shutdown(0);    
}

#endif

