#ifdef __linux__

#include <ch.h>

#include <stdio.h>

extern "C" __attribute__((visibility("default"))) void start(void)
{
    ch_system_info info = { 0 };
    ch_init(&info);
    ch_new_game();

    uint64_t counts[256];
    unsigned int r = (unsigned int)ch_perft(7, counts);

    printf("%u\n", r);

    int ret;
    asm volatile(""
        "syscall"
        : "=a"(ret)
        : "a"(60), "D"(0)
        : "memory", "rcx", "r11"
    );
}

#endif

