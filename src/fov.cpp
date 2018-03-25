#include "fov.hpp"

#include "init.hpp"

#include <math.h>
#include <vector>

#include "line_calc.hpp"
#include "map.hpp"

namespace fov
{

R get_fov_rect(const P& p)
{
    const int radi = fov_radi_int;

    const P p0(std::max(0, p.x - radi),
               std::max(0, p.y - radi));

    const P p1(std::min(map_w - 1, p.x + radi),
               std::min(map_h - 1, p.y + radi));

    return R(p0, p1);
}

bool is_in_fov_range(const P& p0, const P& p1)
{
    return king_dist(p0, p1) <= fov_radi_int;
}

LosResult check_cell(const P& p0,
                      const P& p1,
                      const bool hard_blocked[map_w][map_h])
{
    LosResult los_result;

    los_result.is_blocked_hard = true; // Assume we are blocked initially

    los_result.is_blocked_by_drk = false;

    if (!is_in_fov_range(p0, p1) || !map::is_pos_inside_map(p1))
    {
        // Target too far away, return the hard blocked result
        return los_result;
    }

    const P delta(p1 - p0);

    const std::vector<P>* path_deltas_ptr =
        line_calc::fov_delta_line(delta, fov_radi_db);

    if (!path_deltas_ptr)
    {
        // No valid line to target, return the hard blocked result
        return los_result;
    }

    const std::vector<P>& path_deltas = *path_deltas_ptr;

    const bool tgt_is_lgt = map::light[p1.x][p1.y];

    // Ok, target is in range and we have a line - let's go
    los_result.is_blocked_hard = false;

    P current_p;
    P pre_p;

    const size_t path_size = path_deltas.size();

    for (size_t i = 0; i < path_size; ++i)
    {
        current_p.set(p0 + path_deltas[i]);

        if (i > 1)
        {
            // Check if we are blocked

            pre_p.set(p0 + path_deltas[i - 1]);

            const bool prev_cell_dark =
                map::dark[pre_p.x][pre_p.y];

            const bool current_cell_light =
                map::light[current_p.x][current_p.y];

            const bool current_cell_dark =
                map::dark[current_p.x][current_p.y];

            if (!tgt_is_lgt &&
                !current_cell_light &&
                (current_cell_dark || prev_cell_dark))
            {
                los_result.is_blocked_by_drk = true;
            }
        }

        if (current_p == p1)
        {
            break;
        }

        if ((i > 0) && hard_blocked[current_p.x][current_p.y])
        {
            los_result.is_blocked_hard = true;
            break;
        }
    }

    return los_result;
}

void run(const P& p0,
         const bool hard_blocked[map_w][map_h],
         LosResult out[map_w][map_h])
{
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            LosResult& los = out[x][y];

            los.is_blocked_hard     = true;
            los.is_blocked_by_drk   = false;
        }
    }

    const R r = get_fov_rect(p0);

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            out[x][y] = check_cell(p0, {x, y}, hard_blocked);
        }
    }

    out[p0.x][p0.y].is_blocked_hard = false;
}

} // fov
