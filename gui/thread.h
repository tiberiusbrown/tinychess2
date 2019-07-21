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
typedef pthread_mutex_t mutex;
typedef pthread_t systhread;
typedef pthread_cont_t cond;
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
    threading_avail = load_procs(kernel32, PROCS_THREAD, PROCNAMES_THREAD);
}

static void mutex_create(mutex* m)
{
#if defined(_WIN32)
    FN_InitializeCriticalSection(m);
#else
    pthread_mutex_init(m, NULL);
#endif
}

static void mutex_destroy(mutex* m)
{
#if defined(_WIN32)
    FN_DeleteCriticalSection(m);
#else
    pthread_mutex_destroy(m);
#endif
}

static void mutex_lock(mutex* m)
{
#if defined(_WIN32)
    FN_EnterCriticalSection(m);
#else
    pthread_mutex_lock(m);
#endif
}

static void mutex_unlock(mutex* m)
{
#if defined(_WIN32)
    FN_LeaveCriticalSection(m);
#else
    pthread_mutex_unlock(m);
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
    pthread_create(&t->t, NULL, thread_func_helper, (void*)t);
#endif
}

static void thread_join(thread* t)
{
#if defined(_WIN32)
    FN_WaitForSingleObject(t->t, INFINITE);
    FN_CloseHandle(t->t);
#else
    void* pres;
    pthread_join(t->t, &pres);
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
    sched_yield();
#endif
}

static void cond_create(cond* c)
{
#if defined(_WIN32)
    c->waiters = 0;
    mutex_create(&c->m);
    c->events[0] = FN_CreateEventA(NULL, FALSE, FALSE, NULL);
    c->events[1] = FN_CreateEventA(NULL, TRUE, FALSE, NULL);
#else
    pthread_cond_init(cond, NULL);
#endif
}

static void cond_destroy(cond* c)
{
#if defined(_WIN32)
    FN_CloseHandle(c->events[0]);
    FN_CloseHandle(c->events[1]);
    mutex_destroy(&c->m);
#else
    othread_cond_destroy(c);
#endif
}

static void cond_one(cond* c)
{
#if defined(_WIN32)
    int has_waiters;
    mutex_lock(&c->m);
    has_waiters = (c->waiters > 0);
    mutex_unlock(&c->m);
    if(has_waiters)
        FN_SetEvent(c->events[0]);
#else
    pthread_cond_signal(c);
#endif
}

static void cond_all(cond* c)
{
#if defined(_WIN32)
    int has_waiters;
    mutex_lock(&c->m);
    has_waiters = (c->waiters > 0);
    mutex_unlock(&c->m);
    if(has_waiters)
        FN_SetEvent(c->events[1]);
#else
    pthread_cond_broadcast(c);
#endif
}

static void cond_wait(cond* c)
{
    DWORD r;
    int last;

    mutex_lock(&c->m);
    ++c->waiters;
    mutex_unlock(&c->m);

    r = FN_WaitForMultipleObjects(2, c->events, FALSE, INFINITE);

    mutex_lock(&c->m);
    --c->waiters;
    last = (r == (WAIT_OBJECT_0 + 1)) && (c->waiters == 0);
    mutex_unlock(&c->m);

    if(last) FN_ResetEvent(c->events[1]);
}

#endif

#endif
