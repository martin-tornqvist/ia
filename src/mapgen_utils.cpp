#include "init.hpp"

#include "mapgen.hpp"

#include <vector>
#include <climits>
#include <cstring>

#include "actor_player.hpp"
#include "feature_door.hpp"
#include "feature_rigid.hpp"
#include "game_time.hpp"
#include "init.hpp"
#include "init.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "map_templates.hpp"

#ifndef NDEBUG
#include "io.hpp"
#include "sdl_base.hpp"
#endif // NDEBUG

namespace mapgen
{

bool is_map_valid = true;

Array2<bool> door_proposals(0, 0);

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
                        map::room_map.at(x, y) = &room;
                }
        }
}

void make_floor(const Room& room)
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

        // NOTE: The "cross" dimensons and coordinates refer to the inner
        // rectangle of the plus shape.
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
                                                map::room_map.at(check_p);

                                        const FeatureId id =
                                                map::cells.at(check_p)
                                                .rigid->id();

                                        if ((room_here == &room &&
                                             id != FeatureId::floor) ||
                                            (room_here != &room &&
                                             id != FeatureId::wall)
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
                                        map::room_map.at(x, y) = nullptr;
                                }
                        }
                }
        }
}

void make_pillars_in_room(const Room& room)
{
        const P& room_p0(room.r_.p0);
        const P& room_p1(room.r_.p1);

        auto is_free = [](const P & p) {

                for (const P& d : dir_utils::dir_list_w_center)
                {
                        const P check_p(p + d);

                        const FeatureId id = map::cells.at(check_p).rigid->id();

                        if (id == FeatureId::wall)
                        {
                                return false;
                        }
                }
                return true;
        };

        // Place pillars in rows and columns
        auto step_size = []() {

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
        Array2<bool> is_other_room(map::dims());

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                const auto* const room_here = map::room_map.at(i);

                is_other_room.at(i) =
                        room_here &&
                        room_here != &room;
        }

        const auto blocked = map_parsers::expand(
                is_other_room,
                is_other_room.rect());

        R& room_rect = room.r_;

        std::vector<P> origin_bucket;

        const auto& r = room.r_;

        const int x0 = r.p0.x + 1;
        const int y0 = r.p0.y + 1;
        const int x1 = r.p1.x - 1;
        const int y1 = r.p1.y - 1;

        for (int x = x0; x <= x1; ++x)
        {
                for (int y = y0; y <= y1; ++y)
                {
                        // add to origin bucket if we are on the edge
                        if (x == x0 || x == x1 || y == y0 || y == y1)
                        {
                                origin_bucket.push_back({x, y});
                        }
                }
        }

        for (const P& origin : origin_bucket)
        {
                if (blocked.at(origin) ||
                    map::room_map.at(origin) != &room)
                {
                        continue;
                }

                const auto flood = floodfill(
                        origin,
                        blocked,
                        rnd::range(1, 4),
                        P( -1, -1),
                        false);

                for (int x = 0; x < map::w(); ++x)
                {
                        for (int y = 0; y < map::h(); ++y)
                        {
                                if ((flood.at(x, y) > 0) &&
                                    (map::room_map.at(x, y) != &room))
                                {
                                        map::put(new Floor({x, y}));

                                        map::room_map.at(x, y) = &room;

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

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                if (map::room_map.at(i) == &room)
                {
                        Rigid* const rigid = map::cells.at(i).rigid;

                        if (rigid->id() == FeatureId::floor)
                        {
                                static_cast<Floor*>(rigid)->type_ =
                                        FloorType::cave;
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

        Array2<bool> room_cells(map::dims());
        Array2<bool> room_floor_cells(map::dims());

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                const bool is_room_cell =
                        map::room_map.at(i) == &room;

                room_cells.at(i) = is_room_cell;

                const auto* const f = map::cells.at(i).rigid;

                room_floor_cells.at(i) =
                        is_room_cell &&
                        (f->id() == FeatureId::floor);
        }

        const auto room_cells_expanded = map_parsers::expand(
                room_cells,
                R(P(room.r_.p0 - 2),
                  P(room.r_.p1 + 2)));

        for (int y = room.r_.p0.y - 1; y <= room.r_.p1.y + 1; ++y)
        {
                for (int x = room.r_.p0.x - 1; x <= room.r_.p1.x + 1; ++x)
                {
                        // Condition (1)
                        if (map::cells.at(x, y).rigid->id() != FeatureId::wall)
                        {
                                continue;
                        }

                        // Condition (2)
                        if (map::room_map.at(x, y))
                        {
                                continue;
                        }

                        // Condition (3)
                        if (x <= 1 ||
                            y <= 1 ||
                            x >= (map::w() - 2) ||
                            y >= (map::h() - 2))
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
                                if (room_floor_cells.at(p_adj))
                                {
                                        is_adj_to_floor_in_room = true;
                                }

                                // Condition (5)
                                if (!room_cells_expanded.at(p_adj))
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
                    const Array2<bool>& blocked,
                    ChokePointData* out)
{
        // Assuming that the tested position is free
        ASSERT(!blocked.at(p));

        // Robustness for release mode
        if (blocked.at(p))
        {
                // This is weird, invalidate the map
                is_map_valid = false;

                return false;
        }

        // First, there must be exactly two free cells cardinally adjacent to
        // the tested position
        P p_side1;
        P p_side2;

        for (const P& d : dir_utils::cardinal_list)
        {
                const P adj_p(p + d);

                if (!blocked.at(adj_p))
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
        auto flood_side1 = floodfill(p_side1, blocked);

        if (flood_side1.at(p_side2) == 0)
        {
                // The two sides were already separated from each other
                return false;
        }

        // Check if this position can completely separate the two sides
        auto blocked_cpy(blocked);

        blocked_cpy.at(p) = true;

        // Do another floodfill from side 1
        flood_side1 = floodfill(p_side1, blocked_cpy);

        if (flood_side1.at(p_side2) > 0)
        {
                // The two sides can still reach each other - not a choke point
                return false;
        }

        // OK, this is a "true" choke point, time to gather more information!

        if (out)
        {
                out->p = p;
        }

        // Do a floodfill from side 2
        const auto flood_side2 = floodfill(p_side2, blocked_cpy);

        if (out)
        {
                // Prepare for at least the worst case of push-backs
                out->sides[0].reserve(map::nr_cells());
                out->sides[1].reserve(map::nr_cells());

                // Add the origin positions for both sides (they have flood
                // value 0)
                out->sides[0].push_back(p_side1);
                out->sides[1].push_back(p_side2);

                for (int x = 0; x < map::w(); ++x)
                {
                        for (int y = 0; y < map::h(); ++y)
                        {
                                if (flood_side1.at(x, y) > 0)
                                {
                                        ASSERT(flood_side2.at(x, y) == 0);

                                        out->sides[0].push_back(P(x, y));
                                }
                                else if (flood_side2.at(x, y) > 0)
                                {
                                        out->sides[1].push_back(P(x, y));
                                }
                        }
                }
        }

        return true;
}

void make_pathfind_corridor(Room& room_0,
                            Room& room_1,
                            const Array2<bool>* const door_proposals)
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
                                entries_bucket.push_back(
                                        std::pair<P, P>(p0, p1));
                        }
                }
        }

        TRACE_VERBOSE << "Picking a random stored entry pair" << std::endl;

        const size_t idx = rnd::range(0, entries_bucket.size() - 1);

        const auto& entries = entries_bucket[idx];

        const P& p0 = entries.first;
        const P& p1 = entries.second;

        std::vector<P> path;

        Array2<bool> blocked_expanded(map::dims());

        // Entry points are the same cell (rooms are adjacent)? Then use that
        if (p0 == p1)
        {
                path.push_back(p0);
        }
        else // Entry points are different cells
        {
                // Try to find a path to the other entry point

                Array2<bool> blocked(map::dims());

                // Mark all cells as blocked, which is not a wall, or is a room
                for (size_t i = 0; i < map::nr_cells(); ++i)
                {
                        const bool is_wall =
                                map::cells.at(i).rigid->id() ==
                                FeatureId::wall;

                        const auto* const room_ptr = map::room_map.at(i);

                        blocked.at(i) = !is_wall || room_ptr;
                }

                // Search around p0 and p1 to see if they are OK to build from
                for (const P& d : dir_utils::dir_list)
                {
                        const P p(p0 + d);

                        const auto* const room_ptr = map::room_map.at(p);

                        if (blocked.at(p) && (room_ptr != &room_0))
                        {
                                return;
                        }
                }

                for (const P& d : dir_utils::dir_list)
                {
                        const P p(p1 + d);

                        const auto* const room_ptr = map::room_map.at(p);

                        if (blocked.at(p) && (room_ptr != &room_1))
                        {
                                return;
                        }
                }

                // Expand the blocked cells - we do not want to build adjacent
                // to floor
                blocked_expanded = map_parsers::expand(blocked, blocked.rect());

                // Randomly mark some cells as blocked, this helps avoid boring
                // super long straight corridors
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

                // We know from above that p0 and p1 are actually OK - so mark
                // them as free in the expanded blocking array
                blocked_expanded.at(p0) = false;
                blocked_expanded.at(p1) = false;

                // Allowing diagonal steps creates a more "cave like" path
                const bool allow_diagonal = (map::dlvl >= dlvl_first_late_game);

                // Randomizing step choices (i.e. when to change directions)
                // creates more "snaky" paths (note that this does NOT create
                // longer paths - it just randomizes the variation of optimal
                // path)
                const bool randomize_step_choices = true;

                path = pathfind(
                        p0,
                        p1,
                        blocked_expanded,
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
                                TRACE_FUNC_END_VERBOSE
                                        << "Path circled around room, "
                                        << "aborting corridor"
                                        << std::endl;

                                return;
                        }
                }

                std::vector<Room*> prev_links;

                for (size_t i = 0; i < path.size(); ++i)
                {
                        const P& p(path[i]);

                        // If this is a late level, occasionally put floor in
                        // 3x3 cells around each path point (wide corridors for
                        // more "open" level).
                        if ((map::dlvl >= dlvl_first_late_game) &&
                            rnd::fraction(2, 5))
                        {
                                for (const P d : dir_utils::dir_list_w_center)
                                {
                                        const P p_adj(p + d);

                                        const bool is_inside =
                                                map::is_pos_inside_map(
                                                        p_adj, false);

                                        if (is_inside &&
                                            !blocked_expanded.at(p_adj))
                                        {
                                                map::put(new Floor(p_adj));
                                        }
                                }
                        }

                        map::put(new Floor(p));

                        // Make it possible to branch from the corridor
                        if ((i > 1) &&
                            ((int)i < (int)path.size() - 3) &&
                            (i % 4 == 0))
                        {
                                Room* link =
                                        room_factory::make(
                                                RoomType::corr_link,
                                                R(p, p));

                                map::room_list.push_back(link);

                                map::room_map.at(p) = link;

                                link->rooms_con_to_.push_back(&room_0);
                                link->rooms_con_to_.push_back(&room_1);

                                room_0.rooms_con_to_.push_back(link);
                                room_1.rooms_con_to_.push_back(link);

                                for (Room* prev_link : prev_links)
                                {
                                        link->rooms_con_to_.push_back(
                                                prev_link);

                                        prev_link->rooms_con_to_.push_back(
                                                link);
                                }

                                prev_links.push_back(link);
                        }
                }

                if (door_proposals)
                {
                        door_proposals->at(p0) = true;
                        door_proposals->at(p1) = true;
                }

                room_0.rooms_con_to_.push_back(&room_1);
                room_1.rooms_con_to_.push_back(&room_0);

                TRACE_FUNC_END_VERBOSE << "Successfully connected roooms"
                                       << std::endl;

                return;
        }

        TRACE_FUNC_END_VERBOSE << "Failed to connect roooms" << std::endl;
}

std::vector<P> pathfinder_walk(const P& p0,
                               const P& p1,
                               const bool is_smooth)
{
        std::vector<P> result;

        Array2<bool> blocked(map::dims());

        const auto path = pathfind(p0, p1, blocked);

        std::vector<P> rnd_walk_buffer;

        for (const P& p : path)
        {
                result.push_back(p);

                if (!is_smooth && rnd::one_in(3))
                {
                        const auto rnd_walk_path = rnd_walk(
                                p,
                                rnd::range(1, 6),
                                map::rect(),
                                true);

                        result.reserve(
                                result.size() +
                                rnd_walk_buffer.size());

                        move(begin(rnd_walk_buffer),
                             end(rnd_walk_buffer),
                             back_inserter(result));
                }
        }

        return result;
}

std::vector<P> rnd_walk(
        const P& p0,
        int len,
        R area,
        const bool allow_diagonal)
{
        std::vector<P> result;

        const std::vector<P>& d_list =
                allow_diagonal ?
                dir_utils::dir_list :
                dir_utils::cardinal_list;

        P p(p0);

        while (len > 0)
        {
                result.push_back(p);

                --len;

                while (true)
                {
                        const P nxt_pos = p + rnd::element(d_list);

                        if (is_pos_inside(nxt_pos, area))
                        {
                                p = nxt_pos;
                                break;
                        }
                }
        }

        return result;
}

void make_explore_spawn_weights(const Array2<bool>& blocked,
                                std::vector<P>& positions_out,
                                std::vector<int>& weights_out)
{
        Array2<int> weight_map(map::dims());

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                // Give all cells a base weight of 1
                weight_map.at(i) = 1;

                // Increase weight for dark cells
                if (map::dark.at(i))
                {
                        weight_map.at(i) += 10;
                }
        }

        // Put extra weight for "optional" areas behind choke points
        for (const auto& choke_point : map::choke_point_data)
        {
                // If the player and the stairs are on the same side of the
                // choke point, this means that the "other" side is an optional
                // map branch.
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

                        // NOTE: To avoid leaning heavily towards only putting
                        // stuff in big hidden areas, we divide the weight given
                        // per cell based on the total number of cells in the
                        // area.

                        const int weight_div = std::max(
                                1,
                                (int)other_side_positions.size() / 2);

                        // Increase weight for being in an optional map branch
                        int weight_inc = std::max(
                                1,
                                (250 / weight_div));

                        Rigid* const rigid = map::cells.at(choke_point.p).rigid;

                        // Increase weight if behind hidden/stuck/metal doors
                        if (rigid->id() == FeatureId::door)
                        {
                                Door* const door = static_cast<Door*>(rigid);

                                if (door->is_secret())
                                {
                                        weight_inc += std::max(
                                                1,
                                                200 / weight_div);
                                }

                                if (door->is_stuck())
                                {
                                        weight_inc += std::max(
                                                1,
                                                200 / weight_div);
                                }

                                if (door->type() == DoorType::metal)
                                {
                                        weight_inc += std::max(
                                                1,
                                                500 / weight_div);
                                }
                        }

                        for (const P& p : other_side_positions)
                        {
                                weight_map.at(p) += weight_inc;
                        }
                }
        }

        // Prepare for at least the worst case of push-backs
        positions_out.reserve(map::nr_cells());

        weights_out.reserve(map::nr_cells());

        for (int x = 0; x < map::w(); ++x)
        {
                for (int y = 0; y < map::h(); ++y)
                {
                        if (blocked.at(x, y))
                        {
                                continue;
                        }

                        const int weight = weight_map.at(x, y);

                        if (weight > 0)
                        {
                                // OK, we can spawn here, save the position and
                                // the corresponding spawn chance weight
                                positions_out.push_back(P(x, y));

                                weights_out.push_back(weight);
                        }
                }
        }
}

Array2<bool> allowed_stair_cells()
{
        TRACE_FUNC_BEGIN;

        Array2<bool> result(map::dims());

        // Mark cells as free if all adjacent feature types are allowed
        std::vector<FeatureId> feat_ids_ok
        {
                FeatureId::floor,
                        FeatureId::carpet,
                        FeatureId::grass,
                        FeatureId::bush,
                        FeatureId::rubble_low,
                        FeatureId::vines,
                        FeatureId::chains,
                        FeatureId::trap
                        };

        map_parsers::AllAdjIsAnyOfFeatures(feat_ids_ok)
                .run(result, result.rect());

        // Block cells with items
        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                if (map::cells.at(i).item)
                {
                        result.at(i) = false;
                }
        }

        // Block cells with actors
        for (const auto* const actor : game_time::actors)
        {
                const P& p(actor->pos);

                result.at(p) = false;
        }

        TRACE_FUNC_END;

        return result;
}

