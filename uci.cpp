#include "ch/ch.h"

#include <stdio.h>
#include <stdlib.h>
//#include <time.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static std::chrono::steady_clock::time_point start_time;

static void* alloc(uint32_t bytes) { return malloc((size_t)bytes); }
static void dealloc(void* p) { free(p); }

static uint32_t get_ms(void)
{
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time).count();
}

static void thread_yield(void)
{
    std::this_thread::sleep_for(std::chrono::microseconds(1));
}

static void search_info(
    int depth,
    int seldepth,
    uint64_t nodes,
    int mstime,
    int score,
    uint64_t nps,
    ch_move* pv,
    int pvlen
)
{
    std::cout
        << "info"
        << " depth " << depth
        << " seldepth " << seldepth
        << " time " << mstime;
    std::cout << " score ";
    if(CH_MATE_SCORE - score <= 256)
        std::cout << "mate " << (CH_MATE_SCORE - score + 1) / 2;
    else if(CH_MATED_SCORE - score >= -256)
        std::cout << "mate " << (CH_MATED_SCORE - score - 1) / 2;
    else
        std::cout << "cp " << score;
    std::cout
        << " nps " << nps
        << " nodes " << nodes
        << " pv";
    for(int i = 0; i < pvlen; ++i)
        std::cout << " " << ch_extended_algebraic(pv[i]);
    std::cout << std::endl;
}

static std::atomic<bool> thinking;

static void best_move(ch_move m)
{
    std::cout << "bestmove " << ch_extended_algebraic(m) << std::endl;
    thinking = false;
}

static ch_system_info const SYSINF =
{
    &alloc,
    &dealloc,
    &get_ms,
    &thread_yield,
    &search_info,
    &best_move,
};

static bool startswith(std::string const& str, char const* x)
{
    char const* p = &str[0];
    while(*x)
        if(*x++ != *p++) return false;
    return true;
}

static bool contains(std::string const& str, char const* x)
{
    return str.find(x, 0) != std::string::npos;
}

template<class T> T clamp(T x, T a, T b)
{
    return x < a ? a : x > b ? b : x;
}

static void* hash_mem;
static void set_hash_mem(ch_game* g, int megabytes)
{
    int mpow = 0;
    megabytes = clamp(megabytes, 0, 4096);
    free(hash_mem);
    if(megabytes == 0)
    {
        hash_mem = NULL;
        ch_set_hash(g, NULL, 0);
        return;
    }
    while((1 << mpow) <= megabytes)
        ++mpow;
    --mpow;
    hash_mem = malloc((1 << mpow) << 20);
    if(!hash_mem)
        hash_mem = malloc(64 << 20);
    ch_set_hash(g, hash_mem, mpow);
}

static bool process_command(ch_game* g, std::string const& line)
{
    if(line == "quit")
        return true;
    else if(line == "isready")
    {
        std::cout << "readyok" << std::endl;
    }
    else if(line == "uci")
    {
        std::cout
            << "id name CHRISTINE 0.1" << std::endl
            << "id author Peter Brown" << std::endl
            << "option name Hash type spin default 64 min 0 max 4096" << std::endl
            << "uciok" << std::endl;
    }
    else if(line == "ucinewgame")
    {
        ch_clear_caches(g);
    }
    else if(startswith(line, "setoption "))
    {
        if(contains(line, " name "))
        {
            size_t m = line.find(" name ", 0) + 6;
            size_t n = line.find(" value ", m);
            if(n != std::string::npos)
            {
                if(contains(line, "Hash"))
                    set_hash_mem(g, atoi(line.c_str() + n + 7));
            }
        }
    }
    else if(startswith(line, "position"))
    {
        if(contains(line, "startpos"))
            ch_new_game(g);
        else if(contains(line, "fen "))
            ch_load_fen(g, line.c_str() + line.find("fen ", 0) + 4);
        if(contains(line, "moves "))
        {
            std::stringstream moves_ss(line.substr(line.find("moves ") + 6));
            std::vector<std::string> moves{
                std::istream_iterator<std::string>{moves_ss},
                std::istream_iterator<std::string>{} };
            for(std::string const& m : moves)
            {
                ch_do_move_str(g, m.c_str());
            }
        }
    }
    else if(startswith(line, "go"))
    {
        ch_search_limits limits = { 0 };
        if(contains(line, " nodes "))
            limits.nodes = (uint64_t)atoll(
            line.c_str() + line.find(" nodes ", 0) + 7);
        if(contains(line, " depth "))
            limits.depth = clamp(atoi(
            line.c_str() + line.find(" depth ", 0) + 7), 1, 64);
        if(contains(line, " movetime "))
            limits.mstime = atoi(
            line.c_str() + line.find(" movetime ", 0) + 10);

        if(contains(line, " wtime "))
            limits.remtime[0] = atoi(
            line.c_str() + line.find(" wtime ", 0) + 7);
        if(contains(line, " btime "))
            limits.remtime[1] = atoi(
            line.c_str() + line.find(" btime ", 0) + 7);

        if(contains(line, " mate "))
        {
            limits.depth = clamp(atoi(
                line.c_str() + line.find(" mate ", 0) + 6), 1, 64);
            limits.depth = limits.depth * 2 + 1;
            limits.mate_search = 1;
        }

        if(contains(line, "moves "))
        {
            std::stringstream moves_ss(line.substr(line.find("moves ") + 6));
            std::vector<std::string> moves{
                std::istream_iterator<std::string>{moves_ss},
                std::istream_iterator<std::string>{} };

            int side = ch_current_turn(g);
            for(std::string const& m : moves)
            {
                if(ch_current_turn(g) == side)
                {
                    thinking = true;
                    ch_search(g, &limits);
                    while(thinking)
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                ch_do_move_str(g, m.c_str());
            }
        }

        thinking = true;
        ch_search(g, &limits);
    }
    else if(startswith(line, "stop"))
    {
        ch_stop(g);
    }
    else if(startswith(line, "eval"))
    {
        std::cout << ch_evaluate(g) << std::endl;
    }
    else if(startswith(line, "perft "))
    {
        int depth = atoi(line.c_str() + 6);
        depth = clamp(depth, 1, 64);
        uint64_t counts[256] = { 0 };
        uint64_t t = ch_perft(g, depth, counts);
        std::cout << t << std::endl;
    }
    else if(startswith(line, "see "))
    {
        std::cout << ch_see(g, &line[4]) << std::endl;
    }
    return false;
}

#ifdef _MSC_VER
int __cdecl main(void)
#else
int main(void)
#endif
{
    hash_mem = NULL;
    ch_init(&SYSINF);
    ch_game* g = ch_create();
    set_hash_mem(g, 256);
    ch_new_game(g);

    std::thread thrd1(&ch_thread_start, g);

    start_time = std::chrono::steady_clock::now();

    for(;;)
    {
        std::string line;
        std::getline(std::cin, line);
        if(process_command(g, line))
            break;
    }

    ch_kill_threads(g);

    thrd1.join();

    ch_destroy(g);
    free(hash_mem);

	return 0;
}
