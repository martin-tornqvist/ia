#include "mapgen.hpp"
#include "map.hpp"
#include "feature_rigid.hpp"

namespace mapgen
{

void decorate()
{
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Cell& cell = map::cells[x][y];

            if (cell.rigid->id() == FeatureId::wall)
            {
                // Randomly convert walls to high rubble
                if (rnd::one_in(10))
                {
                    map::put(new RubbleHigh(P(x, y)));
                    continue;
                }

                // Moss grown walls
                Wall* const wall = static_cast<Wall*>(cell.rigid);
                wall->set_random_is_moss_grown();

                // Convert some walls to cave
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
                    bool has_adj_floor      = false;
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

                        if (adj_id == FeatureId::floor  ||
                            adj_id == FeatureId::carpet ||
                            adj_id == FeatureId::trap)
                        {
                            has_adj_floor = true;

                            auto* adj_rigid =
                                static_cast<Floor*>(adj_cell.rigid);

                            if (adj_rigid->type_ == FloorType::cave)
                            {
                                has_adj_cave_floor = true;
                                break;
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
            if (map::cells[x][y].rigid->id() == FeatureId::floor)
            {
                const P p(x, y);

                // Randomly put low rubble
                if (rnd::one_in(100))
                {
                    map::put(new RubbleLow(p));
                }

                // Randomly put vines
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
        }
    }
}

} // mapgen
