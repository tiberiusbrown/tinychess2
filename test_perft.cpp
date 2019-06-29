#include "ch/ch.h"

#include <stdio.h>
#include <stdlib.h>

#include <chrono>

uint64_t total;

void perft_test(int depth, uint64_t expected, char const* fen)
{
    uint64_t counts[256];
    printf("FEN: %s\n", fen);
    ch_load_fen(fen);
    auto t_start = std::chrono::high_resolution_clock::now();
    uint64_t actual = ch_perft(depth, counts);
    auto t_end = std::chrono::high_resolution_clock::now();
    using dps = std::chrono::duration<double, std::chrono::seconds::period>;
    double dt = dps(t_end - t_start).count();
    total += actual;
    if(actual == expected)
        printf("PASS\n");
    else
    {
        printf("FAIL\n   expected: %llu\n   actual:   %llu\n", expected, actual);
        //exit(-1);
    }
    printf("NPS: %.3fM\n\n", double(actual) / dt * 1e-6);
}

#ifdef _MSC_VER
#define CDECL __cdecl
#else
#define CDECL
#endif

static uint64_t hash_mem[(64 << 20) / 8];

int CDECL main()
{
    ch_system_info info = { 0 };
    ch_init(&info);
    ch_set_hash(hash_mem, 6);
    total = 0;

    auto t_start = std::chrono::high_resolution_clock::now();

    //perft_test(7, 3195901860ull, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
    //perft_test(6, 8031647685ull, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    
    //perft_test(6, 119060324ull, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
    perft_test(5, 193690690ull, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    perft_test(6, 71179139ull, "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - -");
    perft_test(7, 178633661ull, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    perft_test(6, 706045033ull, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -");
    perft_test(5, 89941194ull, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -");
    perft_test(5, 164075551ull, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -");

    auto t_end = std::chrono::high_resolution_clock::now();
    using dps = std::chrono::duration<double, std::chrono::seconds::period>;
    double dt = dps(t_end - t_start).count();

    printf("Total time: %.3f\n", dt);
    printf("Total NPS:  %.2f M\n", double(total) / (dt * 1e6));

    return 0;
}
