#pragma once

#include "ch_evaluator.hpp"
#include "ch_internal.hpp"
#include "ch_hash.hpp"
#include "ch_magic.hpp"
#include "ch_move.hpp"
#include "ch_move_list.hpp"

namespace ch
{

struct position
{
    CH_ALIGN(64) std::array<uint8_t, 64> pieces;
    CH_ALIGN(64) std::array<uint64_t, NUM_BB> bbs;

    // written to by move_generator<>::generate

    // all pieces of a color
    uint64_t bb_alls[2];
    // squares that enemy is attacking (king is x-rayed)
    uint64_t attacked_nonking;
    // friendly pieces that are pinned against king
    uint64_t pinned_pieces;
    // whether the side to move is in check
    bool in_check;

    int age;

    static constexpr int const STACK_SIZE = 256;
    struct CH_ALIGN(16) stack_node
    {
        uint64_t hash;
        uint32_t UNUSED1;
        move prev_move;
        int16_t piece_vals[2]; // white and black
        int16_t piece_sq[2]; // middle and end game
        uint16_t ply_irreversible;
        uint16_t UNUSED2;
        uint8_t cap_piece;
        uint8_t ep_sq;
        uint8_t castling_rights;
        uint8_t UNUSED3;
    };
    static_assert(sizeof(stack_node) == 32, "");
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
    void stack_reset()
    {
        if(stack_index > 0)
        {
            stack_data[0] = stack_data[stack_index];
            stack_index = 0;
        }
    }

    uint64_t hash() const { return stack().hash; }

    // repetition history
    std::array<uint32_t, 6000> hash_history;
    int ply;

    // move history
    std::array<move, 6000> move_history;

    CH_FORCEINLINE int repetition_count() const
    {
        uint32_t h = uint32_t(hash());
        int count = 0;
        int irr = stack().ply_irreversible;
        for(int n = ply - 4; n >= irr; n -= 2)
        {
            if(hash_history[n] == h)
                ++count;
        }
        return count;
    }

    CH_FORCEINLINE void update_piece_sq(int pc, int from, int to)
    {
        auto* v = stack().piece_sq;
        v[0] -= piece_tables[0][pc][from];
        v[0] += piece_tables[0][pc][to];
        v[1] -= piece_tables[1][pc][from];
        v[1] += piece_tables[1][pc][to];
    }

    CH_FORCEINLINE void update_piece_sq_cap(int cap, int from)
    {
        auto* v = stack().piece_sq;
        v[0] -= piece_tables[0][cap][from];
        v[1] -= piece_tables[1][cap][from];
    }

    CH_FORCEINLINE int best_case_move_value() const
    {
        int v = PIECE_VALUES[PAWN];
        color ec = opposite(current_turn);
        for(int pc = ec + QUEEN; ec > BLACK + PAWN; pc -= 2)
            if(bbs[pc]) { v = PIECE_VALUES[pc]; break; }
        uint64_t const pro_rank = current_turn == WHITE ? RANK7 : RANK2;
        if(bbs[current_turn + PAWN] & pro_rank)
            v += PIECE_VALUES[QUEEN] - PIECE_VALUES[PAWN];
        return v;
    }

    color current_turn;

    void new_game();
    void load_fen(char const* fen);

    template<acceleration accel>
    void do_move(move const& mv);

    template<acceleration accel>
    void undo_move(move const& mv);

    void do_null_move();
    void undo_null_move();

    CH_FORCEINLINE bool move_is_capture(move mv) const
    {
        return pieces[mv.to()] != EMPTY;
    }
    CH_FORCEINLINE bool move_is_promotion_or_capture(move mv) const
    {
        return move_is_capture(mv) || mv.is_promotion();
    }
    CH_FORCEINLINE bool move_is_geq_capture(move mv) const
    {
        int cap = pieces[mv.to()];
        if(cap == EMPTY) return false;
        int pc = pieces[mv.from()];
        return (cap & ~1) >= (pc & ~1);
    }
    CH_FORCEINLINE bool move_is_promotion_or_geq_capture(move mv) const
    {
        return mv.is_promotion() || move_is_geq_capture(mv);
    }

