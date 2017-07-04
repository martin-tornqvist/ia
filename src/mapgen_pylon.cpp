#include "mapgen.hpp"

#include "map_parsing.hpp"
#include "feature_pylon.hpp"

namespace mapgen
{

void mk_pylons_and_levers()
{
    //
    // Never make Pylons late game (theme)
    //
    if (map::dlvl >= dlvl_first_late_game)
    {
        return;
    }

    // Determine number of Pylons to place, by a weighted choice
    std::vector<int> nr_weights =
    {
        20, // 0 pylon(s)
        4,  // 1 -
        2,  // 2 -
        1,  // 3 -
    };

    const int nr_pylons = rnd::weighted_choice(nr_weights);

    bool blocked[map_w][map_h];

    {
        bool blocked_tmp[map_w][map_h];

        map_parsers::BlocksRigid()
            .run(blocked_tmp);

        map_parsers::expand(blocked_tmp,
                            blocked,
                            2);
    }

    for (Actor* const actor : game_time::actors)
    {
        blocked[actor->pos.x][actor->pos.y] = true;
    }

    for (int i = 0; i < nr_pylons; ++i)
    {
        //
        // Store non-blocked (false) cells in a vector
        //
        const auto p_bucket = to_vec(blocked, false);

        if (p_bucket.empty())
        {
            // No position available to place a Pylon - give up
            return;
        }

        const P pylon_p = rnd::element(p_bucket);

        // Never try this position again, whether we place this Pylon or not
        blocked[pylon_p.x][pylon_p.y] = true;

        //
        // Find position for lever
        //
        std::vector<P> lever_pos_bucket;

        int flood[map_w][map_h];

        {
            const int lever_max_dist = 4;

            floodfill(pylon_p,
                      blocked,
                      flood,
                      lever_max_dist);

            // Reserve worst case of push-backs
            const int side = (lever_max_dist + 1) * 2;

            lever_pos_bucket.reserve((side * side) - 1);

            const int x0 = std::max(0,          pylon_p.x - lever_max_dist);
            const int y0 = std::max(0,          pylon_p.y - lever_max_dist);
            const int x1 = std::min(map_w - 1,  pylon_p.x + lever_max_dist);
            const int y1 = std::min(map_h - 1,  pylon_p.y + lever_max_dist);

            for (int x = x0; x <= x1; ++x)
            {
                for (int y = y0; y <= y1; ++y)
                {
                    if (flood[x][y] > 0)
                    {
                        lever_pos_bucket.emplace_back(P(x, y));
                    }
                }
            }
        }

        if (lever_pos_bucket.empty())
        {
            // Cannot place a Lever near this Pylon - too bad, next!
            continue;
        }

        const P lever_p = rnd::element(lever_pos_bucket);

        //
        // OK, valid positions found - place Pylon and Lever
        //
        auto* const pylon = new Pylon(pylon_p, PylonId::any);

        auto* const lever = new Lever(lever_p);

        lever->set_linked_feature(*pylon);

        map::cells[pylon_p.x][pylon_p.y].rigid = pylon;

        map::cells[lever_p.x][lever_p.y].rigid = lever;

        //
        // Don't place other pylons too near
        //
        {
            const int min_dist_between = 8;

            const int x0 = std::max(0,          pylon_p.x - min_dist_between);
            const int y0 = std::max(0,          pylon_p.y - min_dist_between);
            const int x1 = std::min(map_w - 1,  pylon_p.x + min_dist_between);
            const int y1 = std::min(map_h - 1,  pylon_p.y + min_dist_between);

            for (int x = x0; x <= x1; ++x)
            {
                for (int y = y0; y <= y1; ++y)
                {
                    blocked[x][y] = true;
                }
            }
        }
    } // Nr of Pylons loop
}

} // namespace
