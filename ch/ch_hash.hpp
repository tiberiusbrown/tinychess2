#pragma once

#include <atomic>
#include <stdint.h>

#include "ch_internal.hpp"
#include "ch_move.hpp"
#include "ch_random.hpp"

// lockless transposition table (uses xor trick described by Hyatt)

static constexpr int const X = sizeof(std::atomic_flag);

namespace ch
{

static uint64_t hashes[13][64];

// whose turn is it?
static uint64_t hash_turn;
// castling rights
static uint64_t hash_castling_rights[16];

// en passant columns: use extra pawn storage
static constexpr uint64_t const* const& hash_enp = &hashes[PAWN][0];

static void init_hashes(void)
{
    uint64_t seed[4] =
    {
        0xf29c39b263eef763ull,
        0x7b2e950fc56de271ull,
        0xa0cb3ec26ab650a0ull,
        0x69e6e96592d8af17ull,
    };

    for(auto& ph : hashes)
        for(auto& h : ph)
            h = random(seed);
    for(auto& h : hashes[EMPTY])
        h = 0ull;

    hash_turn = random(seed);

    {
        uint64_t ch[4];
        for(auto& h : ch) h = random(seed);
        for(int i = 0; i < 16; ++i)
        {
            hash_castling_rights[i] = 0ull;
            for(int j = 0; j < 4; ++j)
                if(i & (1 << j))
                    hash_castling_rights[i] ^= ch[j];
        }
    }
}

static constexpr uint8_t TTFLAG_EXACT = 1;
static constexpr uint8_t TTFLAG_LOWER = 2;
static constexpr uint8_t TTFLAG_UPPER = 4;

struct hash_info
{
    move best;
    int16_t value;
    int8_t depth;
    uint8_t flag;
    enum { EXACT, LOWER, UPPER };
};

struct hash_info_perft
{
    int32_t depth;
    uint32_t count;
};

class trans_table
{
private:
    struct entry
    {
        std::atomic<uint64_t> hash;
        std::atomic<uint64_t> info;
    };

public:
    trans_table() {}

    void clear()
    {
#if CH_ENABLE_HASH
        if(entries)
            memzero32(entries, int(mask + 1) * sizeof(entry) / 4);
#endif
    }

    void set_memory(void* mem, int mem_size_mb_log2)
    {
        entries = (entry*)mem;
        if(entries)
        {
            mask = (1ull << (mem_size_mb_log2 + 20 - 4)) - 1;
            clear();
        }
    }

    template<class T>
    CH_FORCEINLINE bool get(uint64_t hash, T& info) const
    {
#if CH_ENABLE_HASH
        static_assert(sizeof(T) == 8, "hash info not 8 bytes");
        if(!entries)
            return false;
        entry const& e = entries[hash & mask];
        union
        {
            uint64_t b;
            T i;
        } u;
        uint64_t a = e.hash.load();
        u.b = e.info.load();
        if(a != (hash ^ u.b))
            return false;
        info = u.i;
        return true;
#else
        (void)hash;
        (void)info;
        return false;
#endif
    }

    template<class T>
    CH_FORCEINLINE void put(uint64_t hash, T info)
    {
#if CH_ENABLE_HASH
        static_assert(sizeof(T) == 8, "hash info not 8 bytes");
        if(!entries)
            return;
        entry& e = entries[hash & mask];
        union
        {
            uint64_t b;
            T i;
        } u;
        u.i = info;
        e.hash.store(hash ^ u.b);
        e.info.store(u.b);
#else
        (void)hash;
        (void)info;
#endif
    }

private:
    uint64_t mask;
    entry* entries;
};

}
