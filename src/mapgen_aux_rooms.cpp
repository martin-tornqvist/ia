#include "mapgen.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "feature_event.hpp"

namespace mapgen
{

namespace
{

void mk_crumble_room(const R& room_area_incl_walls,
                     const P& event_pos)
{
    std::vector<P> wall_cells;
    std::vector<P> inner_cells;

    const R& a = room_area_incl_walls; // Abbreviation

    for (int x = a.p0.x; x <= a.p1.x; ++x)
    {
        for (int y = a.p0.y; y <= a.p1.y; ++y)
        {
            const P p(x, y);

            if (x == a.p0.x ||
                x == a.p1.x ||
                y == a.p0.y ||
                y == a.p1.y)
            {
                wall_cells.push_back(p);
            }
            else // Is inner cell
            {
                inner_cells.push_back(p);
            }

            // Fill the room with walls (so we don't have an inaccessible
            // empty room)
            map::put(new Wall(p));
        }
    }

    game_time::add_mob(
        new EventWallCrumble(event_pos,
                             wall_cells,
                             inner_cells));
} // mk_crumble_room

//
// NOTE: The positions and size can be outside map (e.g. negative positions).
//       This function just returns false in that case.
//
bool try_mk_aux_room(const P& p,
                     const P& d,
                     bool blocked[map_w][map_h],
                     const P& door_p)
{
    const R aux_rect(p, p + d - 1);

    const R aux_rect_with_border(aux_rect.p0 - 1, aux_rect.p1 + 1);

    ASSERT(is_pos_inside(door_p, aux_rect_with_border));

    if (map::is_area_inside_map(aux_rect_with_border))
    {
        // Check if area is blocked
        for (int x = aux_rect_with_border.p0.x;
             x <= aux_rect_with_border.p1.x;
             ++x)
        {
            for (int y = aux_rect_with_border.p0.y;
                 y <= aux_rect_with_border.p1.y;
                 ++y)
            {
                if (blocked[x][y])
                {
                    // Can't build here, bye...
                    return false;
                }
            }
        }

        for (int x = aux_rect.p0.x; x <= aux_rect.p1.x; ++x)
        {
            for (int y = aux_rect.p0.y; y <= aux_rect.p1.y; ++y)
            {
                blocked[x][y] = true;
                ASSERT(!map::room_map[x][y]);
            }
        }

        // Make a "crumble room"?
        if (rnd::one_in(20))
        {
            Room* const room =
                room_factory::mk(RoomType::crumble_room, aux_rect);

            register_room(*room);

            mk_crumble_room(aux_rect_with_border,
                            door_p);
        }
        else // Not "crumble room"
        {
            mk_room(aux_rect, IsSubRoom::no);
        }

        return true;
    }

    return false;

} // try_mk_aux_room

} // namespace

void mk_aux_rooms(Region regions[3][3])
{
    TRACE_FUNC_BEGIN;
    const int nr_tries_per_side = 20;

    auto rnd_aux_room_dim = []()
    {
        const Range range(2, 7);

        return P(range.roll(),
                 range.roll());
    };

    bool floor_cells[map_w][map_h];

    // TODO: It would be better with a parse predicate that checks for free
    //       cells immediately

    // Get blocked cells
    map_parsers::BlocksMoveCommon(ParseActors::no)
        .run(floor_cells);

    // Flip the values so that we get free cells
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            bool& v = floor_cells[x][y];

            v = !v;
        }
    }

    for (int region_x = 0; region_x < 3; region_x++)
    {
        for (int region_y = 0; region_y < 3; region_y++)
        {
            const Region& region = regions[region_x][region_y];

            if (region.main_room)
            {
                Room& main_r = *region.main_room;

                // Right
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < nr_tries_per_side; ++i)
                    {
                        const P con_p(main_r.r_.p1.x + 1,
                                      rnd::range(main_r.r_.p0.y + 1,
                                                 main_r.r_.p1.y - 1));

                        const P aux_d(rnd_aux_room_dim());

                        const P aux_p(con_p.x + 1,
                                      rnd::range(con_p.y - aux_d.y + 1,
                                                 con_p.y));

                        if (floor_cells[con_p.x - 1][con_p.y])
                        {
                            const bool did_place_room =
                                try_mk_aux_room(aux_p,
                                                aux_d,
                                                floor_cells,
                                                con_p);

                            if (did_place_room)
                            {
                                TRACE_VERBOSE << "Aux room placed right"
                                              << std:: endl;
                                break;
                            }
                        }
                    }
                }

                // Up
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < nr_tries_per_side; ++i)
                    {
                        const P con_p(rnd::range(main_r.r_.p0.x + 1,
                                                 main_r.r_.p1.x - 1),
                                      main_r.r_.p0.y - 1);

                        const P aux_d(rnd_aux_room_dim());

                        const P aux_p(rnd::range(con_p.x - aux_d.x + 1,
                                                 con_p.x),
                                      con_p.y - 1);

                        if (floor_cells[con_p.x][con_p.y + 1])
                        {
                            const bool did_place_room =
                                try_mk_aux_room(aux_p,
                                                aux_d,
                                                floor_cells,
                                                con_p);

                            if (did_place_room)
                            {
                                TRACE_VERBOSE << "Aux room placed up"
                                              << std:: endl;
                                break;
                            }
                        }
                    }
                }

                // Left
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < nr_tries_per_side; ++i)
                    {
                        const P con_p(main_r.r_.p0.x - 1,
                                      rnd::range(main_r.r_.p0.y + 1,
                                                 main_r.r_.p1.y - 1));

                        const P aux_d(rnd_aux_room_dim());

                        const P aux_p(con_p.x - 1,
                                      rnd::range(con_p.y - aux_d.y + 1,
                                                 con_p.y));

                        if (floor_cells[con_p.x + 1][con_p.y])
                        {
                            const bool did_place_room =
                                try_mk_aux_room(aux_p,
                                                aux_d,
                                                floor_cells,
                                                con_p);

                            if (did_place_room)
                            {
                                TRACE_VERBOSE << "Aux room placed left"
                                              << std:: endl;
                                break;
                            }
                        }
                    }
                }

                // Down
                if (rnd::one_in(4))
                {
                    for (int i = 0; i < nr_tries_per_side; ++i)
                    {
                        const P con_p(rnd::range(main_r.r_.p0.x + 1,
                                                 main_r.r_.p1.x - 1),
                                      main_r.r_.p1.y + 1);

                        const P aux_d(rnd_aux_room_dim());

                        const P aux_p(rnd::range(con_p.x - aux_d.x + 1,
                                                 con_p.x),
                                      con_p.y + 1);

                        if (floor_cells[con_p.x][con_p.y - 1])
                        {
                            const bool did_place_room =
                                try_mk_aux_room(aux_p,
                                                aux_d,
                                                floor_cells,
                                                con_p);

                            if (did_place_room)
                            {
                                TRACE_VERBOSE << "Aux room placed down"
                                              << std:: endl;
                                break;
                            }
                        }
                    }
                }
            }
        } // Region y loop
    } // Region x loop

    TRACE_FUNC_END;

} // mk_aux_rooms

} // mapgen
