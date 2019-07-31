#if defined(__linux__)

#include "system.h"

#include "common.h"
#include "text.h"
#include "data.h"
#include "run.h"
#include "settings.h"
#include "start_common.h"

#include <ALLDATA_COMPRESSED.h>

static void* hx11;
static void* hc;

#if ENABLE_OPENGL
static void* hgl;
static int support_opengl;
static int using_opengl;
static GLXContext glc;
static GLuint gltex;
#endif

#if ENABLE_XSHM
static void* hxext;
static int support_xshm;
static int using_xshm;
static XShmSegmentInfo shminfo;
#endif

static Display* display;
static Window   window;
static GC gc;
static XGCValues gcv;

static XVisualInfo vinfo;
static int r_shift;
static int g_shift;
static int b_shift;
static int r_preshift;
static int g_preshift;
static int b_preshift;
static uint32_t alpha_mask;
static XImage* image;
static char* putpixel_ptr;

static Atom wm_protocols;
static Atom wm_delete_window;
static Atom net_wm_ping;

#define screen DefaultScreen(display)
#define root RootWindow(display, screen)

static pixel swap_rgb(pixel p)
{
    uint8_t t = p.r;
    p.r = p.b;
    p.b = t;
    return p;
}

static void process_shifts(
    unsigned long bitmask,
    int* preshift,
    int* shift)
{
    *shift = 0;
    while(!(bitmask & 1))
    {
        bitmask >>= 1;
        ++(*shift);
    }
    *preshift = 8;
    while(bitmask & 1)
    {
        bitmask >>= 1;
        --(*preshift);
    }
}

static FORCEINLINE uint32_t process_pixel(pixel p)
{
    uint32_t v = 0;
    v |= (((uint32_t)p.r >> r_preshift) << r_shift);
    v |= (((uint32_t)p.g >> g_preshift) << g_shift);
    v |= (((uint32_t)p.b >> b_preshift) << b_shift);
    return v;
}

static void(*putpixel_func)(pixel);

static void putpixel32(pixel p)
{
    *((uint32_t*)putpixel_ptr) = ((uint32_t)process_pixel(p) | alpha_mask);
    putpixel_ptr += 4;
}

static void putpixel24(pixel p)
{
    *((uint32_t*)putpixel_ptr) = (uint32_t)process_pixel(p);
    putpixel_ptr += 4;
}

static void putpixel16(pixel p)
{
    *((uint16_t*)putpixel_ptr) = (uint16_t)process_pixel(p);
    putpixel_ptr += 2;
}

static void putpixel8(pixel p)
{
    *((uint8_t*)putpixel_ptr) = (uint8_t)process_pixel(p);
    putpixel_ptr += 1;
}

static void paint(void)
{
    {
        Window rr;
        int xr, yr;
        unsigned int br, dr;
        FN_XGetGeometry(
            display,
            window,
            &rr,
            &xr,
            &yr,
            (unsigned int*)&ww,
            (unsigned int*)&wh,
            &br,
            &dr);
        update_im();
    }

#if ENABLE_OPENGL
    if(using_opengl)
    {
        GLfloat VS[6 * 2];
        VS[0] = (GLfloat)(imx) / ww * 2 - 1;
        VS[1] = (GLfloat)(imy) / wh * 2 - 1;
        VS[4] = (GLfloat)(imw + imx) / ww * 2 - 1;
        VS[5] = (GLfloat)(imh + imy) / wh * 2 - 1;
        VS[2] = VS[10] = VS[0];
        VS[3] = VS[7] = VS[5];
        VS[6] = VS[8] = VS[4];
        VS[9] = VS[11] = VS[1];
        FN_glVertexPointer(2, GL_FLOAT, 0, VS);
        FN_glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0, 0,
            FBW, FBH,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels);
        FN_glViewport(0, 0, (GLsizei)ww, (GLsizei)wh);
        FN_glClear(GL_COLOR_BUFFER_BIT);
        FN_glDrawArrays(GL_TRIANGLES, 0, 6);
        FN_glXSwapBuffers(display, window);
    }
    else