    CH_FORCEINLINE bool move_is_check(move mv) const
    {
        // TODO
        int const ek = lsb<ACCEL_UNACCEL>(bbs[opposite(current_turn) + KING]);
        uint64_t const d = (1ull << mv.to());
        uint64_t const occ = bb_alls[WHITE] | bb_alls[BLACK];
        switch(pieces[mv.from()])
        {
        case WHITE + PAWN:
            return (masks[ek].pawn_attacks[BLACK] & d) != 0;
        case BLACK + PAWN:
            return (masks[ek].pawn_attacks[WHITE] & d) != 0;
        case WHITE + KNIGHT:
        case BLACK + KNIGHT:
            return (masks[ek].knight_attacks & d) != 0;
        case WHITE + BISHOP:
        case BLACK + BISHOP:
            return (magic_bishop_attacks(ek, occ) & d) != 0;
        case WHITE + ROOK:
        case BLACK + ROOK:
            return (magic_rook_attacks(ek, occ) & d) != 0;
        case WHITE + QUEEN:
        case BLACK + QUEEN:
            return ((magic_bishop_attacks(ek, occ) |
                magic_rook_attacks(ek, occ)) & d) != 0;
        default:
            break;
        }
        return false;
    }

    template<color c>
    CH_FORCEINLINE bool is_draw_by_insufficient_material_helper() const
    {
        int mv = stack().piece_vals[c];
        int ev = stack().piece_vals[opposite(c)];
        if(mv == 0)
        {
            if(ev < PIECE_VALUES[ROOK])
                return true;
        }
        return false;
    }

    CH_FORCEINLINE bool is_draw_by_fifty_move_rule() const
    {
        return ply >= stack().ply_irreversible + 100;
    }

    CH_FORCEINLINE bool is_draw_by_insufficient_material() const
    {
        if((bbs[WHITE + PAWN] | bbs[BLACK + PAWN]) != 0)
            return false;
        if(stack().piece_vals[WHITE] <= PIECE_VALUES[BISHOP] &&
            stack().piece_vals[WHITE] == stack().piece_vals[BLACK])
            return true;
        return
            is_draw_by_insufficient_material_helper<WHITE>() ||
            is_draw_by_insufficient_material_helper<BLACK>();
    }

    CH_FORCEINLINE int mate_by_material(color c) const
    {
        color ec = opposite(c);
        if(stack().piece_vals[c] == 0)
        {
            if(bbs[ec + ROOK]) return -1;
            if(bbs[ec + QUEEN]) return -1;
        }
        if(stack().piece_vals[ec] == 0)
        {
            if(bbs[c + ROOK]) return 1;
            if(bbs[c + QUEEN]) return 1;
        }
        return 0;
    }

    CH_FORCEINLINE bool has_piece_better_than_pawn(color c) const
    {
        return (
            bbs[c + KNIGHT] |
            bbs[c + BISHOP] |
            bbs[c + ROOK] |
            bbs[c + QUEEN]
            ) != 0;
    }

#if CH_COLOR_TEMPLATE
    template<color c, acceleration accel>
    uint64_t perft(trans_table& tt, int depth);
#else
    template<acceleration accel>
    uint64_t perft(color c, trans_table& tt, int depth);
#endif

    template<acceleration accel>
    uint64_t root_perft(trans_table& tt, int depth, uint64_t* counts);
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
    stack().prev_move = NULL_MOVE;
    stack().ply_irreversible = 0;
    stack().piece_sq[0] = stack().piece_sq[1] = 0;
    stack().piece_vals[0] = stack().piece_vals[1] = 0;
    uint64_t& hash = stack().hash;
    hash = 0ull;

    char c = 1;

    // pieces
    {
        uint64_t m = 1;
        int i = 0;
        while(' ' != (c = *fen++) && c)
        {
#define CH_PUT(p_) do { \
    bbs[p_] |= m; \
    pieces[i] = p_; \
    hash ^= hashes[p_][i]; \
    m <<= 1; ++i; \
    stack().piece_vals[(p_) & 1] += PIECE_VALUES[p_]; \
    stack().piece_sq[0] += piece_tables[0][p_][i]; \
    stack().piece_sq[1] += piece_tables[1][p_][i]; \
    } while(0)
            switch(c)
            {
            case 'p': CH_PUT(BLACK + PAWN);   break;
            case 'n': CH_PUT(BLACK + KNIGHT); break;
            case 'b': CH_PUT(BLACK + BISHOP); break;
            case 'r': CH_PUT(BLACK + ROOK);   break;
            case 'q': CH_PUT(BLACK + QUEEN);  break;
            case 'k': CH_PUT(BLACK + KING);   break;
            case 'P': CH_PUT(WHITE + PAWN);   break;
            case 'N': CH_PUT(WHITE + KNIGHT); break;
            case 'B': CH_PUT(WHITE + BISHOP); break;
            case 'R': CH_PUT(WHITE + ROOK);   break;
            case 'Q': CH_PUT(WHITE + QUEEN);  break;
            case 'K': CH_PUT(WHITE + KING);   break;
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8':
                for(int j = 0; j < int(c - '0'); ++j)
                    CH_PUT(EMPTY);
                break;
#undef CH_PUT
            case '/':
            default:
                break;
            }
        }
    }

