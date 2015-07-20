#include "map_gen.hpp"

#include "init.hpp"

#include <algorithm>
#include <climits>

#include "room.hpp"
#include "map_gen.hpp"
#include "feature_event.hpp"
#include "actor_player.hpp"
#include "feature_door.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "drop.hpp"
#include "item_factory.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "render.hpp"
#include "utils.hpp"
#include "populate_monsters.hpp"
#include "populate_traps.hpp"
#include "populate_items.hpp"
#include "gods.hpp"

#ifdef DEMO_MODE
#include "render.hpp"
#include "sdl_wrapper.hpp"
#include "query.hpp"
#endif // DEMO_MODE

//-------------------------------------
//Some options (comment out to disable)
//-------------------------------------
#define MK_RIVER                1
#define MK_MERGED_REGIONS       1
#define RANDOMLY_BLOCK_REGIONS  1
#define MK_AUX_ROOMS            1
#define MK_CRUMBLE_ROOMS        1
#define MK_SUB_ROOMS            1
//#define FILL_DEAD_ENDS          1
#define DECORATE                1

using namespace std;

namespace map_gen
{

namespace
{

//All cells marked as true in this array will be considered for door placement
bool door_proposals[MAP_W][MAP_H];

bool is_all_rooms_connected()
{
    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    return map_parse::is_map_connected(blocked);
}

//Adds the room to the room list and the room map
void register_room(Room& room)
{
#ifndef NDEBUG

    for (Room* const room_in_list : map::room_list)
    {
        assert(room_in_list != &room); //Check that the room is not already added
    }

#endif // NDEBUG

    map::room_list.push_back(&room);

    for (int x = room.r_.p0.x; x <= room.r_.p1.x; ++x)
    {
        for (int y = room.r_.p0.y; y <= room.r_.p1.y; ++y)
        {
            map::room_map[x][y] = &room;
        }
    }
}

void mk_floor_in_room(const Room& room)
{
    for (int y = room.r_.p0.y; y <= room.r_.p1.y; ++y)
    {
        for (int x = room.r_.p0.x; x <= room.r_.p1.x; ++x)
        {
            map::put(new Floor(Pos(x, y)));
        }
    }
}

void connect_rooms()
{
    TRACE_FUNC_BEGIN;

    int nr_tries_left = 5000;

    while (true)
    {
        //NOTE: Keep this counter at the top of the loop, since otherwise a "continue"
        //statement could bypass it so we get stuck in the loop.
        --nr_tries_left;

        if (nr_tries_left == 0)
        {
            map_gen::is_map_valid = false;
#ifdef DEMO_MODE
            render::cover_panel(Panel::log);
            render::draw_text("Failed to connect map", Panel::screen, {0, 0}, clr_red_lgt);
            render::update_screen();
            sdl_wrapper::sleep(8000);
#endif // DEMO_MODE
            break;
        }

        auto rnd_room = []()
        {
            return map::room_list[rnd::range(0, map::room_list.size() - 1)];
        };

        auto is_std_room = [](const Room & r)
        {
            return int(r.type_) < int(Room_type::END_OF_STD_ROOMS);
        };

        Room* room0 = rnd_room();

        //Room 0 must be a standard room or corridor link
        if (!is_std_room(*room0) && room0->type_ != Room_type::corr_link)
        {
            continue;
        }

        //Finding second room to connect to
        Room* room1 = rnd_room();

        //Room 1 must not be the same as room 0, and it must be a standard room (connections
        //are only allowed between two standard rooms, or from a corridor link to a standard
        //room - never between two corridor links)
        while (room1 == room0 || !is_std_room(*room1))
        {
            room1 = rnd_room();
        }

        //Do not allow two rooms to be connected twice
        const auto& cons_room0 = room0->rooms_con_to_;

        if (find(cons_room0.begin(), cons_room0.end(), room1) != cons_room0.end())
        {
            //Rooms are already connected, trying other combination
            continue;
        }

        //Do not connect room 0 and 1 if another room (except for sub rooms) lies anywhere
        //in a rectangle defined by the two center points of those rooms.
        bool is_other_room_in_way = false;
        const Pos c0(room0->r_.center_pos());
        const Pos c1(room1->r_.center_pos());
        const int X0 = min(c0.x, c1.x);
        const int Y0 = min(c0.y, c1.y);
        const int X1 = max(c0.x, c1.x);
        const int Y1 = max(c0.y, c1.y);

        for (int y = Y0; y <= Y1; ++y)
        {
            for (int x = X0; x <= X1; ++x)
            {
                const Room* const room_here = map::room_map[x][y];

                if (room_here && room_here != room0 && room_here != room1 && !room_here->is_sub_room_)
                {
                    is_other_room_in_way = true;
                }
            }

            if (is_other_room_in_way)
            {
                break;
            }
        }

        if (is_other_room_in_way)
        {
            //Blocked by room between, trying other combination
            continue;
        }

        map_gen_utils::mk_path_find_cor(*room0, *room1, door_proposals);

        if ((nr_tries_left <= 2 || rnd::one_in(4)) && is_all_rooms_connected())
        {
            break;
        }
    }

    TRACE_FUNC_END;
}

void mk_crumble_room(const Rect& room_area_incl_walls, const Pos& event_pos)
{
    vector<Pos> wall_cells;
    vector<Pos> inner_cells;

    const Rect& a = room_area_incl_walls; //abbreviation

    for (int y = a.p0.y; y <= a.p1.y; ++y)
    {
        for (int x = a.p0.x; x <= a.p1.x; ++x)
        {
            const Pos p(x, y);

            if (x == a.p0.x || x == a.p1.x || y == a.p0.y || y == a.p1.y)
            {
                wall_cells.push_back(p);
            }
            else
            {
                inner_cells.push_back(p);
            }

            map::put(new Wall(p));
        }
    }

    game_time::add_mob(new Event_wall_crumble(event_pos, wall_cells, inner_cells));
}

//NOTE: The positions and size can be outside map (e.g. negative positions).
//This function just returns false in that case.
bool try_mk_aux_room(const Pos& p, const Pos& d, bool blocked[MAP_W][MAP_H],
                     const Pos& door_p)
{
    Rect aux_rect(p, p + d - 1);
    Rect aux_rect_with_border(aux_rect.p0 - 1, aux_rect.p1 + 1);

    assert(utils::is_pos_inside(door_p, aux_rect_with_border));

    if (utils::is_area_inside_map(aux_rect_with_border))
    {
        if (!map_parse::is_val_in_area(aux_rect_with_border, blocked))
        {
            for (int y = aux_rect.p0.y; y <= aux_rect.p1.y; ++y)
            {
                for (int x = aux_rect.p0.x; x <= aux_rect.p1.x; ++x)
                {
                    blocked[x][y] = true;
                    assert(!map::room_map[x][y]);
                }
            }

#ifdef MK_CRUMBLE_ROOMS

            if (rnd::one_in(10))
            {
                Room* const room = room_factory::mk(Room_type::crumble_room, aux_rect);
                register_room(*room);
                mk_crumble_room(aux_rect_with_border, door_p);
            }
            else
#endif // MK_CRUMBLE_ROOMS
            {
                Room* const room = room_factory::mk_random_allowed_std_room(aux_rect, false);
                register_room(*room);
                mk_floor_in_room(*room);
            }

            return true;
        }
    }

    return false;
}

void mk_aux_rooms(Region regions[3][3])
{
    TRACE_FUNC_BEGIN;
    const int NR_TRIES_PER_SIDE = 10;

    auto rnd_aux_room_dim = []()
    {
        return Pos(rnd::range(3, 7), rnd::range(3, 7));
    };

    bool floor_cells[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), floor_cells);
    utils::reverse_bool_array(floor_cells);

