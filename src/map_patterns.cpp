#include "map_patterns.hpp"

#include "init.hpp"

#include <vector>

#include "map.hpp"
#include "feature_rigid.hpp"
#include "game_time.hpp"

namespace map_patterns
{

namespace
{

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

            if (!f->can_move_common())
            {
                nr_blockers += 1;
            }
        }
    }
    break;

    case Dir::down:
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            const auto* const f = map::cells[pos.x + dx][pos.y + 1].rigid;

            if (!f->can_move_common())
            {
                nr_blockers += 1;
            }
        }
    }
    break;

    case Dir::left:
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const auto* const f = map::cells[pos.x - 1][pos.y + dy].rigid;

            if (!f->can_move_common())
            {
                nr_blockers += 1;
            }
        }
    }
    break;

    case Dir::up:
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            const auto* const f = map::cells[pos.x + dx][pos.y - 1].rigid;

            if (!f->can_move_common())
            {
                nr_blockers += 1;
            }
        }
    }
    break;

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

} //namespace

void cells_in_room(const Room& room,
                   std::vector<P>& adj_to_walls,
                   std::vector<P>& away_from_walls)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    std::vector<P> pos_bucket;

    pos_bucket.clear();

    const R& r = room.r_;

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            if (map::room_map[x][y] == &room)
            {
                auto* const f = map::cells[x][y].rigid;

                if (f->can_move_common() && f->can_have_rigid())
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
        const int nr_r = walk_blockers_in_dir(Dir::right, pos);
        const int nr_d = walk_blockers_in_dir(Dir::down, pos);
        const int nr_l = walk_blockers_in_dir(Dir::left, pos);
        const int nr_u = walk_blockers_in_dir(Dir::up, pos);

        const bool is_zero_all_dir =
            nr_r == 0 &&
            nr_d == 0 &&
            nr_l == 0 &&
            nr_u == 0;

        if (is_zero_all_dir)
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

                if (f->id() == FeatureId::door)
                {
                    is_door_adjacent = true;
                }
            }
        }

        if (is_door_adjacent)
        {
            continue;
        }

        if (
            (nr_r == 3 && nr_u == 1 && nr_d == 1 && nr_l == 0) ||
            (nr_r == 1 && nr_u == 3 && nr_d == 0 && nr_l == 1) ||
            (nr_r == 1 && nr_u == 0 && nr_d == 3 && nr_l == 1) ||
            (nr_r == 0 && nr_u == 1 && nr_d == 1 && nr_l == 3))
        {
            adj_to_walls.push_back(pos);
            continue;
        }

    }

    TRACE_FUNC_END_VERBOSE;
}

} //map_patterns