    // current turn
    current_turn = WHITE;
    while(' ' != (c = *fen++) && c)
    {
        switch(c)
        {
        case 'w': case 'W': current_turn = WHITE; break;
        case 'b': case 'B': current_turn = BLACK; hash ^= hash_turn; break;
        default:
            break;
        }
    }

    // castling rights
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
    hash ^= hash_castling_rights[stack().castling_rights];

    // en passant target square
    {
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
        {
            stack().ep_sq = uint8_t((8 - ep_rank) * 8 + ep_file);
            hash ^= hash_enp[stack().ep_sq & 7];
        }
    }

    age = 0;
    ply = 0;

    {
        auto& v = stack().piece_vals;
        v[0] = v[1] = 0;
        for(int i : { WHITE, BLACK })
            for(int p : { PAWN, KNIGHT, BISHOP, ROOK, QUEEN })
                v[i] += int16_t(popcnt<ACCEL_UNACCEL>(bbs[i + p]) * PIECE_VALUES[i + p]);
    }
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

void position::do_null_move()
{
    auto& st = stack_push();

    hash_history[ply] = uint32_t(st.hash);
    move_history[ply] = NULL_MOVE;
    ++ply;

    st.cap_piece = EMPTY;
    st.prev_move = NULL_MOVE;
    st.hash ^= hash_turn;
    st.ply_irreversible = uint16_t(ply);
    if(st.ep_sq)
    {
        st.hash ^= hash_enp[st.ep_sq & 7];
        st.ep_sq = 0;
    }
    current_turn = opposite(current_turn);
}

void position::undo_null_move()
{
    stack_pop();
    current_turn = opposite(current_turn);
    --ply;
}

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
    st.cap_piece = uint8_t(cap);
    st.prev_move = mv;

    hash_history[ply] = uint32_t(st.hash);
    move_history[ply] = mv;
    ++ply;

    auto& my_vals = st.piece_vals[p & 1];
    auto& enemy_vals = st.piece_vals[~p & 1];

    {
        uint16_t diff_castling_rights = st.castling_rights;
        st.castling_rights |=
            CASTLING_SPOILERS[a] | CASTLING_SPOILERS[b];
        diff_castling_rights ^= st.castling_rights;
        st.hash ^= hash_castling_rights[diff_castling_rights];
    }

    current_turn = opposite(current_turn);
    st.hash ^= hash_turn;

    if(st.ep_sq)
    {
        st.hash ^= hash_enp[st.ep_sq & 7];
        st.ep_sq = 0;
    }
    
