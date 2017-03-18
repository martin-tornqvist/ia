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
#include "io.hpp"
#include "populate_monsters.hpp"
#include "populate_traps.hpp"
#include "populate_items.hpp"
#include "gods.hpp"
#include "rl_utils.hpp"

#ifdef DEMO_MODE
#include "io.hpp"
#include "sdl_base.hpp"
#include "query.hpp"
#endif // DEMO_MODE

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
        // NOTE: Keep this counter at the top of the loop, since otherwise a
        //       continue statement could bypass it so we get stuck in the loop.
        --nr_tries_left;

        if (nr_tries_left == 0)
        {
            mapgen::is_map_valid = false;

#ifdef DEMO_MODE
            io::cover_panel(Panel::log);
            states::draw();
            io::draw_text("Failed to connect map",
                          Panel::screen,
                          P(0, 0),
                          clr_red_lgt);
            io::update_screen();
            sdl_base::sleep(8000);
#endif // DEMO_MODE
            break;
        }

        auto rnd_room = []()
        {
            return map::room_list[rnd::range(0, map::room_list.size() - 1)];
        };

        // Standard rooms are connectable
        auto is_connectable_room = [](const Room & r)
        {
            return (int)r.type_ < (int)RoomType::END_OF_STD_ROOMS;
        };

        Room* room0 = rnd_room();

        // Room 0 must be a connectable room, or a corridor link
        if (!is_connectable_room(*room0) &&
            room0->type_ != RoomType::corr_link)
        {
            continue;
        }

        // Finding second room to connect to
        Room* room1 = rnd_room();

        // Room 1 must not be the same as room 0, and it must be a connectable
        // room (connections are only allowed between two standard rooms, or
        // from a corridor link to a standard room - never between two corridor
        // links)
        while ((room1 == room0) ||
               !is_connectable_room(*room1))
        {
            room1 = rnd_room();
        }

        // Do not allow two rooms to be connected twice
        const auto& room0_connections = room0->rooms_con_to_;

        if (find(room0_connections.begin(),
                 room0_connections.end(),
                 room1) != room0_connections.end())
        {
            // Rooms are already connected, trying other combination
            continue;
        }

        // Do not connect room 0 and 1 if another room (except for sub rooms)
        // lies anywhere in a rectangle defined by the two center points of
        // those rooms.
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

                if (room_here &&
                    room_here != room0 &&
                    room_here != room1 &&
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
            // Blocked by room between, trying other combination
            continue;
        }

        // Alright, let's try to connect these rooms
        mk_pathfind_corridor(*room0,
                             *room1,
                             door_proposals);

        bool blocked[map_w][map_h];

        map_parsers::BlocksMoveCmn(ParseActors::no).
            run(blocked);

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

        if ((nr_tries_left <= 2 || rnd::one_in(4)) &&
            map_parsers::is_map_connected(blocked))
        {
            break;
        }
    }

    TRACE_FUNC_END;
}

void try_place_door(const P& p)
{
    // Check that no other doors are within a certain distance
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
        //
        // Make most doors "common" wooden doors, and occasionally make gates
        //
        Door* door = nullptr;

        if (rnd::fraction(4, 5))
        {
            const Wall* const mimic = new Wall(p);

            door = new Door(p,
                            mimic,
                            DoorType::wood);
        }
        else // Barred gate
        {
            door = new Door(p,
                            nullptr,
                            DoorType::gate);
        }

        map::put(door);
    }
}

