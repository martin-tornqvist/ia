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

    const int R_INT = fov_max_radi_int;

    for (int x = 0; x <= R_INT * 2; ++x)
    {
        for (int y = 0; y <= R_INT * 2; ++y)
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
    for (int delta_x = -R_INT; delta_x <= R_INT; delta_x++)
    {
        for (int delta_y = -R_INT; delta_y <= R_INT; delta_y++)
        {
            const P origin(0, 0);
            const P tgt(P(delta_x, delta_y));
            std::vector<P> cur_line;
            calc_new_line(origin, tgt, true, 999, true, cur_line);
            fov_delta_lines_[delta_x + R_INT][delta_y + R_INT] = cur_line;
        }
    }
}

const std::vector<P>* fov_delta_line(const P& delta,
                                     const double& MAX_DIST_ABS)
{
    const int X = delta.x + fov_max_radi_int;
    const int Y = delta.y + fov_max_radi_int;

    if (X >= 0 && Y >= 0 && X < fov_max_w_int && Y < fov_max_w_int)
    {
        if (fov_abs_distances_[X][Y] <= MAX_DIST_ABS)
        {
            return &(fov_delta_lines_[X][Y]);
        }
    }

    return nullptr;
}

void calc_new_line(const P& origin,
                   const P& tgt,
                   const bool SHOULD_STOP_AT_TARGET,
                   const int CHEB_TRAVEL_LIMIT,
                   const bool ALLOW_OUTSIDE_MAP,
                   std::vector<P>& line_ref)
{
    line_ref.clear();

    if (tgt == origin)
    {
        line_ref.push_back(origin);
        return;
    }

    const double DELTA_X_DB = double(tgt.x - origin.x);
    const double DELTA_Y_DB = double(tgt.y - origin.y);

    const double HYPOT_DB   = sqrt((DELTA_X_DB * DELTA_X_DB) + (DELTA_Y_DB * DELTA_Y_DB));

    const double X_INCR_DB  = (DELTA_X_DB / HYPOT_DB);
    const double Y_INCR_DB  = (DELTA_Y_DB / HYPOT_DB);

    double cur_x_db = double(origin.x) + 0.5;
    double cur_y_db = double(origin.y) + 0.5;

    P cur_pos = P(int(cur_x_db), int(cur_y_db));

    const double STEP_SIZE_DB = 0.04;

    for (double i = 0.0; i <= 9999.0; i += STEP_SIZE_DB)
    {
        cur_x_db += X_INCR_DB * STEP_SIZE_DB;
        cur_y_db += Y_INCR_DB * STEP_SIZE_DB;

        cur_pos.set(floor(cur_x_db), floor(cur_y_db));

        if (!ALLOW_OUTSIDE_MAP && !map::is_pos_inside_map(cur_pos))
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
            is_pos_ok_to_add = line_ref.back() != cur_pos;
        }

        if (is_pos_ok_to_add)
        {
            line_ref.push_back(cur_pos);
        }

        //Check distance limits
        if (SHOULD_STOP_AT_TARGET && (cur_pos == tgt))
        {
            return;
        }

        const int DISTANCE_TRAVELED = king_dist(origin.x, origin.y, cur_pos.x, cur_pos.y);

        if (DISTANCE_TRAVELED >= CHEB_TRAVEL_LIMIT)
        {
            return;
        }
    }
}

} //line_calc
