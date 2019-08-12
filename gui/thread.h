#ifndef THREAD_H
#define THREAD_H

#include "config.h"

#if ENABLE_THREAD

#include <stdint.h>
#include "system.h"

typedef void(*threadfunc)(void*);

#if defined(_WIN32)
typedef CRITICAL_SECTION mutex;
typedef HANDLE systhread;
typedef struct
{
    HANDLE events[2];
    uint32_t waiters;
    mutex m;
} cond;
#else
#include <pthread.h>
typedef pthread_mutex_t mutex;
typedef pthread_t systhread;
typedef pthread_cond_t cond;
#endif

typedef struct
{
    systhread t;
    threadfunc f;
    void* u;
    mutex mtx;
    int running;
} thread;

static int threading_avail = 0;

/* called by entry function */
static void init_threading(void)
{
#if defined(_WIN32)
    threading_avail = load_procs(kernel32, PROCS_THREAD, PROCNAMES_THREAD);
#else
    threading_avail = load_procs(hpthread, PROCS_THREAD, PROCNAMES_THREAD);
#endif
}

static void mutex_create(mutex* m)
{
#if defined(_WIN32)
    FN_InitializeCriticalSection(m);
#else
    //FN_pthread_mutex_init(m, NULL);
    mutex t = PTHREAD_MUTEX_INITIALIZER;
    *m = t;
#endif
}

static void mutex_destroy(mutex* m)
{
#if defined(_WIN32)
    FN_DeleteCriticalSection(m);
#else
    //FN_pthread_mutex_destroy(m);
    (void)m;
#endif
}

static void mutex_lock(mutex* m)
{
#if defined(_WIN32)
    FN_EnterCriticalSection(m);
#else
    FN_pthread_mutex_lock(m);
#endif
}

static void mutex_unlock(mutex* m)
{
#if defined(_WIN32)
    FN_LeaveCriticalSection(m);
#else
    FN_pthread_mutex_unlock(m);
#endif
}

#if defined(_WIN32)
static DWORD WINAPI thread_func_helper(LPVOID param)
#else
static void* thread_func_helper(void* param)
#endif
{
    thread* t = (thread*)param;
    t->f(t->u);
    mutex_lock(&t->mtx);
    t->running = 0;
    mutex_unlock(&t->mtx);
    return 0;
}

static void thread_create(thread* t, threadfunc f, void* u)
{
    t->f = f;
    t->u = u;
    mutex_create(&t->mtx);
    t->running = 1;
#if defined(_WIN32)
    t->t = FN_CreateThread(NULL, 0, thread_func_helper, (LPVOID)t, 0, NULL);
#else
    FN_pthread_create(&t->t, NULL, thread_func_helper, (void*)t);
#endif
}

static void thread_join(thread* t)
{
#if defined(_WIN32)
    FN_WaitForSingleObject(t->t, INFINITE);
    FN_CloseHandle(t->t);
#else
    void* pres;
    FN_pthread_join(t->t, &pres);
#endif
    mutex_destroy(&t->mtx);
}

static int thread_running(thread* t)
{
    int r;
    mutex_lock(&t->mtx);
    r = t->running;
    mutex_unlock(&t->mtx);
    return r;
}

static void thread_yield(void)
{
#if defined(_WIN32)
    FN_Sleep(1);
#else
    FN_usleep(1);
#endif
}

#endif

#endif
