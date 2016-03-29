#include "populate_traps.hpp"

#include <algorithm>

#include "init.hpp"
#include "map.hpp"
#include "map_gen.hpp"
#include "map_parsing.hpp"
#include "feature_data.hpp"
#include "feature_trap.hpp"
#include "game_time.hpp"

namespace populate_traps
{

namespace
{

Trap* mk_trap(const Trap_id id, const P& pos)
{
    const auto* const   f       = map::cells[pos.x][pos.y].rigid;
    const auto&         d       = feature_data::data(f->id());
    const auto* const   mimic   = static_cast<Rigid*>(d.mk_obj(pos));

    if (!f->can_have_rigid())
    {
        TRACE << "Cannot place trap on feature id: " << int(f->id()) << std::endl
              << "Trap id: " << int(id) << std::endl;
        ASSERT(false);
        return nullptr;
    }

    Trap* const trap = new Trap(pos, mimic, id);

    return trap;
}

} //namespace

void populate_std_lvl()
{
    TRACE_FUNC_BEGIN;

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    //Put traps in non-plain rooms
    for (Room* const room : map::room_list)
    {
        const Room_type type = room->type_;

        if (type != Room_type::plain)
        {
            Fraction chance_for_trapped_room(-1, -1);

            switch (type)
            {
            case Room_type::human:
                chance_for_trapped_room.set(1, 3);
                break;

            case Room_type::ritual:
                chance_for_trapped_room.set(1, 4);
                break;

            case Room_type::spider:
                chance_for_trapped_room.set(2, 3);
                break;

            case Room_type::crypt:
                chance_for_trapped_room.set(3, 4);
                break;

            case Room_type::monster:
                chance_for_trapped_room.set(1, 4);
                break;

            case Room_type::chasm:
                chance_for_trapped_room.set(1, 4);
                break;

            case Room_type::snake_pit:
            case Room_type::forest:
            case Room_type::plain:
            case Room_type::flooded:
            case Room_type::muddy:
            case Room_type::cave:
            case Room_type::END_OF_STD_ROOMS:
            case Room_type::river:
            case Room_type::corr_link:
            case Room_type::crumble_room:
                break;
            }

            if (chance_for_trapped_room.num != -1 && chance_for_trapped_room.roll())
            {
                TRACE_VERBOSE << "Trapping non-plain room" << std::endl;

                std::vector<P> trap_pos_bucket;

                const P& p0 = room->r_.p0;
                const P& p1 = room->r_.p1;

                for (int y = p0.y; y <= p1.y; ++y)
                {
                    for (int x = p0.x; x <= p1.x; ++x)
                    {
                        if (!blocked[x][y] && map::cells[x][y].rigid->can_have_rigid())
                        {
                            trap_pos_bucket.push_back(P(x, y));
                        }
                    }
                }

                const bool  IS_SPIDER_ROOM  = type == Room_type::spider;

                const int   NR_BASE_TRAPS   = std::min(int(trap_pos_bucket.size()) / 2,
                                                       (IS_SPIDER_ROOM ? 3 : 1));

                for (int i = 0; i < NR_BASE_TRAPS; ++i)
                {
                    if (trap_pos_bucket.empty())
                    {
                        break;
                    }

                    const Trap_id trap_type = IS_SPIDER_ROOM ?
                                              Trap_id::web :
                                              Trap_id::any;

                    const int ELEMENT   = rnd::range(0, trap_pos_bucket.size() - 1);
                    const P pos       = trap_pos_bucket[ELEMENT];

                    blocked[pos.x][pos.y] = true;

                    trap_pos_bucket.erase(trap_pos_bucket.begin() + ELEMENT);

                    TRACE_VERBOSE << "Placing base trap" << std::endl;
                    Trap* const base_trap = mk_trap(trap_type, pos);

                    if (base_trap->valid())
                    {
                        map::put(base_trap);

                        //Spawn up to N traps in nearest cells (not necessarily adjacent)
                        Is_closer_to_pos sorter(pos);

                        std::sort(trap_pos_bucket.begin(), trap_pos_bucket.end(), sorter);

                        //NOTE: Trap type may have been randomized by the trap.
                        //We retrieve the actual trap resulting id here:
                        const Trap_id base_trap_type = base_trap->trap_type();

                        const int NR_ADJ = base_trap_type == Trap_id::web ? 0 :
                                           std::min(rnd::range(0, 2), int(trap_pos_bucket.size()));

                        TRACE_VERBOSE << "Placing adjacent traps" << std::endl;

                        for (int i_adj = 0; i_adj < NR_ADJ; ++i_adj)
                        {
                            //Make additional traps with the same id as the original trap
                            const P adj_pos = trap_pos_bucket.front();

                            blocked[adj_pos.x][adj_pos.y] = true;

                            trap_pos_bucket.erase(trap_pos_bucket.begin());

                            Trap* trap = mk_trap(base_trap_type, adj_pos);

                            if (trap->valid())
                            {
                                map::put(trap);
                            }
                            else //Adjacent trap invalid
                            {
                                delete trap;
                            }
                        }
                    }
                    else //Trap invalid
                    {
                        delete base_trap;
                    }
                }
            }
        }
    }

    const int CHANCE_ALLOW_TRAPPED_PLAIN_AREAS = std::min(85, 10 + ((map::dlvl - 1) * 8));

    if (rnd::percent(CHANCE_ALLOW_TRAPPED_PLAIN_AREAS))
    {
        TRACE_VERBOSE << "Trapping plain room" << std::endl;

        std::vector<P> trap_pos_bucket;

        for (int x = 1; x < MAP_W - 1; ++x)
        {
            for (int y = 1; y < MAP_H - 1; ++y)
            {
                if (map::room_map[x][y])
                {
                    if (
                        !blocked[x][y]                                  &&
                        map::room_map[x][y]->type_ == Room_type::plain  &&
                        map::cells[x][y].rigid->can_have_rigid())
                    {
                        trap_pos_bucket.push_back(P(x, y));
                    }
                }
            }
        }

        const int NR_BASE_TRAPS = std::min(int(trap_pos_bucket.size()) / 2, rnd::range(1, 3));

        for (int i = 0; i < NR_BASE_TRAPS; ++i)
        {
            if (trap_pos_bucket.empty())
            {
                break;
            }

            const int ELEMENT = rnd::range(0, trap_pos_bucket.size() - 1);

            const P pos = trap_pos_bucket[ELEMENT];

            TRACE_VERBOSE << "Placing base trap" << std::endl;

            trap_pos_bucket.erase(trap_pos_bucket.begin() + ELEMENT);

            blocked[pos.x][pos.y] = true;

            Trap* const base_trap = mk_trap(Trap_id::any, pos);

            if (base_trap->valid())
            {
                map::put(base_trap);

                //Spawn up to N traps in nearest cells (not necessarily adjacent)
                Is_closer_to_pos sorter(pos);

                std::sort(trap_pos_bucket.begin(), trap_pos_bucket.end(), sorter);

                //NOTE: Trap type may have been randomized by the trap.
                //We retrieve the actual trap resulting id here:
                const Trap_id base_trap_type = base_trap->trap_type();

                const int NR_ADJ = base_trap_type == Trap_id::web ? 0 :
                                   std::min(rnd::range(0, 2), int(trap_pos_bucket.size()));

                TRACE_VERBOSE << "Placing adjacent traps" << std::endl;

                for (int i_adj = 0; i_adj < NR_ADJ; ++i_adj)
                {
                    //Make additional traps with the same id as the original trap
                    const P adj_pos = trap_pos_bucket.front();

                    blocked[adj_pos.x][adj_pos.y] = true;

                    trap_pos_bucket.erase(trap_pos_bucket.begin());

                    Trap* trap = mk_trap(base_trap_type, adj_pos);

                    if (trap->valid())
                    {
                        map::put(trap);
                    }
                    else //Trap invalid
                    {
                        delete trap;
                    }
                }
            }
            else //Trap invalid
            {
                delete base_trap;
            }
        }
    }

    TRACE_FUNC_END;
}

} //Populate_traps
