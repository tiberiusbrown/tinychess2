#pragma once

#include <atomic>
#include <stdint.h>

#include "ch_internal.hpp"

// lockless transposition table (based on Hyatt's description)

static constexpr int const X = sizeof(std::atomic_flag);

namespace ch
{

class trans_table
{
public:
    struct entry_info
    {
        move best;
        int16_t value;
        int8_t depth;
        uint8_t flags;
    };
    static_assert(sizeof(entry_info) <= 8, "entry_info too large");

private:
    struct entry
    {
        std::atomic_uint64_t hash;
        std::atomic_uint64_t info;
    };

public:
    trans_table() {}

    void set_memory(void* mem, int mem_size_mb_log2)
    {
        entries = (entry*)mem;
        mask = (1ull << (mem_size_mb_log2 + 20 - 3)) - 1;
    }

    bool get(uint64_t hash, entry_info* info) const
    {
        entry const& e = entries[hash & mask];

        union
        {
            uint64_t b;
            entry_info i;
        } u;

        uint64_t a = e.hash.load();
        u.b = e.info.load();

        if(a != (hash ^ u.b))
            return false;

        *info = u.i;
        return true;
    }

    void put(uint64_t hash, entry_info info)
    {
        entry& e = entries[hash & mask];
        union
        {
            uint64_t b;
            entry_info i;
        } u;
        u.i = info;
        e.hash.store(hash ^ u.b);
        e.info.store(u.b);
    }

private:
    uint64_t mask;
    entry* entries;
};

}
