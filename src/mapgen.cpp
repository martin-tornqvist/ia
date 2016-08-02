#include "mapgen.hpp"

#include "init.hpp"

#include <algorithm>
#include <climits>

#include "room.hpp"
#include "mapgen.hpp"
#include "feature_event.hpp"
#include "actor_player.hpp"
#include "feature_door.hpp"
#include "feature_monolith.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "drop.hpp"
#include "item_factory.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "render.hpp"
#include "populate_monsters.hpp"
#include "populate_traps.hpp"
#include "populate_items.hpp"
#include "gods.hpp"
#include "rl_utils.hpp"

#ifdef DEMO_MODE
#include "render.hpp"
#include "sdl_wrapper.hpp"
#include "query.hpp"
#endif //DEMO_MODE

namespace mapgen
{

bool door_proposals[map_w][map_h];

namespace
{

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
            mapgen::is_map_valid = false;
#ifdef DEMO_MODE
            render::cover_panel(Panel::log);
            render::draw_text("Failed to connect map",
                              Panel::screen,
                              P(0, 0),
                              clr_red_lgt);
            render::update_screen();
            sdl_wrapper::sleep(8000);
#endif //DEMO_MODE
            break;
        }

        auto rnd_room = []()
        {
            return map::room_list[rnd::range(0, map::room_list.size() - 1)];
        };

        auto is_std_room = [](const Room & r)
        {
            return (int)r.type_ < (int)RoomType::END_OF_STD_ROOMS;
        };

        Room* room0 = rnd_room();

        //Room 0 must be a standard room or corridor link
        if (
            !is_std_room(*room0) &&
            room0->type_ != RoomType::corr_link)
        {
            continue;
        }

        //Finding second room to connect to
        Room* room1 = rnd_room();

        //Room 1 must not be the same as room 0, and it must be a standard room
        //(connections are only allowed between two standard rooms, or from a
        //corridor link to a standard room - never between two corridor links)
        while (
            room1 == room0 ||
            !is_std_room(*room1))
        {
            room1 = rnd_room();
        }

        //Do not allow two rooms to be connected twice
        const auto& room0_connections = room0->rooms_con_to_;

        if (
            find(room0_connections.begin(),
                 room0_connections.end(),
                 room1) != room0_connections.end())
        {
            //Rooms are already connected, trying other combination
            continue;
        }

        //Do not connect room 0 and 1 if another room (except for sub rooms)
        //lies anywhere in a rectangle defined by the two center points of
        //those rooms.
        bool is_other_room_in_way = false;

        const P c0(room0->r_.center());
        const P c1(room1->r_.center());

        const int X0 = std::min(c0.x, c1.x);
        const int Y0 = std::min(c0.y, c1.y);
        const int X1 = std::max(c0.x, c1.x);
        const int Y1 = std::max(c0.y, c1.y);

        for (int x = X0; x <= X1; ++x)
        {
            for (int y = Y0; y <= Y1; ++y)
            {
                const Room* const room_here = map::room_map[x][y];

                if (
                    room_here           &&
                    room_here != room0  &&
                    room_here != room1  &&
                    !room_here->is_sub_room_)
                {
                    is_other_room_in_way = true;
                    break;
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

        //Alright, let's try to connect these rooms
        mk_pathfind_corridor(*room0,
                             *room1,
                             door_proposals);

        if (
            (nr_tries_left <= 2 || rnd::one_in(4)) &&
            is_all_rooms_connected())
        {
            break;
        }
    }

    TRACE_FUNC_END;
}

void try_place_door(const P& p)
{
    //Check that no other doors are within a certain distance
    const int r = 2;

    for (int dx = -r; dx <= r; ++dx)
    {
        for (int dy = -r; dy <= r; ++dy)
        {
            const P check_pos = p + P(dx, dy);

            if ((dx != 0 || dy != 0) && map::is_pos_inside_map(check_pos))
            {
                const Cell& cell = map::cells[check_pos.x][check_pos.y];

                if (cell.rigid->id() == FeatureId::door)
                {
                    return;
                }
            }
        }
    }

    bool is_good_ver = true;
    bool is_good_hor = true;

    for (int d = -1; d <= 1; d++)
    {
        if (map::cells[p.x + d][p.y].rigid->id() == FeatureId::wall)
        {
            is_good_hor = false;
        }

        if (map::cells[p.x][p.y + d].rigid->id() == FeatureId::wall)
        {
            is_good_ver = false;
        }

        if (d != 0)
        {
            if (map::cells[p.x][p.y + d].rigid->id() != FeatureId::wall)
            {
                is_good_hor = false;
            }

            if (map::cells[p.x + d][p.y].rigid->id() != FeatureId::wall)
            {
                is_good_ver = false;
            }
        }
    }

    if (is_good_hor || is_good_ver)
    {
        const auto& d = feature_data::data(FeatureId::wall);
        const auto* const mimic = static_cast<const Rigid*>(d.mk_obj(p));
        map::put(new Door(p, mimic));
    }
}

void allowed_stair_cells(bool out[map_w][map_h])
{
    TRACE_FUNC_BEGIN;

    //Mark cells as free if all adjacent feature types are allowed
    std::vector<FeatureId> feat_ids_ok
    {
        FeatureId::floor,
        FeatureId::carpet,
        FeatureId::grass
    };

    map_parse::run(cell_check::AllAdjIsAnyOfFeatures(feat_ids_ok), out);

    //Block cells with item
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (map::cells[x][y].item)
            {
                out[x][y] = false;
            }
        }
    }

