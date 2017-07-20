#include "init.hpp"

#include "mapgen.hpp"

#include <vector>
#include <climits>

#include "init.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "map_templates.hpp"
#include "feature_rigid.hpp"
#include "game_time.hpp"
#include "feature_door.hpp"
#include "init.hpp"

namespace mapgen
{

bool is_map_valid = true;

bool is_all_rooms_connected()
{
    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCommon(ParseActors::no)
        .run(blocked);

    //
    // Do not consider doors blocking
    //
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (map::cells[x][y].rigid->id() == FeatureId::door)
            {
                blocked[x][y] = false;
            }
        }
    }

    return map_parsers::is_map_connected(blocked);
}

// Adds the room to the room list and the room map
void register_room(Room& room)
{
#ifndef NDEBUG
    // Check that the room has not already been added
    for (Room* const room_in_list : map::room_list)
    {
        ASSERT(room_in_list != &room);
    }
#endif // NDEBUG

    map::room_list.push_back(&room);

    const P room_p0(room.r_.p0);
    const P room_p1(room.r_.p1);

    ASSERT(map::is_pos_inside_map(room_p0, false));
    ASSERT(map::is_pos_inside_map(room_p1, false));

    for (int x = room_p0.x; x <= room_p1.x; ++x)
    {
        for (int y = room_p0.y; y <= room_p1.y; ++y)
        {
            map::room_map[x][y] = &room;
        }
    }
}

void mk_floor(const Room& room)
{
    for (int x = room.r_.p0.x; x <= room.r_.p1.x; ++x)
    {
        for (int y = room.r_.p0.y; y <= room.r_.p1.y; ++y)
        {
            map::put(new Floor(P(x, y)));
        }
    }
}

void cut_room_corners(const Room& room)
{
    // Never cut the room corners if it's a "small" room
    if (room.r_.min_dim() < 6)
    {
        return;
    }

    const P max_dims(room.r_.dims() - 4);

    const P room_p0(room.r_.p0);
    const P room_p1(room.r_.p1);

    // NOTE: The "cross" dimensons and coordinates refer to the inner rectangle
    //       of the plus shape.
    const P cross_dims(rnd::range(2, max_dims.x),
                       rnd::range(2, max_dims.y));

    const P cross_x0y0(rnd::range(room_p0.x + 2,
                                  room_p1.x - cross_dims.x - 1),
                       rnd::range(room_p0.y + 2,
                                  room_p1.y - cross_dims.y - 1));

    const P cross_x1y1(cross_x0y0 + cross_dims - 1);

    // Which corners to place (up-left, up-right, down-left, down-right)
    // By default, we cut all corners
    bool c[4] = {true, true, true, true};

    // Ocassionally cut only some corners
    if (rnd::fraction(2, 3))
    {
        while (true)
        {
            int nr_corners = 0;

            for (int i = 0; i < 4; ++i)
            {
                if (rnd::coin_toss())
                {
                    c[i] = true;
                    ++nr_corners;
                }
                else // Do not cut this corner
                {
                    c[i] = false;
                }
            }

            if (nr_corners > 0)
            {
                break;
            }
        }
    }

    for (size_t corner_idx = 0; corner_idx < 4; ++corner_idx)
    {
        // This corner has been marked for skipping?
        if (!c[corner_idx])
        {
            continue;
        }

        R r;

        switch (corner_idx)
        {
        // Up left
        case 0:
            r.p0 = room_p0;
            r.p1 = cross_x0y0 - 1;
            break;

        // Up right
        case 1:
            r.p0 = P(cross_x1y1.x + 1, room_p0.y);
            r.p1 = P(room_p1.x, cross_x0y0.y - 1);
            break;

        // Down left
        case 2:
            r.p0 = P(room_p0.x, cross_x1y1.y + 1);
            r.p1 = P(cross_x0y0.x - 1, room_p1.y);
            break;

        // Down right
        case 3:
            r.p0 = cross_x1y1 + 1;
            r.p1 = room_p1;
            break;
        }

        // Check if these positions can be cut
        bool allow_cut = true;

        for (int x = r.p0.x; x <= r.p1.x; ++x)
        {
            for (int y = r.p0.y; y <= r.p1.y; ++y)
            {
                for (const P& d : dir_utils::dir_list_w_center)
                {
                    const P p(x, y);

                    const P check_p(p + d);

                    const Room* const room_here =
                        map::room_map[check_p.x][check_p.y];

                    const FeatureId id =
                        map::cells[check_p.x][check_p.y].rigid->id();

                    if ((room_here == &room && id != FeatureId::floor) ||
                        (room_here != &room && id != FeatureId::wall)
                    )
                    {
                        allow_cut = false;
                        break;
                    }
                }

                if (!allow_cut)
                {
                    break;
                }
            }
        }

        if (allow_cut)
        {
            // OK, nothing is preventing us from building walls here
            for (int x = r.p0.x; x <= r.p1.x; ++x)
            {
                for (int y = r.p0.y; y <= r.p1.y; ++y)
                {
                    map::put(new Wall(P(x, y)));
                    map::room_map[x][y] = nullptr;
                }
            }
        }
    }
}

