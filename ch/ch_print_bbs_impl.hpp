#pragma once

#include <stdint.h>
#include <stdio.h>

#include "ch_position.hpp"

namespace ch
{

#ifndef NDEBUG
void print_bbs(uint64_t const bbs[], int n)
{
    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            uint64_t m = (1ull << (i * 8));
            uint64_t bb = bbs[j];
            printf(" ");
            for(int k = 0; k < 8; ++k, m <<= 1)
                printf("%c", (bb & m) ? 'X' : '.');
            printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}

void print_position(position const& p)
{
    char const* const C = ".PNBRQKpnbrqk";
    for(int i = 0; i < 8; ++i)
    {
        uint64_t m = (1ull << (i * 8));
        int bbi;
        for(int k = 0; k < 8; ++k, m <<= 1)
        {
            for(bbi = 1; bbi < (int)p.bbs.size(); ++bbi)
                if(p.bbs[bbi] & m) { printf("%c", C[bbi]); break; }
            if(bbi >= (int)p.bbs.size()) printf("%c", C[0]);
        }
        printf("\n");
    }
    printf("\n");
}
#else
void print_bbs(uint64_t const [], int) {}
void print_position(position const&) {}
#endif

}
