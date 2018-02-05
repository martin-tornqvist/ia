#include "rl_utils.hpp"

#include <climits>
#include <sstream>

void set_constr_in_range(const int min, int& val, const int max)
{
    if (max >= min)
    {
        val = std::min(max, std::max(val, min));
    }
}

void set_constr_in_range(const double min, double& val, const double max)
{
    if (max > min)
    {
        val = std::min(max, std::max(val, min));
    }
}

int constr_in_range(const int min, const int val, const int max)
{
    if (max < min)
    {
        return -1;
    }

    return std::min(max, std::max(val, min));
}

int constr_in_range(const double min, const double val, const double max)
{
    if (max < min)
    {
        return -1;
    }

    return std::min(max, std::max(val, min));
}

std::vector<P> to_vec(const bool a[map_w][map_h],
                      const bool value_to_store,
                      const R& area_to_parse)
{
    std::vector<P> result;

    // Reserve space for worst case of push-backs
    result.reserve(area_to_parse.area());

    for (int x = area_to_parse.p0.x; x < area_to_parse.p1.x; ++x)
    {
        for (int y = area_to_parse.p0.y; y < area_to_parse.p1.y; ++y)
        {
            if (a[x][y] == value_to_store)
            {
                result.emplace_back(P(x, y));
            }
        }
    }

    return result;
}

bool is_pos_inside(const P& pos, const R& area)
{
    return
        pos.x >= area.p0.x &&
        pos.x <= area.p1.x &&
        pos.y >= area.p0.y &&
        pos.y <= area.p1.y;
}

bool is_area_inside(const R& inner,
                    const R& outer,
                    const bool count_equal_as_inside)
{
    if (count_equal_as_inside)
    {
        return inner.p0.x >= outer.p0.x &&
               inner.p1.x <= outer.p1.x &&
               inner.p0.y >= outer.p0.y &&
               inner.p1.y <= outer.p1.y;
    }
    else
    {
        return inner.p0.x > outer.p0.x &&
               inner.p1.x < outer.p1.x &&
               inner.p0.y > outer.p0.y &&
               inner.p1.y < outer.p1.y;
    }
}

int king_dist(const int x0, const int y0, const int x1, const int y1)
{
    return std::max(abs(x1 - x0), abs(y1 - y0));
}

int king_dist(const P& p0, const P& p1)
{
    return std::max(abs(p1.x - p0.x), abs(p1.y - p0.y));
}

int taxi_dist(const P& p0, const P& p1)
{
    return abs(p1.x - p0.x) + abs(p1.y - p0.y);
}

P closest_pos(const P& p, const std::vector<P>& positions)
{
    int dist_to_nearest = INT_MAX;

    P closest_pos;

    for (P p_cmp : positions)
    {
        const int dist = king_dist(p, p_cmp);

        if (dist < dist_to_nearest)
        {
            dist_to_nearest = dist;
            closest_pos     = p_cmp;
        }
    }

    return closest_pos;
}

bool is_pos_adj(const P& pos1,
                const P& pos2,
                const bool count_same_cell_as_adj)
{
    if (
        pos1.x < pos2.x - 1 ||
        pos1.x > pos2.x + 1 ||
        pos1.y < pos2.y - 1 ||
        pos1.y > pos2.y + 1)
    {
        return false;
    }
    else if (pos1.x == pos2.x && pos1.y == pos2.y)
    {
        return count_same_cell_as_adj;
    }

    return true;
}

bool is_val_in_range(const int V, const Range range)
{
    return range.is_in_range(V);
}

int to_int(const std::string& in)
{
    int _nr;
    std::istringstream buffer(in);
    buffer >> _nr;
    return _nr;
}
