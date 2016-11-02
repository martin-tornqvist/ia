#include "populate_traps.hpp"

#include <algorithm>

#include "init.hpp"
#include "map.hpp"
#include "mapgen.hpp"
#include "map_parsing.hpp"
#include "feature_data.hpp"
#include "feature_trap.hpp"
#include "game_time.hpp"

namespace populate_traps
{

namespace
{

Trap* mk_trap(const TrapId id, const P& pos)
{
    const auto* const   f       = map::cells[pos.x][pos.y].rigid;
    const auto&         d       = feature_data::data(f->id());
    auto* const         mimic   = static_cast<Rigid*>(d.mk_obj(pos));

    if (!f->can_have_rigid())
    {
        TRACE << "Cannot place trap on feature id: "
              << (int)f->id() << std::endl
              << "Trap id: "
              << int(id) << std::endl;

        ASSERT(false);

        return nullptr;
    }

    Trap* const trap = new Trap(pos, mimic, id);

    return trap;
}

} // namespace

void populate_std_lvl()
{
    TRACE_FUNC_BEGIN;

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::BlocksMoveCmn(false), blocked);

    //Put traps in non-plain rooms
    for (Room* const room : map::room_list)
    {
        const RoomType type = room->type_;

        if (type != RoomType::plain)
        {
            Fraction chance_for_trapped_room(-1, -1);

            switch (type)
            {
            case RoomType::human:
                chance_for_trapped_room.set(1, 3);
                break;

            case RoomType::ritual:
                chance_for_trapped_room.set(1, 4);
                break;

            case RoomType::spider:
                chance_for_trapped_room.set(2, 3);
                break;

            case RoomType::crypt:
                chance_for_trapped_room.set(3, 4);
                break;

            case RoomType::monster:
                chance_for_trapped_room.set(1, 4);
                break;

            case RoomType::chasm:
                chance_for_trapped_room.set(1, 4);
                break;

            case RoomType::snake_pit:
            case RoomType::forest:
            case RoomType::plain:
            case RoomType::flooded:
            case RoomType::muddy:
            case RoomType::cave:
            case RoomType::END_OF_STD_ROOMS:
            case RoomType::template_room:
            case RoomType::river:
            case RoomType::corr_link:
            case RoomType::crumble_room:
                break;
            }

            if (chance_for_trapped_room.num != -1 &&
                chance_for_trapped_room.roll())
            {
                TRACE_VERBOSE << "Trapping non-plain room" << std::endl;

                std::vector<P> trap_pos_bucket;

                const P& p0 = room->r_.p0;
                const P& p1 = room->r_.p1;

                for (int y = p0.y; y <= p1.y; ++y)
                {
                    for (int x = p0.x; x <= p1.x; ++x)
                    {
                        if (!blocked[x][y] &&
                            map::cells[x][y].rigid->can_have_rigid())
                        {
                            trap_pos_bucket.push_back(P(x, y));
                        }
                    }
                }

                const bool is_spider_room = (type == RoomType::spider);

                const int nr_origin_traps =
                    std::min((int)trap_pos_bucket.size() / 2,
                             (is_spider_room ? 3 : 1));

                for (int i = 0; i < nr_origin_traps; ++i)
                {
                    if (trap_pos_bucket.empty())
                    {
                        break;
                    }

                    const TrapId trap_type =
                        is_spider_room ?
                        TrapId::web :
                        TrapId::any;

                    const int idx = rnd::range(0, trap_pos_bucket.size() - 1);

                    const P pos = trap_pos_bucket[idx];

                    blocked[pos.x][pos.y] = true;

                    trap_pos_bucket.erase(trap_pos_bucket.begin() + idx);

                    TRACE_VERBOSE << "Placing base trap" << std::endl;
                    Trap* const origin_trap = mk_trap(trap_type, pos);

                    if (origin_trap->valid())
                    {
                        map::put(origin_trap);

                        // Spawn up to n traps in nearest cells (not necessarily
                        // adjacent)
                        IsCloserToPos sorter(pos);

                        std::sort(trap_pos_bucket.begin(),
                                  trap_pos_bucket.end(),
                                  sorter);

                        //NOTE: Trap type may have been randomized by the trap.
                        //We retrieve the actual trap resulting id here:
                        const TrapId origin_trap_type = origin_trap->type();

                        const int nr_adj =
                            (origin_trap_type == TrapId::web) ?
                            0 :
                            std::min(rnd::range(0, 2),
                                     (int)trap_pos_bucket.size());

                        TRACE_VERBOSE << "Placing adjacent traps" << std::endl;

                        for (int i_adj = 0; i_adj < nr_adj; ++i_adj)
                        {
                            // Make additional traps with the same id as the
                            // original trap
                            const P adj_pos = trap_pos_bucket.front();

                            blocked[adj_pos.x][adj_pos.y] = true;

                            trap_pos_bucket.erase(trap_pos_bucket.begin());

                            Trap* extra_trap =
                                mk_trap(origin_trap_type, adj_pos);

                            if (extra_trap->valid())
                            {
                                map::put(extra_trap);
                            }
                            else //Extra trap invalid
                            {
                                delete extra_trap;
                            }
                        }
                    }
                    else //Origin trap invalid
                    {
                        delete origin_trap;
                    }
                }
            }
        }
    }