void allowed_stair_cells(bool out[map_w][map_h])
{
    TRACE_FUNC_BEGIN;

    // Mark cells as free if all adjacent feature types are allowed
    std::vector<FeatureId> feat_ids_ok
    {
        FeatureId::floor,
        FeatureId::carpet,
        FeatureId::grass
    };

    map_parsers::AllAdjIsAnyOfFeatures(feat_ids_ok)
        .run(out);

    // Block cells with item
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

    // Block cells with actors
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

    auto allowed_cells_list = to_vec(allowed_cells, true);

    const int nr_ok_cells = allowed_cells_list.size();

    const int min_nr_ok_cells_req = 3;

    if (nr_ok_cells < min_nr_ok_cells_req)
    {
        TRACE << "Nr available cells to place stairs too low "
              << "(" << nr_ok_cells << "), discarding map" << std:: endl;
        is_map_valid = false;
#ifdef DEMO_MODE
        io::cover_panel(Panel::log);
        states::draw();
        io::draw_text("To few cells to place stairs",
                          Panel::screen,
                          P(0, 0),
                          clr_red_lgt);
        io::update_screen();
        sdl_base::sleep(8000);
#endif // DEMO_MODE
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

    auto allowed_cells_list = to_vec(allowed_cells, true);

    if (allowed_cells_list.empty())
    {
        is_map_valid = false;
    }
    else // Valid cells exists
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
    // Determine number of Monoliths to place, by a weighted choice
    std::vector<int> nr_weights =
    {
        50, // 0 monolith(s)
        50, // 1 -
        1,  // 2 -
    };

    const int nr_monoliths = rnd::weighted_choice(nr_weights);

    bool blocked_expanded[map_w][map_h];

    {
        bool blocked[map_w][map_h] = {};

        map_parsers::BlocksRigid()
            .run(blocked);

        map_parsers::expand(blocked, blocked_expanded);

        for (Actor* const actor : game_time::actors)
        {
            blocked_expanded[actor->pos.x][actor->pos.y] = true;
        }
    }

    // Block the area around the player
    const P& player_p = map::player->pos;

    const int r = fov_std_radi_int;

    const R fov_r(std::max(0,           player_p.x - r),
                  std::max(0,           player_p.y - r),
                  std::max(map_w - 1,   player_p.y + r),
                  std::max(map_h - 1,   player_p.y + r));

    for (int x = fov_r.p0.x; x <= fov_r.p1.x; ++x)
    {
        for (int y = fov_r.p0.y; y <= fov_r.p1.y; ++y)
        {
            blocked_expanded[x][y] = true;
        }
    }

    for (int i = 0; i < nr_monoliths; ++i)
    {
        const auto p_bucket = to_vec(blocked_expanded, false);

        if (p_bucket.empty())
        {
            // Unable to place Monolith (too small map?), invalidate the map!
            is_map_valid = false;
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

void mk_metal_doors_and_levers()
{
    //
    // TODO: Perhaps the levers should be placed as far as possible away from
    //       the player AND the door?
    //

    //
    // Only make metal doors and levers on some maps, and never late game
    //
    const Fraction chance(3, 4);

    if ((map::dlvl >= dlvl_first_late_game) ||
        !chance.roll())
    {
        return;
    }

    //
    // Find all chokepoints with a door
    //
    std::vector<const ChokePointData*> chokepoint_bucket;

    for (const auto& chokepoint : map::choke_point_data)
    {
        if (chokepoint.sides[0].empty() ||
            chokepoint.sides[1].empty())
        {
            continue;
        }

        const P& p = chokepoint.p;

        auto id = map::cells[p.x][p.y].rigid->id();

        if (id == FeatureId::door)
        {
            chokepoint_bucket.push_back(&chokepoint);
        }
    }

    if (chokepoint_bucket.empty())
    {
        return;
    }

    //
    // Shuffle the chokepoint list
    //
    rnd::shuffle(chokepoint_bucket);

    auto get_valid_pos = [](const std::vector<P>& bucket,
                            const bool valid[map_w][map_h])
    {
        for (const P& p : bucket)
        {
            if (valid[p.x][p.y])
            {
                return p;
            }
        }

        return P(-1, -1);
    };

    for (const auto* const chokepoint : chokepoint_bucket)
    {
        //
        // Find a lever position both sides of the chokepoint
        //
        bool valid[map_w][map_h];

        // All adjacent cells must be floor
        map_parsers::AllAdjIsAnyOfFeatures(FeatureId::floor)
             .run(valid);

        // Do not allow cells with actors (the only actor on the map so far is
        // probably the player, but let's just check anyway for robustness sake)
        for (const auto* const actor : game_time::actors)
        {
            valid[actor->pos.x][actor->pos.y] = false;
        }

        auto side_1_cpy = chokepoint->sides[0];
        auto side_2_cpy = chokepoint->sides[1];

        rnd::shuffle(side_1_cpy);
        rnd::shuffle(side_2_cpy);

        const auto lever_1_pos = get_valid_pos(side_1_cpy, valid);
        const auto lever_2_pos = get_valid_pos(side_2_cpy, valid);

        if ((lever_1_pos.x == -1) ||
            (lever_2_pos.x == -1))
        {
            // Failed to find valid lever position on one or both sides, try
            // next chokepoint
            continue;
        }

        //
        // OK, we have found valid positions for the door and for both levers,
        // now we can place them on the map
        //
        auto* door = new Door(chokepoint->p,
                              nullptr, // No mimic needed
                              DoorType::metal,
                              DoorSpawnState::closed);

        map::put(door);

        auto* const lever_1 = new Lever(lever_1_pos);
        auto* const lever_2 = new Lever(lever_2_pos);

        lever_1->link_door(door);
        lever_2->link_door(door);

        map::put(lever_1);
        map::put(lever_2);

        // We are done
        return;
    }
}

void reveal_doors_on_path_to_stairs(const P& stairs_pos)
{
    TRACE_FUNC_BEGIN;

    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCmn(ParseActors::no)
        .run(blocked);

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

    pathfind(map::player->pos,
             stairs_pos,
             blocked,
             path);

    ASSERT(!path.empty());

    TRACE << "Travelling along path and revealing all doors" << std:: endl;

    for (P& pos : path)
    {
        auto* const feature = map::cells[pos.x][pos.y].rigid;

        if (feature->id() == FeatureId::door)
        {
            static_cast<Door*>(feature)->reveal(Verbosity::silent);
        }
    }

    TRACE_FUNC_END;
}

} // namespace

bool mk_std_lvl()
{
    TRACE_FUNC_BEGIN;

    is_map_valid = true;

    io::clear_screen();
    io::update_screen();

    map::reset_map();

    TRACE << "Resetting helper arrays" << std:: endl;

    std::fill_n(*door_proposals, nr_map_cells, false);

    //
    // NOTE: This must be called before any rooms are created
    //
    room_factory::init_room_bucket();

    TRACE << "Init regions" << std:: endl;
    const int map_w_third = map_w / 3;
    const int map_h_third = map_h / 3;

    const int spl_x0 = map_w_third;
    const int spl_x1 = (map_w_third * 2) + 1;
    const int spl_y0 = map_h_third;
    const int spl_y1 = map_h_third * 2;

    Region regions[3][3];

    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            const R r((x == 0) ? 1 :
                      (x == 1) ? (spl_x0 + 1) : (spl_x1 + 1),

                      (y == 0) ? 1 :
                      (y == 1) ? (spl_y0 + 1) : spl_y1 + 1,

                      (x == 0) ? (spl_x0 - 1) :
                      (x == 1) ? (spl_x1 - 1) : map_w - 2,

                      (y == 0) ? (spl_y0 - 1) :
                      (y == 1) ? (spl_y1 - 1) : (map_h - 2));

            regions[x][y] = Region(r);
        }
    }

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Reserve regions for a "river"
    //
#ifndef DISABLE_MK_RIVER
    const int river_one_in_n = 8;

    if (map::dlvl >= dlvl_first_mid_game &&
        rnd::one_in(river_one_in_n))
    {
        reserve_river(regions);
    }

    if (!is_map_valid)
    {
        return false;
    }
#endif // MK_RIVER

    //
    // Merge some regions
    //
#ifndef DISABLE_MERGED_REGIONS
    merge_regions(regions);

    if (!is_map_valid)
    {
        return false;
    }
#endif // MK_MERGED_REGIONS

    //
    // Make main rooms
    //
    TRACE << "Making main rooms" << std:: endl;

    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            auto& region = regions[x][y];

            if (!region.main_room && region.is_free)
            {
                mk_room(region);
            }
        }
    }

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Make auxiliary rooms
    //
