#include "init.hpp"

#include "map_gen.hpp"

#include <vector>
#include <cassert>

#include "map.hpp"
#include "map_parsing.hpp"
#include "utils.hpp"
#include "map_templates.hpp"
#include "feature_rigid.hpp"
#include "game_time.hpp"

namespace map_gen
{

bool is_map_valid = true;

}

namespace map_gen_utils
{

namespace
{

Feature_id backup[MAP_W][MAP_H];

void floor_cells_in_room(const Room& room, const bool floor[MAP_W][MAP_H],
                         std::vector<P>& out)
{
    assert(utils::is_area_inside_map(room.r_));

    for (int y = room.r_.p0.y; y <= room.r_.p1.y; ++y)
    {
        for (int x = room.r_.p0.x; x <= room.r_.p1.x; ++x)
        {
            if (floor[x][y]) {out.push_back(P(x, y));}
        }
    }
}

} //namespace

void cut_room_corners(const Room& room)
{
    if (!room.sub_rooms_.empty() || room.r_.min_dim() < 6)
    {
        return;
    }

    const P max_dims(room.r_.dims() - 4);

    const P room_p0(room.r_.p0);
    const P room_p1(room.r_.p1);

    const P cross_dims(rnd::range(2, max_dims.x), rnd::range(2, max_dims.y));

    const P cross_x0Y0(rnd::range(room_p0.x + 2, room_p1.x - cross_dims.x - 1),
                       rnd::range(room_p0.y + 2, room_p1.y - cross_dims.y - 1));

    const P cross_x1Y1(cross_x0Y0 + cross_dims - 1);

    //Which corners to place - up-left, up-right, down-left, down-right
    bool c[4] = {true, true, true, true};

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
                    nr_corners++;
                }
                else
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

    for (int y = room_p0.y; y <= room_p1.y; ++y)
    {
        for (int x = room_p0.x; x <= room_p1.x; ++x)
        {
            const bool X_OK = x < cross_x0Y0.x ? (c[0] || c[2]) :
                              x > cross_x1Y1.x ? (c[1] || c[3]) : false;
            const bool Y_OK = y < cross_x0Y0.y ? (c[0] || c[1]) :
                              y > cross_x1Y1.y ? (c[2] || c[3]) : false;

            if (X_OK && Y_OK)
            {
                map::put(new Wall(P(x, y)));
                map::room_map[x][y] = nullptr;
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
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                const auto* const f = map::cells[p.x + dx][p.y + dy].rigid;

                if (f->id() == Feature_id::wall)
                {
                    return false;
                }
            }
        }

        return true;
    };

    if (rnd::fraction(2, 3))
    {
        //Place pillars in rows and columns (but occasionally skip a pillar)
        auto step_size = []()
        {
            return rnd::range(1, 2);
        };
        const int DX = step_size();
        const int DY = step_size();

        for (int y = room_p0.y + 1; y <= room_p1.y - 1; y += DY)
        {
            for (int x = room_p0.x + 1; x <= room_p1.x - 1; x += DX)
            {
                const P p(x, y);

                if (is_free(p) && rnd::fraction(2, 3)) {map::put(new Wall(p));}
            }
        }
    }
    else //Random chance failed
    {
        //Scatter pillars randomly
        for (int y = room_p0.y + 1; y <= room_p1.y - 1; ++y)
        {
            for (int x = room_p0.x + 1; x <= room_p1.x - 1; ++x)
            {
                const P p(x + rnd::range(-1, 1), y + rnd::range(-1, 1));

                if (is_free(p) && rnd::one_in(5)) {map::put(new Wall(p));}
            }
        }
    }
}

void cavify_room(Room& room)
{
    bool is_other_room[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const auto* const room_here  = map::room_map[x][y];
            is_other_room[x][y]           = room_here && room_here != &room;
        }
    }

    bool blocked[MAP_W][MAP_H];

    map_parse::expand(is_other_room, blocked);

    Rect& room_rect = room.r_;

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
            //Add to origin bucket if we are on the edge
            if (x == X0 || x == X1 || y == Y0 || y == Y1)
            {
                origin_bucket.push_back({x, y});
            }
        }
    }

    for (const P& origin : origin_bucket)
    {
        if (blocked[origin.x][origin.y] || map::room_map[origin.x][origin.y] != &room)
        {
            continue;
        }

        int flood[MAP_W][MAP_H];

        flood_fill::run(origin, blocked, flood, rnd::range(1, 4), { -1, -1}, false);

        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                if (flood[x][y] > 0)
                {
                    map::put(new Floor({x, y}));

                    map::room_map[x][y] = &room;

                    if (x < room_rect.p0.x) {room_rect.p0.x = x;}

                    if (y < room_rect.p0.y) {room_rect.p0.y = y;}

                    if (x > room_rect.p1.x) {room_rect.p1.x = x;}

                    if (y > room_rect.p1.y) {room_rect.p1.y = y;}
                }
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (map::room_map[x][y] == &room)
            {
                Rigid* const rigid = map::cells[x][y].rigid;

                if (rigid->id() == Feature_id::floor)
                {
                    static_cast<Floor*>(rigid)->type_ = Floor_type::cave;
                }
            }
        }
    }
}