void move_player_to_nearest_allowed_pos()
{
        TRACE_FUNC_BEGIN;

        const auto allowed_cells = allowed_stair_cells();

        auto pos_bucket = to_vec(allowed_cells, true, allowed_cells.rect());

        if (pos_bucket.empty())
        {
                is_map_valid = false;
        }
        else // Valid cells exists
        {
                TRACE << "Sorting the allowed cells vector "
                      << "(" << pos_bucket.size() << " cells)" << std:: endl;

                IsCloserToPos is_closer_to_origin(map::player->pos);

                sort(pos_bucket.begin(),
                     pos_bucket.end(),
                     is_closer_to_origin);

                map::player->pos = pos_bucket.front();

                // Ensure that the player always descends to a floor cell (and
                // not into a bush or something)
                map::put(new Floor(map::player->pos));
        }

        TRACE_FUNC_END;
}

P make_stairs_at_random_pos()
{
        TRACE_FUNC_BEGIN;

        const auto allowed_cells = allowed_stair_cells();

        auto pos_bucket = to_vec(allowed_cells, true, allowed_cells.rect());

        const int nr_ok_cells = pos_bucket.size();

        const int min_nr_ok_cells_req = 3;

        if (nr_ok_cells < min_nr_ok_cells_req)
        {
                TRACE << "Nr available cells to place stairs too low "
                      << "(" << nr_ok_cells << "), discarding map"
                      << std:: endl;

                is_map_valid = false;

#ifndef NDEBUG
                if (init::is_demo_mapgen)
                {
                        io::cover_panel(Panel::log);
                        states::draw();
                        io::draw_text(
                                "Too few cells to place stairs",
                                Panel::screen,
                                P(0, 0),
                                colors::light_red());
                        io::update_screen();
                        sdl_base::sleep(8000);
                }
#endif // NDEBUG
                return P(-1, -1);
        }

        TRACE << "Sorting the allowed cells vector "
              << "(" << pos_bucket.size() << " cells)" << std:: endl;

        Array2<bool> blocks_move(map::dims());

        map_parsers::BlocksMoveCommon(ParseActors::no)
                .run(blocks_move, blocks_move.rect());

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                if (map::cells.at(i).rigid->id() == FeatureId::door)
                {
                        blocks_move.at(i) = false;
                }
        }

        const auto flood = floodfill(map::player->pos, blocks_move);

        std::sort(pos_bucket.begin(),
                  pos_bucket.end(),
                  [flood](const P& p1, const P& p2)
                  {
                          return flood.at(p1) < flood.at(p2);
                  });

        TRACE << "Picking one of the furthest cells" << std:: endl;

        const int cell_idx_range_size = std::max(1, nr_ok_cells / 5);

        const int cell_idx = nr_ok_cells - rnd::range(1, cell_idx_range_size);

        if ((cell_idx < 0) ||
            (cell_idx > (int)pos_bucket.size()))
        {
                ASSERT(false);

                is_map_valid = false;

                return P(-1, -1);
        }

        const P stairs_pos(pos_bucket[cell_idx]);

        TRACE << "Spawning stairs at chosen cell" << std:: endl;
        map::put(new Stairs(stairs_pos));

        TRACE_FUNC_END;
        return stairs_pos;
}

void reveal_doors_on_path_to_stairs(const P& stairs_pos)
{
        TRACE_FUNC_BEGIN;

        Array2<bool> blocked(map::dims());

        map_parsers::BlocksMoveCommon(ParseActors::no)
                .run(blocked, blocked.rect());

        blocked.at(stairs_pos) = false;

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                if (map::cells.at(i).rigid->id() == FeatureId::door)
                {
                        blocked.at(i) = false;
                }
        }

        const auto path = pathfind(map::player->pos, stairs_pos, blocked);

        ASSERT(!path.empty());

        TRACE << "Travelling along path and revealing all doors" << std:: endl;

        for (const P& pos : path)
        {
                auto* const feature = map::cells.at(pos).rigid;

                if (feature->id() == FeatureId::door)
                {
                        static_cast<Door*>(feature)->reveal(Verbosity::silent);
                }
        }

        TRACE_FUNC_END;
}

} // mapgen