    //Block cells with actors
    for (const auto* const actor : game_time::actors)
    {
        const P& p(actor->pos);
        out[p.x][p.y] = false;
    }

    TRACE_FUNC_END;
}

P place_stairs()
{
    TRACE_FUNC_BEGIN;

    bool allowed_cells[map_w][map_h];
    allowed_stair_cells(allowed_cells);

    std::vector<P> allowed_cells_list;

    to_vec(allowed_cells,
           true,
           allowed_cells_list);

    const int nr_ok_cells = allowed_cells_list.size();

    const int min_nr_ok_cells_req = 3;

    if (nr_ok_cells < min_nr_ok_cells_req)
    {
        TRACE << "Nr available cells to place stairs too low "
              << "(" << nr_ok_cells << "), discarding map" << std:: endl;
        is_map_valid = false;
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("To few cells to place stairs",
                          Panel::screen,
                          P(0, 0),
                          clr_red_lgt);
        render::update_screen();
        sdl_wrapper::sleep(8000);
#endif //DEMO_MODE
        return P(-1, -1);
    }

    TRACE << "Sorting the allowed cells vector "
          << "(" << allowed_cells_list.size() << " cells)" << std:: endl;

    IsCloserToPos is_closer_to_origin(map::player->pos);

    sort(allowed_cells_list.begin(),
         allowed_cells_list.end(),
         is_closer_to_origin);

    TRACE << "Picking furthest cell" << std:: endl;
    const P stairs_pos(allowed_cells_list[nr_ok_cells - 1]);

    TRACE << "Spawning stairs at chosen cell" << std:: endl;
    map::put(new Stairs(stairs_pos));

    TRACE_FUNC_END;
    return stairs_pos;
}

void move_player_to_nearest_allowed_pos()
{
    TRACE_FUNC_BEGIN;

    bool allowed_cells[map_w][map_h];
    allowed_stair_cells(allowed_cells);

    std::vector<P> allowed_cells_list;

    to_vec(allowed_cells,
           true,
           allowed_cells_list);

    if (allowed_cells_list.empty())
    {
        is_map_valid = false;
    }
    else //Valid cells exists
    {
        TRACE << "Sorting the allowed cells vector "
              << "(" << allowed_cells_list.size() << " cells)" << std:: endl;

        IsCloserToPos is_closer_to_origin(map::player->pos);

        sort(allowed_cells_list.begin(),
             allowed_cells_list.end(),
             is_closer_to_origin);

        map::player->pos = allowed_cells_list.front();

    }

    TRACE_FUNC_END;
}

