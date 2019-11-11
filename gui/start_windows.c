#if defined(_WIN32)

#include "system.h"

#pragma pack(push, 4)

#include "common.h"
#include "text.h"
#include "data.h"
#include "run.h"
#include "settings.h"
#include "start_common.h"
#include "thread.h"

#ifndef NDEBUG
#include <stdlib.h>
#endif

#pragma pack(pop)

#include <ALLDATA_COMPRESSED.h>

#ifdef NDEBUG
/*
MSVC generates calls to these. >:|
*/
#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count)
{
    char* bytes = (char*)dest;
    while(count--) *bytes++ = (char)c;
    return dest;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, void const* src, size_t count)
{
    char* dest8 = (char*)dest;
    char const* src8 = (char const*)src;
    while(count--) *dest8++ = *src8++;
    return dest;
}
#pragma function(memmove)
void* __cdecl memmove(void* dest, const void* src, size_t count)
{
    signed char operation;
    size_t end;
    size_t current;

    if(dest != src)
    {
        if(dest < src)
        {
            operation = 1;
            current = 0;
            end = count;
        }
        else
        {
            operation = -1;
            current = count - 1;
            end = (size_t)(-1);
        }

        for(; current != end; current += operation)
        {
            *(((unsigned char*)dest) + current) = *(((unsigned char*)src) + current);
        }
    }
    return dest;
}
#ifdef _M_IX86
__declspec(naked) void __cdecl _allmul(void)
{
#define A       esp + 8
#define B       esp + 16
#define CRT_LOWORD(x) dword ptr [x+0]
#define CRT_HIWORD(x) dword ptr [x+4]

    __asm
    {
        push    ebx

        mov     eax, CRT_HIWORD(A)
        mov     ecx, CRT_LOWORD(B)
        mul     ecx; eax has AHI, ecx has BLO, so AHI * BLO
        mov     ebx, eax; save result

        mov     eax, CRT_LOWORD(A)
        mul     CRT_HIWORD(B); ALO * BHI
        add     ebx, eax; ebx = ((ALO * BHI) + (AHI * BLO))

        mov     eax, CRT_LOWORD(A); ecx = BLO
        mul     ecx; so edx : eax = ALO * BLO
        add     edx, ebx; now edx has all the LO*HI stuff

        pop     ebx

        ret     16; callee restores the stack
    }

#undef A
#undef B
#undef CRT_LOWORD
#undef CRT_HIWORD
}
#endif
#endif

static HDC hdc = NULL;
static HDC hdc_mem = NULL;
static HBITMAP dib = NULL;
static PAINTSTRUCT paint = { 0 };

static HINSTANCE hInstance = NULL;
static WNDCLASSEX wc = { 0 };
static HWND hwnd = NULL;

static HBRUSH brush_black = NULL;

#if USE_COLORMAP
ALIGN(4) static uint8_t
binfo_mem[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256] = { 0 };
#define binfo (*(BITMAPINFO*)binfo_mem)
#else
static BITMAPINFO binfo = { 0 };
#endif

#define ENABLE_RESIZING 1
#if ENABLE_RESIZING
static DWORD const dwStyle = WS_VISIBLE |
WS_OVERLAPPEDWINDOW;
#else
static DWORD const dwStyle = WS_VISIBLE |
WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
#endif
static DWORD const dwExStyle = WS_EX_CLIENTEDGE;

