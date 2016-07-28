#include "line_calc.hpp"

#include <math.h>
#include <vector>

#include "global.hpp"
#include "map.hpp"

namespace line_calc
{

namespace
{

double          fov_abs_distances_[fov_max_w_int][fov_max_w_int];
std::vector<P>  fov_delta_lines_[fov_max_w_int][fov_max_w_int];

} //namespace

void init()
{
    //Calculate FOV absolute distances
    for (int y = 0; y < fov_max_w_int; ++y)
    {
        for (int x = 0; x < fov_max_w_int; ++x)
        {
            fov_abs_distances_[x][y] = 0;
        }
    }

    double delta_x, delta_y, hypot;

    //delta_x corresponds to i - fov_max_w_int, and vice versa for delta_y and j.
    //So:
    //i = 0                 => delta_x = -fov_max_w_int
    //i = fov_max_w_int * 2 => delta_x =  fov_max_w_int

    const int r_int = fov_max_radi_int;

    for (int x = 0; x <= r_int * 2; ++x)
    {
        for (int y = 0; y <= r_int * 2; ++y)
        {
            delta_x = double(x);
            delta_x -= fov_max_radi_db;
            delta_y = double(y);
            delta_y -= fov_max_radi_db;
            hypot = sqrt((delta_x * delta_x) + (delta_y * delta_y));
            fov_abs_distances_[x][y] = floor(hypot);
        }
    }

    //Calculate FOV delta lines
    for (int delta_x = -r_int; delta_x <= r_int; delta_x++)
    {
        for (int delta_y = -r_int; delta_y <= r_int; delta_y++)
        {
            const P origin(0, 0);
            const P tgt(P(delta_x, delta_y));
            std::vector<P> current_line;
            calc_new_line(origin, tgt, true, 999, true, current_line);
            fov_delta_lines_[delta_x + r_int][delta_y + r_int] = current_line;
        }
    }
}

const std::vector<P>* fov_delta_line(const P& delta,
                                     const double& max_dist_abs)
{
    const int x = delta.x + fov_max_radi_int;
    const int y = delta.y + fov_max_radi_int;

    if (x >= 0 && y >= 0 && x < fov_max_w_int && y < fov_max_w_int)
    {
        if (fov_abs_distances_[x][y] <= max_dist_abs)
        {
            return &(fov_delta_lines_[x][y]);
        }
    }

    return nullptr;
}

void calc_new_line(const P& origin,
                   const P& tgt,
                   const bool should_stop_at_target,
                   const int cheb_travel_limit,
                   const bool allow_outside_map,
                   std::vector<P>& line_ref)
{
    line_ref.clear();

    if (tgt == origin)
    {
        line_ref.push_back(origin);
        return;
    }

    const double delta_x_db = double(tgt.x - origin.x);
    const double delta_y_db = double(tgt.y - origin.y);

    const double hypot_db   = sqrt((delta_x_db * delta_x_db) + (delta_y_db * delta_y_db));

    const double x_incr_db  = (delta_x_db / hypot_db);
    const double y_incr_db  = (delta_y_db / hypot_db);

    double current_x_db = double(origin.x) + 0.5;
    double current_y_db = double(origin.y) + 0.5;

    P current_pos = P(int(current_x_db), int(current_y_db));

    const double step_size_db = 0.04;

    for (double i = 0.0; i <= 9999.0; i += step_size_db)
    {
        current_x_db += x_incr_db * step_size_db;
        current_y_db += y_incr_db * step_size_db;

        current_pos.set(floor(current_x_db), floor(current_y_db));

        if (!allow_outside_map && !map::is_pos_inside_map(current_pos))
        {
            return;
        }

        bool is_pos_ok_to_add = false;

        if (line_ref.empty())
        {
            is_pos_ok_to_add = true;
        }
        else
        {
            is_pos_ok_to_add = line_ref.back() != current_pos;
        }

        if (is_pos_ok_to_add)
        {
            line_ref.push_back(current_pos);
        }

        //Check distance limits
        if (should_stop_at_target && (current_pos == tgt))
        {
            return;
        }

        const int distance_traveled = king_dist(origin.x, origin.y, current_pos.x, current_pos.y);

        if (distance_traveled >= cheb_travel_limit)
        {
            return;
        }
    }
}

} //line_calc
