#include "map_builder.hpp"

#include "mapgen.hpp"
#include "map_controller.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "actor_player.hpp"
#include "feature_door.hpp"
#include "populate_monsters.hpp"
#include "populate_traps.hpp"
#include "populate_items.hpp"
#include "feature_event.hpp"
#include "game_time.hpp"

// For map generation demo
#ifndef NDEBUG
#include "init.hpp"
#include "io.hpp"
#include "sdl_base.hpp"
#include "query.hpp"
#endif // NDEBUG

// -----------------------------------------------------------------------------
// MapBuilderStd
// -----------------------------------------------------------------------------
bool MapBuilderStd::build_specific()
{
        TRACE_FUNC_BEGIN;

        mapgen::is_map_valid = true;

        TRACE << "Resetting helper arrays" << std:: endl;

        std::fill_n(*mapgen::door_proposals, nr_map_cells, false);

        // NOTE: This must be called before any rooms are created
        room_factory::init_room_bucket();

        TRACE << "Init regions" << std:: endl;

        const int split_x_interval = map_w / 3;
        const int split_y_interval = map_h / 3;

        const int split_x1 = split_x_interval;
        const int split_x2 = (split_x_interval * 2) + 1;

        const int split_y1 = split_y_interval;
        const int split_y2 = split_y_interval * 2;

        std::vector<int> x0_list = {
                1,
                split_x1 + 1,
                split_x2 + 1
        };

        std::vector<int> x1_list = {
                split_x1 - 1,
                split_x2 - 1,
                map_w - 2
        };

        std::vector<int> y0_list = {
                1,
                split_y1 + 1,
                split_y2 + 1
        };

        std::vector<int> y1_list = {
                split_y1 - 1,
                split_y2 - 1,
                map_h - 2
        };

        Region regions[3][3];

        for (int x_region = 0; x_region < 3; ++x_region)
        {
                const int x0 = x0_list[x_region];
                const int x1 = x1_list[x_region];

                for (int y_region = 0; y_region < 3; ++y_region)
                {
                        const int y0 = y0_list[y_region];
                        const int y1 = y1_list[y_region];

                        regions[x_region][y_region] = Region({x0, y0, x1, y1});
                }
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Reserve regions for a "river"
        // ---------------------------------------------------------------------
        const int river_one_in_n = 12;

        if (map::dlvl >= dlvl_first_mid_game &&
            rnd::one_in(river_one_in_n))
        {
                mapgen::reserve_river(regions);
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Merge some regions
        // ---------------------------------------------------------------------
        mapgen::merge_regions(regions);

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Make main rooms
        // ---------------------------------------------------------------------
        TRACE << "Making main rooms" << std:: endl;

        for (int x = 0; x < 3; ++x)
        {
                for (int y = 0; y < 3; ++y)
                {
                        auto& region = regions[x][y];

                        if (!region.main_room && region.is_free)
                        {
                                mapgen::make_room(region);
                        }
                }
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Make auxiliary rooms
        // ---------------------------------------------------------------------
#ifndef NDEBUG
        if (init::is_demo_mapgen)
        {
                io::cover_panel(Panel::log);
                states::draw();
                io::draw_text("Press any key to make aux rooms...",
                              Panel::screen,
                              P(0, 0),
                              colors::white());
                io::update_screen();
                query::wait_for_key_press();
                io::cover_panel(Panel::log);
        }
#endif // NDEBUG

        mapgen::make_aux_rooms(regions);

        // ---------------------------------------------------------------------
        // Make sub-rooms
        // ---------------------------------------------------------------------
        if (!mapgen::is_map_valid)
        {
                return false;
        }

        if (map::dlvl <= dlvl_last_mid_game)
        {
#ifndef NDEBUG
                if (init::is_demo_mapgen)
                {
                        io::cover_panel(Panel::log);
                        states::draw();
                        io::draw_text("Press any key to make sub rooms...",
                                      Panel::screen,
                                      P(0, 0),
                                      colors::white());
                        io::update_screen();
                        query::wait_for_key_press();
                        io::cover_panel(Panel::log);
                }
#endif // NDEBUG

                mapgen::make_sub_rooms();
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        TRACE << "Sorting the room list according to room type" << std:: endl;

        // NOTE: This allows common rooms to assume that they are rectangular
        // and have their walls untouched when their reshaping functions run.

        sort(begin(map::room_list),
             end(map::room_list),
             [](const auto r0, const auto r1)
             {
                     return r0->type_ < r1->type_;
             });

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // If there are too few rooms at this point (including main rooms,
        // sub rooms, aux rooms, ...), then invalidate the map
        // ---------------------------------------------------------------------
        const size_t min_nr_rooms = 8;

        if (map::room_list.size() < min_nr_rooms)
        {
                mapgen::is_map_valid = false;

                return false;
        }

        // ---------------------------------------------------------------------
        // Run the pre-connect hook on all rooms
        // ---------------------------------------------------------------------
        TRACE << "Running pre-connect for all rooms" << std:: endl;

#ifndef NDEBUG
        if (init::is_demo_mapgen)
        {
                io::cover_panel(Panel::log);
                states::draw();
                io::draw_text(
                        "Press any key to run pre-connect on rooms...",
                        Panel::screen,
                        P(0, 0),
                        colors::white());
                io::update_screen();
                query::wait_for_key_press();
                io::cover_panel(Panel::log);
        }
#endif // NDEBUG

        for (Room* room : map::room_list)
        {
                room->on_pre_connect(mapgen::door_proposals);
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Connect the rooms
        // ---------------------------------------------------------------------
#ifndef NDEBUG
        if (init::is_demo_mapgen)
        {
                io::cover_panel(Panel::log);
                states::draw();
                io::draw_text("Press any key to connect rooms...",
                              Panel::screen,
                              P(0, 0),
                              colors::white());
                io::update_screen();
                query::wait_for_key_press();
                io::cover_panel(Panel::log);
        }
#endif // NDEBUG

        mapgen::connect_rooms();

        // ---------------------------------------------------------------------
        // Run the post-connect hook on all rooms
        // ---------------------------------------------------------------------
        if (!mapgen::is_map_valid)
        {
                return false;
        }

        TRACE << "Running post-connect for all rooms" << std:: endl;
#ifndef NDEBUG
        if (init::is_demo_mapgen)
        {
                io::cover_panel(Panel::log);
                states::draw();
                io::draw_text(
                        "Press any key to run post-connect on rooms...",
                        Panel::screen,
                        P(0, 0),
                        colors::white());
                io::update_screen();
                query::wait_for_key_press();
                io::cover_panel(Panel::log);
        }
#endif // NDEBUG

        for (Room* room : map::room_list)
        {
                room->on_post_connect(mapgen::door_proposals);
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Place doors
        // ---------------------------------------------------------------------
        if (map::dlvl <= dlvl_last_mid_game)
        {
                mapgen::make_doors();
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Move player to the nearest free position
        // ---------------------------------------------------------------------
        mapgen::move_player_to_nearest_allowed_pos();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Decorate the map
        // ---------------------------------------------------------------------
        mapgen::decorate();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Place the stairs
        // ---------------------------------------------------------------------
        // NOTE: The choke point information gathering below depends on the
        // stairs having been placed.
        P stairs_pos;

        stairs_pos = mapgen::make_stairs_at_random_pos();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Gather data on choke points in the map (check every position where a
        // door has previously been "proposed")
        // ---------------------------------------------------------------------
        bool blocked[map_w][map_h];

        map_parsers::BlocksMoveCommon(ParseActors::no)
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
                        if (blocked[x][y] || !mapgen::door_proposals[x][y])
                        {
                                continue;
                        }

                        ChokePointData d;

                        const bool is_choke =
                                mapgen::is_choke_point(
                                        P(x, y),
                                        blocked,
                                        &d);

                        // 'is_choke_point' called above may invalidate the map
                        if (!mapgen::is_map_valid)
                        {
                                return false;
                        }

                        if (!is_choke)
                        {
                                continue;
                        }

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
                                // Invalidate the map
                                mapgen::is_map_valid = false;

                                return false;
                        }

                        map::choke_point_data.emplace_back(d);
                } // y loop
        } // x loop

        TRACE << "Found " << map::choke_point_data.size()
              << " choke points" << std::endl;

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Make metal doors and levers
        // ---------------------------------------------------------------------
        mapgen::make_metal_doors_and_levers();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Make some doors leading to "optional" areas secret or stuck
        // ---------------------------------------------------------------------
        for (const auto& choke_point : map::choke_point_data)
        {
                if (choke_point.player_side != choke_point.stairs_side)
                {
                        continue;
                }

                Rigid* const rigid =
                        map::cells[choke_point.p.x][choke_point.p.y].rigid;

                if (rigid->id() == FeatureId::door)
                {
                        Door* const door = static_cast<Door*>(rigid);

                        if ((door->type() != DoorType::gate) &&
                            (door->type() != DoorType::metal) &&
                            rnd::one_in(6))
                        {
                                door->set_secret();
                        }

                        if (rnd::one_in(6))
                        {
                                door->set_stuck();
                        }
                }
        }

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Place Monoliths
        // ---------------------------------------------------------------------
        // NOTE: This depends on choke point data having been gathered
        // (including player side and stairs side)
        mapgen::make_monoliths();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Place pylons and levers
        // ---------------------------------------------------------------------
        mapgen::make_pylons_and_levers();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Populate the map with monsters
        // ---------------------------------------------------------------------
        populate_mon::populate_std_lvl();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Populate the map with traps
        // ---------------------------------------------------------------------
        populate_traps::populate_std_lvl();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Populate the map with items on the floor
        // ---------------------------------------------------------------------
        populate_items::make_items_on_floor();

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Place "snake emerge" events
        // ---------------------------------------------------------------------
        const int nr_snake_emerge_events_to_try =
                rnd::one_in(60) ? 2 :
                rnd::one_in(16) ? 1 : 0;

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

        if (!mapgen::is_map_valid)
        {
                return false;
        }

        // ---------------------------------------------------------------------
        // Reveal all doors on the path to the stairs (if "early" dungeon level)
        // ---------------------------------------------------------------------
        const int last_lvl_to_reveal_stairs_path = 6;

        if (map::dlvl <= last_lvl_to_reveal_stairs_path)
        {
                mapgen::reveal_doors_on_path_to_stairs(stairs_pos);
        }

        // ---------------------------------------------------------------------
        // Occasionally make the whole level dark
        // ---------------------------------------------------------------------
        if (map::dlvl > 1)
        {
                const int make_drk_pct =
                        (map::dlvl <= dlvl_last_early_game) ? 1 :
                        (map::dlvl <= dlvl_last_mid_game) ? 2 :
                        15;

                if (rnd::percent(make_drk_pct))
                {
                        for (int x = 0; x < map_w; ++x)
                        {
                                for (int y = 0; y < map_h; ++y)
                                {
                                        map::dark[x][y] = true;
                                }
                        }
                }
        }

        if (!mapgen::is_map_valid)
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
        return mapgen::is_map_valid;
}