    for (int region_y = 0; region_y < 3; region_y++)
    {
        for (int region_x = 0; region_x < 3; region_x++)
        {
            const Region& region = regions[region_x][region_y];

            if (region.main_room_)
            {
                Room& main_r = *region.main_room_;

                //Right
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < NR_TRIES_PER_SIDE; ++i)
                    {
                        const Pos con_p(main_r.r_.p1.x + 1,
                                        rnd::range(main_r.r_.p0.y + 1, main_r.r_.p1.y - 1));
                        const Pos aux_d(rnd_aux_room_dim());
                        const Pos aux_p(con_p.x + 1, rnd::range(con_p.y - aux_d.y + 1, con_p.y));

                        if (floor_cells[con_p.x - 1][con_p.y])
                        {
                            if (try_mk_aux_room(aux_p, aux_d, floor_cells, con_p))
                            {
                                TRACE_VERBOSE << "Aux room placed right" << endl;
                                break;
                            }
                        }
                    }
                }

                //Up
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < NR_TRIES_PER_SIDE; ++i)
                    {
                        const Pos con_p(rnd::range(main_r.r_.p0.x + 1, main_r.r_.p1.x - 1),
                                        main_r.r_.p0.y - 1);
                        const Pos aux_d(rnd_aux_room_dim());
                        const Pos aux_p(rnd::range(con_p.x - aux_d.x + 1, con_p.x), con_p.y - 1);

                        if (floor_cells[con_p.x][con_p.y + 1])
                        {
                            if (try_mk_aux_room(aux_p, aux_d, floor_cells, con_p))
                            {
                                TRACE_VERBOSE << "Aux room placed up" << endl;
                                break;
                            }
                        }
                    }
                }