void mk_pillars_in_room(const Room& room)
{
    const P& room_p0(room.r_.p0);
    const P& room_p1(room.r_.p1);

    auto is_free = [](const P & p)
    {
        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P check_p(p + d);

            const FeatureId id = map::cells[check_p.x][check_p.y].rigid->id();

            if (id == FeatureId::wall)
            {
                return false;
            }
        }
        return true;
    };

    // Place pillars in rows and columns
    auto step_size = []()
    {
        return rnd::range(1, 3);
    };

    const int dx = step_size();
    const int dy = step_size();

    const int place_one_in_n = rnd::range(2, 3);

    for (int y = room_p0.y + 1; y <= room_p1.y - 1; y += dy)
    {
        for (int x = room_p0.x + 1; x <= room_p1.x - 1; x += dx)
        {
            const P p(x, y);

            if (is_free(p) && rnd::one_in(place_one_in_n))
            {
                map::put(new Wall(p));
            }
        }
    }
}

void cavify_room(Room& room)
{
    bool is_other_room[map_w][map_h];

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const auto* const room_here = map::room_map[x][y];

            is_other_room[x][y] = room_here && room_here != &room;
        }
    }

    bool blocked[map_w][map_h];

    map_parsers::expand(is_other_room, blocked);

    R& room_rect = room.r_;

    std::vector<P> origin_bucket;

    const auto& r = room.r_;

    const int X0 = r.p0.x + 1;
    const int Y0 = r.p0.y + 1;
    const int X1 = r.p1.x - 1;
    const int Y1 = r.p1.y - 1;

    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            // Add to origin bucket if we are on the edge
            if (x == X0 || x == X1 || y == Y0 || y == Y1)
            {
                origin_bucket.push_back({x, y});
            }
        }
    }

    for (const P& origin : origin_bucket)
    {
        if (blocked[origin.x][origin.y] ||
            map::room_map[origin.x][origin.y] != &room)
        {
            continue;
        }

        int flood[map_w][map_h];

        floodfill(origin,
                  blocked,
                  flood,
                  rnd::range(1, 4),
                  P( -1, -1),
                  false);

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                if (flood[x][y] > 0)
                {
                    map::put(new Floor({x, y}));

                    map::room_map[x][y] = &room;

                    if (x < room_rect.p0.x)
                    {
                        room_rect.p0.x = x;
                    }

                    if (y < room_rect.p0.y)
                    {
                        room_rect.p0.y = y;
                    }

                    if (x > room_rect.p1.x)
                    {
                        room_rect.p1.x = x;
                    }

                    if (y > room_rect.p1.y)
                    {
                        room_rect.p1.y = y;
                    }
                }
            }
        }
    }

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (map::room_map[x][y] == &room)
            {
                Rigid* const rigid = map::cells[x][y].rigid;

                if (rigid->id() == FeatureId::floor)
                {
                    static_cast<Floor*>(rigid)->type_ = FloorType::cave;
                }
            }
        }
    }
}