#endif
    {
        int sx, sy;
        int mx = (imw + FBW - 1) / FBW;
        int my = (imh + FBH - 1) / FBH;
        for(sy = 0; sy < my; ++sy)
        {
            for(sx = 0; sx < mx; ++sx)
            {
                int x, y;
                putpixel_ptr = image->data;
                for(y = 0; y < FBH; ++y)
                {
                    int py = (y + sy * FBH) * FBH / imh;
                    if(py >= FBH) break;
                    for(x = 0; x < FBW; ++x)
                    {
                        int px = (x + sx * FBW) * FBW / imw;
                        if(px >= FBW) break;
                        putpixel_func(pixels[py * FBW + px]);
                    }
                }
#if ENABLE_XSHM
                if(using_xshm)
                {
                    FN_XShmPutImage(
                        display, window, gc, image,
                        0, 0, sx * FBW + imx, sy * FBH + imy, FBW, FBH, False);
                    FN_XSync(display, False);
                }
                else
#endif
                {
                    FN_XPutImage(
                        display, window, gc, image,
                        0, 0, sx * FBW + imx, sy * FBH + imy, FBW, FBH);
                }
            }
        }
        FN_XSync(display, False);
    }
}

static void refresh(void)
{
    paint();
}

static int load_procs(void* m, void** procs, char const* names)
{
    int r = 1;
    do
    {
        *procs = (void*)dlsym(m, names);
        if(!(*procs))
            r = 0;
        ++procs;
        while(*names++)
            ;
    } while(*names);
    return r;
}

static int create_sw_window(void)
{
    static uint32_t image_pixels[FBW * FBH * 4];
    XSetWindowAttributes swa;
    Colormap cmap;

#if ENABLE_OPENGL
    using_opengl = 0;
#endif

    if(0)
        ;
    else if(FN_XMatchVisualInfo(display, screen, 24, TrueColor, &vinfo))
        putpixel_func = putpixel24;
    else if(FN_XMatchVisualInfo(display, screen, 32, TrueColor, &vinfo))
        putpixel_func = putpixel32;
    else if(FN_XMatchVisualInfo(display, screen, 16, TrueColor, &vinfo))
        putpixel_func = putpixel16;
    else if(FN_XMatchVisualInfo(display, screen,  8, TrueColor, &vinfo))
        putpixel_func = putpixel8;
    else
        return 0;

    process_shifts(vinfo.red_mask  , &r_preshift, &r_shift);
    process_shifts(vinfo.green_mask, &g_preshift, &g_shift);
    process_shifts(vinfo.blue_mask , &b_preshift, &b_shift);
    alpha_mask = ~(vinfo.red_mask | vinfo.green_mask | vinfo.blue_mask);

    cmap = FN_XCreateColormap(display, root, vinfo.visual, AllocNone);
    swa.colormap = cmap;
    swa.border_pixel = 0;
    swa.background_pixel = 0;

    window = FN_XCreateWindow(
        display,
        root,
        0, 0,
        ww, wh,
        0,
        vinfo.depth,
        InputOutput,
        vinfo.visual,
        CWBorderPixel | CWBackPixel | CWColormap,
        &swa);
    FN_XSync(display, False);

    FN_XMapWindow(display, window);
    gc = FN_XCreateGC(display, window, GCGraphicsExposures, &gcv);
    if(!gc) return 0;

#if ENABLE_XSHM
    if(support_xshm)
    {
        using_xshm = 0;
        shminfo.shmid = FN_shmget(IPC_PRIVATE, FBW * FBH * 4, IPC_CREAT | 0777);
        if(shminfo.shmid >= 0)
        {
            shminfo.shmaddr = (char*)FN_shmat(shminfo.shmid, 0, 0);
            shminfo.readOnly = False;
            FN_XSync(display, False);
            if(FN_XShmAttach(display, &shminfo))
            {
                FN_XSync(display, False);
                FN_shmctl(shminfo.shmid, IPC_RMID, NULL);
                image = FN_XShmCreateImage(
                    display,
                    vinfo.visual,
                    vinfo.depth,
                    ZPixmap,
                    shminfo.shmaddr,
                    &shminfo,
                    FBW,
                    FBH);
                if(image)
                {
                    using_xshm = 1;
                }
                else
                {
                    FN_XShmDetach(display, &shminfo);
                    FN_XSync(display, False);
                    FN_shmdt(shminfo.shmaddr);
                }
            }
        }
    }

    if(!using_xshm)
#endif
    {
        image = FN_XCreateImage(
            display,
            vinfo.visual,
            vinfo.depth,
            ZPixmap,
            0,
            (char*)image_pixels,
            FBW,
            FBH,
            32,
            0);
    }
    if(!image) return 0;

    return 1;
}

