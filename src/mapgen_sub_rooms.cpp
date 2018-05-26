#include "mapgen.hpp"
#include "map.hpp"
#include "feature_rigid.hpp"

namespace mapgen
{

// Assumes that all rooms are rectangular
void make_sub_rooms()
{
    TRACE_FUNC_BEGIN;

    const int nr_tries_to_make_room = 100;

    const int max_nr_sub_rooms =
            rnd::one_in(3)
            ? 1
            : 7;

    // Minimum allowed size of the sub room, including the walls
    const P walls_min_d(4, 4);

    for (size_t i = 0; i < map::room_list.size(); ++i)
    {
        auto* const outer_room = map::room_list[i];

        if (!outer_room->allow_sub_rooms())
        {
            continue;
        }

        const R outer_room_rect = outer_room->r_;

        const P outer_room_d(outer_room_rect.dims());

        // Maximum sub room size, including the walls, in this outer room
        const P walls_max_d(outer_room_d + 2);

        if (walls_max_d.x < walls_min_d.x ||
            walls_max_d.y < walls_min_d.y)
        {
            // We cannot even build the smallest possible inner room inside this
            // outer room - no point in trying.
            continue;
        }

        const bool is_outer_big =
            (outer_room_d.x > 16) || (outer_room_d.y > 8);

        const bool is_outer_std_room =
            (int)outer_room->type_ < (int)RoomType::END_OF_STD_ROOMS;

        // To build a room inside a room, the outer room shall:
        // * Be a standard room, and
        // * Be a "big room" - but we occasionally allow "small rooms"
        if (!is_outer_std_room ||
            (!is_outer_big && !rnd::one_in(4)))
        {
            // Outer room does not meet dimensions criteria - next room
            continue;
        }

        for (int nr_inner = 0;
             nr_inner < max_nr_sub_rooms;
             ++nr_inner)
        {
            for (int try_count = 0;
                 try_count < nr_tries_to_make_room;
                 ++try_count)
            {
                // Determine the rectangle (p0, p1) of the inner room's walls

                // NOTE: The rectangle of the OUTER room is different - there it
                //       only represents the floor area of that room (this is
                //       how room areas are normally represented).

                const P walls_d(rnd::range(walls_min_d.x, walls_max_d.x),
                                rnd::range(walls_min_d.y, walls_max_d.y));

                const P p0(rnd::range(outer_room_rect.p0.x - 1,
                                      outer_room_rect.p1.x - walls_d.x + 2),
                           rnd::range(outer_room_rect.p0.y - 1,
                                      outer_room_rect.p1.y - walls_d.y + 2));

                const P p1(p0 + walls_d - 1);

                ASSERT(map::is_pos_inside_map(p0, true));
                ASSERT(map::is_pos_inside_map(p1, true));

                if (p0.x <= outer_room_rect.p0.x &&
                    p0.y <= outer_room_rect.p0.y &&
                    p1.x >= outer_room_rect.p1.x &&
                    p1.y >= outer_room_rect.p1.y)
                {
                    // None of the inner room's walls are inside the edge of the
                    // outer room - there is no point in building such a room!
                    continue;
                }

                // Check if map features allow us to build here
                bool is_area_free = true;

                for (int x = p0.x - 1; x <= p1.x + 1; ++x)
                {
                    for (int y = p0.y - 1; y <= p1.y + 1; ++y)
                    {
                        const P p_check(x, y);

                        if (!map::is_pos_inside_map(p_check, true))
                        {
                            continue;
                        }

                        const auto& f_id = map::cells.at(x, y).rigid->id();

                        const Room* const room = map::room_map.at(x, y);

                        // Rules to allow building:
                        //* Cells belonging to the outer room must be floor
                        //* Cells not belonging to the outer room must be walls
                        if ((room == outer_room && f_id != FeatureId::floor) ||
                            (room != outer_room && f_id != FeatureId::wall))
                        {
                            is_area_free = false;

                            break;
                        }

                    } // y loop

                    if (!is_area_free)
                    {
                        break;
                    }
                } // x loop

                if (!is_area_free)
                {
                    // Map features prevents us from building here - next try
                    continue;
                }

                // Alright, we can build the inner room

                // Room area of the inner room.
                // p0 and p1 represents the inner room's walls, so the actual
                // room area lies inside these points
                const R sub_room_rect(p0 + 1, p1 - 1);

                Room* const sub_room = make_room(sub_room_rect, IsSubRoom::yes);

                outer_room->sub_rooms_.push_back(sub_room);

                // Time to make walls and entrance(s) for our new room
                std::vector<P> entrance_bucket;

                for (int x = p0.x; x <= p1.x; ++x)
                {
                    for (int y = p0.y; y <= p1.y; ++y)
                    {
                        // Position is on the walls of the inner room?
                        if (x == p0.x ||
                            x == p1.x ||
                            y == p0.y ||
                            y == p1.y)
                        {
                            const P p(x, y);

                            map::put(new Wall(p));

                            // Only consider this position if it is completely
                            // inside the edge of the inner room
                            if (p.x > outer_room_rect.p0.x &&
                                p.y > outer_room_rect.p0.y &&
                                p.x < outer_room_rect.p1.x &&
                                p.y < outer_room_rect.p1.y)
                            {
                                // Do not put entrances on the corners of the
                                // inner room
                                if ((x != p0.x && x != p1.x) ||
                                    (y != p0.y && y != p1.y))
                                {
                                    entrance_bucket.push_back(P(x, y));
                                }
                            }
                        }
                    } // y loop
                } // x loop

                if (entrance_bucket.empty())
                {
                    // Not possible to place an entrance to the inner room,
                    // Discard this map!
                    is_map_valid = false;
                    return;
                }

                // Sometimes place one entrance, which may have a door
                // (always do this if there are very few possible entries)
                if (rnd::coin_toss() || entrance_bucket.size() <= 4)
                {
                    const size_t door_pos_idx =
                        rnd::range(0, entrance_bucket.size() - 1);

                    const P& door_pos = entrance_bucket[door_pos_idx];

                    map::put(new Floor(door_pos));

                    door_proposals.at(door_pos) = true;
                }
                else // Place multiple "doorless" entrances
                {
                    std::vector<P> positions_placed;
                    const int nr_tries = rnd::range(1, 10);

                    for (int j = 0; j < nr_tries; ++j)
                    {
                        const size_t door_pos_idx =
                            rnd::range(0, entrance_bucket.size() - 1);

                        const P& try_p = entrance_bucket[door_pos_idx];

                        bool is_pos_ok = true;

                        // Never make an entrance adjacent to an existing
                        for (P& prev_pos : positions_placed)
                        {
                            if (is_pos_adj(try_p, prev_pos, true))
                            {
                                is_pos_ok = false;
                                break;
                            }
                        }

                        if (is_pos_ok)
                        {
                            map::put(new Floor(try_p));
                            positions_placed.push_back(try_p);
                        }
                    }
                }

                // This point reached means the room has been built
                break;

            } // Try count loop
        } // Inner room count loop
    } // Room list loop

    TRACE_FUNC_END;

} // make_sub_rooms

} // make_sub_rooms
