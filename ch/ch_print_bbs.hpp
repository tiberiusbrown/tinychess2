#pragma once

#include <stdint.h>
#include <stdio.h>

#include "ch_internal.hpp"
#include "ch_position.hpp"

namespace ch
{

#if !defined(NDEBUG)
void print_bbs(uint64_t bbs[], int n)
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
    char const* const C = "PpNnBbRrQqKk.";
    for(int i = 0; i < 8; ++i)
    {
        uint64_t m = (1ull << (i * 8));
        int bbi;
        for(int k = 0; k < 8; ++k, m <<= 1)
        {
            for(bbi = 0; bbi < (int)p.bbs.size() - 1; ++bbi)
                if(p.bbs[bbi] & m) { printf("%c", C[bbi]); break; }
            if(bbi >= (int)p.bbs.size() - 1) printf(".");
        }
        printf("  ");
        for(int k = 0; k < 8; ++k)
            printf("%c", C[p.pieces[i * 8 + k]]);
        printf("\n");
    }
    printf("\n");
}
#else
void print_bbs(uint64_t [], int) {}
void print_position(position const&) {}
#endif

}
