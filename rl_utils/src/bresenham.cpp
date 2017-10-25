#include "rl_utils.hpp"

#include <cstdlib>

void bresenham(P p0, const P& p1, std::vector<P>& out)
{
    out.clear();

    const P deltas(p1 - p0);

    P abs_deltas(std::abs(deltas.x),
                 std::abs(deltas.y));

    out.reserve(std::max(
                    abs_deltas.x,
                    abs_deltas.y));

    abs_deltas.x = abs_deltas.x << 1;
    abs_deltas.y = abs_deltas.y << 1;

    const P signs(deltas.signs());

    // Walk the line and add positions on the way
    if (abs_deltas.x >= abs_deltas.y)
    {
        // Calculate the error factor, which may go below zero
        int error = abs_deltas.y - (abs_deltas.x >> 1);

        while (p0.x != p1.x)
        {
            if (error > 0)
            {
                if (error || (signs.x > 0))
                {
                    p0.y    += signs.y;
                    error   -= abs_deltas.x;
                }
            }

            p0.x    += signs.x;
            error   += abs_deltas.y;

            out.push_back(p0);
        }
    }
    else // abs_deltas.x < abs_deltas.y
    {
        // Calculate the error factor, which may go below zero
        int error = abs_deltas.x - (abs_deltas.y >> 1);

        while (p0.y != p1.y)
        {
            if (error > 0)
            {
                if (error || (signs.y > 0))
                {
                    p0.x    += signs.x;
                    error   -= abs_deltas.y;
                }
            }
            p0.y    += signs.y;
            error   += abs_deltas.x;

            out.push_back(p0);
        }
    }
}