#ifndef DISABLE_AUX_ROOMS
#ifdef DEMO_MODE
    io::cover_panel(Panel::log);
    states::draw();
    io::draw_text("Press any key to make aux rooms...",
                      Panel::screen,
                      P(0, 0),
                      clr_white);
    io::update_screen();
    query::wait_for_key_press();
#endif // DEMO_MODE

    mk_aux_rooms(regions);

    //
    // Make sub-rooms
    //
    if (!is_map_valid)
    {
        return false;
    }
#endif // DISABLE_AUX_ROOMS

#ifndef DISABLE_MK_SUB_ROOMS
    if (map::dlvl <= dlvl_last_mid_game)
    {
#ifdef DEMO_MODE
        io::cover_panel(Panel::log);
        states::draw();
        io::draw_text("Press any key to make sub rooms...",
                          Panel::screen,
                          P(0, 0),
                          clr_white);
        io::update_screen();
        query::wait_for_key_press();
#endif // DEMO_MODE

        mk_sub_rooms();
    }

    if (!is_map_valid)
    {
        return false;
    }
#endif // DISABLE_MK_SUB_ROOMS

    TRACE << "Sorting the room list according to room type" << std:: endl;
    // NOTE: This allows common rooms to assume that they are rectangular and
    //       have their walls untouched when their reshaping functions run.
    auto cmp = [](const Room * r0, const Room * r1)
    {
        return (int)r0->type_ < (int)r1->type_;
    };

    sort(map::room_list.begin(), map::room_list.end(), cmp);

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Run the pre-connect hook on all rooms
    //
    TRACE << "Running pre-connect functions for all rooms" << std:: endl;

#ifdef DEMO_MODE
    io::cover_panel(Panel::log);
    states::draw();
    io::draw_text("Press any key to run pre-connect functions on rooms...",
                      Panel::screen,
                      P(0, 0),
                      clr_white);
    io::update_screen();
    query::wait_for_key_press();
