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
    const int RADI = FOV_STD_RADI_INT;

    const P p0(std::max(0, p.x - RADI),
               std::max(0, p.y - RADI));

    const P p1(std::min(MAP_W - 1, p.x + RADI),
               std::min(MAP_H - 1, p.y + RADI));

    return R(p0, p1);
}

bool is_in_fov_range(const P& p0, const P& p1)
{
    return king_dist(p0, p1) <= FOV_STD_RADI_INT;
}

Los_result check_cell(const P& p0,
                      const P& p1,
                      const bool hard_blocked[MAP_W][MAP_H])
{
    Los_result los_result;

    los_result.is_blocked_hard      = true; //Assume we are blocked initially
    los_result.is_blocked_by_drk    = false;

    if (!is_in_fov_range(p0, p1) || !map::is_pos_inside_map(p1))
    {
        //Target too far away, return the hard blocked result
        return los_result;
    }

    const P delta(p1 - p0);

    const std::vector<P>* path_deltas_ptr =
        line_calc::fov_delta_line(delta, FOV_STD_RADI_DB);

    if (!path_deltas_ptr)
    {
        //No valid line to target, return the hard blocked result
        return los_result;
    }

    const std::vector<P>& path_deltas = *path_deltas_ptr;

    const bool TGT_IS_LGT = map::cells[p1.x][p1.y].is_lit;

    //Ok, target is in range and we have a line - let's go
    los_result.is_blocked_hard = false;

    P cur_p;
    P pre_p;

    const size_t PATH_SIZE = path_deltas.size();

    for (size_t i = 0; i < PATH_SIZE; ++i)
    {
        cur_p.set(p0 + path_deltas[i]);

        if (i > 1)
        {
            //Check if we are blocked

            pre_p.set(p0 + path_deltas[i - 1]);

            const auto& pre_cell = map::cells[pre_p.x][pre_p.y];
            const auto& cur_cell = map::cells[cur_p.x][cur_p.y];

            if (!TGT_IS_LGT && !cur_cell.is_lit && (cur_cell.is_dark || pre_cell.is_dark))
            {
                los_result.is_blocked_by_drk = true;
            }
        }

        if (cur_p == p1)
        {
            break;
        }

        if (i > 0 && hard_blocked[cur_p.x][cur_p.y])
        {
            los_result.is_blocked_hard = true;
            break;
        }
    }

    return los_result;
}

void run(const P& p0,
         const bool hard_blocked[MAP_W][MAP_H],
         Los_result out[MAP_W][MAP_H])
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Los_result& los = out[x][y];

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

} //fov
