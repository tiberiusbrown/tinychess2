#pragma once

#include <array>
#include <atomic>

#include "ch_internal.hpp"
#include "ch_move.hpp"
#include "ch_position.hpp"

namespace ch
{

struct history_heuristic
{
    // butterfly
    std::array<std::array<std::atomic<uint32_t>, 64>, 64> bf;

    //  relative history
    std::array<std::array<std::atomic<uint32_t>, 64>, 64> hh;

    // countermove
    std::array<std::array<std::atomic<uint32_t>, 64>, 64> cm;

    void clear()
    {
        memzero32(&bf[0][0], sizeof(bf) / sizeof(bf[0][0]));
        memzero32(&hh[0][0], sizeof(hh) / sizeof(hh[0][0]));
        memzero32(&cm[0][0], sizeof(cm) / sizeof(cm[0][0]));
    }

    CH_FORCEINLINE void increment_hh(move mv, int inc)
    {
        hh[mv.from()][mv.to()] += inc;
    }

    CH_FORCEINLINE void increment_bf(move mv, int inc)
    {
        bf[mv.from()][mv.to()] += inc;
    }

    CH_FORCEINLINE void set_countermove(move mv, move cmv)
    {
        cm[mv.from()][mv.to()] = cmv;
    }

    CH_FORCEINLINE int get_hh_score(move mv) const
    {
        static constexpr int const HISTORY_HEURISTIC_SCALE = 32;
        int pc = mv.from();
        int to = mv.to();
        int den = bf[pc][to];
        int num = hh[pc][to];
        //return std::min(num, HISTORY_HEURISTIC_SCALE);
        return num * HISTORY_HEURISTIC_SCALE / std::max(den, 1);
    }

    CH_FORCEINLINE move get_countermove(move mv)
    {
        return move(uint32_t(cm[mv.from()][mv.to()]));
    }
};


}