                //Left
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < NR_TRIES_PER_SIDE; ++i)
                    {
                        const Pos con_p(main_r.r_.p0.x - 1,
                                        rnd::range(main_r.r_.p0.y + 1, main_r.r_.p1.y - 1));
                        const Pos aux_d(rnd_aux_room_dim());
                        const Pos aux_p(con_p.x - 1, rnd::range(con_p.y - aux_d.y + 1, con_p.y));

                        if (floor_cells[con_p.x + 1][con_p.y])
                        {
                            if (try_mk_aux_room(aux_p, aux_d, floor_cells, con_p))
                            {
                                TRACE_VERBOSE << "Aux room placed left" << endl;
                                break;
                            }
                        }
                    }
                }

                //Down
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < NR_TRIES_PER_SIDE; ++i)
                    {
                        const Pos con_p(rnd::range(main_r.r_.p0.x + 1, main_r.r_.p1.x - 1),
                                        main_r.r_.p1.y + 1);
                        const Pos aux_d(rnd_aux_room_dim());
                        const Pos aux_p(rnd::range(con_p.x - aux_d.x + 1, con_p.x), con_p.y + 1);

                        if (floor_cells[con_p.x][con_p.y - 1])
                        {
                            if (try_mk_aux_room(aux_p, aux_d, floor_cells, con_p))
                            {
                                TRACE_VERBOSE << "Aux room placed down" << endl;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    TRACE_FUNC_END;
}

void mk_merged_regions_and_rooms(Region regions[3][3])
{
    TRACE_FUNC_BEGIN;

    const int NR_ATTEMPTS = rnd::range(0, 2);

    for (int attempt_cnt = 0; attempt_cnt < NR_ATTEMPTS; ++attempt_cnt)
    {

        Pos reg_i1, reg_i2;

        //Find two non-occupied regions
        int nr_tries_to_find_regions = 100;
        bool is_good_regions_found = false;

        while (!is_good_regions_found)
        {
            --nr_tries_to_find_regions;

            if (nr_tries_to_find_regions <= 0) {return;}

            reg_i1 = Pos(rnd::range(0, 2), rnd::range(0, 1));
            reg_i2 = Pos(reg_i1 + Pos(0, 1));
            is_good_regions_found = regions[reg_i1.x][reg_i1.y].is_free_ &&
                                    regions[reg_i2.x][reg_i2.y].is_free_;
        }

        //Expand region 1 over both areas
        auto& reg1    = regions[reg_i1.x][reg_i1.y];
        auto& reg2    = regions[reg_i2.x][reg_i2.y];

        reg1.r_       = Rect(reg1.r_.p0, reg2.r_.p1);

        reg2.r_       = Rect(-1, -1, -1, -1);

        reg1.is_free_  = reg2.is_free_ = false;

        //Make a room for region 1
        auto rnd_padding = []() {return rnd::range(0, 4);};
        const Rect padding(rnd_padding(), rnd_padding(), rnd_padding(), rnd_padding());

        const Rect room_rect(reg1.r_.p0 + padding.p0, reg1.r_.p1 - padding.p1);
        Room* const room  = room_factory::mk_random_allowed_std_room(room_rect, false);
        reg1.main_room_    = room;
        register_room(*room);
        mk_floor_in_room(*room);
    }

    TRACE_FUNC_END;
}

void randomly_block_regions(Region regions[3][3])
{
    TRACE_FUNC_BEGIN << "Marking some (possibly zero) regions as built, to "
                     << "prevent rooms there (so it's not always 3x3 rooms)"
                     << endl;
    //NOTE: The max number to try can go above the hard limit of regions that
    //could ever be blocked (i.e five regions - blocking is only allowed if
    //no cardinally adjacent region is already blocked). However, this will push
    //the number of attempts towards the upper limit, and increase the chance
    //of a higher number of attempts.
    const int MAX_NR_TO_TRY_BLOCK = max(1, map::dlvl / 3);
    const int NR_TO_TRY_BLOCK     = min(rnd::range(0, MAX_NR_TO_TRY_BLOCK), 5);

    for (int i = 0; i < NR_TO_TRY_BLOCK; ++i)
    {
        TRACE_VERBOSE << "Attempting to block region " << i + 1 << "/"
                      << NR_TO_TRY_BLOCK << endl;
        vector<Pos> block_bucket;

        for (int y = 0; y < 3; ++y)
        {
            for (int x = 0; x < 3; ++x)
            {
                if (regions[x][y].is_free_)
                {
                    bool is_all_adj_free = true;
                    const Pos p(x, y);

                    for (const Pos& d : dir_utils::cardinal_list)
                    {
                        const Pos p_adj(p + d);

                        if (p_adj >= 0 && p_adj <= 2 && p_adj != p)
                        {
                            if (!regions[p_adj.x][p_adj.y].is_free_)
                            {
                                is_all_adj_free = false;
                                break;
                            }
                        }
                    }

                    if (is_all_adj_free) {block_bucket.push_back(p);}
                }
            }
        }

        if (block_bucket.empty())
        {
            TRACE_VERBOSE << "Failed to find eligible regions to block, after "
                          << "blocking " << i << " regions" << endl;
            break;
        }
        else
        {
            const Pos& p(block_bucket[rnd::range(0, block_bucket.size() - 1)]);
            TRACE_VERBOSE << "Blocking region at " << p.x << "," << p.y << endl;
            regions[p.x][p.y].is_free_ = false;
        }
    }

    TRACE_FUNC_END;
}

void reserve_river(Region regions[3][3])
{
    TRACE_FUNC_BEGIN;

    Rect      room_rect;
    Region*   river_region       = nullptr;
    const int RESERVED_PADDING  = 2;

    auto init_room_rect = [&](int& len0, int& len1, int& breadth0, int& breadth1,
                              const Pos & reg0, const Pos & reg2)
    {
        const Rect regions_tot_rect(regions[reg0.x][reg0.y].r_.p0,
                                    regions[reg2.x][reg2.y].r_.p1);
        room_rect    = regions_tot_rect;
        river_region = &regions[reg0.x][reg0.y];
        const int C = (breadth1 + breadth0) / 2;
        breadth0    = C - RESERVED_PADDING;
        breadth1    = C + RESERVED_PADDING;

        assert(utils::is_area_inside_other(room_rect, regions_tot_rect, true));

        len0--; //Extend room rectangle to map edge
        len1++;
    };

    const Axis axis = rnd::coin_toss() ? Axis::hor : Axis::ver;

    if (axis == Axis::hor)
    {
        init_room_rect(room_rect.p0.x, room_rect.p1.x, room_rect.p0.y, room_rect.p1.y,
                       Pos(0, 1), Pos(2, 1));
    }
    else
    {
        init_room_rect(room_rect.p0.y, room_rect.p1.y, room_rect.p0.x, room_rect.p1.x,
                       Pos(1, 0), Pos(1, 2));
    }

    Room* const       room      = room_factory::mk(Room_type::river, room_rect);
    River_room* const  river_room = static_cast<River_room*>(room);
    river_room->axis_             = axis;
    river_region->main_room_      = room;
    river_region->is_free_        = false;

    if (axis == Axis::hor)
    {
        regions[1][1] = regions[2][1] = *river_region;
    }
    else
    {
        regions[1][1] = regions[1][2] = *river_region;
    }

    map::room_list.push_back(room);

    auto mk = [&](const int X0, const int X1, const int Y0, const int Y1)
    {
        TRACE_VERBOSE << "Reserving river space with floor cells "
                      << "X0: " << X0 << " X1: " << X1 << " Y0: " << Y0 << " Y1: " << Y1
                      << endl;

        for (int y = Y0; y <= Y1; ++y)
        {
            for (int x = X0; x <= X1; ++x)
            {
                //Just put floor for now, river feature will be placed later
                map::put(new Floor(Pos(x, y)));
                map::room_map[x][y] = room;
            }
        }
    };

    if (axis == Axis::hor)
    {
        mk(room_rect.p0.x + 1, room_rect.p1.x - 1,  room_rect.p0.y,      room_rect.p1.y);
    }
    else //Vertical axis
    {
        mk(room_rect.p0.x,     room_rect.p1.x,      room_rect.p0.y + 1,  room_rect.p1.y - 1);
    }

    TRACE_FUNC_END;
}

void place_door_at_pos_if_allowed(const Pos& p)
{
    //Check that no other doors are within a certain distance
    const int R = 2;

    for (int dx = -R; dx <= R; ++dx)
    {
        for (int dy = -R; dy <= R; ++dy)
        {
            const Pos check_pos = p + Pos(dx, dy);

            if ((dx != 0 || dy != 0) && utils::is_pos_inside_map(check_pos))
            {
                const Cell& cell = map::cells[check_pos.x][check_pos.y];

                if (cell.rigid->id() == Feature_id::door) {return;}
            }
        }
    }

    bool is_good_ver = true;
    bool is_good_hor = true;

    for (int d = -1; d <= 1; d++)
    {
        if (map::cells[p.x + d][p.y].rigid->id() == Feature_id::wall)
        {
            is_good_hor = false;
        }

        if (map::cells[p.x][p.y + d].rigid->id() == Feature_id::wall)
        {
            is_good_ver = false;
        }

        if (d != 0)
        {
            if (map::cells[p.x][p.y + d].rigid->id() != Feature_id::wall)
            {
                is_good_hor = false;
            }

            if (map::cells[p.x + d][p.y].rigid->id() != Feature_id::wall)
            {
                is_good_ver = false;
            }
        }
    }

    if (is_good_hor || is_good_ver)
    {
        const auto& d = feature_data::data(Feature_id::wall);
        const auto* const mimic = static_cast<const Rigid*>(d.mk_obj(p));
        map::put(new Door(p, mimic));
    }
}

//Assumes that all rooms are rectangular
void mk_sub_rooms()
{
    TRACE_FUNC_BEGIN;

    const int NR_TRIES_TO_MK_ROOM = 40;
    const int MAX_NR_INNER_ROOMS  = 7;

    const Pos min_d(4, 4);

    for (size_t i = 0; i < map::room_list.size(); ++i)
    {
        auto* const outer_room     = map::room_list[i];

        const Rect  outer_room_rect = outer_room->r_;
        const Pos   outer_room_d    = (outer_room_rect.p1 - outer_room_rect.p0) + 1;

        const bool IS_ROOM_BIG = outer_room_d.x > 16 || outer_room_d.y > 8;

        const bool IS_STD_ROOM = int(outer_room->type_) < int(Room_type::END_OF_STD_ROOMS);

        if (IS_STD_ROOM && (IS_ROOM_BIG || rnd::one_in(4)))
        {
            const Pos max_d(min(16, outer_room_d.x),  min(16, outer_room_d.y));

            if (max_d >= min_d)
            {
                for (int nr_rooms = 0; nr_rooms < MAX_NR_INNER_ROOMS; nr_rooms++)
                {
                    for (int try_count = 0; try_count < NR_TRIES_TO_MK_ROOM; try_count++)
                    {
                        const Pos d(rnd::range(min_d.x, max_d.x),
                                    rnd::range(min_d.y, max_d.y));

                        const Pos p0(rnd::range(outer_room_rect.p0.x,
                                                outer_room_rect.p1.x - d.x + 1),
                                     rnd::range(outer_room_rect.p0.y,
                                                outer_room_rect.p1.y - d.y + 1));

                        const Pos p1(p0 + d - 1);

                        const Rect r(p0, p1);

                        if (r.p0 == outer_room_rect.p0 && r.p1 == outer_room_rect.p1)
                        {
                            continue;
                        }

                        bool is_area_free = true;

                        for (int x = p0.x - 1; x <= p1.x + 1; ++x)
                        {
                            for (int y = p0.y - 1; y <= p1.y + 1; ++y)
                            {
                                const Pos p_check(x, y);

                                const auto& f_id = map::cells[x][y].rigid->id();

                                if (utils::is_pos_inside(p_check, outer_room_rect))
                                {
                                    if (f_id != Feature_id::floor) {is_area_free = false;}
                                }
                                else
                                {
                                    if (f_id != Feature_id::wall)  {is_area_free = false;}
                                }

                                if (!is_area_free) {break;}
                            }

                            if (!is_area_free) {break;}
                        }

                        if (!is_area_free)
                        {
                            continue;
                        }

                        const Rect room_rect(r.p0 + 1, r.p1 - 1);

                        Room* const room =
                            room_factory::mk_random_allowed_std_room(room_rect, true);
                        register_room(*room);

                        outer_room->sub_rooms_.push_back(room);

                        vector<Pos> door_bucket;

                        for (int y = p0.y; y <= p1.y; ++y)
                        {
                            for (int x = p0.x; x <= p1.x; ++x)
                            {
                                if (x == p0.x || x == p1.x || y == p0.y || y == p1.y)
                                {
                                    map::put(new Wall(Pos(x, y)));

                                    if (
                                        x != outer_room_rect.p0.x - 1 &&
                                        x != outer_room_rect.p0.x     &&
                                        x != outer_room_rect.p1.x     &&
                                        x != outer_room_rect.p1.x + 1 &&
                                        y != outer_room_rect.p0.y - 1 &&
                                        y != outer_room_rect.p0.y     &&
                                        y != outer_room_rect.p1.y     &&
                                        y != outer_room_rect.p1.y + 1)
                                    {
                                        if (
                                            (x != p0.x && x != p1.x) ||
                                            (y != p0.y && y != p1.y))
                                        {
                                            door_bucket.push_back(Pos(x, y));
                                        }
                                    }
                                }
                            }
                        }

                        if (rnd::coin_toss() || door_bucket.size() <= 2)
                        {
                            const int DOOR_POS_IDX =
                                rnd::range(0, door_bucket.size() - 1);
                            const Pos& door_pos = door_bucket[DOOR_POS_IDX];

                            map::put(new Floor(door_pos));
                            door_proposals[door_pos.x][door_pos.y] = true;
                        }
                        else
                        {
                            vector<Pos> positions_with_door;
                            const int NR_TRIES = rnd::range(1, 10);

                            for (int j = 0; j < NR_TRIES; j++)
                            {
                                const int DOOR_POS_IDX =
                                    rnd::range(0, door_bucket.size() - 1);
                                const Pos pos_cand = door_bucket[DOOR_POS_IDX];

                                bool is_pos_ok = true;

                                for (Pos& pos_with_door : positions_with_door)
                                {
                                    if (utils::is_pos_adj(pos_cand, pos_with_door, false))
                                    {
                                        is_pos_ok = false;
                                        break;
                                    }
                                }

                                if (is_pos_ok)
                                {
                                    map::put(new Floor(pos_cand));
                                    positions_with_door.push_back(pos_cand);
                                }
                            }
                        }

                        break;
                    }
                }
            }
        }
    }

    TRACE_FUNC_END;
}

void fill_dead_ends()
{
    //Find an origin with no adjacent walls, to ensure not starting in a dead end
    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    bool expanded_blockers[MAP_W][MAP_H];
    map_parse::expand(blocked, expanded_blockers);

    Pos origin;
    bool is_done = false;

    for (int y = 2; y < MAP_H - 2; ++y)
    {
        for (int x = 2; x < MAP_W - 2; ++x)
        {
            if (!expanded_blockers[x][y])
            {
                origin = Pos(x, y);
                is_done = true;
                break;
            }
        }

        if (is_done) {break;}
    }

    //Floodfill from origin, then sort the positions for flood value
    int flood_fill[MAP_W][MAP_H];
    flood_fill::run(origin, blocked, flood_fill, INT_MAX, Pos(-1, -1), true);
    vector<Pos_and_val> flood_fill_vector;

    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            if (!blocked[x][y])
            {
                flood_fill_vector.push_back(Pos_and_val(Pos(x, y), flood_fill[x][y]));
            }
        }
    }

    std::sort(flood_fill_vector.begin(), flood_fill_vector.end(),
    [](const Pos_and_val & a, const Pos_and_val & b) {return a.val < b.val;});

    //Fill all positions with only one cardinal floor neighbour
    for (int i = int(flood_fill_vector.size()) - 1; i >= 0; --i)
    {
        const Pos& pos = flood_fill_vector[i].pos;
        const int x = pos.x;
        const int y = pos.y;
        const int NR_ADJ_CARDINAL_WALLS = blocked[x + 1][y] + blocked[x - 1][y] +
                                          blocked[x][y + 1] + blocked[x][y - 1];

        if (NR_ADJ_CARDINAL_WALLS == 3)
        {
            map::put(new Wall(pos));
            blocked[x][y] = true;
        }
    }
}