void place_monoliths()
{
    int nr_monoliths = 1;

    //Guarantee exactly one monolith on level 1
    if (map::dlvl > 1)
    {
        //Determine number of Monoliths to place, by a weighted choice
        std::vector<int> nr_weights =
        {
            5,  //0 monolith(s)
            50, //1 -
            5,  //2 -
            1   //3 -
        };

        nr_monoliths = rnd::weighted_choice(nr_weights);
    }

    bool blocked[map_w][map_h] = {};

    map_parse::run(cell_check::BlocksRigid(), blocked);

    bool blocked_expanded[map_w][map_h];

    map_parse::expand(blocked, blocked_expanded);

    for (Actor* const actor : game_time::actors)
    {
        blocked_expanded[actor->pos.x][actor->pos.y] = true;
    }

    std::vector<P> p_bucket;

    for (int i = 0; i < nr_monoliths; ++i)
    {
        to_vec(blocked_expanded,
               false,
               p_bucket);

        ASSERT(!p_bucket.empty());

        //Robustness for release mode
        if (p_bucket.empty())
        {
            return;
        }

        const P& p = rnd::element(p_bucket);

        map::cells[p.x][p.y].rigid = new Monolith(p);

        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P adj_p(p + d);

            blocked_expanded[adj_p.x][adj_p.y] = true;
        }
    }
}

//void mk_levers() {
//  TRACE_FUNC_BEGIN;
//
//  TRACE << "Picking a random door" << std:: endl;
//  vector<Door*> door_bucket;
//  for(int y = 1; y < map_h - 1; ++y) {
//    for(int x = 1; x < map_w - 1; ++x) {
//      Feature* const feature = map::features_static[x][y];
//      if(feature->id() == FeatureId::door) {
//        Door* const door = static_cast<Door*>(feature);
//        door_bucket.push_back(door);
//      }
//    }
//  }
//  Door* const door_to_link = door_bucket[rnd::range(0, door_bucket.size() - 1)];
//
//  TRACE << "Making floodfill and keeping only positions with lower value than the door" << std:: endl;
//  bool blocked[map_w][map_h];
//  eng.map_tests->mk_move_blocker_array_for_body_type_features_only(body_type_normal, blocked);
//  for(int y = 1; y < map_h - 1; ++y) {
//    for(int x = 1; x < map_w - 1; ++x) {
//      Feature* const feature = map::features_static[x][y];
//      if(feature->id() == FeatureId::door) {
//        blocked[x][y] = false;
//      }
//    }
//  }
//  int floodfill[map_w][map_h];
//  floodfill::run(map::player->pos, blocked, floodfill, INT_MAX, P(-1, -1));
//  const int flood_value_at_door = floodfill[door_to_link->pos_.x][door_to_link->pos_.y];
//  vector<P> lever_pos_bucket;
//  for(int y = 1; y < map_h - 1; ++y) {
//    for(int x = 1; x < map_w - 1; ++x) {
//      if(floodfill[x][y] < flood_value_at_door) {
//        if(map::features_static[x][y]->can_have_rigid()) {
//          lever_pos_bucket.push_back(P(x, y));
//        }
//      }
//    }
//  }
//
//  if(lever_pos_bucket.size() > 0) {
//    const int idx = rnd::range(0, lever_pos_bucket.size() - 1);
//    const P lever_pos(lever_pos_bucket[idx]);
//    spawn_lever_adapt_and_link_door(lever_pos, *door_to_link);
//  } else {
//    TRACE << "Failed to find position to place lever" << std:: endl;
//  }
//  TRACE_FUNC_END;
//}

//void spawn_lever_adapt_and_link_door(const P& lever_pos, Door& door) {
//  TRACE << "Spawning lever and linking it to the door" << std:: endl;
//  FeatureFactory::mk(FeatureId::lever, lever_pos, new LeverSpawnData(&door));
//
//  TRACE << "Changing door properties" << std:: endl;
//  door.matl_ = DoorType::metal;
//  door.is_open_ = false;
//  door.is_stuck_ = false;
//  door.is_handled_externally_ = true;
//}

void reveal_doors_on_path_to_stairs(const P& stairs_pos)
{
    TRACE_FUNC_BEGIN;

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::BlocksMoveCmn(false), blocked);

    blocked[stairs_pos.x][stairs_pos.y] = false;

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

    std::vector<P> path;
    pathfind::run(map::player->pos, stairs_pos, blocked, path);

    ASSERT(!path.empty());

    TRACE << "Travelling along path and revealing all doors" << std:: endl;

    for (P& pos : path)
    {
        auto* const feature = map::cells[pos.x][pos.y].rigid;

        if (feature->id() == FeatureId::door)
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

    TRACE << "Resetting helper arrays" << std:: endl;

    std::fill_n(*door_proposals, nr_map_cells, false);

    //NOTE: This must be called before any rooms are created
    room_factory::init_room_bucket();

    TRACE << "Init regions" << std:: endl;
    const int map_w_third = map_w / 3;
    const int map_h_third = map_h / 3;
    const int SPL_X0      = map_w_third;
    const int SPL_X1      = (map_w_third * 2) + 1;
    const int SPL_Y0      = map_h_third;
    const int SPL_Y1      = map_h_third * 2;

    Region regions[3][3];

    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
             const R r(x == 0 ? 1 : x == 1 ? SPL_X0 + 1 : SPL_X1 + 1,
                      y == 0 ? 1 : y == 1 ? SPL_Y0 + 1 : SPL_Y1 + 1,
                      x == 0 ? SPL_X0 - 1 : x == 1 ? SPL_X1 - 1 : map_w - 2,
                      y == 0 ? SPL_Y0 - 1 : y == 1 ? SPL_Y1 - 1 : map_h - 2);

            regions[x][y] = Region(r);
        }
    }

