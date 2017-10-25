#include "rl_utils.hpp"

void floodfill(const P& p0,
               const bool blocked[map_w][map_h],
               int out[map_w][map_h],
               int travel_lmt,
               const P& p1,
               const bool allow_diagonal)
{
    std::fill_n(*out, nr_map_cells, 0);

    // List of positions to travel to
    std::vector<P> positions;

    // In the worst case we need to visit every position, reserve elements
    positions.reserve(nr_map_cells);

    // Instead of removing evaluated positions from the vector, we track which
    // index to try next (cheaper than erasing front elements).
    size_t next_p_idx = 0;

    int val = 0;
    bool path_exists = true;
    bool is_at_tgt = false;
    bool is_stopping_at_tgt = p1.x != -1;

    const R bounds(P(1, 1), P(map_w, map_h) - 2);

    P p(p0);

    const auto& dirs =
        allow_diagonal ?
        dir_utils::dir_list :
        dir_utils::cardinal_list;

    bool done = false;

    while (!done)
    {
        // "Flood" around the current position, and add those to the list of
        // positions to travel to.
        for (const P& d : dirs)
        {
            const P new_p(p + d);

            if (!blocked[new_p.x][new_p.y] &&
                bounds.is_p_inside(new_p) &&
                (out[new_p.x][new_p.y] == 0) &&
                (new_p != p0))
            {
                val = out[p.x][p.y];

                if ((travel_lmt == -1) ||
                    (val < travel_lmt))
                {
                    out[new_p.x][new_p.y] = val + 1;
                }

                if (is_stopping_at_tgt && new_p == p1)
                {
                    is_at_tgt = true;
                    break;
                }

                if (!is_stopping_at_tgt || !is_at_tgt)
                {
                    positions.push_back(new_p);
                }
            }
        } // Offset loop

        if (is_stopping_at_tgt)
        {
            if (positions.size() == next_p_idx)
            {
                path_exists = false;
            }

            if (is_at_tgt || !path_exists)
            {
                done = true;
            }
        }
        else if (positions.size() == next_p_idx)
        {
            done = true;
        }

        if (val == travel_lmt)
        {
            done = true;
        }

        if (!is_stopping_at_tgt || !is_at_tgt)
        {
            if (positions.size() == next_p_idx)
            {
                // No more positions to evaluate
                path_exists = false;
            }
            else // There are more positions to evaluate
            {
                p = positions[next_p_idx];

                ++next_p_idx;
            }
        }
    } // while
}