void decorate()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Cell& cell = map::cells[x][y];

            if (cell.rigid->id() == Feature_id::wall)
            {
                //Randomly convert walls to rubble
                if (rnd::one_in(10))
                {
                    map::put(new Rubble_high(Pos(x, y)));
                    continue;
                }

                //Moss grown walls
                Wall* const wall = static_cast<Wall*>(cell.rigid);
                wall->set_random_is_moss_grown();

                //Convert some walls to cave
                bool should_convert_to_cave_wall = false;

                if (map::dlvl >= DLVL_FIRST_LATE_GAME)
                {
                    //If this is late game - convert all walls to cave
                    should_convert_to_cave_wall = true;
                }
                else //Not late game
                {
                    //Convert walls with no adjacent floor or with adjacent cave floor
                    //to cave
                    bool has_adj_floor      = false;
                    bool has_adj_cave_floor  = false;

                    for (const Pos& d : dir_utils::dir_list)
                    {
                        const Pos p_adj(Pos(x, y) + d);

                        if (utils::is_pos_inside_map(p_adj))
                        {
                            auto& adj_cell = map::cells[p_adj.x][p_adj.y];

                            const auto adj_id = adj_cell.rigid->id();

                            //TODO: Traps count as floor here - otherwise walls that are
                            //only adjacent to traps (and not to any "real" floor) would
                            //be converted to cave walls, which would spoil the presence
                            //of the trap for the player, and just be weird in general.
                            //This works for now, but it should probably be handled
                            //better in the future. Currently, traps are the only rigid
                            //that can "mimic" floor, but if some other feature like that
                            //is added, it could be a problem.

                            if (adj_id == Feature_id::floor || adj_id == Feature_id::trap)
                            {
                                has_adj_floor = true;

                                auto* adj_rigid = static_cast<Floor*>(adj_cell.rigid);

                                if (adj_rigid->type_ == Floor_type::cave)
                                {
                                    has_adj_cave_floor = true;
                                    break;
                                }
                            }
                        }
                    }

                    should_convert_to_cave_wall = !has_adj_floor || has_adj_cave_floor;
                }

                if (should_convert_to_cave_wall)
                {
                    wall->type_ = Wall_type::cave;
                }
                else
                {
                    wall->set_rnd_cmn_wall();
                }
            }
        }
    }

    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            if (map::cells[x][y].rigid->id() == Feature_id::floor)
            {
                //Randomly convert stone floor to low rubble
                if (rnd::one_in(100))
                {
                    map::put(new Rubble_low(Pos(x, y)));
                    continue;
                }
            }
        }
    }
}

