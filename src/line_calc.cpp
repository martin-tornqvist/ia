#include "line_calc.hpp"

#include <math.h>
#include <vector>

#include "utils.hpp"

using namespace std;

namespace line_calc
{

namespace
{

double      fov_abs_distances_[FOV_MAX_W_INT][FOV_MAX_W_INT];
vector<Pos> fov_delta_lines_[FOV_MAX_W_INT][FOV_MAX_W_INT];

} //Namespace

void init()
{
    //----------------------------------------------------------
    //Calculate FOV absolute distances
    for (int y = 0; y < FOV_MAX_W_INT; ++y)
    {
        for (int x = 0; x < FOV_MAX_W_INT; ++x)
        {
            fov_abs_distances_[x][y] = 0;
        }
    }

    double delta_x, delta_y, hypot;

    //delta_x corresponds to i - FOV_MAX_W_INT, and vice versa for delta_y and j.
    //So:
    //i = 0                 => delta_x = -FOV_MAX_W_INT
    //i = FOV_MAX_W_INT * 2 => delta_x =  FOV_MAX_W_INT

    const int R_INT = FOV_MAX_RADI_INT;

    for (int x = 0; x <= R_INT * 2; ++x)
    {
        for (int y = 0; y <= R_INT * 2; ++y)
        {
            delta_x = double(x);
            delta_x -= FOV_MAX_RADI_DB;
            delta_y = double(y);
            delta_y -= FOV_MAX_RADI_DB;
            hypot = sqrt((delta_x * delta_x) + (delta_y * delta_y));
            fov_abs_distances_[x][y] = floor(hypot);
        }
    }

    //----------------------------------------------------------
    //Calculate FOV delta lines
    for (int delta_x = -R_INT; delta_x <= R_INT; delta_x++)
    {
        for (int delta_y = -R_INT; delta_y <= R_INT; delta_y++)
        {
            const Pos origin(0, 0);
            const Pos tgt(Pos(delta_x, delta_y));
            vector<Pos> cur_line;
            calc_new_line(origin, tgt, true, 999, true, cur_line);
            fov_delta_lines_[delta_x + R_INT][delta_y + R_INT] = cur_line;
        }
    }
}

const vector<Pos>* fov_delta_line(const Pos& delta,
                                  const double& MAX_DIST_ABS)
{
    const int X = delta.x + FOV_MAX_RADI_INT;
    const int Y = delta.y + FOV_MAX_RADI_INT;

    if (X >= 0 && Y >= 0 && X < FOV_MAX_W_INT && Y < FOV_MAX_W_INT)
    {
        if (fov_abs_distances_[X][Y] <= MAX_DIST_ABS)
        {
            return &(fov_delta_lines_[X][Y]);
        }
    }

    return nullptr;
}

void calc_new_line(const Pos& origin, const Pos& tgt,
                   const bool SHOULD_STOP_AT_TARGET, const int CHEB_TRAVEL_LIMIT,
                   const bool ALLOW_OUTSIDE_MAP, vector<Pos>& line_ref)
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

    Pos cur_pos = Pos(int(cur_x_db), int(cur_y_db));

    const double STEP_SIZE_DB = 0.04;

    for (double i = 0.0; i <= 9999.0; i += STEP_SIZE_DB)
    {
        cur_x_db += X_INCR_DB * STEP_SIZE_DB;
        cur_y_db += Y_INCR_DB * STEP_SIZE_DB;

        cur_pos.set(floor(cur_x_db), floor(cur_y_db));

        if (!ALLOW_OUTSIDE_MAP && !utils::is_pos_inside_map(cur_pos)) {return;}

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

        const int DISTANCE_TRAVELED = utils::king_dist(origin.x, origin.y, cur_pos.x, cur_pos.y);

        if (DISTANCE_TRAVELED >= CHEB_TRAVEL_LIMIT)
        {
            return;
        }
    }
}

} //line_calc