#endif // DEMO_MODE

    gods::set_no_god();

    for (Room* room : map::room_list)
    {
        room->on_pre_connect(door_proposals);
    }

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Connect the rooms
    //
#ifdef DEMO_MODE
    io::cover_panel(Panel::log);
    states::draw();
    io::draw_text("Press any key to connect rooms...",
                  Panel::screen,
                  P(0, 0),
                  clr_white);
    io::update_screen();
    query::wait_for_key_press();
#endif // DEMO_MODE

    connect_rooms();

    //
    // Run the post-connect hook on all rooms
    //
    if (!is_map_valid)
    {
        return false;
    }

    TRACE << "Running post-connect functions for all rooms" << std:: endl;
#ifdef DEMO_MODE
    io::cover_panel(Panel::log);
    states::draw();
    io::draw_text("Press any key to run post-connect functions on rooms...",
                  Panel::screen,
                  P(0, 0),
                  clr_white);
    io::update_screen();
    query::wait_for_key_press();
#endif // DEMO_MODE

    for (Room* room : map::room_list)
    {
        room->on_post_connect(door_proposals);
    }

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Place doors
    //
    if (map::dlvl <= dlvl_last_mid_game)
    {
        TRACE << "Placing doors" << std:: endl;

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                if (door_proposals[x][y] &&
                    rnd::fraction(4, 5))
                {
                    try_place_door(P(x, y));
                }
            }
        }
    }

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Move player to the nearest free position
    //
    move_player_to_nearest_allowed_pos();

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Decorate the map
    //
#ifndef DISABLE_DECORATE
    decorate();

    if (!is_map_valid)
    {
        return false;
    }
#endif // DISABLE_DECORATE

    //
    // Place the stairs
    //
    // NOTE: The choke point information gathering below depends on the stairs
    //       having been placed.
    //
    P stairs_pos;

    stairs_pos = place_stairs();

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Gather data on choke points in the map (check every position where a door
    // has previously been "proposed")
    //
    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCmn(ParseActors::no)
        .run(blocked);

    // Consider stairs and doors as non-blocking
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
                    // Find player and stair side
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

                    // Robustness for release mode
                    if ((d.player_side != 0 && d.player_side != 1) ||
                        (d.player_side != 0 && d.player_side != 1))
                    {
                        // Go to next map position
                        continue;
                    }

                    map::choke_point_data.emplace_back(d);
                }
            }
        } // y loop
    } // x loop

    TRACE << "Found " << map::choke_point_data.size()
          << " choke points" << std::endl;

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Make metal doors and levers
    //
    mk_metal_doors_and_levers();

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Explicitly make some doors leading to "optional" areas secret or stuck
    //
    for (const auto& choke_point : map::choke_point_data)
    {
        if (choke_point.player_side == choke_point.stairs_side)
        {
            Rigid* const rigid =
                map::cells[choke_point.p.x][choke_point.p.y].rigid;

            if (rigid->id() == FeatureId::door)
            {
                Door* const door = static_cast<Door*>(rigid);

                if ((door->type() != DoorType::gate) &&
                    (door->type() != DoorType::metal) &&
                    rnd::one_in(3))
                {
                    door->set_secret();
                }

                if (rnd::one_in(4))
                {
                    door->set_stuck();
                }
            }
        }
    }

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Place Monoliths
    //
    // NOTE: This depends on choke point data having been gathered (including
    //       player side and stairs side)
    //
    place_monoliths();

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Populate the map with monsters
    //
    populate_mon::populate_std_lvl();

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Populate the map with traps
    //
    populate_traps::populate_std_lvl();

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Populate the map with items on the floor
    //
    populate_items::mk_items_on_floor();

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Place "snake emerge" events
    //
    const int nr_snake_emerge_events_to_try =
        rnd::one_in(30) ? 2 :
        rnd::one_in(8)  ? 1 : 0;

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

    if (!is_map_valid)
    {
        return false;
    }

    //
    // Reveal all doors on the path to the stairs (if "early" dungeon level)
    //
    const int last_lvl_to_reveal_stairs_path = 6;

    if (map::dlvl <= last_lvl_to_reveal_stairs_path)
    {
        reveal_doors_on_path_to_stairs(stairs_pos);
    }

    //
    // Occasionally make the whole level dark
    //
    if (map::dlvl > 1)
    {
        const int make_drk_pct = 2 + (map::dlvl / 4);

        if (rnd::percent(make_drk_pct))
        {
            for (int x = 0; x < map_w; ++x)
            {
                for (int y = 0; y < map_h; ++y)
                {
                    map::cells[x][y].is_dark = true;
                }
            }
        }
    }

    if (!is_map_valid)
    {
        return false;
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

} // mapgen