void allowed_stair_cells(bool out[MAP_W][MAP_H])
{
    TRACE_FUNC_BEGIN;

    //Mark cells as free if all adjacent feature types are allowed
    vector<Feature_id> feat_ids_ok {Feature_id::floor, Feature_id::carpet, Feature_id::grass};

    map_parse::run(cell_check::All_adj_is_any_of_features(feat_ids_ok), out);

    //Block cells with item
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (map::cells[x][y].item)
            {
                out[x][y] = false;
            }
        }
    }

    //Block cells with actors
    for (const auto* const actor : game_time::actors_)
    {
        const Pos& p(actor->pos);
        out[p.x][p.y] = false;
    }

    TRACE_FUNC_END;
}

Pos place_stairs()
{
    TRACE_FUNC_BEGIN;

    bool allowed_cells[MAP_W][MAP_H];
    allowed_stair_cells(allowed_cells);

    vector<Pos> allowed_cells_list;
    utils::mk_vector_from_bool_map(true, allowed_cells, allowed_cells_list);

    const int NR_OK_CELLS = allowed_cells_list.size();

    const int MIN_NR_OK_CELLS_REQ = 3;

    if (NR_OK_CELLS < MIN_NR_OK_CELLS_REQ)
    {
        TRACE << "Nr available cells to place stairs too low "
              << "(" << NR_OK_CELLS << "), discarding map" << endl;
        is_map_valid = false;
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("To few cells to place stairs", Panel::screen, {0, 0},
                          clr_red_lgt);
        render::update_screen();
        sdl_wrapper::sleep(8000);
#endif // DEMO_MODE
        return Pos(-1, -1);
    }

    TRACE << "Sorting the allowed cells vector "
          << "(" << allowed_cells_list.size() << " cells)" << endl;
    Is_closer_to_pos is_closer_to_origin(map::player->pos);
    sort(allowed_cells_list.begin(), allowed_cells_list.end(), is_closer_to_origin);

    TRACE << "Picking furthest cell" << endl;
    const Pos stairs_pos(allowed_cells_list[NR_OK_CELLS - 1]);

    TRACE << "Spawning stairs at chosen cell" << endl;
    map::put(new Stairs(stairs_pos));

    TRACE_FUNC_END;
    return stairs_pos;
}

