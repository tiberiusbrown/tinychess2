#pragma once

#include <stdint.h>
#include <limits.h>

#ifdef _MSC_VER
#define CHAPI __cdecl
#else
#define CHAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef uint32_t ch_move;

#define CH_MATE_SCORE 31000
#define CH_MATED_SCORE -31000

    typedef struct ch_system_info
    {
        // allocate memory
        void*(*alloc)(uint32_t bytes);

        // deallocate memory
        void(*dealloc)(void* p);

        // return monotonic millisecond clock
        uint32_t(*get_ms)(void);

        // sleep this thread briefly or yield to scheduler
        void(*thread_yield)(void);

        // sent periodically during search
        void(*search_info)(
            int depth,
            int seldepth,
            uint64_t nodes,
            int mstime,
            int score,
            uint64_t nps,
            ch_move* pv,
            int pvlen
            );

        // callback for when a search has ended
        void(*best_move)(ch_move m);
    } ch_system_info;

    typedef struct ch_search_limits
    {
        // hard limits
        uint64_t nodes;
        int depth;   // stop at a given depth in plies
        int mstime;  // stop after a given number of milliseconds

        // TODO: if remtime is nonzero, time management will be used
        int remtime[2];
        int inctime[2];

        int mate_search; // disallow reductions
    } ch_search_limits;

    typedef struct ch_game ch_game;

    void CHAPI ch_init(ch_system_info const* info);

    ch_game* CHAPI ch_create();
    void CHAPI ch_destroy(ch_game* g);

    // if this method is not called, searches will not be run in
    // the background. create as many threads as desired with this
    // method as the thread entry.
    void CHAPI ch_thread_start(ch_game* g);
    
    // kill all threads. this can be used to change the number of
    // threads: first kill threads, then create new ones with
    // ch_thread_start entries.
    void CHAPI ch_kill_threads(ch_game* g);

    // mem must be at least 8-byte aligned
    void CHAPI ch_set_hash(ch_game* g, void* mem, int size_megabyte_log2);

    // clear any hash tables and heuristics (ucinewgame)
    void CHAPI ch_clear_caches(ch_game* g);

    void CHAPI ch_new_game(ch_game* g);
    void CHAPI ch_load_fen(ch_game* g, char const* fen);

    void CHAPI ch_do_move(ch_game* g, ch_move m);
    // extended algebraic, e.g. e7f8q
    void CHAPI ch_do_move_str(ch_game* g, char const* str);

    ch_move CHAPI ch_last_move(ch_game* g);
    // returns the move undone, or 0 if no moves to undo
    ch_move CHAPI ch_undo_move(ch_game* g);
    // returns the move redone, or 0 if no moves to redo
    ch_move CHAPI ch_redo_move(ch_game* g);

    int CHAPI ch_move_fr_sq(ch_move mv);
    int CHAPI ch_move_to_sq(ch_move mv);

    int CHAPI ch_num_moves(ch_game* g);
    ch_move CHAPI ch_get_move(ch_game* g, int n);

    int CHAPI ch_evaluate(ch_game* g);
    int CHAPI ch_evaluate_white(ch_game* g);

    // perform qsearch and return score from white's perspective
    int CHAPI ch_qsearch(ch_game* g);

    void CHAPI ch_search(ch_game* g, ch_search_limits const* limits);

    void CHAPI ch_stop(ch_game* g);

    // get node count from last search
    uint64_t CHAPI ch_get_nodes(ch_game* g);

    // convert move to string
    char const* CHAPI ch_extended_algebraic(ch_move m);

    //
    // game state retrieval
    //

    enum
    {
        CH_WP, CH_BP,
        CH_WN, CH_BN,
        CH_WB, CH_BB,
        CH_WR, CH_BR,
        CH_WQ, CH_BQ,
        CH_WK, CH_BK,
        CH_EMPTY
    };
    // A8: sq = 0
    // A1: sq = 54
    // H8: sq = 7
    // H1: sq = 63
    int CHAPI ch_get_piece_at(ch_game* g, int sq);

    // draw types
    enum
    {
        CH_DRAW_NONE,
        CH_DRAW_FIFTY_MOVE,
        CH_DRAW_REPETITION,
        CH_DRAW_MATERIAL,
        CH_DRAW_STALEMATE,
    };
    int CHAPI ch_is_draw(ch_game* g);

    // whether the side to move is in check
    int CHAPI ch_is_check(ch_game* g);

    // whether the side to move is in checkmate
    int CHAPI ch_is_checkmate(ch_game* g);

    // CH_WHITE (0) or CH_BLACK (1)
    int CHAPI ch_current_turn(ch_game* g);

    //
    // debug methods
    //

    uint64_t CHAPI ch_perft(ch_game* g, int depth, uint64_t counts[256]);
    int CHAPI ch_see(ch_game* g, char const* mvstr);

#ifdef __cplusplus
}
#endif
