#include "mapgen.hpp"

#include "map_parsing.hpp"
#include "feature_monolith.hpp"

namespace mapgen
{

void mk_monoliths()
{
    // Determine number of Monoliths to place, by a weighted choice
    std::vector<int> nr_weights =
    {
        50, // 0 monolith(s)
        50, // 1 -
        1,  // 2 -
    };

    const int nr_monoliths = rnd::weighted_choice(nr_weights);

    bool blocked[map_w][map_h];

    {
        bool blocked_tmp[map_w][map_h];

        map_parsers::BlocksRigid()
            .run(blocked_tmp);

        map_parsers::expand(blocked_tmp, blocked);
    }

    for (Actor* const actor : game_time::actors)
    {
        blocked[actor->pos.x][actor->pos.y] = true;
    }

    // Block the area around the player
    const P& player_p = map::player->pos;

    const int r = fov_std_radi_int;

    const R fov_r(std::max(0,           player_p.x - r),
                  std::max(0,           player_p.y - r),
                  std::min(map_w - 1,   player_p.y + r),
                  std::min(map_h - 1,   player_p.y + r));

    for (int x = fov_r.p0.x; x <= fov_r.p1.x; ++x)
    {
        for (int y = fov_r.p0.y; y <= fov_r.p1.y; ++y)
        {
            blocked[x][y] = true;
        }
    }

    for (int i = 0; i < nr_monoliths; ++i)
    {
        // Store non-blocked (false) cells in a vector
        const auto p_bucket = to_vec(blocked, false);

        if (p_bucket.empty())
        {
            // Unable to place Monolith
            return;
        }

        const P p = rnd::element(p_bucket);

        map::cells[p.x][p.y].rigid = new Monolith(p);

        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P adj_p(p + d);

            blocked[adj_p.x][adj_p.y] = true;
        }
    }
}

} // namespace