void move_player_to_nearest_allowed_pos()
{
    TRACE_FUNC_BEGIN;

    bool allowed_cells[MAP_W][MAP_H];
    allowed_stair_cells(allowed_cells);

    vector<Pos> allowed_cells_list;
    utils::mk_vector_from_bool_map(true, allowed_cells, allowed_cells_list);

    if (allowed_cells_list.empty())
    {
        is_map_valid = false;
    }
    else
    {
        TRACE << "Sorting the allowed cells vector "
              << "(" << allowed_cells_list.size() << " cells)" << endl;
        Is_closer_to_pos is_closer_to_origin(map::player->pos);
        sort(allowed_cells_list.begin(), allowed_cells_list.end(), is_closer_to_origin);

        map::player->pos = allowed_cells_list.front();

    }

    TRACE_FUNC_END;
}

//void mk_levers() {
//  TRACE_FUNC_BEGIN;
//
//  TRACE << "Picking a random door" << endl;
//  vector<Door*> door_bucket;
//  for(int y = 1; y < MAP_H - 1; ++y) {
//    for(int x = 1; x < MAP_W - 1; ++x) {
//      Feature* const feature = map::features_static[x][y];
//      if(feature->id() == Feature_id::door) {
//        Door* const door = static_cast<Door*>(feature);
//        door_bucket.push_back(door);
//      }
//    }
//  }
//  Door* const door_to_link = door_bucket[rnd::range(0, door_bucket.size() - 1)];
//
//  TRACE << "Making floodfill and keeping only positions with lower value than the door" << endl;
//  bool blocked[MAP_W][MAP_H];
//  eng.map_tests->mk_move_blocker_array_for_body_type_features_only(body_type_normal, blocked);
//  for(int y = 1; y < MAP_H - 1; ++y) {
//    for(int x = 1; x < MAP_W - 1; ++x) {
//      Feature* const feature = map::features_static[x][y];
//      if(feature->id() == Feature_id::door) {
//        blocked[x][y] = false;
//      }
//    }
//  }
//  int flood_fill[MAP_W][MAP_H];
//  flood_fill::run(map::player->pos, blocked, flood_fill, INT_MAX, Pos(-1, -1));
//  const int FLOOD_VALUE_AT_DOOR = flood_fill[door_to_link->pos_.x][door_to_link->pos_.y];
//  vector<Pos> lever_pos_bucket;
//  for(int y = 1; y < MAP_H - 1; ++y) {
//    for(int x = 1; x < MAP_W - 1; ++x) {
//      if(flood_fill[x][y] < FLOOD_VALUE_AT_DOOR) {
//        if(map::features_static[x][y]->can_have_rigid()) {
//          lever_pos_bucket.push_back(Pos(x, y));
//        }
//      }
//    }
//  }
//
//  if(lever_pos_bucket.size() > 0) {
//    const int IDX = rnd::range(0, lever_pos_bucket.size() - 1);
//    const Pos lever_pos(lever_pos_bucket[IDX]);
//    spawn_lever_adapt_and_link_door(lever_pos, *door_to_link);
//  } else {
//    TRACE << "Failed to find position to place lever" << endl;
//  }
//  TRACE_FUNC_END;
//}

