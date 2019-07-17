#pragma once

#include <algorithm>
#include <array>

#include "ch_move.hpp"
#include "ch_genmoves.hpp"

namespace ch
{

struct move_list
{
    move& operator[](int i) { return m[i]; }
    move const& operator[](int i) const { return m[i]; }

#if CH_COLOR_TEMPLATE
    template<color c, acceleration accel> void generate(position& p)
    {
        n = move_generator<c, accel>::generate(&m[0], p);
    }
    template<acceleration accel> void generate(color c, position& p)
    {
        if(c == WHITE)
            n = move_generator<WHITE, accel>::generate(&m[0], p);
        else
            n = move_generator<BLACK, accel>::generate(&m[0], p);
    }
#else
    template<acceleration accel> void generate(color c, position& p)
    {
        n = move_generator<accel>::generate(c, &m[0], p);
    }
#endif

    struct sort_descending
    {
        bool operator()(move const &a, move const &b) const { return a.d > b.d; }
    };
    void sort()
    {
        std::sort(begin(), end(), sort_descending());
    }

    constexpr int size() const { return n; }
    constexpr bool empty() const { return n == 0; }

    std::array<move, 256>::iterator begin() { return m.begin(); }
    std::array<move, 256>::iterator end() { return m.begin() + n; }
    std::array<move, 256>::const_iterator begin() const { return m.begin(); }
    std::array<move, 256>::const_iterator end() const { return m.begin() + n; }

    CH_FORCEINLINE move pop_back()
    {
        assert(n > 0);
        return m[--n];
    }
    CH_FORCEINLINE void pop_n(int num)
    {
        assert(n >= num);
        n -= num;
    }

private:

    std::array<move, 256> m;
    int n;
};

}