    const int chance_allow_trapped_plain_areas =
        std::min(85,
                 10 + ((map::dlvl - 1) * 8));

    if (rnd::percent(chance_allow_trapped_plain_areas))
    {
        TRACE_VERBOSE << "Trapping plain room" << std::endl;

        std::vector<P> trap_pos_bucket;

        for (int x = 1; x < map_w - 1; ++x)
        {
            for (int y = 1; y < map_h - 1; ++y)
            {
                if (map::room_map[x][y])
                {
                    if (!blocked[x][y] &&
                        map::room_map[x][y]->type_ == RoomType::plain &&
                        map::cells[x][y].rigid->can_have_rigid())
                    {
                        trap_pos_bucket.push_back(P(x, y));
                    }
                }
            }
        }

        const int nr_origin_traps =
            std::min((int)trap_pos_bucket.size() / 2,
                     rnd::range(1, 3));

        for (int i = 0; i < nr_origin_traps; ++i)
        {
            if (trap_pos_bucket.empty())
            {
                break;
            }

            const int element = rnd::range(0, trap_pos_bucket.size() - 1);

            const P pos = trap_pos_bucket[element];

            TRACE_VERBOSE << "Placing base trap" << std::endl;

            trap_pos_bucket.erase(trap_pos_bucket.begin() + element);

            blocked[pos.x][pos.y] = true;

            Trap* const origin_trap = mk_trap(TrapId::any, pos);

            if (origin_trap->valid())
            {
                map::put(origin_trap);

                // Spawn up to n traps in nearest cells (not necessarily
                // adjacent)
                IsCloserToPos sorter(pos);

                std::sort(trap_pos_bucket.begin(),
                          trap_pos_bucket.end(),
                          sorter);

                // NOTE: Trap type may have been randomized by the trap. We
                //       retrieve the actual trap resulting id here:
                const TrapId origin_trap_type = origin_trap->type();

                const int nr_adj =
                    (origin_trap_type == TrapId::web) ?
                    0 :
                    std::min(rnd::range(0, 2),
                             (int)trap_pos_bucket.size());

                TRACE_VERBOSE << "Placing adjacent traps" << std::endl;

                for (int i_adj = 0; i_adj < nr_adj; ++i_adj)
                {
                    // Make additional traps with the same id as the original
                    // trap
                    const P adj_pos = trap_pos_bucket.front();

                    blocked[adj_pos.x][adj_pos.y] = true;

                    trap_pos_bucket.erase(trap_pos_bucket.begin());

                    Trap* extra_trap = mk_trap(origin_trap_type, adj_pos);

                    if (extra_trap->valid())
                    {
                        map::put(extra_trap);
                    }
                    else // Extra trap invalid
                    {
                        delete extra_trap;
                    }
                }
            }
            else // Origin trap invalid
            {
                delete origin_trap;
            }
        }
    }

    TRACE_FUNC_END;
}

} // populate_traps