void valid_corridor_entries(const Room& room, std::vector<P>& out)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    // Find all cells that meets all of the following criteria:
    //  (1) Is a wall cell
    //  (2) Is a cell not belonging to any room
    //  (3) Is not on the edge of the map
    //  (4) Is cardinally adjacent to a floor cell belonging to the room
    //  (5) Is cardinally adjacent to a cell not in the room or room outline

    out.clear();

    bool room_cells[map_w][map_h];
    bool room_floor_cells[map_w][map_h];

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const bool is_room_cell =
                map::room_map[x][y] == &room;

            room_cells[x][y] = is_room_cell;

            const auto* const f = map::cells[x][y].rigid;

            room_floor_cells[x][y] =
                is_room_cell && f->id() == FeatureId::floor;
        }
    }

    bool room_cells_expanded[map_w][map_h];

    map_parsers::expand(room_cells,
                        room_cells_expanded,
                        R(P(room.r_.p0 - 2),
                          P(room.r_.p1 + 2)));

    for (int y = room.r_.p0.y - 1; y <= room.r_.p1.y + 1; ++y)
    {
        for (int x = room.r_.p0.x - 1; x <= room.r_.p1.x + 1; ++x)
        {
            // Condition (1)
            if (map::cells[x][y].rigid->id() != FeatureId::wall)
            {
                continue;
            }

            // Condition (2)
            if (map::room_map[x][y]) {continue;}

            // Condition (3)
            if (x <= 1 ||
                y <= 1 ||
                x >= (map_w - 2) ||
                y >= (map_h - 2))
            {
                continue;
            }

            bool is_adj_to_floor_in_room = false;
            bool is_adj_to_cell_outside = false;

            const P p(x, y);

            bool is_adj_to_floor_not_in_room = false;

            for (const P& d : dir_utils::cardinal_list)
            {
                const P& p_adj(p + d);

                // Condition (4)
                if (room_floor_cells[p_adj.x][p_adj.y])
                {
                    is_adj_to_floor_in_room = true;
                }

                // Condition (5)
                if (!room_cells_expanded[p_adj.x][p_adj.y])
                {
                    is_adj_to_cell_outside = true;
                }
            }

            if (!is_adj_to_floor_not_in_room &&
                is_adj_to_floor_in_room &&
                is_adj_to_cell_outside)
            {
                out.push_back(p);
            }
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

bool is_choke_point(const P& p,
                    const bool blocked[map_w][map_h],
                    ChokePointData& out)
{
    // Assuming that the tested position is free
    ASSERT(!blocked[p.x][p.y]);

    // Robustness for release mode
    if (blocked[p.x][p.y])
    {
        // This is weird, invalidate the map
        is_map_valid = false;

        return false;
    }

    // First, there must be exactly two free cells cardinally adjacent to the
    // tested position
    P p_side1;
    P p_side2;

    for (const P& d : dir_utils::cardinal_list)
    {
        const P adj_p(p + d);

        if (!blocked[adj_p.x][adj_p.y])
        {
            if (p_side1.x == 0)
            {
                p_side1 = adj_p;
            }
            else if (p_side2.x == 0)
            {
                p_side2 = adj_p;
            }
            else // Both p0 and p1 has already been set
            {
                // This is not a choke point, bye!
                return false;
            }
        }
    }

    // OK, the position has exactly two free cardinally adjacent cells

    // Check that the two sides can reach each other
    int flood_side1[map_w][map_h];

    floodfill(p_side1,
              blocked,
              flood_side1);

    if (flood_side1[p_side2.x][p_side2.y] == 0)
    {
        // The two sides was already separated from each other, nevermind
        return false;
    }

    // Check if this position can completely separate the two sides
    bool blocked_cpy[map_w][map_h];

    std::copy_n(*blocked, nr_map_cells, *blocked_cpy);

    blocked_cpy[p.x][p.y] = true;

    // Do another floodfill from side 1
    floodfill(p_side1,
                   blocked_cpy,
                   flood_side1);

    if (flood_side1[p_side2.x][p_side2.y] > 0)
    {
        // The two sides can still reach each other - not a choke point
        return false;
    }

    // OK, this is a "true" choke point, time to gather more information!

    out.p = p;

    int flood_side2[map_w][map_h];

    // Do a floodfill from side 2
    floodfill(p_side2,
                   blocked_cpy,
                   flood_side2);

    // Prepare for at lease the worst case of push-backs
    out.sides[0].reserve(nr_map_cells);
    out.sides[1].reserve(nr_map_cells);

    // Add the origin positions for both sides (they have flood value 0)
    out.sides[0].push_back(p_side1);
    out.sides[1].push_back(p_side2);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (flood_side1[x][y] > 0)
            {
                ASSERT(flood_side2[x][y] == 0);

                out.sides[0].push_back(P(x, y));
            }
            else if (flood_side2[x][y] > 0)
            {
                out.sides[1].push_back(P(x, y));
            }
        }
    }

    return true;
}