#ifndef DISABLE_MK_RIVER
    const int river_one_in_n = 8;

    if (
        is_map_valid                        &&
        map::dlvl >= dlvl_first_mid_game    &&
        rnd::one_in(river_one_in_n))
    {
        reserve_river(regions);
    }
#endif //MK_RIVER

#ifndef DISABLE_MERGED_REGIONS
    if (is_map_valid)
    {
        mk_merged_regions_and_rooms(regions);
    }
#endif //MK_MERGED_REGIONS

#ifndef DISABLE_RANDOMLY_BLOCK_REGIONS
    if (is_map_valid)
    {
        randomly_block_regions(regions);
    }
#endif //RANDOMLY_BLOCK_REGIONS

    if (is_map_valid)
    {
        TRACE << "Making main rooms" << std:: endl;

        for (int x = 0; x < 3; ++x)
        {
            for (int y = 0; y < 3; ++y)
            {
                auto& region = regions[x][y];

                if (!region.main_room && region.is_free)
                {
                    const R room_rect = region.rnd_room_rect();
                    auto* room = room_factory::mk_random_allowed_std_room(room_rect, false);
                    register_room(*room);
                    mk_floor_in_room(*room);
                    region.main_room    = room;
                    region.is_free      = false;
                }
            }
        }
    }

#ifndef DISABLE_AUX_ROOMS
#ifdef DEMO_MODE
    render::cover_panel(Panel::log);
    render::draw_map();
    render::draw_text("Press any key to make aux rooms...",
                      Panel::screen,
                      P(0, 0),
                      clr_white);
    render::update_screen();
    query::wait_for_key_press();
#endif //DEMO_MODE
    if (is_map_valid)
    {
        mk_aux_rooms(regions);
    }
#endif //DISABLE_AUX_ROOMS

#ifndef DISABLE_MK_SUB_ROOMS
    if (is_map_valid && map::dlvl <= dlvl_last_mid_game)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to make sub rooms...",
                          Panel::screen,
                          P(0, 0),
                          clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif //DEMO_MODE
        mk_sub_rooms();
    }
#endif //DISABLE_MK_SUB_ROOMS

    if (is_map_valid)
    {
        TRACE << "Sorting the room list according to room type" << std:: endl;
        //NOTE: This allows common rooms to assume that they are rectangular and
        //have their walls untouched when their reshaping functions run.
        auto cmp = [](const Room * r0, const Room * r1)
        {
            return (int)r0->type_ < (int)r1->type_;
        };
        sort(map::room_list.begin(), map::room_list.end(), cmp);
    }

    TRACE << "Running pre-connect functions for all rooms" << std:: endl;

    if (is_map_valid)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to run pre-connect functions on rooms...",
                          Panel::screen,
                          P(0, 0),
                          clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif //DEMO_MODE

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
        render::draw_text("Press any key to connect rooms...",
                          Panel::screen,
                          P(0, 0),
                          clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif //DEMO_MODE
        connect_rooms();
    }

    TRACE << "Running post-connect functions for all rooms" << std:: endl;

    if (is_map_valid)
    {
#ifdef DEMO_MODE
        render::cover_panel(Panel::log);
        render::draw_map();
        render::draw_text("Press any key to run post-connect functions on rooms...",
                          Panel::screen,
                          P(0, 0),
                          clr_white);
        render::update_screen();
        query::wait_for_key_press();
#endif //DEMO_MODE

        for (Room* room : map::room_list)
        {
            room->on_post_connect(door_proposals);
        }
    }

    if (is_map_valid && map::dlvl <= dlvl_last_mid_game)
    {
        TRACE << "Placing doors" << std:: endl;

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                if (
                    door_proposals[x][y] &&
                    rnd::fraction(4, 5))
                {
                    try_place_door(P(x, y));
                }
            }
        }
    }

    if (is_map_valid)
    {
        move_player_to_nearest_allowed_pos();
    }

