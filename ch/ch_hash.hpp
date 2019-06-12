#pragma once

#include <atomic>
#include <stdint.h>

#include "ch_internal.hpp"

// lockless transposition table

static_assert(ATOMIC_LLONG_LOCK_FREE == 2, "atomic_uint64_t must be lock free");

namespace ch
{

class trans_table
{
public:
    CH_ALIGN(4) struct entry_info
    {
        int16_t value;
        int8_t depth;
        uint8_t flags;
    };

private:
    CH_ALIGN(8) struct entry_data
    {
        uint32_t hash_hi;
        entry_info info;
    };
    static_assert(sizeof(entry_data) == 8, "entry_data size mismatch");
    typedef std::atomic<entry_data> entry;

public:
    trans_table() {}

    void set_memory(void* mem, int mem_size_mb_log2)
    {
        entries = (entry*)mem;
        mask = (1ull << (mem_size_mb_log2 - 3)) - 1;
    }

    bool get(uint64_t hash, entry_info* info) const
    {
        entry_data t = entries[hash & mask].load();
        if(t.hash_hi == uint32_t(hash >> 32))
        {
            *info = t.info;
            return true;
        }
        return false;
    }

    void put(uint64_t hash, entry_info info)
    {
        entry_data t;
        t.hash_hi = uint32_t(hash >> 32);
        t.info = info;
        entries[hash & mask].store(t);
    }

private:
    uint64_t mask;
    entry* entries;
};

}