void valid_room_corr_entries(const Room& room, std::vector<P>& out)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    //Find all cells that meets all of the following criteria:
    //(1) Is a wall cell
    //(2) Is a cell not belonging to any room
    //(3) Is not on the edge of the map
    //(4) Is cardinally adjacent to a floor cell belonging to the room
    //(5) Is cardinally adjacent to a cell not in the room or room outline

    out.clear();

    bool room_cells[MAP_W][MAP_H];
    bool room_floor_cells[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const bool IS_ROOM_CELL = map::room_map[x][y] == &room;
            room_cells[x][y]         = IS_ROOM_CELL;
            const auto* const f     = map::cells[x][y].rigid;
            room_floor_cells[x][y]    = IS_ROOM_CELL && f->id() == Feature_id::floor;
        }
    }

    bool room_cells_expanded[MAP_W][MAP_H];
    map_parse::expand(room_cells, room_cells_expanded,
                      Rect(P(room.r_.p0 - 2), P(room.r_.p1 + 2)));

    for (int y = room.r_.p0.y - 1; y <= room.r_.p1.y + 1; ++y)
    {
        for (int x = room.r_.p0.x - 1; x <= room.r_.p1.x + 1; ++x)
        {
            //Condition (1)
            if (map::cells[x][y].rigid->id() != Feature_id::wall) {continue;}

            //Condition (2)
            if (map::room_map[x][y]) {continue;}

            //Condition (3)
            if (x <= 1 || y <= 1 || x >= MAP_W - 2 || y >= MAP_H - 2) {continue;}

            bool is_adj_to_floor_in_room = false;
            bool is_adj_to_cell_outside = false;

            const P p(x, y);

            bool is_adj_to_floor_not_in_room = false;

            for (const P& d : dir_utils::cardinal_list)
            {
                const P& p_adj(p + d);

                //Condition (4)
                if (room_floor_cells[p_adj.x][p_adj.y])      {is_adj_to_floor_in_room = true;}

                //Condition (5)
                if (!room_cells_expanded[p_adj.x][p_adj.y])  {is_adj_to_cell_outside = true;}
            }

            if (!is_adj_to_floor_not_in_room && is_adj_to_floor_in_room && is_adj_to_cell_outside)
            {
                out.push_back(p);
            }
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

void mk_path_find_cor(Room& r0, Room& r1, bool door_proposals[MAP_W][MAP_H])
{
    TRACE_FUNC_BEGIN_VERBOSE << "Making corridor between rooms "
                             << &r0 << " and " << &r1 << std::endl;

    assert(utils::is_area_inside_map(r0.r_));
    assert(utils::is_area_inside_map(r1.r_));

    std::vector<P> p0_bucket;
    std::vector<P> p1_bucket;

    valid_room_corr_entries(r0, p0_bucket);
    valid_room_corr_entries(r1, p1_bucket);

    if (p0_bucket.empty())
    {
        TRACE_FUNC_END_VERBOSE << "No entry points found in room 0" << std::endl;
        return;
    }

    if (p1_bucket.empty())
    {
        TRACE_FUNC_END_VERBOSE << "No entry points found in room 1" << std::endl;
        return;
    }

    int shortest_dist = INT_MAX;

    TRACE_VERBOSE << "Finding shortest possible dist between entries" << std::endl;

    for (const P& p0 : p0_bucket)
    {
        for (const P& p1 : p1_bucket)
        {
            const int CUR_DIST = utils::king_dist(p0, p1);

            if (CUR_DIST < shortest_dist) {shortest_dist = CUR_DIST;}
        }
    }

    TRACE_VERBOSE << "Storing entry pairs with shortest dist (" << shortest_dist << ")"
                  << std::endl;

    std::vector< std::pair<P, P> > entries_bucket;

    for (const P& p0 : p0_bucket)
    {
        for (const P& p1 : p1_bucket)
        {
            const int CUR_DIST = utils::king_dist(p0, p1);

            if (CUR_DIST == shortest_dist)
            {
                entries_bucket.push_back(std::pair<P, P>(p0, p1));
            }
        }
    }

    TRACE_VERBOSE << "Picking a random stored entry pair" << std::endl;
    const size_t IDX = rnd::range(0, entries_bucket.size() - 1);

    const std::pair<P, P>& entries = entries_bucket[IDX];

    const P& p0 = entries.first;
    const P& p1 = entries.second;

    std::vector<P> path;
    bool blocked_expanded[MAP_W][MAP_H];

    //Is entry points same cell (rooms are adjacent)? Then simply use that
    if (p0 == p1)
    {
        path.push_back(p0);
    }
    else
    {
        //Else, try to find a path to the other entry point
        bool blocked[MAP_W][MAP_H];
        utils::reset_array(blocked, false);

        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                blocked[x][y] = map::room_map[x][y] ||
                                map::cells[x][y].rigid->id() != Feature_id::wall;
            }
        }

        map_parse::expand(blocked, blocked_expanded);

        blocked_expanded[p0.x][p0.y] = blocked_expanded[p1.x][p1.y] = false;

        //Allowing diagonal steps makes a more "cave like" path
        const bool ALLOW_DIAGONAL = map::dlvl >= DLVL_FIRST_LATE_GAME;

        const bool RANDOMIZE_STEP_CHOICES = map::dlvl >= DLVL_FIRST_LATE_GAME ? true :
                                            rnd::one_in(5);

        path_find::run(p0, p1, blocked_expanded, path, ALLOW_DIAGONAL,
                       RANDOMIZE_STEP_CHOICES);
    }

    if (!path.empty())
    {
        path.push_back(p0);

        TRACE_VERBOSE << "Check that the path doesn't circle around the origin or targt "
                      << "room (looks bad)" << std::endl;
        std::vector<Room*> rooms {&r0, &r1};

        for (Room* room : rooms)
        {
            bool is_left_of_room   = false;
            bool is_right_of_room  = false;
            bool is_above_room    = false;
            bool is_below_room    = false;

            for (const P& p : path)
            {
                if (p.x < room->r_.p0.x) {is_left_of_room   = true;}

                if (p.x > room->r_.p1.x) {is_right_of_room  = true;}

                if (p.y < room->r_.p0.y) {is_above_room    = true;}

                if (p.y > room->r_.p1.y) {is_below_room    = true;}
            }

            if ((is_left_of_room && is_right_of_room) || (is_above_room && is_below_room))
            {
                TRACE_FUNC_END_VERBOSE << "Path circled around room, aborting corridor"
                                       << std::endl;
                return;
            }
        }

        std::vector<Room*> prev_links;

        for (size_t i = 0; i < path.size(); ++i)
        {
            const P& p(path[i]);

            //If this is a late game level, occasionally put floor in 3x3 cells around each point
            //in the path (wide corridors for more "open" level).
            if (map::dlvl >= DLVL_FIRST_LATE_GAME && rnd::fraction(2, 5))
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        const P p_adj(p + P(dx, dy));

                        if (
                            utils::is_pos_inside_map(p_adj, false) &&
                            !blocked_expanded[p_adj.x][p_adj.y])
                        {
                            map::put(new Floor(p_adj));
                        }
                    }
                }
            }

            map::put(new Floor(p));

            if (i > 1 && int(i) < int(path.size() - 3) && i % 6 == 0)
            {
                Room* link = room_factory::mk(Room_type::corr_link, Rect(p, p));
                map::room_list.push_back(link);
                map::room_map[p.x][p.y] = link;
                link->rooms_con_to_.push_back(&r0);
                link->rooms_con_to_.push_back(&r1);
                r0.rooms_con_to_.push_back(link);
                r1.rooms_con_to_.push_back(link);

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

        r0.rooms_con_to_.push_back(&r1);
        r1.rooms_con_to_.push_back(&r0);
        TRACE_FUNC_END_VERBOSE << "Successfully connected roooms" << std::endl;
        return;
    }

    TRACE_FUNC_END_VERBOSE << "Failed to connect roooms" << std::endl;
}