void mk_pathfind_corridor(Room& room_0,
                          Room& room_1,
                          bool door_proposals[map_w][map_h])
{
    TRACE_FUNC_BEGIN_VERBOSE << "Making corridor between rooms "
                             << &room_0 << " and " << &room_1
                             << std::endl;

    ASSERT(map::is_area_inside_map(room_0.r_));
    ASSERT(map::is_area_inside_map(room_1.r_));

    std::vector<P> p0_bucket;
    std::vector<P> p1_bucket;

    valid_corridor_entries(room_0, p0_bucket);
    valid_corridor_entries(room_1, p1_bucket);

    if (p0_bucket.empty())
    {
        TRACE_FUNC_END_VERBOSE << "No entry points found in room 0"
                               << std::endl;
        return;
    }

    if (p1_bucket.empty())
    {
        TRACE_FUNC_END_VERBOSE << "No entry points found in room 1"
                               << std::endl;
        return;
    }

    int shortest_dist = INT_MAX;

    TRACE_VERBOSE << "Finding shortest possible dist between entries"
                  << std::endl;

    for (const P& p0 : p0_bucket)
    {
        for (const P& p1 : p1_bucket)
        {
            const int dist = king_dist(p0, p1);

            if (dist < shortest_dist)
            {
                shortest_dist = dist;
            }
        }
    }

    TRACE_VERBOSE << "Storing entry pairs with shortest dist ("
                  << shortest_dist << ")"
                  << std::endl;

    std::vector< std::pair<P, P> > entries_bucket;

    for (const P& p0 : p0_bucket)
    {
        for (const P& p1 : p1_bucket)
        {
            const int dist = king_dist(p0, p1);

            if (dist == shortest_dist)
            {
                entries_bucket.push_back(std::pair<P, P>(p0, p1));
            }
        }
    }

    TRACE_VERBOSE << "Picking a random stored entry pair" << std::endl;

    const size_t idx = rnd::range(0, entries_bucket.size() - 1);

    const auto& entries = entries_bucket[idx];

    const P& p0 = entries.first;
    const P& p1 = entries.second;

    std::vector<P> path;

    bool blocked_expanded[map_w][map_h];

    // Entry points are the same cell (rooms are adjacent)? Then simply use that
    if (p0 == p1)
    {
        path.push_back(p0);
    }
    else // Entry points are different cells
    {
        // Try to find a path to the other entry point

        bool blocked[map_w][map_h] = {};

        // Mark all cells as blocked, which is not a wall, or is a room
        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                const bool is_wall =
                    map::cells[x][y].rigid->id() == FeatureId::wall;

                const auto* const room_ptr = map::room_map[x][y];

                blocked[x][y] = !is_wall || room_ptr;
            }
        }

        // Search around p0 and p1, to see if they are actually OK to build from
        for (const P& d : dir_utils::dir_list)
        {
            const P p(p0 + d);

            const auto* const room_ptr = map::room_map[p.x][p.y];

            if (blocked[p.x][p.y] &&
                room_ptr != &room_0)
            {
                return;
            }
        }

        for (const P& d : dir_utils::dir_list)
        {
            const P p(p1 + d);

            const auto* const room_ptr = map::room_map[p.x][p.y];

            if (blocked[p.x][p.y] &&
                room_ptr != &room_1)
            {
                return;
            }
        }

        // Expand the blocked cells - we do not want to build adjacent to floor
        map_parsers::expand(blocked, blocked_expanded);

        // Randomly mark some cells as blocked, this helps avoid boring super
        // long straight corridors
        // for (int x = 0; x < map_w; ++x)
        // {
        //     for (int y = 0; y < map_h; ++y)
        //     {
        //         // Blocking a high amount of cells in the late game levels
        //         // creates very nice cave corridors
        //         const int block_one_in_n =
        //             map::dlvl >= dlvl_first_late_game   ? 3 :
        //             map::dlvl >= dlvl_first_mid_game    ? 6 :
        //             10;

        //         if (rnd::one_in(block_one_in_n))
        //         {
        //             blocked_expanded[x][y] = true;

        //             continue;
        //         }
        //     }
        // }

        // We know from above that p0 and p1 are actually OK - so mark them as
        // free in the expanded blocking array
        blocked_expanded[p0.x][p0.y] = blocked_expanded[p1.x][p1.y] = false;

        // Allowing diagonal steps creates a more "cave like" path
        const bool allow_diagonal = map::dlvl >= dlvl_first_late_game;

        // Randomizing step choices (i.e. when to change directions) creates
        // more "snaky" paths (note that this does NOT create longer paths - it
        // just randomizes the variation of optimal path)
        const bool randomize_step_choices = true;

        pathfind(p0,
                 p1,
                 blocked_expanded,
                 path,
                 allow_diagonal,
                 randomize_step_choices);
    }

    if (!path.empty())
    {
        path.push_back(p0);

        TRACE_VERBOSE << "Check that we don't circle around the "
                      << "origin or target room (looks bad)"
                      << std::endl;

        std::vector<Room*> rooms {&room_0, &room_1};

        for (Room* room : rooms)
        {
            bool is_left_of_room = false;
            bool is_right_of_room = false;
            bool is_above_room = false;
            bool is_below_room = false;

            for (const P& p : path)
            {
                if (p.x < room->r_.p0.x)
                {
                    is_left_of_room = true;
                }

                if (p.x > room->r_.p1.x)
                {
                    is_right_of_room = true;
                }

                if (p.y < room->r_.p0.y)
                {
                    is_above_room = true;
                }

                if (p.y > room->r_.p1.y)
                {
                    is_below_room = true;
                }
            }

            if ((is_left_of_room && is_right_of_room) ||
                (is_above_room && is_below_room))
            {
                TRACE_FUNC_END_VERBOSE << "Path circled around room, "
                                       << "aborting corridor"
                                       << std::endl;

                return;
            }
        }

        std::vector<Room*> prev_links;

        for (size_t i = 0; i < path.size(); ++i)
        {
            const P& p(path[i]);

            // If this is a late level, occasionally put floor in 3x3 cells
            // around each path point (wide corridors for more "open" level).
            if (map::dlvl >= dlvl_first_late_game && rnd::fraction(2, 5))
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        const P p_adj(p + P(dx, dy));

                        if (map::is_pos_inside_map(p_adj, false) &&
                            !blocked_expanded[p_adj.x][p_adj.y])
                        {
                            map::put(new Floor(p_adj));
                        }
                    }
                }
            }

            map::put(new Floor(p));

            // Make it possible to branch from the corridor
            if ((i > 1) &&
                ((int)i < (int)path.size() - 3) &&
                (i % 4 == 0))
            {
                Room* link = room_factory::mk(RoomType::corr_link, R(p, p));

                map::room_list.push_back(link);

                map::room_map[p.x][p.y] = link;

                link->rooms_con_to_.push_back(&room_0);
                link->rooms_con_to_.push_back(&room_1);

                room_0.rooms_con_to_.push_back(link);
                room_1.rooms_con_to_.push_back(link);

                for (Room* prev_link : prev_links)
                {
                    link->rooms_con_to_.push_back(prev_link);
                    prev_link->rooms_con_to_.push_back(link);
                }

                prev_links.push_back(link);
            }
        }

        if (door_proposals)
        {
            door_proposals[p0.x][p0.y] = door_proposals[p1.x][p1.y] = true;
        }

        room_0.rooms_con_to_.push_back(&room_1);
        room_1.rooms_con_to_.push_back(&room_0);

        TRACE_FUNC_END_VERBOSE << "Successfully connected roooms" << std::endl;

        return;
    }

    TRACE_FUNC_END_VERBOSE << "Failed to connect roooms" << std::endl;
}

