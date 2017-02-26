#include "mapgen.hpp"
#include "map.hpp"
#include "feature_rigid.hpp"
#include "map_parsing.hpp"

namespace mapgen
{

void decorate()
{
    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCmn(ParseActors::no).
        run(blocked);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Cell& cell = map::cells[x][y];

            if (cell.rigid->id() == FeatureId::wall)
            {
                //
                // Convert some walls to high rubble
                //
                if (rnd::one_in(10))
                {
                    map::put(new RubbleHigh(P(x, y)));
                    continue;
                }

                //
                // Moss grown walls
                //
                Wall* const wall = static_cast<Wall*>(cell.rigid);
                wall->set_random_is_moss_grown();

                //
                // Convert some walls to cave
                //
                bool should_convert_to_cave_wall = false;

                if (map::dlvl >= dlvl_first_late_game)
                {
                    // If this is late game - convert all walls to cave
                    should_convert_to_cave_wall = true;
                }
                else // Not late game
                {
                    // Convert walls with no adjacent floor or with adjacent
                    // cave floor to cave
                    bool has_adj_floor = false;
                    bool has_adj_cave_floor = false;

                    for (const P& d : dir_utils::dir_list)
                    {
                        const P p_adj(P(x, y) + d);

                        if (!map::is_pos_inside_map(p_adj))
                        {
                            continue;
                        }

                        auto& adj_cell = map::cells[p_adj.x][p_adj.y];

                        const auto adj_id = adj_cell.rigid->id();

                        // TODO: Traps count as floor here - otherwise walls
                        //       that are only adjacent to traps would be
                        //       converted to cave walls, which would spoil the
                        //       presence of the trap, and just be weird in
                        //       general. This works for now, but it should
                        //       probably be handled better. Currently, traps
                        //       are the only rigid that can "mimic" floor, but
                        //       if some other feature like that is added, it
                        //       could be a problem.

                        if (adj_id == FeatureId::floor ||
                            adj_id == FeatureId::carpet ||
                            adj_id == FeatureId::trap)
                        {
                            has_adj_floor = true;

                            //
                            // TODO: Currently, traps always prevents converting
                            //       adjacent walls to cave wall - even if the
                            //       trap mimics cave floor
                            //

                            // Cave floor?
                            if (adj_id == FeatureId::floor)
                            {
                                auto* adj_floor =
                                    static_cast<Floor*>(adj_cell.rigid);

                                if (adj_floor->type_ == FloorType::cave)
                                {
                                    has_adj_cave_floor = true;
                                    break;
                                }
                            }
                        }
                    }

                    should_convert_to_cave_wall =
                        !has_adj_floor ||
                        has_adj_cave_floor;
                }

                if (should_convert_to_cave_wall)
                {
                    wall->type_ = WallType::cave;
                }
                else // Should not convert to cave wall
                {
                    wall->set_rnd_cmn_wall();
                }
            }
        }
    }

    for (int x = 1; x < map_w - 1; ++x)
    {
        for (int y = 1; y < map_h - 1; ++y)
        {
            const P p(x, y);

            const auto& cell = map::cells[x][y];

            if (cell.rigid->id() == FeatureId::floor)
            {

                //
                // Randomly put low rubble
                //
                if (rnd::one_in(100))
                {
                    map::put(new RubbleLow(p));
                }

                //
                // Randomly put vines
                //
                if (rnd::one_in(150))
                {
                    for (const P& d : dir_utils::dir_list_w_center)
                    {
                        const P adj_p(p + d);

                        const bool is_floor =
                            map::cells[adj_p.x][adj_p.y].rigid->id() ==
                            FeatureId::floor;

                        if (is_floor &&
                            rnd::one_in(3))
                        {
                            map::put(new Vines(adj_p));
                        }
                    }
                }
            }
            // Not floor
            else if (cell.rigid->id() == FeatureId::wall)
            {
                //
                // Convert some walls to gratings
                //
                if (rnd::one_in(6))
                {
                    bool is_allowed = true;

                    // Never allow two gratings next to each other
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        for (int dy = -1; dy <= 1; ++dy)
                        {
                            const P p_adj(p + P(dx, dy));

                            auto adj_id =
                                map::cells[p_adj.x][p_adj.y].rigid->id();

                            if (adj_id == FeatureId::grating)
                            {
                                is_allowed = false;

                                break;
                            }
                        }

                        if (!is_allowed)
                        {
                            break;
                        }
                    }

                    if (is_allowed)
                    {
                        const P adj_hor_1(p + P(-1, 0));
                        const P adj_hor_2(p + P(1, 0));
                        const P adj_ver_1(p + P(0, 1));
                        const P adj_ver_2(p + P(0, -1));

                        auto is_free = [&](const P& p)
                        {
                            return !blocked[p.x][p.y];
                        };

                        const bool is_free_hor_1 = is_free(adj_hor_1);
                        const bool is_free_hor_2 = is_free(adj_hor_2);
                        const bool is_free_ver_1 = is_free(adj_ver_1);
                        const bool is_free_ver_2 = is_free(adj_ver_2);

                        const bool is_blocked_hor =
                            !is_free_hor_1 && !is_free_hor_2;

                        const bool is_free_hor =
                            is_free_hor_1 && is_free_hor_2;

                        const bool is_blocked_ver =
                            !is_free_ver_1 && !is_free_ver_2;

                        const bool is_free_ver =
                            is_free_ver_1 && is_free_ver_2;

                        is_allowed =
                            (is_blocked_hor && is_free_ver) ||
                            (is_free_hor && is_blocked_ver);
                    }

                    if (is_allowed)
                    {
                        map::put(new Grating(p));
                    }
                }
            }
        }
    }
}

} // mapgen
