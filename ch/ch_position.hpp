#pragma once

#include "ch_internal.hpp"

#include "ch_genmoves.hpp"

#include "ch_hash.hpp"

#include "ch_move.hpp"

namespace ch
{

struct position
{
    CH_ALIGN(64) std::array<uint64_t, BLACK + END_BB> bbs;

    std::array<uint8_t, 64> pieces;

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

CH_OPT_SIZE void position::new_game()
{
    load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
}

CH_OPT_SIZE void position::load_fen(char const* fen)
{
    memzero(&bbs[0], int(bbs.size() * sizeof(bbs[0])));
    memzero(&pieces[0], int(pieces.size() * sizeof(pieces[0])));

    stack_index = 0;
    stack().cap_piece = EMPTY;

    char c = 1;
    uint64_t m = 1;
    int i = 0;
    while(' ' != (c = *fen++) && c)
    {
#define CH_PUT(c_, p_) \
    bbs[c_ + p_] |= m; \
    pieces[i] = c_ + p_; \
    m <<= 1; ++i; break
        switch(c)
        {
        case 'p': CH_PUT(BLACK, PAWN);
        case 'n': CH_PUT(BLACK, KNIGHT);
        case 'b': CH_PUT(BLACK, BISHOP);
        case 'r': CH_PUT(BLACK, ROOK);
        case 'q': CH_PUT(BLACK, QUEEN);
        case 'k': CH_PUT(BLACK, KING);
        case 'P': CH_PUT(WHITE, PAWN);
        case 'N': CH_PUT(WHITE, KNIGHT);
        case 'B': CH_PUT(WHITE, BISHOP);
        case 'R': CH_PUT(WHITE, ROOK);
        case 'Q': CH_PUT(WHITE, QUEEN);
        case 'K': CH_PUT(WHITE, KING);
#undef CH_PUT
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8':
            m <<= int(c - '0');
            i += int(c - '0');
            break;
        case '/':
        default:
            break;
        }
    }

    // TODO: current turn
    current_turn = WHITE;
    while(' ' != (c = *fen++) && c)
    {
        switch(c)
        {
        case 'w': case 'W': current_turn = WHITE; break;
        case 'b': case 'B': current_turn = BLACK; break;
        default:
            break;
        }
    }

    stack().castling_rights =
        CASTLE_WQ_MASK |
        CASTLE_BQ_MASK |
        CASTLE_WK_MASK |
        CASTLE_BK_MASK;
    while(c && ' ' != (c = *fen++))
    {
        switch(c)
        {
        case 'K': stack().castling_rights &= ~CASTLE_WK_MASK; break;
        case 'Q': stack().castling_rights &= ~CASTLE_WQ_MASK; break;
        case 'k': stack().castling_rights &= ~CASTLE_BK_MASK; break;
        case 'q': stack().castling_rights &= ~CASTLE_BQ_MASK; break;
        default:
            break;
        }
    }

    // en passant target square
    int ep_rank = -1, ep_file = -1;
    stack().ep_sq = 0;
    while(' ' != (c = *fen++) && c)
    {
        switch(c)
        {
        case 'a': case 'b': case 'c': case 'd':
        case 'e': case 'f': case 'g': case 'h':
            ep_file = (c - 'a');
            break;
        case '3':
            ep_rank = 4;
            break;
        case '6':
            ep_rank = 5;
            break;
        default:
            break;
        }
    }
    if(ep_rank >= 1 && ep_file >= 0)
        stack().ep_sq = (8 - ep_rank) * 8 + ep_file;
}

static constexpr uint8_t const CASTLING_SPOILERS[64] =
{
    CASTLE_BQ_MASK, 0, 0, 0,
    CASTLE_BQ_MASK | CASTLE_BK_MASK,
    0, 0, CASTLE_BK_MASK,

    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    CASTLE_WQ_MASK, 0, 0, 0,
    CASTLE_WQ_MASK | CASTLE_WK_MASK,
    0, 0, CASTLE_WK_MASK,
};

template<acceleration accel>
void position::do_move(move const& mv)
{
    int a = mv.from();
    int b = mv.to();
    int p = pieces[a];
    int cap = pieces[b];
    uint64_t cap_bb = (1ull << b);
    uint64_t p_bb = (1ull << a) | cap_bb;

    assert(cap != WHITE + KING && cap != BLACK + KING);

    auto& st = stack_push();
    st.cap_piece = cap;

    st.castling_rights |=
        CASTLING_SPOILERS[a] | CASTLING_SPOILERS[b];

    current_turn = opposite(current_turn);

    st.ep_sq = 0;
    if(mv.is_special())
    {
        if(mv.is_pawn_dmove())
            st.ep_sq = b;
        else if(mv.is_castleq())
        {
            int rook = p + ROOK - KING;
            bbs[rook] ^= (0x9ull << (a - 4));
            pieces[a - 1] = uint8_t(rook);
            pieces[a - 4] = EMPTY;
        }
        else if(mv.is_castlek())
        {
            int rook = p + ROOK - KING;
            bbs[rook] ^= (0xAull << a);
            pieces[a + 1] = uint8_t(rook);
            pieces[a + 3] = EMPTY;
        }
        else if(mv.is_en_passant())
        {
            int sq = (mv >> 16) & 0xff;
            st.cap_piece = pieces[sq];
            bbs[pieces[sq]] ^= (1ull << sq);
            pieces[sq] = EMPTY;
        }
        else if(mv.is_promotion())
        {
            int t = (mv >> 16) & 0xff;
            bbs[cap] ^= cap_bb;
            bbs[p] ^= (1ull << a);
            bbs[t] ^= cap_bb;
            pieces[a] = EMPTY;
            pieces[b] = uint8_t(t);
            return;
        }
    }

    bbs[cap] ^= cap_bb;
    bbs[p] ^= p_bb;
    pieces[a] = EMPTY;
    pieces[b] = uint8_t(p);
}

template<acceleration accel>
void position::undo_move(move const& mv)
{
    int a = mv.from();
    int b = mv.to();
    int p = pieces[b];
    int cap = stack().cap_piece;
    uint64_t cap_bb = (1ull << b);
    uint64_t p_bb = (1ull << a) | cap_bb;

    stack_pop();

    current_turn = opposite(current_turn);

    if(mv.is_special())
    {
        if(mv.is_castleq())
        {
            int rook = p + ROOK - KING;
            bbs[rook] ^= (0x9ull << (a - 4));
            pieces[a - 1] = EMPTY;
            pieces[a - 4] = uint8_t(rook);
        }
        else if(mv.is_castlek())
        {
            int rook = p + ROOK - KING;
            bbs[rook] ^= (0xAull << a);
            pieces[a + 1] = EMPTY;
            pieces[a + 3] = uint8_t(rook);
        }
        else if(mv.is_en_passant())
        {
            int sq = (mv >> 16) & 0xff;
            pieces[sq] = uint8_t(cap);
            bbs[pieces[sq]] ^= (1ull << sq);
            cap = EMPTY;
        }
        else if(mv.is_promotion())
        {
            int t = (mv >> 16) & 0xff;
            p = (p >= BLACK ? BLACK : WHITE) + PAWN;
            bbs[cap] ^= cap_bb;
            bbs[p] ^= (1ull << a);
            bbs[t] ^= cap_bb;
            pieces[a] = uint8_t(p);
            pieces[b] = uint8_t(cap);
            return;
        }
    }

    bbs[cap] ^= cap_bb;
    bbs[p] ^= p_bb;
    pieces[a] = uint8_t(p);
    pieces[b] = uint8_t(cap);
}

#if CH_COLOR_TEMPLATE
template<color c, acceleration accel>
uint64_t position::perft(int depth)
#else
template<acceleration accel>
uint64_t position::perft(color c, int depth)
#endif
{
    move mvs[256];
    int num;

    //if(depth <= 0)
    //    return 1;

#if CH_COLOR_TEMPLATE
    num = move_generator<c, accel>::generate(mvs, *this);
#else
    num = move_generator<accel>::generate(c, mvs, *this);
#endif
    if(depth <= 1)
        return num;

    uint64_t r = 0;
    for(int n = 0; n < num; ++n)
    {
        do_move<accel>(mvs[n]);
#if CH_COLOR_TEMPLATE
        r += perft<opposite(c), accel>(depth - 1);
#else
        r += perft<accel>(opposite(c), depth - 1);
#endif
        undo_move<accel>(mvs[n]);
    }
    return r;
}

template<acceleration accel>
uint64_t position::root_perft(int depth, uint64_t* counts)
{
    move mvs[256];
    uint64_t total = 0;
    int num;

#if CH_COLOR_TEMPLATE
    if(current_turn == WHITE)
        num = move_generator<WHITE, accel>::generate(mvs, *this);
    else
        num = move_generator<BLACK, accel>::generate(mvs, *this);
#else
    num = move_generator<accel>::generate(current_turn, mvs, *this);
#endif

    for(int n = 0; n < num; ++n)
    {
        uint64_t count = 1;
        do_move<accel>(mvs[n]);
        if(depth > 1)
        {
#if CH_COLOR_TEMPLATE
            if(current_turn == WHITE)
                count = perft<WHITE, accel>(depth - 1);
            else
                count = perft<BLACK, accel>(depth - 1);
#else
            count = perft<accel>(current_turn, depth - 1);
#endif
        }
        undo_move<accel>(mvs[n]);
        //if(!brief)
        //    printf("%s: %llu\n", mvs[n].extended_algebraic(), count);
        total += count;
        *counts++ = count;
    }

    return total;
}

}
