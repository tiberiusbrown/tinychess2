#ifndef SYSTEM_H
#define SYSTEM_H

#if defined(_WIN32)

#include "config.h"

/* Windows 2000 */
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

#include <stddef.h>

static HMODULE user32 = NULL;
static HMODULE kernel32 = NULL;
static HMODULE gdi32 = NULL;

static int load_procs(HMODULE m, void** procs, char const* names);

WINBASEAPI FARPROC WINAPI
GetProcAddress(HMODULE hModule, LPCSTR lpProcName);

WINBASEAPI HMODULE WINAPI
LoadLibraryA(LPCSTR lpLibFileName);

#define DEFPROC(rettype, name, args) \
    typedef rettype(WINAPI *PFN_##name)args

/* kernel32 */
DEFPROC(BOOL, FreeLibrary, (HMODULE));
DEFPROC(VOID, ExitProcess, (UINT));
DEFPROC(HMODULE, GetModuleHandleA, (LPCSTR));
DEFPROC(void, Sleep, (DWORD));
DEFPROC(HANDLE, GetProcessHeap, (void));
DEFPROC(LPVOID, HeapAlloc, (HANDLE, DWORD, SIZE_T));
DEFPROC(BOOL, HeapFree, (HANDLE, DWORD, LPVOID));
#if ENABLE_MS
DEFPROC(BOOL, QueryPerformanceFrequency, (LARGE_INTEGER*));
DEFPROC(BOOL, QueryPerformanceCounter, (LARGE_INTEGER*));
static void* PROCS_KERNEL32[9] = { NULL };
#else
static void* PROCS_KERNEL32[7] = { NULL };
#endif
#define FN_FreeLibrary      ((PFN_FreeLibrary     )PROCS_KERNEL32[0])
#define FN_ExitProcess      ((PFN_ExitProcess     )PROCS_KERNEL32[1])
#define FN_GetModuleHandleA ((PFN_GetModuleHandleA)PROCS_KERNEL32[2])
#define FN_Sleep            ((PFN_Sleep           )PROCS_KERNEL32[3])
#define FN_GetProcessHeap   ((PFN_GetProcessHeap  )PROCS_KERNEL32[4])
#define FN_HeapAlloc        ((PFN_HeapAlloc       )PROCS_KERNEL32[5])
#define FN_HeapFree         ((PFN_HeapFree        )PROCS_KERNEL32[6])
#if ENABLE_MS
#define FN_QueryPerformanceFrequency ((PFN_QueryPerformanceFrequency)PROCS_KERNEL32[7])
#define FN_QueryPerformanceCounter ((PFN_QueryPerformanceCounter)PROCS_KERNEL32[8])
#endif

/* user32 */
DEFPROC(ATOM, RegisterClassExA, (WNDCLASSEXA const*));
DEFPROC(HCURSOR, LoadCursorA, (HINSTANCE, LPCSTR));
DEFPROC(HWND, CreateWindowExA, (
    DWORD     dwExStyle,
    LPCSTR    lpClassName,
    LPCSTR    lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
));
DEFPROC(BOOL, DestroyWindow, (HWND));
DEFPROC(BOOL, ShowWindow, (HWND, int));
DEFPROC(BOOL, UpdateWindow, (HWND));
DEFPROC(LRESULT, DefWindowProcA, (HWND, UINT, WPARAM, LPARAM));
DEFPROC(BOOL, AdjustWindowRectEx, (LPRECT, DWORD, BOOL, DWORD));
DEFPROC(BOOL, SetWindowPlacement, (HWND, WINDOWPLACEMENT const*));
DEFPROC(BOOL, GetWindowPlacement, (HWND, WINDOWPLACEMENT*));
DEFPROC(LONG, SetWindowLongA, (HWND, int, LONG));
DEFPROC(BOOL, SetWindowPos, (HWND, HWND, int, int, int, int, UINT));
DEFPROC(BOOL, GetMonitorInfoA, (HMONITOR, LPMONITORINFO));
DEFPROC(HMONITOR, MonitorFromWindow, (HWND, DWORD));
DEFPROC(VOID, PostQuitMessage, (int));
DEFPROC(BOOL, TranslateMessage, (MSG const*));
DEFPROC(LRESULT, DispatchMessageA, (MSG const*));
DEFPROC(BOOL, PeekMessageA, (LPMSG, HWND, UINT, UINT, UINT));
DEFPROC(HDC, GetDC, (HWND));
DEFPROC(int, ReleaseDC, (HWND, HDC));
DEFPROC(HDC, BeginPaint, (HWND, LPPAINTSTRUCT));
DEFPROC(BOOL, EndPaint, (HWND, PAINTSTRUCT const*));
DEFPROC(BOOL, InvalidateRect, (HWND, RECT const*, BOOL));
DEFPROC(BOOL, GetCursorPos, (LPPOINT));
DEFPROC(BOOL, ScreenToClient, (HWND, LPPOINT));
DEFPROC(int, FillRect, (HDC, RECT const*, HBRUSH));
static void* PROCS_USER32[26] = { NULL };
#define FN_RegisterClassExA   ((PFN_RegisterClassExA  )PROCS_USER32[ 0])
#define FN_LoadCursorA        ((PFN_LoadCursorA       )PROCS_USER32[ 1])
#define FN_CreateWindowExA    ((PFN_CreateWindowExA   )PROCS_USER32[ 2])
#define FN_DestroyWindow      ((PFN_DestroyWindow     )PROCS_USER32[ 3])
#define FN_ShowWindow         ((PFN_ShowWindow        )PROCS_USER32[ 4])
#define FN_UpdateWindow       ((PFN_UpdateWindow      )PROCS_USER32[ 5])
#define FN_DefWindowProcA     ((PFN_DefWindowProcA    )PROCS_USER32[ 6])
#define FN_AdjustWindowRectEx ((PFN_AdjustWindowRectEx)PROCS_USER32[ 7])
#define FN_SetWindowPlacement ((PFN_SetWindowPlacement)PROCS_USER32[ 8])
#define FN_GetWindowPlacement ((PFN_GetWindowPlacement)PROCS_USER32[ 9])
#define FN_SetWindowLongA     ((PFN_SetWindowLongA    )PROCS_USER32[10])
#define FN_SetWindowPos       ((PFN_SetWindowPos      )PROCS_USER32[11])
#define FN_GetMonitorInfoA    ((PFN_GetMonitorInfoA   )PROCS_USER32[12])
#define FN_MonitorFromWindow  ((PFN_MonitorFromWindow )PROCS_USER32[13])
#define FN_PostQuitMessage    ((PFN_PostQuitMessage   )PROCS_USER32[14])
#define FN_TranslateMessage   ((PFN_TranslateMessage  )PROCS_USER32[15])
#define FN_DispatchMessageA   ((PFN_DispatchMessageA  )PROCS_USER32[16])
#define FN_PeekMessageA       ((PFN_PeekMessageA      )PROCS_USER32[17])
#define FN_GetDC              ((PFN_GetDC             )PROCS_USER32[18])
#define FN_ReleaseDC          ((PFN_ReleaseDC         )PROCS_USER32[19])
#define FN_BeginPaint         ((PFN_BeginPaint        )PROCS_USER32[20])
#define FN_EndPaint           ((PFN_EndPaint          )PROCS_USER32[21])
#define FN_InvalidateRect     ((PFN_InvalidateRect    )PROCS_USER32[22])
#define FN_GetCursorPos       ((PFN_GetCursorPos      )PROCS_USER32[23])
#define FN_ScreenToClient     ((PFN_ScreenToClient    )PROCS_USER32[24])
#define FN_FillRect           ((PFN_FillRect          )PROCS_USER32[25])

/* gdi32 */
DEFPROC(HDC, CreateCompatibleDC, (HDC));
DEFPROC(HBITMAP, CreateDIBSection, (
    HDC, BITMAPINFO const*, UINT, VOID**, HANDLE, DWORD));
DEFPROC(HBRUSH, CreateSolidBrush, (COLORREF));
DEFPROC(BOOL, DeleteDC, (HDC));
DEFPROC(BOOL, DeleteObject, (HGDIOBJ));
DEFPROC(HGDIOBJ, SelectObject, (HDC, HGDIOBJ));
DEFPROC(BOOL, BitBlt, (
    HDC, int, int, int, int, HDC, int, int, DWORD));
DEFPROC(BOOL, StretchBlt, (
    HDC, int, int, int, int, HDC, int, int, int, int, DWORD));
DEFPROC(int, SetStretchBltMode, (HDC, int));
static void* PROCS_GDI32[9] = { NULL };
#define FN_CreateCompatibleDC ((PFN_CreateCompatibleDC)PROCS_GDI32[0])
#define FN_CreateDIBSection   ((PFN_CreateDIBSection  )PROCS_GDI32[1])
#define FN_CreateSolidBrush   ((PFN_CreateSolidBrush  )PROCS_GDI32[2])
#define FN_DeleteDC           ((PFN_DeleteDC          )PROCS_GDI32[3])
#define FN_DeleteObject       ((PFN_DeleteObject      )PROCS_GDI32[4])
#define FN_SelectObject       ((PFN_SelectObject      )PROCS_GDI32[5])
#define FN_BitBlt             ((PFN_BitBlt            )PROCS_GDI32[6])
#define FN_StretchBlt         ((PFN_StretchBlt        )PROCS_GDI32[7])
#define FN_SetStretchBltMode  ((PFN_SetStretchBltMode )PROCS_GDI32[8])

/* thread (kernel32) */
DEFPROC(void, InitializeCriticalSection, (LPCRITICAL_SECTION));
DEFPROC(void, DeleteCriticalSection, (LPCRITICAL_SECTION));
DEFPROC(void, EnterCriticalSection, (LPCRITICAL_SECTION));
DEFPROC(void, LeaveCriticalSection, (LPCRITICAL_SECTION));
DEFPROC(HANDLE, CreateThread, (
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    SIZE_T                  dwStackSize,
    LPTHREAD_START_ROUTINE  lpStartAddress,
    __drv_aliasesMem LPVOID lpParameter,
    DWORD                   dwCreationFlags,
    LPDWORD                 lpThreadId
));
DEFPROC(BOOL, CloseHandle, (HANDLE));
DEFPROC(DWORD, WaitForSingleObject, (HANDLE, DWORD));
DEFPROC(DWORD, WaitForMultipleObjects, (DWORD, HANDLE const*, BOOL, DWORD));
DEFPROC(HANDLE, CreateEventA, (LPSECURITY_ATTRIBUTES, BOOL, BOOL, LPCSTR));
DEFPROC(BOOL, SetEvent, (HANDLE));
DEFPROC(BOOL, ResetEvent, (HANDLE));
static void* PROCS_THREAD[11] = { NULL };
#define FN_InitializeCriticalSection ((PFN_InitializeCriticalSection)PROCS_THREAD[ 0])
#define FN_DeleteCriticalSection     ((PFN_DeleteCriticalSection    )PROCS_THREAD[ 1])
#define FN_EnterCriticalSection      ((PFN_EnterCriticalSection     )PROCS_THREAD[ 2])
#define FN_LeaveCriticalSection      ((PFN_LeaveCriticalSection     )PROCS_THREAD[ 3])
#define FN_CreateThread              ((PFN_CreateThread             )PROCS_THREAD[ 4])
#define FN_CloseHandle               ((PFN_CloseHandle              )PROCS_THREAD[ 5])
#define FN_WaitForSingleObject       ((PFN_WaitForSingleObject      )PROCS_THREAD[ 6])
#define FN_WaitForMultipleObjects    ((PFN_WaitForMultipleObjects   )PROCS_THREAD[ 7])
#define FN_CreateEventA              ((PFN_CreateEventA             )PROCS_THREAD[ 8])
#define FN_SetEvent                  ((PFN_SetEvent                 )PROCS_THREAD[ 9])
#define FN_ResetEvent                ((PFN_ResetEvent               )PROCS_THREAD[10])

#elif defined(__linux__)

#include "config.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__GNUC__)
#if defined(__arm__)
#define GLIBC_VER "2.4"
#elif defined(__x86_64__)
#define GLIBC_VER "2.2.5"
#else
#define GLIBC_VER "2.0"
#endif
__asm__(".symver dlopen,dlopen@GLIBC_" GLIBC_VER);
__asm__(".symver dlsym,dlsym@GLIBC_" GLIBC_VER);
__asm__(".symver dlclose,dlclose@GLIBC_" GLIBC_VER);
void* memmove(void* dest, const void* src, size_t count);
#endif
#include <dlfcn.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#if ENABLE_OPENGL
#include <GL/gl.h>
#include <GL/glx.h>
#endif

#if ENABLE_XSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#if defined(__arm__)
int raise(int sig)
{
    /* TODO: implement? */
    (void)sig;
    return 0;
}
#endif

static int load_procs(void* m, void** procs, char const* names);

#define DEFPROC(rettype, name, args) \
    typedef rettype(*PFN_##name)args

DEFPROC(Display*, XOpenDisplay, (char*));
DEFPROC(void, XCloseDisplay, (Display*));
DEFPROC(GC, XCreateGC, (Display*, Drawable, unsigned long, XGCValues*));
DEFPROC(Colormap, XCreateColormap, (Display*, Window, Visual*, int));
DEFPROC(Window, XCreateWindow, (
    Display*,
    Window,
    int, int,
    unsigned int, unsigned int,
    unsigned int,
    int,
    unsigned int,
    Visual*,
    unsigned long,
    XSetWindowAttributes*));
DEFPROC(void, XDestroyWindow, (Display*, Window));
DEFPROC(XImage*, XCreateImage, (
    Display*, Visual*, unsigned int, int, int, char*,
    unsigned int, unsigned int, int, int));
DEFPROC(void, XPutImage, (
    Display*,
    Drawable,
    GC,
    XImage*,
    int, int,
    int, int,
    unsigned int, unsigned int));
DEFPROC(XImage*, XGetImage, (
    Display*,
    Drawable,
    int, int,
    unsigned int, unsigned int,
    unsigned long,
    int));
DEFPROC(void, XDestroyImage, (XImage*));
DEFPROC(void, XSelectInput, (Display*, Window, long));
DEFPROC(Atom, XInternAtom, (Display*, char*, Bool));
DEFPROC(Status, XSetWMProtocols, (Display*, Window, Atom*, int));
DEFPROC(void, XMapWindow, (Display*, Window));
DEFPROC(void, XStoreName, (Display*, Window, char*));
DEFPROC(void, XNextEvent, (Display*, XEvent*));
DEFPROC(void, XSync, (Display*, Bool));
DEFPROC(Bool, XQueryPointer, (
    Display*,
    Window,
    Window*, Window*,
    int*, int*,
    int*, int*,
    unsigned int*));
DEFPROC(int, XMatchVisualInfo, (Display*, int, int, int, XVisualInfo*));
DEFPROC(Status, XGetGeometry, (
    Display*,
    Drawable,
    Window*,
    int*, int*,
    unsigned int*, unsigned int*,
    unsigned int*,
    unsigned int*));
DEFPROC(int, XPending, (Display*));
void* PROCS_X11[21];
#define FN_XOpenDisplay        ((PFN_XOpenDisplay       )PROCS_X11[ 0])
#define FN_XCloseDisplay       ((PFN_XCloseDisplay      )PROCS_X11[ 1])
#define FN_XCreateGC           ((PFN_XCreateGC          )PROCS_X11[ 2])
#define FN_XCreateColormap     ((PFN_XCreateColormap    )PROCS_X11[ 3])
#define FN_XCreateWindow       ((PFN_XCreateWindow      )PROCS_X11[ 4])
#define FN_XDestroyWindow      ((PFN_XDestroyWindow     )PROCS_X11[ 5])
#define FN_XCreateImage        ((PFN_XCreateImage       )PROCS_X11[ 6])
#define FN_XPutImage           ((PFN_XPutImage          )PROCS_X11[ 7])
#define FN_XGetImage           ((PFN_XGetImage          )PROCS_X11[ 8])
#define FN_XDestroyImage       ((PFN_XDestroyImage      )PROCS_X11[ 9])
#define FN_XSelectInput        ((PFN_XSelectInput       )PROCS_X11[10])
#define FN_XInternAtom         ((PFN_XInternAtom        )PROCS_X11[11])
#define FN_XSetWMProtocols     ((PFN_XSetWMProtocols    )PROCS_X11[12])
#define FN_XMapWindow          ((PFN_XMapWindow         )PROCS_X11[13])
#define FN_XStoreName          ((PFN_XStoreName         )PROCS_X11[14])
#define FN_XNextEvent          ((PFN_XNextEvent         )PROCS_X11[15])
#define FN_XSync               ((PFN_XSync              )PROCS_X11[16])
#define FN_XQueryPointer       ((PFN_XQueryPointer      )PROCS_X11[17])
#define FN_XMatchVisualInfo    ((PFN_XMatchVisualInfo   )PROCS_X11[18])
#define FN_XGetGeometry        ((PFN_XGetGeometry       )PROCS_X11[19])
#define FN_XPending            ((PFN_XPending           )PROCS_X11[20])

DEFPROC(void, exit, (int));
DEFPROC(int, usleep, (useconds_t));
DEFPROC(int, gettimeofday, (struct timeval*, struct timezone*));
DEFPROC(void*, malloc, (size_t));
DEFPROC(void, free, (void*));
void* PROCS_C[5];
#define FN_exit         ((PFN_exit        )PROCS_C[0])
#define FN_usleep       ((PFN_usleep      )PROCS_C[1])
#define FN_gettimeofday ((PFN_gettimeofday)PROCS_C[2])
#define FN_malloc       ((PFN_malloc      )PROCS_C[3])
#define FN_free         ((PFN_free        )PROCS_C[4])

#if ENABLE_OPENGL
DEFPROC(XVisualInfo*, glXChooseVisual, (Display*, int, int*));
DEFPROC(GLXContext, glXCreateContext, (Display*, XVisualInfo*, GLXContext, Bool));
DEFPROC(void, glXDestroyContext, (Display*, GLXContext));
DEFPROC(Bool, glXMakeCurrent, (Display*, GLXDrawable, GLXContext));
DEFPROC(void, glXSwapBuffers, (Display*, GLXDrawable));
DEFPROC(void, glGenTextures, (GLsizei, GLuint*));
DEFPROC(void, glDeleteTextures, (GLsizei, GLuint*));
DEFPROC(void, glBindTexture, (GLenum, GLuint));
DEFPROC(void, glTexImage2D, (
    GLenum,
    GLint,
    GLint,
    GLsizei, GLsizei,
    GLint,
    GLenum,
    GLenum,
    GLvoid const*));
DEFPROC(void, glTexSubImage2D, (
    GLenum,
    GLint,
    GLint, GLint,
    GLsizei, GLsizei,
    GLenum,
    GLenum,
    GLvoid const*));
DEFPROC(void, glTexParameteri, (GLenum, GLenum, GLint));
DEFPROC(void, glDrawArrays, (GLenum, GLint, GLsizei));
DEFPROC(void, glVertexPointer, (GLint, GLenum, GLsizei, GLvoid const*));
DEFPROC(void, glTexCoordPointer, (GLint, GLenum, GLsizei, GLvoid const*));
DEFPROC(void, glEnableClientState, (GLenum));
DEFPROC(void, glEnable, (GLenum));
DEFPROC(void, glViewport, (GLint, GLint, GLsizei, GLsizei));
DEFPROC(void, glClear, (GLbitfield));
static void* PROCS_GL[18];
#define FN_glXChooseVisual     ((PFN_glXChooseVisual    )PROCS_GL[ 0])
#define FN_glXCreateContext    ((PFN_glXCreateContext   )PROCS_GL[ 1])
#define FN_glXDestroyContext   ((PFN_glXDestroyContext  )PROCS_GL[ 2])
#define FN_glXMakeCurrent      ((PFN_glXMakeCurrent     )PROCS_GL[ 3])
#define FN_glXSwapBuffers      ((PFN_glXSwapBuffers     )PROCS_GL[ 4])
#define FN_glGenTextures       ((PFN_glGenTextures      )PROCS_GL[ 5])
#define FN_glDeleteTextures    ((PFN_glDeleteTextures   )PROCS_GL[ 6])
#define FN_glBindTexture       ((PFN_glBindTexture      )PROCS_GL[ 7])
#define FN_glTexImage2D        ((PFN_glTexImage2D       )PROCS_GL[ 8])
#define FN_glTexSubImage2D     ((PFN_glTexSubImage2D    )PROCS_GL[ 9])
#define FN_glTexParameteri     ((PFN_glTexParameteri    )PROCS_GL[10])
#define FN_glDrawArrays        ((PFN_glDrawArrays       )PROCS_GL[11])
#define FN_glVertexPointer     ((PFN_glVertexPointer    )PROCS_GL[12])
#define FN_glTexCoordPointer   ((PFN_glTexCoordPointer  )PROCS_GL[13])
#define FN_glEnableClientState ((PFN_glEnableClientState)PROCS_GL[14])
#define FN_glEnable            ((PFN_glEnable           )PROCS_GL[15])
#define FN_glViewport          ((PFN_glViewport         )PROCS_GL[16])
#define FN_glClear             ((PFN_glClear            )PROCS_GL[17])
#endif

#if ENABLE_THREAD
static void* hpthread;
#include <pthread.h>
DEFPROC(int, pthread_mutex_init, (pthread_mutex_t*, pthread_mutexattr_t const*));
DEFPROC(int, pthread_mutex_destroy, (pthread_mutex_t*));
DEFPROC(int, pthread_mutex_lock, (pthread_mutex_t*));
DEFPROC(int, pthread_mutex_unlock, (pthread_mutex_t*));
DEFPROC(int, pthread_create, (pthread_t*, pthread_attr_t const*, void*(*)(void*), void*));
DEFPROC(int, pthread_join, (pthread_t, void**));
static void* PROCS_THREAD[6];
#define FN_pthread_mutex_init ((PFN_pthread_mutex_init)PROCS_THREAD[0])
#define FN_pthread_mutex_destroy ((PFN_pthread_mutex_destroy)PROCS_THREAD[1])
#define FN_pthread_mutex_lock ((PFN_pthread_mutex_lock)PROCS_THREAD[2])
#define FN_pthread_mutex_unlock ((PFN_pthread_mutex_unlock)PROCS_THREAD[3])
#define FN_pthread_create ((PFN_pthread_create)PROCS_THREAD[4])
#define FN_pthread_join ((PFN_pthread_join)PROCS_THREAD[5])
#endif

#if ENABLE_XSHM
DEFPROC(Bool, XShmAttach, (Display*, XShmSegmentInfo*));
DEFPROC(Bool, XShmDetach, (Display*, XShmSegmentInfo*));
DEFPROC(XImage*, XShmCreateImage, (
    Display*, Visual*, unsigned int, int, char*, XShmSegmentInfo*,
    unsigned int, unsigned int));
DEFPROC(Bool, XShmPutImage, (
    Display*, Drawable, GC, XImage*, int, int, int, int,
    unsigned int, unsigned int, Bool));
static void* PROCS_XEXT[4];
#define FN_XShmAttach      ((PFN_XShmAttach     )PROCS_XEXT[0])
#define FN_XShmDetach      ((PFN_XShmDetach     )PROCS_XEXT[1])
#define FN_XShmCreateImage ((PFN_XShmCreateImage)PROCS_XEXT[2])
#define FN_XShmPutImage    ((PFN_XShmPutImage   )PROCS_XEXT[3])

DEFPROC(int, shmget, (key_t, size_t, int));
DEFPROC(void*, shmat, (int, void const*, int));
DEFPROC(int, shmdt, (void const*));
DEFPROC(int, shmctl, (int, int, struct shmid_ds*));
void* PROCS_C_SHM[4];
#define FN_shmget ((PFN_shmget)PROCS_C_SHM[0])
#define FN_shmat  ((PFN_shmat )PROCS_C_SHM[1])
#define FN_shmdt  ((PFN_shmdt )PROCS_C_SHM[2])
#define FN_shmctl ((PFN_shmctl)PROCS_C_SHM[3])
#endif

#endif

#endif