#if ENABLE_OPENGL
static int create_opengl_window(void)
{
    XSetWindowAttributes swa;
    Colormap cmap;
    XVisualInfo* vi;
    GLint att[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };

    using_opengl = 0;
    if(!support_opengl) return 0;

    vi = FN_glXChooseVisual(display, 0, att);
    if(!vi) return 0;

    cmap = FN_XCreateColormap(display, root, vi->visual, AllocNone);
    swa.colormap = cmap;

    window = FN_XCreateWindow(
        display,
        root,
        0, 0,
        ww, wh,
        0,
        vi->depth,
        InputOutput,
        vi->visual,
        CWColormap,
        &swa);
    FN_XMapWindow(display, window);

    glc = FN_glXCreateContext(display, vi, NULL, GL_TRUE);
    if(NULL == glc)
    {
        FN_XDestroyWindow(display, window);
        return 0;
    }

    FN_glXMakeCurrent(display, window, glc);

    {
        void*(*FN_glXGetProcAddress)(GLubyte const*) =
            dlsym(hgl, (char const*)PROCNAME_GLXGETPROCADDRESSARB);
        if(FN_glXGetProcAddress)
        {
            void* FN_glXSwapInterval = FN_glXGetProcAddress(
                (GLubyte const*)PROCNAME_GLXSWAPINTERVALEXT);
            if(FN_glXSwapInterval)
            {
                ((void(*)(Display*, GLXDrawable, int))FN_glXSwapInterval)
                    (display, window, 0);
            }
            else
            {
                FN_glXSwapInterval = FN_glXGetProcAddress(
                    (GLubyte const*)PROCNAME_GLXSWAPINTERVALMESA);
                if(FN_glXSwapInterval)
                    ((void(*)(int))FN_glXSwapInterval)(0);
                else
                {
                    FN_glXSwapInterval = FN_glXGetProcAddress(
                        (GLubyte const*)PROCNAME_GLXSWAPINTERVALSGI);
                    if(FN_glXSwapInterval)
                        ((void(*)(int))FN_glXSwapInterval)(0);
                }
            }
        }
    }

    FN_glEnable(GL_TEXTURE_2D);
    FN_glGenTextures(1, &gltex);
    FN_glBindTexture(GL_TEXTURE_2D, gltex);
    FN_glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        512,
        256,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL);
    {
        static float const TS[6 * 2] =
        {
            0.f, (GLfloat)FBH / 256,
            0.f, 0.f,
            (GLfloat)FBW / 512, 0.f,
            (GLfloat)FBW / 512, 0.f,
            (GLfloat)FBW / 512, (GLfloat)FBH / 256,
            0.f, (GLfloat)FBH / 256,
        };

        FN_glEnableClientState(GL_VERTEX_ARRAY);
        FN_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        FN_glTexCoordPointer(2, GL_FLOAT, 0, TS);
    }
    FN_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    FN_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    using_opengl = 1;
    return 1;
}
#endif

