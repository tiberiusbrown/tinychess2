#pragma once

#include <atomic>
#include <stdint.h>

#include "ch_internal.hpp"

// lockless transposition table

// TODO: MSVC generates memcpy call -- fix!

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

private:
    struct entry_data
    {
        uint64_t hash;
        entry_info info;
    };
    static_assert(sizeof(entry_data) == 16, "entry_data size mismatch");
    typedef std::atomic<entry_data> entry;

public:
    trans_table() {}

    void set_memory(void* mem, int mem_size_mb_log2)
    {
        entries = (entry*)mem;
        mask = (1ull << (mem_size_mb_log2 + 20 - 3)) - 1;
    }

    bool get(uint64_t hash, entry_info* info) const
    {
        entry_data t = entries[hash & mask].load(std::memory_order::memory_order_relaxed);
        if(t.hash == hash)
        {
            *info = t.info;
            return true;
        }
        return false;
    }

    void put(uint64_t hash, entry_info info)
    {
        entry_data t;
        t.hash = hash;
        t.info = info;
        entries[hash & mask].store(t, std::memory_order::memory_order_relaxed);
    }

private:
    uint64_t mask;
    entry* entries;
};

}
