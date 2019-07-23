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
        STAGE_QUIETS,
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

    move get()
    {
        switch(stage)
        {
        case STAGE_HASH:
            if(index < num)
                return mvs[index++];
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
                    std::swap(mvs[i], mvs[--num]);
            }
            std::sort(
                mvs.begin() + knum,
                mvs.begin() + num,
                move_list::sort_descending());
            index = knum;
            stage = STAGE_WINNING_CAPTURES;
            // fallthrough

        case STAGE_WINNING_CAPTURES:
            if(index < num)
                return mvs[index++];

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
                return mvs[index++];

            // group all quiets
            knum = num;
            for(int i = num; i < mvs.size(); ++i)
            {
                if(!p.move_is_capture(mvs[i]))
                    std::swap(mvs[num++], mvs[i]);
            }
#if CH_ENABLE_HISTORY_HEURISTIC
            if(depth > 1) // don't do expensive sorting at tail nodes
            {
                // sort quiets by history heuristics
                for(int i = knum; i < num; ++i)
                    mvs[i].sort_key() = uint8_t(hh->get_hh_score(mvs[i]));
                std::sort(
                    mvs.begin() + knum,
                    mvs.begin() + num,
                    move_list::sort_descending());
            }
#endif
            index = knum;
            stage = STAGE_QUIETS;
            // fallthrough

        case STAGE_QUIETS:
            if(index < num)
                return mvs[index++];
            stage = STAGE_LOSING_CAPTURES;
            // fallthrough

        case STAGE_LOSING_CAPTURES:
            if(index < mvs.size())
                return mvs[index++];
            stage = STAGE_DONE;
            // fallthrough

        case STAGE_DONE:
        default:
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