//void spawn_lever_adapt_and_link_door(const Pos& lever_pos, Door& door) {
//  TRACE << "Spawning lever and linking it to the door" << endl;
//  Feature_factory::mk(Feature_id::lever, lever_pos, new Lever_spawn_data(&door));
//
//  TRACE << "Changing door properties" << endl;
//  door.matl_ = Door_type::metal;
//  door.is_open_ = false;
//  door.is_stuck_ = false;
//  door.is_handled_externally_ = true;
//}

void reveal_doors_on_path_to_stairs(const Pos& stairs_pos)
{
    TRACE_FUNC_BEGIN;

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    blocked[stairs_pos.x][stairs_pos.y] = false;

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (map::cells[x][y].rigid->id() == Feature_id::door)
            {
                blocked[x][y] = false;
            }
        }
    }

    vector<Pos> path;
    path_find::run(map::player->pos, stairs_pos, blocked, path);

    assert(!path.empty());

    TRACE << "Travelling along path and revealing all doors" << endl;

    for (Pos& pos : path)
    {
        auto* const feature = map::cells[pos.x][pos.y].rigid;

        if (feature->id() == Feature_id::door)
        {
            static_cast<Door*>(feature)->reveal(false);
        }
    }

    TRACE_FUNC_END;
}

} //namespace

bool mk_std_lvl()
{
    TRACE_FUNC_BEGIN;

    is_map_valid = true;

    render::clear_screen();
    render::update_screen();

    map::reset_map();

    TRACE << "Resetting helper arrays" << endl;

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            door_proposals[x][y] = false;
        }
    }

    //NOTE: This must be called before any rooms are created
    room_factory::init_room_bucket();

    TRACE << "Init regions" << endl;
    const int MAP_W_THIRD = MAP_W / 3;
    const int MAP_H_THIRD = MAP_H / 3;
    const int SPL_X0      = MAP_W_THIRD;
    const int SPL_X1      = (MAP_W_THIRD * 2) + 1;
    const int SPL_Y0      = MAP_H_THIRD;
    const int SPL_Y1      = MAP_H_THIRD * 2;

    Region regions[3][3];

    for (int y = 0; y < 3; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            const Rect r(x == 0 ? 1 : x == 1 ? SPL_X0 + 1 : SPL_X1 + 1,
                         y == 0 ? 1 : y == 1 ? SPL_Y0 + 1 : SPL_Y1 + 1,
                         x == 0 ? SPL_X0 - 1 : x == 1 ? SPL_X1 - 1 : MAP_W - 2,
                         y == 0 ? SPL_Y0 - 1 : y == 1 ? SPL_Y1 - 1 : MAP_H - 2);

            regions[x][y] = Region(r);
        }
    }

#ifdef MK_RIVER
    const int RIVER_ONE_IN_N = 8;

    if (is_map_valid && map::dlvl >= DLVL_FIRST_MID_GAME && rnd::one_in(RIVER_ONE_IN_N))
    {
        reserve_river(regions);
    }
#endif // MK_RIVER

#ifdef MK_MERGED_REGIONS
    if (is_map_valid)
    {
        mk_merged_regions_and_rooms(regions);
    }
#endif // MK_MERGED_REGIONS

#ifdef RANDOMLY_BLOCK_REGIONS
    if (is_map_valid)
    {
        randomly_block_regions(regions);
    }
