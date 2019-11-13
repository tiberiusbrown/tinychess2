#pragma once

// controls move ordering

#include "ch_history_heuristic.hpp"
#include "ch_move_list.hpp"
#include "ch_position.hpp"
#include "ch_see.hpp"

#include <array>

namespace ch
{

template<acceleration accel>
struct move_picker
{

    enum
    {
        STAGE_HASH,
        STAGE_PREP_WINNING_CAPTURES,
        STAGE_WINNING_CAPTURES,
        STAGE_KILLERS,
        STAGE_COUNTERS,
        STAGE_QUIETS,
        STAGE_PREP_LOSING_CAPTURES,
        STAGE_LOSING_CAPTURES,
        STAGE_DONE
    } stage;

    move_picker(
        move_list& mvs_,
        position& p_,
        move hash_move_,
        std::array<move, CH_NUM_KILLERS> const& killers_,
        history_heuristic const* hh_,
        int depth_
        )
        : stage()
        , mvs(mvs_)
        , p(p_)
        , killers(killers_)
        , hh(hh_)
        , depth(depth_)
    {
        stage = STAGE_HASH;
        index = num = 0;
        for(int i = 0; i < mvs.size(); ++i)
            if(mvs[i].is_similar_to(hash_move_))
                std::swap(mvs[num++], mvs[i]);
        //num = 0;
        stage = STAGE_HASH;
    }

    move get(int& out_stage)
    {
        switch(stage)
        {
        case STAGE_HASH:
            if(index < num)
            {
                out_stage = STAGE_HASH;
                return mvs[index++];
            }
            // fallthrough

        case STAGE_PREP_WINNING_CAPTURES:
            // group all non-losing captures
            knum = num;
            for(int i = knum; i < mvs.size(); ++i)
            {
                //if(p.move_is_promotion_or_geq_capture(mvs[i]))
                if(p.move_is_promotion_or_capture(mvs[i]))
                    std::swap(mvs[num++], mvs[i]);
            }
            // sort non-losing captures by SEE
            for(int i = knum; i < num; ++i)
            {
                int sv = see<accel>(mvs[i], p);
                if(sv >= 0)
                    mvs[i].sort_key() = uint8_t(sv);
                else
                {
                    mvs[i].sort_key() = uint8_t(sv + 128);
                    std::swap(mvs[i], mvs[--num]);
                }
            }
            insertion_sort(
                mvs.begin() + knum,
                mvs.begin() + num,
                move_list::sort_descending());
            index = knum;
            stage = STAGE_WINNING_CAPTURES;
            // fallthrough

        case STAGE_WINNING_CAPTURES:
            if(index < num)
            {
                out_stage = STAGE_WINNING_CAPTURES;
                return mvs[index++];
            }

            knum = num;
            for(int k = 0; k < CH_NUM_KILLERS; ++k)
            {
                if(killers[k] == NULL_MOVE) continue;
                for(int i = num; i < mvs.size(); ++i)
                    if(mvs[i] == killers[k])
                    {
                        std::swap(mvs[num++], mvs[i]);
                        break;
                    }
            }
            index = knum;
            // fallthrough

        case STAGE_KILLERS:
            if(index < num)
            {
                out_stage = STAGE_KILLERS;
                return mvs[index++];
            }

            // group all quiets
            knum = num;
            for(int i = num; i < mvs.size(); ++i)
            {
                if(!p.move_is_promotion_or_capture(mvs[i]))
                    std::swap(mvs[num++], mvs[i]);
            }
#if CH_ENABLE_HISTORY_HEURISTIC
            if(depth > 1) // don't do expensive sorting at tail nodes
            {
                // sort quiets by history heuristics
                for(int i = knum; i < num; ++i)
                    mvs[i].sort_key() = uint8_t(hh->get_hh_score(mvs[i]));
                insertion_sort(
                    mvs.begin() + knum,
                    mvs.begin() + num,
                    move_list::sort_descending());
            }
#endif
            index = knum;
            stage = STAGE_COUNTERS;
            // fallthrough

        case STAGE_COUNTERS:
#if 1
            stage = STAGE_QUIETS;
#else
        {
            move mv = hh->get_countermove(p.stack().prev_move);
            move counter = NULL_MOVE;
            for(int i = index; i < num; ++i)
            {
                if(mvs[i].is_similar_to(mv))
                {
                    counter = mvs[i];
                    mvs[i] = mvs[--num];
                    break;
                }
            }
            stage = STAGE_QUIETS;
            if(counter)
                return counter;
        }
#endif
            // fallthrough

        case STAGE_QUIETS:
            if(index < num)
            {
                out_stage = STAGE_QUIETS;
                return mvs[index++];
            }
            stage = STAGE_PREP_LOSING_CAPTURES;
            // fallthrough

        case STAGE_PREP_LOSING_CAPTURES:
            if(index >= mvs.size())
            {
                stage = STAGE_DONE;
                out_stage = STAGE_DONE;
                return NULL_MOVE;
            }
            insertion_sort(
                mvs.begin() + index,
                mvs.begin() + num,
                move_list::sort_descending());
            stage = STAGE_LOSING_CAPTURES;
            // fallthrough

        case STAGE_LOSING_CAPTURES:
            if(index < mvs.size())
            {
                out_stage = STAGE_LOSING_CAPTURES;
                return mvs[index++];
            }
            stage = STAGE_DONE;
            // fallthrough

        case STAGE_DONE:
        default:
            out_stage = STAGE_DONE;
            return NULL_MOVE;
        }
    }

    int num;
    int index;
    int knum;

    move_list& mvs;
    position& p;
    std::array<move, CH_NUM_KILLERS> const& killers;
    history_heuristic const* hh;
    int depth;
};

}
