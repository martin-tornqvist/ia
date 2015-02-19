#include "populate_traps.hpp"

#include <algorithm>

#include "init.hpp"
#include "map.hpp"
#include "map_gen.hpp"
#include "map_parsing.hpp"
#include "utils.hpp"
#include "feature_data.hpp"
#include "feature_trap.hpp"

using namespace std;

namespace populate_traps
{

namespace
{

void mk_trap_at(const Trap_id id, const Pos& pos)
{
    const auto* const f     = Map::cells[pos.x][pos.y].rigid;
    const auto&       d     = Feature_data::get_data(f->get_id());
    const auto* const mimic = static_cast<Rigid*>(d.mk_obj(pos));
    Map::put(new Trap(pos, mimic, id));
}

} //namespace

void populate_std_lvl()
{
    TRACE_FUNC_BEGIN;

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    //Put traps in non-plain rooms
    for (Room* const room : Map::room_list)
    {
        const Room_type type = room->type_;

        if (type != Room_type::plain)
        {
            Fraction chance_for_trapped_room(0, 0);

            switch (type)
            {
            case Room_type::human:   chance_for_trapped_room.set(1, 4); break;
            case Room_type::ritual:  chance_for_trapped_room.set(1, 4); break;
            case Room_type::spider:  chance_for_trapped_room.set(2, 3); break;
            case Room_type::crypt:   chance_for_trapped_room.set(3, 4); break;
            case Room_type::monster: chance_for_trapped_room.set(1, 4); break;
            case Room_type::chasm:   chance_for_trapped_room.set(1, 4); break;
            case Room_type::forest:
            case Room_type::plain:
            case Room_type::flooded:
            case Room_type::muddy:
            case Room_type::cave:
            case Room_type::END_OF_STD_ROOMS:
            case Room_type::river:
            case Room_type::corr_link:
            case Room_type::crumble_room: {} break;
            }

            if (Rnd::fraction(chance_for_trapped_room))
            {
                TRACE_VERBOSE << "Trapping non-plain room" << endl;

                vector<Pos> trap_pos_bucket;

                const Pos& p0 = room->r_.p0;
                const Pos& p1 = room->r_.p1;
                for (int y = p0.y; y <= p1.y; ++y)
                {
                    for (int x = p0.x; x <= p1.x; ++x)
                    {
                        if (!blocked[x][y] && Map::cells[x][y].rigid->can_have_rigid())
                        {
                            trap_pos_bucket.push_back(Pos(x, y));
                        }
                    }
                }

                int         nr_pos_cand       = int(trap_pos_bucket.size());
                const bool  IS_SPIDER_ROOM  = type == Room_type::spider;
                const int   NR_BASE_TRAPS   = min(nr_pos_cand / 2, IS_SPIDER_ROOM ? 3 : 1);

                for (int i = 0; i < NR_BASE_TRAPS; ++i)
                {
                    if (nr_pos_cand == 0)
                    {
                        break;
                    }

                    const Trap_id trap_type = IS_SPIDER_ROOM ?
                                            Trap_id::web :
                                            Trap_id(Rnd::range(0, int(Trap_id::END) - 1));

                    const int ELEMENT = Rnd::range(0, trap_pos_bucket.size() - 1);
                    const Pos& pos    = trap_pos_bucket[ELEMENT];

                    TRACE_VERBOSE << "Placing base trap" << endl;
                    mk_trap_at(trap_type, pos);
                    blocked[pos.x][pos.y] = true;
                    trap_pos_bucket.erase(trap_pos_bucket.begin() + ELEMENT);
                    nr_pos_cand--;

                    //Spawn up to N traps in nearest cells (not necessarily adjacent)
                    Is_closer_to_pos sorter(pos);
                    sort(trap_pos_bucket.begin(), trap_pos_bucket.end(), sorter);
                    const int NR_ADJ = trap_type == Trap_id::web ? 0 :
                                       min(Rnd::range(0, 2), nr_pos_cand);
                    TRACE_VERBOSE << "Placing adjacent traps" << endl;
                    for (int i_adj = 0; i_adj < NR_ADJ; i_adj++)
                    {
                        const Pos& adj_pos = trap_pos_bucket.front();
                        mk_trap_at(trap_type, adj_pos);
                        blocked[adj_pos.x][adj_pos.y] = true;
                        trap_pos_bucket.erase(trap_pos_bucket.begin());
                        nr_pos_cand--;
                    }
                }
            }
        }
    }

    const int CHANCE_ALLOW_TRAPPED_PLAIN_AREAS = min(85, 30 + (Map::dlvl * 5));
    if (Rnd::percent() < CHANCE_ALLOW_TRAPPED_PLAIN_AREAS)
    {
        TRACE_VERBOSE << "Trapping plain room" << endl;

        vector<Pos> trap_pos_bucket;
        for (int y = 1; y < MAP_H - 1; ++y)
        {
            for (int x = 1; x < MAP_W - 1; ++x)
            {
                if (Map::room_map[x][y])
                {
                    if (
                        !blocked[x][y]                                &&
                        Map::room_map[x][y]->type_ == Room_type::plain  &&
                        Map::cells[x][y].rigid->can_have_rigid())
                    {
                        trap_pos_bucket.push_back(Pos(x, y));
                    }
                }
            }
        }

        int         nr_pos_cand       = int(trap_pos_bucket.size());
        const int   NR_BASE_TRAPS   = min(nr_pos_cand / 2, Rnd::range(1, 3));

        for (int i = 0; i < NR_BASE_TRAPS; ++i)
        {
            if (nr_pos_cand == 0)
            {
                break;
            }

            const Trap_id trap_type = Trap_id(Rnd::range(0, int(Trap_id::END) - 1));

            const int ELEMENT = Rnd::range(0, trap_pos_bucket.size() - 1);
            const Pos& pos = trap_pos_bucket[ELEMENT];

            TRACE_VERBOSE << "Placing base trap" << endl;
            mk_trap_at(trap_type, pos);
            trap_pos_bucket.erase(trap_pos_bucket.begin() + ELEMENT);
            nr_pos_cand--;

            //Spawn up to N traps in nearest cells (not necessarily adjacent)
            Is_closer_to_pos sorter(pos);
            sort(trap_pos_bucket.begin(), trap_pos_bucket.end(), sorter);
            const int NR_ADJ = trap_type == Trap_id::web ? 0 :
                               min(Rnd::range(0, 2), nr_pos_cand);
            TRACE_VERBOSE << "Placing adjacent traps" << endl;
            for (int i_adj = 0; i_adj < NR_ADJ; i_adj++)
            {
                const Pos& adj_pos = trap_pos_bucket.front();
                mk_trap_at(trap_type, adj_pos);
                trap_pos_bucket.erase(trap_pos_bucket.begin());
                nr_pos_cand--;
            }
        }
    }
    TRACE_FUNC_END;
}

} //Populate_traps