static LRESULT CALLBACK WndProc(HWND w, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
#if ENABLE_RESIZE_SNAP
    case WM_SIZING:
    {
        RECT* r = (RECT*)lParam;
        RECT cr = { 0, 0, 0, 0 };
        int dw, dh;
        FN_AdjustWindowRectEx(&cr, dwStyle, FALSE, dwExStyle);
        dw = cr.right - cr.left;
        dh = cr.bottom - cr.top;
        ww = r->right - r->left - dw;
        wh = r->bottom - r->top - dh;
        calculate_resize_snap();
        dw += ww;
        dh += wh;
        switch(wParam)
        {
        case WMSZ_BOTTOMLEFT:
            r->left = r->right - dw;
        case WMSZ_BOTTOM:
            r->bottom = r->top + dh;
            break;
        case WMSZ_TOPLEFT:
            r->top = r->bottom - dh;
        case WMSZ_LEFT:
            r->left = r->right - dw;
            break;
        case WMSZ_TOPRIGHT:
            r->right = r->left + dw;
        case WMSZ_TOP:
            r->top = r->bottom - dh;
            break;
        case WMSZ_BOTTOMRIGHT:
            r->bottom = r->top + dh;
        case WMSZ_RIGHT:
            r->right = r->left + dw;
            break;
        }
        break;
    }
#endif
    case WM_SIZE:
        ww = (int)LOWORD(lParam);
        wh = (int)HIWORD(lParam);
        update_im();
        refresh();
        break;
    case WM_CLOSE:
        shutdown(0);
        break;
    case WM_DESTROY:
        FN_PostQuitMessage(0);
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        FN_BeginPaint(w, &paint);
        if(ww == FBW && wh == FBH)
            FN_BitBlt(hdc, 0, 0, FBW, FBH, hdc_mem, 0, 0, SRCCOPY);
        else
        {
            if(imdirty)
            {
                RECT r;
                r.top = 0;
                r.left = 0;
                r.bottom = wh;
                r.right = ww;
                FN_FillRect(hdc, &r, brush_black);
                imdirty = 0;
            }
            FN_StretchBlt(hdc, imx, imy, imw, imh, hdc_mem, 0, 0, FBW, FBH, SRCCOPY);
        }
        FN_EndPaint(w, &paint);
        return 0;
    default:
        return FN_DefWindowProcA(w, msg, wParam, lParam);
    }
    return 0;
}

static int load_procs(HMODULE m, void** procs, char const* names)
{
    do
    {
        if(NULL == (*procs++ = (void*)GetProcAddress(m, names)))
            return 0;
        while(*names++)
            ;
    } while(*names);
    return 1;
}

static void refresh(void)
{
    FN_InvalidateRect(hwnd, NULL, FALSE);
}

#if ENABLE_MS
#if defined(_M_IX86)

static FORCEINLINE uint32_t div_large_integer_times_1000_approx(LARGE_INTEGER a, LARGE_INTEGER b)
{
    static int16_t const C1000 = 1000;
    uint32_t r;

#ifdef _MSC_VER
    __asm
    {
        fild   QWORD PTR a
        fimul   WORD PTR C1000
        fild   QWORD PTR b
        fdiv
        fisttp DWORD PTR r
    }
#else
    /* assume gcc-style (mingw) */
    /* warning: not tested!! might not work... */
    asm(
        "fild   %[input0]\n\t"
        "fimul  %[input1]\n\t"
        "fild   %[input2]\n\t"
        "fdivp           \n\t"
        "fisttp %[output0]\n\t"
        : [output0] "=m" (r)
        : [input0] "m" (a)
        , [input1] "m" (C1000)
        , [input2] "m" (b)
        : );
#endif
    return r;
}

#endif

static LARGE_INTEGER counter_freq = { 0 };
static LARGE_INTEGER counter_start = { 0 };

static uint32_t get_ms(void)
{
    LARGE_INTEGER counter, dc;
    if(counter_freq.QuadPart == 0)
        return 0;
    FN_QueryPerformanceCounter(&counter);
    dc.QuadPart = counter.QuadPart - counter_start.QuadPart;

#if defined(_M_IX86)
    return div_large_integer_times_1000_approx(dc, counter_freq);
#else
    return (uint32_t)(dc.QuadPart * 1000 / counter_freq.QuadPart);
#endif
}

static void update_ms(void)
{
    ms = get_ms();
}
#else
#define get_ms() (uint32_t)0
#define update_ms()
#endif

static void shutdown(int code)
{
    postrun();

    if(hwnd)
    {
        if(hdc) FN_ReleaseDC(hwnd, hdc);
        FN_DestroyWindow(hwnd);
    }

    if(hdc_mem) FN_DeleteDC(hdc_mem);
    if(dib) FN_DeleteObject(dib);
    if(brush_black) FN_DeleteObject(brush_black);

    if(gdi32) FN_FreeLibrary(gdi32);
    if(user32) FN_FreeLibrary(user32);
    if(kernel32) FN_FreeLibrary(kernel32);

#ifdef NDEBUG
    FN_ExitProcess(code);
#else
    exit(code);
#endif
}

