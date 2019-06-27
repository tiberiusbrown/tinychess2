#include "ch/ch.h"

#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static uint32_t get_ms(void)
{
    return clock() * 1000 / CLOCKS_PER_SEC;
}

static void thread_yield(void)
{
    std::this_thread::yield();
}

static void uci_info(char const* info)
{
    std::cout << info << std::endl;
}

static ch_system_info const SYSINF =
{
    &get_ms,
    &thread_yield,
    &uci_info,
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

int main(void)
{
    ch_init(&SYSINF);
    ch_new_game();

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
                << "id author Peter Brown" << std::endl;
        }
        else if(startswith(line, "position"))
        {
            if(contains(line, "startpos"))
                ch_new_game();
            else if(contains(line, "fen "))
                ch_load_fen(&line[line.find("fen ", 0) + 4]);
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
            ch_move m = ch_depth_search(4);
            ch_do_move(m);
            std::cout << "info nodes " << ch_get_nodes() << std::endl;
            std::cout << "bestmove " << ch_extended_algebraic(m) << std::endl;
        }
    }

	return 0;
}