#ifndef DISABLE_DECORATE
    if (is_map_valid)
    {
        decorate();
    }
#endif //DISABLE_DECORATE

    //NOTE: The choke point data below depends on the stairs being placed, so
    //      we need to do this first.
    P stairs_pos;

    if (is_map_valid)
    {
        stairs_pos = place_stairs();
    }

    //Gather data on choke points in the map (check every position where a door
    //has previously been "proposed")
    if (is_map_valid)
    {
        bool blocked[map_w][map_h];
    
        map_parse::run(cell_check::BlocksMoveCmn(false), blocked);

        //Consider stairs and doors as non-blocking
        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                const FeatureId id = map::cells[x][y].rigid->id();
                
                if (id == FeatureId::stairs ||
                    id == FeatureId::door)
                {
                    blocked[x][y] = false;
                }
            }
        }

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                if (door_proposals[x][y])
                {
                    ChokePointData d;

                    const bool is_choke = is_choke_point(P(x, y),
                                                         blocked,
                                                         d);

                    if (is_choke)
                    {
                        //Find player and stair side
                        for (size_t side_idx = 0; side_idx < 2; ++side_idx)
                        {
                            for (const P& p : d.sides[side_idx])
                            {
                                if (p == map::player->pos)
                                {
                                    ASSERT(d.player_side == -1);

                                    d.player_side = side_idx;
                                }

                                if (p == stairs_pos)
                                {
                                    ASSERT(d.stairs_side == -1);

                                    d.stairs_side = side_idx;
                                }
                            }
                        }

                        ASSERT(d.player_side == 0 || d.player_side == 1);
                        ASSERT(d.stairs_side == 0 || d.stairs_side == 1);

                        //Robustness for release mode
                        if (
                            (d.player_side != 0 && d.player_side != 1) ||
                            (d.player_side != 0 && d.player_side != 1))
                        {
                            //Go to next map position
                            continue;
                        }

                        map::choke_point_data.emplace_back(d);
                    }
                }
            } //y loop
        } //x loop

        TRACE << "Found " << map::choke_point_data.size()
              << " choke points" << std::endl;
    }

    //Explicitly make some doors leading to "optional" areas secret and/or stuck
    if (is_map_valid)
    {
        for (const auto& choke_point : map::choke_point_data)
        {
            if (choke_point.player_side == choke_point.stairs_side)
            {
                Rigid* const rigid = map::cells[choke_point.p.x][choke_point.p.y].rigid;

                if (rigid->id() == FeatureId::door)
                {
                    Door* const door = static_cast<Door*>(rigid);

                    if (rnd::coin_toss())
                    {
                        door->set_to_secret();
                    }

                    if (rnd::coin_toss())
                    {
                        door->set_to_stuck();
                    }
                }
            }
        }
    }

    //NOTE: This depends on choke point data having been gathered (including
    //      player side and stairs side)
    if (is_map_valid)
    {
        place_monoliths();
    }

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

    if (is_map_valid)
    {
        //Occasionally place some snake emerge events
        const int nr_snake_emerge_events_to_try =
            rnd::one_in(20) ? 2 :
            rnd::one_in(4)  ? 1 : 0;

        for (int i = 0; i < nr_snake_emerge_events_to_try; ++i)
        {
            EventSnakeEmerge* const event = new EventSnakeEmerge();

            if (event->try_find_p())
            {
                game_time::add_mob(event);
            }
            else
            {
                delete event;
            }
        }
    }

    if (is_map_valid)
    {
        const int last_lvl_to_reveal_stairs_path = 6;

        if (map::dlvl <= last_lvl_to_reveal_stairs_path)
        {
            reveal_doors_on_path_to_stairs(stairs_pos);
        }
    }

    for (auto* r : map::room_list)
    {
        delete r;
    }

    map::room_list.clear();

    std::fill_n(*map::room_map, nr_map_cells, nullptr);

    TRACE_FUNC_END;
    return is_map_valid;
}

} //mapgen
