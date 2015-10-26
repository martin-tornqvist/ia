#include "map_patterns.hpp"

#include "init.hpp"

#include <vector>

#include "map.hpp"
#include "feature_rigid.hpp"
#include "game_time.hpp"

namespace map_patterns
{

void cells_in_room(const Room& room, std::vector<P>& adj_to_walls,
                   std::vector<P>& away_from_walls)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    std::vector<P> pos_bucket;
    pos_bucket.clear();

    const Rect& r = room.r_;

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            if (map::room_map[x][y] == &room)
            {
                auto* const f = map::cells[x][y].rigid;

                if (f->can_move_cmn() && f->can_have_rigid())
                {
                    pos_bucket.push_back(P(x, y));
                }
            }
        }
    }

    adj_to_walls.clear();
    away_from_walls.clear();

    for (P& pos : pos_bucket)
    {
        const int NR_BLK_R = walk_blockers_in_dir(Dir::right, pos);
        const int NR_BLK_D = walk_blockers_in_dir(Dir::down, pos);
        const int NR_BLK_L = walk_blockers_in_dir(Dir::left, pos);
        const int NR_BLK_U = walk_blockers_in_dir(Dir::up, pos);

        const bool IS_ZERO_BLK_ALL_DIR =
            NR_BLK_R == 0 && NR_BLK_D == 0 && NR_BLK_L == 0 && NR_BLK_U == 0;

        if (IS_ZERO_BLK_ALL_DIR)
        {
            away_from_walls.push_back(pos);
            continue;
        }

        bool is_door_adjacent = false;

        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                const auto* const f = map::cells[pos.x + dx][pos.y + dy].rigid;

                if (f->id() == Feature_id::door) {is_door_adjacent = true;}
            }
        }

        if (is_door_adjacent) {continue;}

        if (
            (NR_BLK_R == 3 && NR_BLK_U == 1 && NR_BLK_D == 1 && NR_BLK_L == 0) ||
            (NR_BLK_R == 1 && NR_BLK_U == 3 && NR_BLK_D == 0 && NR_BLK_L == 1) ||
            (NR_BLK_R == 1 && NR_BLK_U == 0 && NR_BLK_D == 3 && NR_BLK_L == 1) ||
            (NR_BLK_R == 0 && NR_BLK_U == 1 && NR_BLK_D == 1 && NR_BLK_L == 3))
        {
            adj_to_walls.push_back(pos);
            continue;
        }

    }

    TRACE_FUNC_END_VERBOSE;
}

int walk_blockers_in_dir(const Dir dir, const P& pos)
{
    int nr_blockers = 0;

    switch (dir)
    {
    case Dir::right:
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const auto* const f = map::cells[pos.x + 1][pos.y + dy].rigid;

            if (!f->can_move_cmn()) {nr_blockers += 1;}
        }
    } break;

    case Dir::down:
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            const auto* const f = map::cells[pos.x + dx][pos.y + 1].rigid;

            if (!f->can_move_cmn()) {nr_blockers += 1;}
        }
    } break;

    case Dir::left:
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const auto* const f = map::cells[pos.x - 1][pos.y + dy].rigid;

            if (!f->can_move_cmn()) {nr_blockers += 1;}
        }
    } break;

    case Dir::up:
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            const auto* const f = map::cells[pos.x + dx][pos.y - 1].rigid;

            if (!f->can_move_cmn()) {nr_blockers += 1;}
        }
    } break;

    case Dir::down_left:
    case Dir::down_right:
    case Dir::up_left:
    case Dir::up_right:
    case Dir::center:
    case Dir::END:
        break;
    }

    return nr_blockers;
}

} //map_patterns