/*
TODO: monitor info not available before Windows 2000
      so detect this and use alternate code
*/
static void toggle_fullscreen(void)
{
    static WINDOWPLACEMENT prev = { 0 };
    static int pww, pwh;
    if(!settings.fullscreen)
    {
        MONITORINFO mi = { sizeof(mi) };
        prev.length = sizeof(prev);
        if(FN_GetWindowPlacement(hwnd, &prev) &&
            FN_GetMonitorInfoA(FN_MonitorFromWindow(hwnd,
            MONITOR_DEFAULTTONEAREST), &mi))
        {
            int nww = mi.rcMonitor.right - mi.rcMonitor.left;
            int nwh = mi.rcMonitor.bottom - mi.rcMonitor.top;
            pww = ww;
            pwh = wh;
            FN_SetWindowLongA(hwnd, GWL_STYLE,
                dwStyle & ~WS_OVERLAPPEDWINDOW);
            FN_SetWindowLongA(hwnd, GWL_EXSTYLE,
                (dwExStyle & ~WS_EX_OVERLAPPEDWINDOW) | WS_EX_TOPMOST);
            FN_SetWindowPos(hwnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top, nww, nwh,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            settings.fullscreen = 1;
            ww = nww;
            wh = nwh;
            update_im();
            refresh();
        }
    }
    else
    {
        FN_SetWindowLongA(hwnd, GWL_STYLE, dwStyle);
        FN_SetWindowLongA(hwnd, GWL_EXSTYLE, dwExStyle);
        FN_SetWindowPlacement(hwnd, &prev);
        FN_SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        settings.fullscreen = 0;
        ww = pww;
        wh = pwh;
        update_im();
        refresh();
    }
}

#ifdef NDEBUG
void start(void)
#else
int WINAPI WinMain(
    HINSTANCE arg_hInstance,
    HINSTANCE arg_hPrevInstance,
    LPSTR     arg_lpCmdLine,
    int       arg_nShowCmd
)
#endif
{
#ifndef NDEBUG
    (void)arg_hInstance;
    (void)arg_hPrevInstance;
    (void)arg_lpCmdLine;
    (void)arg_nShowCmd;
#endif

    uncompress_data();
    settings_init();

    ww = settings.ww;
    wh = settings.wh;
    update_im();

    if(NULL == (kernel32 = LoadLibraryA(KERNEL32))) shutdown(-1);
    if(NULL == (user32 = LoadLibraryA(USER32))) shutdown(-1);
    if(NULL == (gdi32 = LoadLibraryA(GDI32))) shutdown(-1);

    if(!load_procs(kernel32, PROCS_KERNEL32, PROCNAMES_KERNEL32))
        shutdown(-1);
    if(!load_procs(user32, PROCS_USER32, PROCNAMES_USER32))
        shutdown(-1);
    if(!load_procs(gdi32, PROCS_GDI32, PROCNAMES_GDI32))
        shutdown(-1);

#if ENABLE_THREAD
    init_threading();
#endif

    hInstance = FN_GetModuleHandleA(NULL);

#if ENABLE_MS
    FN_QueryPerformanceFrequency(&counter_freq);
    FN_QueryPerformanceCounter(&counter_start);
#endif


    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = FN_LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if(!FN_RegisterClassExA(&wc))
        shutdown(-1);

    {
        RECT wr;
        wr.left = wr.top = 0;
        wr.right = ww;
        wr.bottom = wh;
        FN_AdjustWindowRectEx(&wr, dwStyle, FALSE, dwExStyle);
        hwnd = FN_CreateWindowExA(
            dwExStyle,
            CLASS_NAME,
            WINDOW_TITLE,
            dwStyle,
            CW_USEDEFAULT, CW_USEDEFAULT,
            (int)(wr.right - wr.left),
            (int)(wr.bottom - wr.top),
            NULL, NULL, hInstance, NULL);
        if(settings.fullscreen)
        {
            settings.fullscreen = 0;
            toggle_fullscreen();
        }
    }

    if(hwnd == NULL)
        shutdown(-1);

    hdc = FN_GetDC(hwnd);
    hdc_mem = FN_CreateCompatibleDC(hdc);

#if USE_COLORMAP
    {
        uint32_t i;
        for(i = 0; i < (COLORMAP666 ? 216 : 256); ++i)
        {
#if COLORMAP666
            binfo.bmiColors[i].rgbRed = (BYTE)(PIXEL_DECODE_R(i) * 42 + 0);
            binfo.bmiColors[i].rgbGreen = (BYTE)(PIXEL_DECODE_G(i) * 42 + 0);
            binfo.bmiColors[i].rgbBlue = (BYTE)(PIXEL_DECODE_B(i) * 42 + 0);
#else
            binfo.bmiColors[i].rgbRed = (BYTE)((i & 0x7) << 5) + 16;
            binfo.bmiColors[i].rgbGreen = (BYTE)((i & 0x38) << 2) + 16;
            binfo.bmiColors[i].rgbBlue = (BYTE)(i & 0xc0) + 16;
#endif
            binfo.bmiColors[i].rgbReserved = 0;
        }
    }
#endif

    binfo.bmiHeader.biSize = sizeof(binfo.bmiHeader);
    binfo.bmiHeader.biWidth = FBW;
    binfo.bmiHeader.biHeight = -FBH;
    binfo.bmiHeader.biPlanes = 1;
#if USE_COLORMAP
    binfo.bmiHeader.biBitCount = 8;
#else
    binfo.bmiHeader.biBitCount = 32;
#endif
    binfo.bmiHeader.biCompression = BI_RGB;
#if 0
    binfo.bmiHeader.biSizeImage = 0;
    binfo.bmiHeader.biClrUsed = 0;
    binfo.bmiHeader.biClrImportant = 0;
#endif

    dib = FN_CreateDIBSection(hdc, &binfo, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    FN_SelectObject(hdc_mem, dib);

    brush_black = FN_CreateSolidBrush(RGB(0, 0, 0));

    FN_SetStretchBltMode(hdc, COLORONCOLOR);

    FN_ShowWindow(hwnd, SW_SHOW);
    FN_UpdateWindow(hwnd);

    update_ms();

    run();

    /* unreachable */
#if 0
    shutdown(0);
#endif
}

static int prev_mouse_x = 0;
static int prev_mouse_y = 0;

static void get_mouse_pos(int* x, int* y)
{
    POINT p;
    *x = prev_mouse_x;
    *y = prev_mouse_y;
    if(ww == 0 || wh == 0)
        return;
    if(FN_GetCursorPos(&p))
    {
        if(FN_ScreenToClient(hwnd, &p))
        {
            *x = ((int)p.x - imx) * FBW / imw;
            *y = ((int)p.y - imy) * FBH / imh;
        }
    }
}

static int mouse_moved(input* i)
{
    int mouse_x, mouse_y;
    get_mouse_pos(&mouse_x, &mouse_y);
    if(mouse_x != prev_mouse_x || mouse_y != prev_mouse_y)
    {
        i->type = INPUT_MOUSE_MOVE;
        mx = prev_mouse_x = mouse_x;
        my = prev_mouse_y = mouse_y;
        return 1;
    }
    return 0;
}

static int process_message(input* i, MSG* msg)
{
    if(msg->hwnd == hwnd)
    {
        FN_TranslateMessage(msg);
        FN_DispatchMessageA(msg);
        switch(msg->message)
        {
        case WM_LBUTTONDOWN:
            i->type = INPUT_MOUSE_DOWN;
            return 1;
        case WM_LBUTTONUP:
            i->type = INPUT_MOUSE_UP;
            return 1;
#if ENABLE_RBUTTON
        case WM_RBUTTONDOWN:
            i->type = INPUT_MOUSE2_DOWN;
            return 1;
        case WM_RBUTTONUP:
            i->type = INPUT_MOUSE2_UP;
            return 1;
#endif
        case WM_KEYDOWN:
            if(msg->wParam == VK_ESCAPE)
                shutdown(0);
            else if(msg->wParam == VK_F11)
                toggle_fullscreen();
            break;
        default:
            break;
        }
    }
    return 0;
}

static
#if !ENABLE_MS
FORCEINLINE
#endif
int wait_for_input_timeout(input* i, int* timeout_ms)
{
    MSG msg;
#if ENABLE_MS
    int msprev = ms;
    if(timeout_ms && *timeout_ms <= 0)
    {
        i->type = INPUT_TIMER;
        return 1;
    }
#else
    (void)timeout_ms;
#endif
    while(hwnd)
    {
        BOOL ret = FN_PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE);
#if ENABLE_MS
        update_ms();
        if(timeout_ms)
            *timeout_ms -= (ms - msprev);
        msprev = ms;
#endif

        if(ret)
        {
            if(process_message(i, &msg) || mouse_moved(i))
                return 1;
        }
#if ENABLE_MS
        else if(timeout_ms && *timeout_ms <= 0)
        {
            i->type = INPUT_TIMER;
            return 1;
        }
#endif
        else
        {
            FN_Sleep(1);
        }
    }
    shutdown(0);
    return 0;
}

static int wait_for_input(input* i)
{
    return wait_for_input_timeout(i, NULL);
}

void* alloc(uint32_t bytes)
{
    return FN_HeapAlloc(FN_GetProcessHeap(), 0, (SIZE_T)bytes);
}
void dealloc(void* p)
{
    FN_HeapFree(FN_GetProcessHeap(), 0, p);
}

#else
typedef int avoid_unused_translation_unit;
#endif