void pathfinder_walk(const P& p0,
                     const P& p1,
                     std::vector<P>& pos_list_ref,
                     const bool is_smooth)
{
    pos_list_ref.clear();

    bool blocked[map_w][map_h] = {};

    std::vector<P> path;
    pathfind(p0, p1, blocked, path);

    std::vector<P> rnd_walk_buffer;

    for (const P& p : path)
    {
        pos_list_ref.push_back(p);

        if (!is_smooth && rnd::one_in(3))
        {
            rnd_walk(p, rnd::range(1, 6), rnd_walk_buffer, true);

            pos_list_ref.reserve(pos_list_ref.size() + rnd_walk_buffer.size());

            move(begin(rnd_walk_buffer),
                 end(rnd_walk_buffer),
                 back_inserter(pos_list_ref));
        }
    }
}

void rnd_walk(const P& p0,
              int len,
              std::vector<P>& pos_list_ref,
              const bool allow_diagonal,
              R area)
{
    pos_list_ref.clear();

    const std::vector<P>& d_list =
        allow_diagonal ?
        dir_utils::dir_list :
        dir_utils::cardinal_list;

    const int d_list_size = d_list.size();

    P p(p0);

    while (len > 0)
    {
        pos_list_ref.push_back(p);
        --len;

        while (true)
        {
            const P nxt_pos = p + d_list[rnd::range(0, d_list_size - 1)];

            if (is_pos_inside(nxt_pos, area))
            {
                p = nxt_pos;
                break;
            }
        }
    }
}