void backup_map()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            backup[x][y] = map::cells[x][y].rigid->id();
        }
    }
}

void restore_map()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const auto& data = feature_data::data(backup[x][y]);
            map::put(static_cast<Rigid*>(data.mk_obj(P(x, y))));
        }
    }
}

void pathfinder_walk(const P& p0, const P& p1,
                     std::vector<P>& pos_list_ref,
                     const bool IS_SMOOTH)
{
    pos_list_ref.clear();

    bool blocked[MAP_W][MAP_H];
    utils::reset_array(blocked, false);
    std::vector<P> path;
    path_find::run(p0, p1, blocked, path);

    std::vector<P> rnd_walk_buffer;

    for (const P& p : path)
    {
        pos_list_ref.push_back(p);

        if (!IS_SMOOTH && rnd::one_in(3))
        {
            rnd_walk(p, rnd::range(1, 6), rnd_walk_buffer, true);
            pos_list_ref.reserve(pos_list_ref.size() + rnd_walk_buffer.size());
            move(begin(rnd_walk_buffer), end(rnd_walk_buffer), back_inserter(pos_list_ref));
        }
    }
}

void rnd_walk(const P& p0, int len, std::vector<P>& pos_list_ref,
              const bool ALLOW_DIAGONAL, Rect area)
{
    pos_list_ref.clear();

    const std::vector<P>& d_list = ALLOW_DIAGONAL ?
                                   dir_utils::dir_list : dir_utils::cardinal_list;

    const int D_LIST_SIZE = d_list.size();

    P p(p0);

    while (len > 0)
    {
        pos_list_ref.push_back(p);
        --len;

        while (true)
        {
            const P nxt_pos = p + d_list[rnd::range(0, D_LIST_SIZE - 1)];

            if (utils::is_pos_inside(nxt_pos, area))
            {
                p = nxt_pos;
                break;
            }
        }
    }
}

} //map_gen_utils