#endif // RANDOMLY_BLOCK_REGIONS

    if (is_map_valid)
    {
        TRACE << "Making main rooms" << endl;

        for (int x = 0; x < 3; ++x)
        {
            for (int y = 0; y < 3; ++y)
            {
                auto& region = regions[x][y];

                if (!region.main_room_ && region.is_free_)
                {
                    const Rect room_rect = region.rnd_room_rect();
                    auto* room = room_factory::mk_random_allowed_std_room(room_rect, false);
                    register_room(*room);
                    mk_floor_in_room(*room);
                    region.main_room_   = room;
                    region.is_free_     = false;
                }
            }
        }
    }

#ifdef MK_AUX_ROOMS
#ifdef DEMO_MODE
    render::cover_panel(Panel::log);
    render::draw_map();
    render::draw_text("Press any key to make aux rooms...", Panel::screen, {0, 0},
                      clr_white);
    render::update_screen();
    query::wait_for_key_press();
#endif // DEMO_MODE
    if (is_map_valid)
    {
        mk_aux_rooms(regions);
    }
#endif // MK_AUX_ROOMS

#ifdef MK_SUB_ROOMS
    if (is_map_valid && map::dlvl <= DLVL_LAST_MID_GAME)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to make sub rooms...", Panel::screen, {0, 0},
                          clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif // DEMO_MODE
        mk_sub_rooms();
    }
#endif // MK_SUB_ROOMS

    if (is_map_valid)
    {
        TRACE << "Sorting the room list according to room type" << endl;
        //NOTE: This allows common rooms to assume that they are rectangular and
        //have their walls untouched when their reshaping functions run.
        auto cmp = [](const Room * r0, const Room * r1)
        {
            return int(r0->type_) < int(r1->type_);
        };
        sort(map::room_list.begin(), map::room_list.end(), cmp);
    }

    TRACE << "Running pre-connect functions for all rooms" << endl;

    if (is_map_valid)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to run pre-connect functions on rooms...",
                          Panel::screen, {0, 0}, clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif // DEMO_MODE

        gods::set_no_god();

        for (Room* room : map::room_list)
        {
            room->on_pre_connect(door_proposals);
        }
    }

    //Connect
    if (is_map_valid)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to connect rooms...", Panel::screen, {0, 0},
                          clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif // DEMO_MODE
        connect_rooms();
    }

    TRACE << "Running post-connect functions for all rooms" << endl;

    if (is_map_valid)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to run post-connect functions on rooms...",
                          Panel::screen, {0, 0}, clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif // DEMO_MODE

        for (Room* room : map::room_list)
        {
            room->on_post_connect(door_proposals);
        }
    }

#ifdef FILL_DEAD_ENDS
    if (is_map_valid)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to fill dead ends...", Panel::screen, {0, 0},
                          clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif // DEMO_MODE
        fill_dead_ends();
    }
#endif // FILL_DEAD_ENDS

    if (is_map_valid && map::dlvl <= DLVL_LAST_MID_GAME)
    {
        TRACE << "Placing doors" << endl;

        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                if (door_proposals[x][y] && rnd::fraction(7, 10))
                {
                    place_door_at_pos_if_allowed(Pos(x, y));
                }
            }
        }
    }

    if (is_map_valid)
    {
        move_player_to_nearest_allowed_pos();
    }

#ifdef DECORATE
    if (is_map_valid)
    {
        decorate();
    }
#endif // DECORATE

    if (is_map_valid)
    {
        populate_mon::populate_std_lvl();
    }

    if (is_map_valid)
    {
        populate_traps::populate_std_lvl();
    }

    if (is_map_valid)
    {
        populate_items::mk_items_on_floor();
    }

    Pos stairs_pos;

    if (is_map_valid)
    {
        stairs_pos = place_stairs();
    }

    if (is_map_valid)
    {
        const int LAST_LVL_TO_REVEAL_STAIRS_PATH = 9;

        if (map::dlvl <= LAST_LVL_TO_REVEAL_STAIRS_PATH)
        {
            reveal_doors_on_path_to_stairs(stairs_pos);
        }
    }

    for (auto* r : map::room_list)
    {
        delete r;
    }

    map::room_list.clear();
    utils::reset_array(map::room_map);

    TRACE_FUNC_END;
    return is_map_valid;
}

} //map_gen

//=============================================================== REGION
Rect Region::rnd_room_rect() const
{
    const bool ALLOW_TINY_W = rnd::coin_toss();

    const Pos min_size_lmt(ALLOW_TINY_W ? 2 : 4, ALLOW_TINY_W ? 4 : 2);
    const Pos max_size_lmt(r_.p1 - r_.p0 + 1);

    const int   H           = rnd::range(min_size_lmt.y, max_size_lmt.y);
    const int   W_MAX_SMALL = min_size_lmt.x + ((max_size_lmt.x - min_size_lmt.x) / 5);
    const int   W_MAX_BIG   = max_size_lmt.x;
    const bool  ALLOW_BIG_W = H > (max_size_lmt.y * 5) / 6;
    const int   W_MAX       = ALLOW_BIG_W ? W_MAX_BIG : W_MAX_SMALL;
    const int   W           = rnd::range(min_size_lmt.x, W_MAX);

    const Pos p0(r_.p0.x + rnd::range(0, max_size_lmt.x - W),
                 r_.p0.y + rnd::range(0, max_size_lmt.y - H));

    const Pos p1(p0.x + W - 1, p0.y + H - 1);

    return Rect(p0, p1);
}