void mk_explore_spawn_weights(const bool blocked[map_w][map_h],
                              std::vector<P>& positions_out,
                              std::vector<int>& weights_out)
{
    int weight_map[map_w][map_h] = {};

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            // Give all cells a base weight of 1
            weight_map[x][y] = 1;

            // Increase weight for dark cells
            if (map::cells[x][y].is_dark)
            {
                weight_map[x][y] += 10;
            }
        }
    }

    // Put extra weight for "optional" areas behind choke points
    for (const auto& choke_point : map::choke_point_data)
    {
        // If the player and the stairs are on the same side of the choke point,
        // this means that the "other" side is an optional map branch.
        if (choke_point.player_side == choke_point.stairs_side)
        {
            ASSERT(choke_point.player_side == 0 ||
                   choke_point.player_side == 1);

            // Robustness for release mode
            if (choke_point.player_side != 0 &&
                choke_point.player_side != 1)
            {
                continue;
            }

            const int other_side_idx =
                (choke_point.player_side == 0) ?
                1 : 0;

            const auto& other_side_positions =
                choke_point.sides[other_side_idx];

            //
            // NOTE: To avoid leaning heavily towards only putting stuff in big
            //       hidden areas, we divide the weight given per cell based on
            //       the total number of cells in the area.
            //

            const int weight_div =
                std::max(1, (int)other_side_positions.size() / 2);

            // Increase weight for being in an optional map branch
            int weight_inc =
                std::max(1, (250 / weight_div));

            Rigid* const rigid =
                map::cells[choke_point.p.x][choke_point.p.y].rigid;

            // Increase weight if behind hidden, stuck, or metal doors
            if (rigid->id() == FeatureId::door)
            {
                Door* const door = static_cast<Door*>(rigid);

                if (door->is_secret())
                {
                    weight_inc += std::max(1, 200 / weight_div);
                }

                if (door->is_stuck())
                {
                    weight_inc += std::max(1, 200 / weight_div);
                }

                if (door->type() == DoorType::metal)
                {
                    weight_inc += std::max(1, 500 / weight_div);
                }
            }

            for (const P& p : other_side_positions)
            {
                weight_map[p.x][p.y] += weight_inc;
            }
        }
    }

    // Prepare for at least the worst case of push-backs
    positions_out.reserve(nr_map_cells);

    weights_out.reserve(nr_map_cells);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (blocked[x][y])
            {
                continue;
            }

            const int weight = weight_map[x][y];

            if (weight > 0)
            {
                // OK, we can spawn here, save the position and the
                // corresponding spawn chance weight
                positions_out.push_back(P(x, y));

                weights_out.push_back(weight);
            }
        }
    }
}

} // mapgen
