#include "ch/ch.h"

#include <stdio.h>
//#include <time.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static std::chrono::steady_clock::time_point start_time;

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

static void best_move(ch_move m)
{
    std::cout << "bestmove " << ch_extended_algebraic(m) << std::endl;
}

static ch_system_info const SYSINF =
{
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
static void set_hash_mem(int megabytes)
{
    int mpow = 0;
    megabytes = clamp(megabytes, 0, 4096);
    free(hash_mem);
    if(megabytes == 0)
    {
        hash_mem = NULL;
        ch_set_hash(NULL, 0);
        return;
    }
    while((1 << mpow) <= megabytes)
        ++mpow;
    --mpow;
    hash_mem = malloc((1 << mpow) << 20);
    if(!hash_mem)
        hash_mem = malloc(64 << 20);
    ch_set_hash(hash_mem, mpow);
}

int __cdecl main(void)
{
    hash_mem = NULL;
    ch_init(&SYSINF);
    set_hash_mem(256);
    ch_new_game();

    std::thread thrd1(&ch_thread_start);

    start_time = std::chrono::steady_clock::now();

    for(;;)
    {
        std::string line;
        std::getline(std::cin, line);

        if(line == "quit")
            break;
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
            ch_clear_caches();
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
                        set_hash_mem(atoi(line.c_str() + n + 7));
                }
            }
        }
        else if(startswith(line, "position"))
        {
            if(contains(line, "startpos"))
                ch_new_game();
            else if(contains(line, "fen "))
                ch_load_fen(line.c_str() + line.find("fen ", 0) + 4);
            if(contains(line, "moves "))
            {
                std::stringstream moves_ss(line.substr(line.find("moves ") + 6));
                std::vector<std::string> moves {
                    std::istream_iterator<std::string>{moves_ss},
                    std::istream_iterator<std::string>{} };
                for(std::string const& m : moves)
                {
                    ch_do_move_str(m.c_str());
                }
            }
        }
        else if(startswith(line, "go"))
        {
            ch_search_limits limits = { 0 };
            if(contains(line, " depth "))
                limits.depth = clamp(atoi(
                    line.c_str() + line.find(" depth ", 0) + 7), 1, 64);
            if(contains(line, " movetime "))
                limits.mstime = atoi(
                    line.c_str() + line.find(" movetime ", 0) + 10);
            ch_search(&limits);
        }
        else if(startswith(line, "stop"))
        {
            ch_stop();
        }
        else if(line == "eval")
        {
            std::cout << ch_evaluate() << std::endl;
        }
        else if(startswith(line, "perft "))
        {
            int depth = atoi(line.c_str() + 6);
            depth = clamp(depth, 1, 64);
            uint64_t counts[256] = { 0 };
            uint64_t t = ch_perft(depth, counts);
            std::cout << t << std::endl;
        }
        else if(startswith(line, "see "))
        {
            std::cout << ch_see(&line[4]) << std::endl;
        }
    }

    ch_kill_threads();

    thrd1.join();

    free(hash_mem);

	return 0;
}
