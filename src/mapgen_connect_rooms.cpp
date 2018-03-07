#include "mapgen.hpp"

#include "map_parsing.hpp"
#include "feature_rigid.hpp"

#ifndef NDEBUG
#include "init.hpp"
#include "io.hpp"
#include "sdl_base.hpp"
#endif // NDEBUG

namespace mapgen
{

void connect_rooms()
{
        TRACE_FUNC_BEGIN;

        int nr_tries_left = 5000;

        while (true)
        {
                // NOTE: Keep this counter at the top of the loop, since
                // otherwise a continue statement could bypass it so we get
                // stuck in the loop.
                --nr_tries_left;

                if (nr_tries_left == 0)
                {
                        mapgen::is_map_valid = false;

#ifndef NDEBUG
                        if (init::is_demo_mapgen)
                        {
                                io::cover_panel(Panel::log);
                                states::draw();
                                io::draw_text("Failed to connect map",
                                              Panel::screen,
                                              P(0, 0),
                                              colors::light_red());
                                io::update_screen();
                                sdl_base::sleep(8000);
                        }
#endif // NDEBUG

                        break;
                }

                auto rnd_room = []() {
                        return map::room_list[
                                rnd::range(0, map::room_list.size() - 1)];
                };

                // Standard rooms are connectable
                auto is_connectable_room = [](const Room & r) {
                        return r.type_ < RoomType::END_OF_STD_ROOMS;
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

                // Room 1 must not be the same as room 0, and it must be a
                // connectable room (connections are only allowed between two
                // standard rooms, or from a corridor link to a standard room -
                // never between two corridor links)
                while ((room1 == room0) || !is_connectable_room(*room1))
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

                // Do not connect room 0 and 1 if another room (except for
                // sub rooms) lies anywhere in a rectangle defined by the two
                // center points of those rooms.
                bool is_other_room_in_way = false;

                const P c0(room0->r_.center());
                const P c1(room1->r_.center());

                const int x0 = std::min(c0.x, c1.x);
                const int y0 = std::min(c0.y, c1.y);
                const int x1 = std::max(c0.x, c1.x);
                const int y1 = std::max(c0.y, c1.y);

                for (int x = x0; x <= x1; ++x)
                {
                        for (int y = y0; y <= y1; ++y)
                        {
                                const Room* const room_here =
                                        map::room_map[x][y];

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
                make_pathfind_corridor(*room0,
                                       *room1,
                                       door_proposals);

                bool blocked[map_w][map_h];

                map_parsers::BlocksMoveCommon(ParseActors::no).
                        run(blocked);

                // Do not consider doors blocking
                for (int x = 0; x < map_w; ++x)
                {
                        for (int y = 0; y < map_h; ++y)
                        {
                                const auto id = map::cells[x][y].rigid->id();

                                if (id == FeatureId::door)
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

} // mapgen
