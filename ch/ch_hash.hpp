#pragma once

#include <atomic>
#include <stdint.h>

#include "ch_config.h"
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

// specialize this to override
template<class T>
CH_FORCEINLINE bool hash_info_should_overwrite(
    T const& new_info, T const& old_info)
{
    (void)new_info;
    (void)old_info;
    return true;
}

struct hash_info
{
    packed_move best;
    int16_t value;
    int8_t depth;
    uint8_t age;
    uint8_t flag;
    uint8_t pro_piece;
    enum { LOWER, UPPER, EXACT };
};

template<>
CH_FORCEINLINE bool hash_info_should_overwrite(
    hash_info const& new_info, hash_info const& old_info)
{
    int da = int(int8_t(new_info.age - old_info.age));
    int dd = int(int8_t(new_info.depth - old_info.depth));
    int score = dd * 2 + da * 8;
    score += (new_info.flag == hash_info::EXACT) * 2;
    score -= (old_info.flag == hash_info::EXACT) * 1;
    return score >= 0;
}

struct hash_info_perft
{
    int32_t depth;
    uint32_t count;
};

class trans_table
{
private:
    static constexpr int const NUM_BUCKETS = 1 << CH_HASH_BUCKETS_POW;

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
            mask = ((1ull << (mem_size_mb_log2 + 20 - 4 -
                CH_HASH_BUCKETS_POW)) - 1) << CH_HASH_BUCKETS_POW;
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
        uint64_t index = hash & mask;
        for(int i = 0; i < NUM_BUCKETS; ++i)
        {
            entry const& e = entries[index + i];
            uint64_t h = e.hash.load();
            union
            {
                uint64_t b;
                T i;
            } u;
            u.b = e.info.load();
            if(hash != (h ^ u.b))
                continue;
            info = u.i;
            return true;
        }
        return false;
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

        union
        {
            uint64_t b;
            T i;
        } u;

        uint64_t index = hash & mask;

        // look for exact hash match first
        for(int i = 0; i < NUM_BUCKETS; ++i)
        {
            entry& e = entries[index + i];
            uint64_t h = e.hash.load();
            u.b = e.info.load();
            if(hash == (h ^ u.b))
            {
                if(hash_info_should_overwrite<T>(info, u.i))
                {
                    u.i = info;
                    e.hash.store(hash ^ u.b);
                    e.info.store(u.b);
                }
                return;
            }
        }

        // overwrite any available
        for(int i = 0; i < NUM_BUCKETS; ++i)
        {
            entry& e = entries[index + i];
            u.b = e.info.load();
            if(hash_info_should_overwrite<T>(info, u.i))
            {
                u.i = info;
                e.hash.store(hash ^ u.b);
                e.info.store(u.b);
                return;
            }
        }
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
