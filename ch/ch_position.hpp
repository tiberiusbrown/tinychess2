#pragma once

#include "ch_internal.hpp"

#include "ch_genmoves.hpp"

#include "ch_hash.hpp"

#include "ch_move.hpp"

namespace ch
{

struct position
{
    CH_ALIGN(64) std::array<uint8_t, 64> pieces;
    CH_ALIGN(64) std::array<uint64_t, NUM_BB> bbs;

    static constexpr int const STACK_SIZE = 64;
    struct stack_node
    {
        int cap_piece;
        int ep_sq;
        int castling_rights;
    };
    std::array<stack_node, STACK_SIZE> stack_data;
    int stack_index;
    stack_node& stack() { return stack_data[stack_index]; }
    stack_node const& stack() const { return stack_data[stack_index]; }
    stack_node& stack_push()
    {
        stack_data[stack_index + 1] = stack_data[stack_index];
        ++stack_index;
        return stack();
    }
    inline void stack_pop() { --stack_index; }

    color current_turn;

    void new_game();
    void load_fen(char const* fen);

    template<acceleration accel>
    void do_move(move const& mv);

    template<acceleration accel>
    void undo_move(move const& mv);

#if CH_COLOR_TEMPLATE
    template<color c, acceleration accel>
    uint64_t perft(int depth);
#else
    template<acceleration accel>
    uint64_t perft(color c, int depth);
#endif

    template<acceleration accel>
    uint64_t root_perft(int depth, uint64_t* counts);
};

}