    if(mv.is_special())
    {
        if(mv.is_pawn_dmove())
        {
            st.hash ^= hash_enp[b & 7];
            st.ep_sq = uint8_t(b);
        }
        else if(mv.is_castleq())
        {
            int rook = p + ROOK - KING;
            bbs[rook] ^= (0x9ull << (a - 4));
            pieces[a - 1] = uint8_t(rook);
            pieces[a - 4] = EMPTY;
            st.hash ^= (hashes[rook][a - 1] ^ hashes[rook][a - 4]);
            update_piece_sq(rook, a - 4, a - 1);
        }
        else if(mv.is_castlek())
        {
            int rook = p + ROOK - KING;
            bbs[rook] ^= (0xAull << a);
            pieces[a + 1] = uint8_t(rook);
            pieces[a + 3] = EMPTY;
            st.hash ^= (hashes[rook][a + 1] ^ hashes[rook][a + 3]);
            update_piece_sq(rook, a + 3, a + 1);
        }
        else if(mv.is_en_passant())
        {
            int sq = mv.en_passant_sq();
            st.cap_piece = pieces[sq];
            bbs[pieces[sq]] ^= (1ull << sq);
            pieces[sq] = EMPTY;
            st.hash ^= hashes[st.cap_piece][sq];
            enemy_vals -= PIECE_VALUES[PAWN];
            st.piece_sq[0] -= piece_tables[0][st.cap_piece][sq];
            st.piece_sq[1] -= piece_tables[1][st.cap_piece][sq];
        }
        else if(mv.is_promotion())
        {
            int t = mv.promotion_piece();
            bbs[cap] ^= cap_bb;
            bbs[p] ^= (1ull << a);
            bbs[t] ^= cap_bb;
            pieces[a] = EMPTY;
            pieces[b] = uint8_t(t);
            st.hash ^= (
                hashes[p][a] ^
                hashes[t][b] ^
                hashes[cap][b]
                );
            my_vals += PIECE_VALUES[t] - PIECE_VALUES[PAWN];
            enemy_vals -= PIECE_VALUES[cap];

            st.piece_sq[0] -= piece_tables[0][p][a];
            st.piece_sq[0] += piece_tables[0][t][b];
            st.piece_sq[0] -= piece_tables[0][cap][b];
            st.piece_sq[1] -= piece_tables[1][p][a];
            st.piece_sq[1] += piece_tables[1][t][b];
            st.piece_sq[1] -= piece_tables[1][cap][b];

            st.ply_irreversible = uint16_t(ply);
            return;
        }
    }

    bbs[cap] ^= cap_bb;
    bbs[p] ^= p_bb;
    pieces[a] = EMPTY;
    pieces[b] = uint8_t(p);

    enemy_vals -= PIECE_VALUES[cap];
    update_piece_sq(p, a, b);
    update_piece_sq_cap(cap, b);

    // pawn move or capture
    if(cap != EMPTY || p < KNIGHT)
        st.ply_irreversible = uint16_t(ply);

    st.hash ^= (
        hashes[p][a] ^
        hashes[p][b] ^
        hashes[cap][b]
        );
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

    --ply;

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
            int sq = mv.en_passant_sq();
            pieces[sq] = uint8_t(cap);
            bbs[pieces[sq]] ^= (1ull << sq);
            cap = EMPTY;
        }
        else if(mv.is_promotion())
        {
            int t = mv.promotion_piece();
            p = (is_black(p) ? BLACK : WHITE) + PAWN;
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
uint64_t position::perft(trans_table& tt, int depth)
#else
template<acceleration accel>
uint64_t position::perft(color c, trans_table& tt, int depth)
#endif
{
    move_list mvs;

#if CH_ENABLE_HASH_PERFT
    // transposition table lookup
    {
        hash_info_perft i;
        if(tt.get(stack().hash, i))
            if(i.depth == depth)
                return i.count;
    }
#endif

#if CH_COLOR_TEMPLATE
    mvs.generate<c, accel>(*this);
#else
    mvs.generate<accel>(c, *this);
#endif
    if(depth <= 1)
        return mvs.size();

    uint64_t r = 0;
    for(move mv : mvs)
    {
        do_move<accel>(mv);
#if CH_COLOR_TEMPLATE
        r += perft<opposite(c), accel>(tt, depth - 1);
#else
        r += perft<accel>(opposite(c), tt, depth - 1);
#endif
        undo_move<accel>(mv);
    }

#if CH_ENABLE_HASH_PERFT
    // transposition table store
    {
        hash_info_perft i;
        i.depth = depth;
        i.count = uint32_t(r);
        tt.put(stack().hash, i);
    }
#endif

    return r;
}

template<acceleration accel>
uint64_t position::root_perft(trans_table& tt, int depth, uint64_t* counts)
{
    uint64_t total = 0;
    move_list mvs;

    mvs.generate<accel>(current_turn, *this);

    for(move mv : mvs)
    {
        uint64_t count = 1;
        do_move<accel>(mv);
        if(depth > 1)
        {
#if CH_COLOR_TEMPLATE
            if(current_turn == WHITE)
                count = perft<WHITE, accel>(tt, depth - 1);
            else
                count = perft<BLACK, accel>(tt, depth - 1);
#else
            count = perft<accel>(current_turn, tt, depth - 1);
#endif
        }
        undo_move<accel>(mv);
        total += count;
        *counts++ = count;
    }

    return total;
}

}