uint32_t get_ms(void)
{
    struct timeval tv;
    FN_gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void update_ms(void)
{
    ms = get_ms();
}

static void shutdown(int code)
{

#if ENABLE_OPENGL
    if(using_opengl)
    {
        FN_glDeleteTextures(1, &gltex);
        FN_glXMakeCurrent(display, None, NULL);
        FN_glXDestroyContext(display, glc);
    }
#endif

    if(display)
    {
        FN_XDestroyWindow(display, window);

        if(image)
        {
#if ENABLE_XSHM
            if(!using_xshm)
#endif
                image->data = NULL;
            FN_XDestroyImage(image);
            image = NULL;

#if ENABLE_XSHM
            FN_XShmDetach(display, &shminfo);
            FN_XSync(display, False);
            FN_shmdt(shminfo.shmaddr);
#endif
        }

        FN_XCloseDisplay(display);
    }

#if ENABLE_DLCLOSE

    if(hx11) dlclose(hx11);

#if ENABLE_OPENGL
    if(hgl) dlclose(hgl);
#endif

#if ENABLE_THREAD
    if(hpthread) dlclose(hpthread);
#endif

#endif

    FN_exit(code);
}

#ifndef NDEBUG
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
#else
void __attribute__((force_align_arg_pointer, externally_visible)) start(void)
{
#endif
    uncompress_data();
    settings_init();

    ww = settings.ww;
    wh = settings.wh;

    /* do this first to load exit */
    hc = dlopen((char const*)LIBC_SO, RTLD_NOW);
    if(!hc) shutdown(-1);

    if(!load_procs(hc, PROCS_C, (char const*)PROCNAMES_C))
        shutdown(-1);

    hx11 = dlopen((char const*)LIBX11_SO, RTLD_NOW);
    if(!hx11) shutdown(-1);
    
    if(!load_procs(hx11, PROCS_X11, (char const*)PROCNAMES_X11))
        shutdown(-1);

#if ENABLE_THREAD
    hpthread = dlopen(LIBPTHREAD_SO, RTLD_NOW);
    if(!hpthread) shutdown(-1);
    init_threading();
    if(!threading_avail) shutdown(-1);
#endif

#if ENABLE_OPENGL
    support_opengl = 1;
    hgl = dlopen((char const*)LIBGL_SO, RTLD_NOW);
    if(!hgl || !load_procs(hgl, PROCS_GL, (char const*)PROCNAMES_GL))
        support_opengl = 0;
#endif

    display = FN_XOpenDisplay(NULL);

#if ENABLE_XSHM
    support_xshm = 1;
    hxext = dlopen((char const*)LIBXEXT_SO, RTLD_NOW);
    if(!hxext || !load_procs(hxext, PROCS_XEXT, (char const*)PROCNAMES_XEXT))
        support_xshm = 0;
    if(!load_procs(hc, PROCS_C_SHM, (char const*)PROCNAMES_C_SHM))
        support_xshm = 0;

    if(1 &&
        DisplayString(display)[0] != ':' &&
        !memequal(DisplayString(display), "unix:", 5))
    {
        support_xshm = 0;
    }
#endif

    if(
#if ENABLE_OPENGL
        !create_opengl_window() &&
#endif
        !create_sw_window()
        )
    {
        shutdown(-1);
    }

    FN_XSelectInput(display, window,
        ExposureMask |
        StructureNotifyMask |
        ButtonPressMask |
        ButtonReleaseMask |
        PointerMotionMask |
        0);

    wm_protocols = FN_XInternAtom(display, (char*)ATOM_WM_PROTOCOLS, False);
    wm_delete_window = FN_XInternAtom(display, (char*)ATOM_WM_DELETE_WINDOW, False);
    net_wm_ping = FN_XInternAtom(display, (char*)ATOM__NEW_WM_PING, False);

    FN_XSetWMProtocols(display, window, &wm_delete_window, 1);

    FN_XStoreName(display, window, "tiny");

    update_ms();

    run();

    shutdown(0);

#ifndef NDEBUG
    return 0;
#endif
}

static int prev_mouse_x = INT32_MIN;
static int prev_mouse_y = INT32_MIN;

static void get_mouse_pos(int* x, int* y)
{
    Window winroot, winchild;
    int xroot, yroot;
    unsigned int mask;
    if(!FN_XQueryPointer(
        display,
        window,
        &winroot,
        &winchild,
        &xroot, &yroot,
        x, y,
        &mask))
    {
        *x = prev_mouse_x;
        *y = prev_mouse_y;
    }
    else
    {
        *x = (*x - imx) * FBW / imw;
        *y = (*y - imy) * FBH / imh;
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

static int process_event(input* i, XEvent const* e)
{
    switch(e->type)
    {
    case ClientMessage:
        if(e->xclient.message_type == wm_protocols)
        {
            Atom const protocol = e->xclient.data.l[0];
            if(protocol == wm_delete_window)
                shutdown(0);
        }
        break;
    case Expose:
        paint();
        break;
    case ButtonPress:
        if(0);
        else if(e->xbutton.button == Button1)
            i->type = INPUT_MOUSE_DOWN;
        else if(e->xbutton.button == Button2)
            i->type = INPUT_MOUSE2_DOWN;
        else
            break;
        return 1;
    case ButtonRelease:
        if(0);
        else if(e->xbutton.button == Button1)
            i->type = INPUT_MOUSE_UP;
        else if(e->xbutton.button == Button2)
            i->type = INPUT_MOUSE2_UP;
        else
            break;
        return 1;
    default:
        break;
    }
    return 0;
}

static int wait_for_input(input* i)
{
    XEvent e;
    if(mouse_moved(i)) return 1;
    while(1)
    {
        int p;
        FN_XNextEvent(display, &e);
        update_ms();
        p = process_event(i, &e);
        if(p || mouse_moved(i))
            return 1;
    }
    shutdown(0);
    return 0;
}

static int wait_for_input_timeout(input* i, int* timeout_ms)
{
    int msprev = ms;
    XEvent e;
    if(*timeout_ms <= 0)
    {
        i->type = INPUT_TIMER;
        return 1;
    }
    while(1)
    {
        int ret = FN_XPending(display);
        update_ms();
        *timeout_ms -= (ms - msprev);
        msprev = ms;

        if(ret > 0)
        {
            FN_XNextEvent(display, &e);
            if(process_event(i, &e) || mouse_moved(i))
                return 1;            
        }
        else if(*timeout_ms <= 0)
        {
            i->type = INPUT_TIMER;
            return 1;
        }
        else
        {
            FN_usleep(1000);
        }
    }
    shutdown(0);
    return 0;
}

void* memmove(void* dest, const void* src, size_t count)
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


#else
typedef int avoid_unused_translation_unit;
#endif

