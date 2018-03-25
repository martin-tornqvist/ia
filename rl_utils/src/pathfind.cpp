#include "rl_utils.hpp"

void pathfind(const P& p0,
              const P& p1,
              const bool blocked[map_w][map_h],
              std::vector<P>& out,
              const bool allow_diagonal,
              const bool randomize_steps)
{
    int flood[map_w][map_h];

    floodfill(
        p0,
        blocked,
        flood,
        -1,
        p1,
        allow_diagonal);

    pathfind_with_flood(
        p0,
        p1,
        flood,
        out,
        allow_diagonal,
        randomize_steps);
}

void pathfind_with_flood(const P& p0,
                         const P& p1,
                         const int flood[map_w][map_h],
                         std::vector<P>& out,
                         const bool allow_diagonal,
                         const bool randomize_steps)
{
    out.clear();

    if (p0 == p1)
    {
        // Origin and target is same cell
        return;
    }

    if (flood[p1.x][p1.y] == 0)
    {
        // No path exists
        return;
    }

    const std::vector<P>& dirs = allow_diagonal ?
                                 dir_utils::dir_list :
                                 dir_utils::cardinal_list;

    const size_t nr_dirs = dirs.size();

    // Corresponds to the elements in "dirs"
    std::vector<bool> valid_offsets(nr_dirs, false);

    // The path length will be equal to the flood value at the target cell, so
    // we can reserve that many elements beforehand.
    out.reserve(flood[p1.x][p1.y]);

    // We start at the target cell
    P p(p1);
    out.push_back(p);

    const R map_r(P(0, 0), P(map_w, map_h) - 1);

    while (true)
    {
        const int current_val = flood[p.x][p.y];

        P adj_p;

        // Find valid offsets, and check if origin is reached
        for (size_t i = 0; i < nr_dirs; ++i)
        {
            const P& d(dirs[i]);

            adj_p = p + d;

            if (adj_p == p0)
            {
                // Origin reached
                return;
            }

            if (map_r.is_pos_inside(adj_p))
            {
                const int adj_val = flood[adj_p.x][adj_p.y];

                // Mark this as a valid travel direction if it is not blocked,
                // and is fewer steps from the target than the current cell.
                valid_offsets[i] =
                    (adj_val != 0) &&
                    (adj_val < current_val);
            }
        }

        // Set the next position to one of the valid offsets - either pick one
        // randomly, or iterate over the list and pick the first valid choice.
        if (randomize_steps)
        {
            std::vector<P> adj_p_bucket;

            for (size_t i = 0; i < nr_dirs; ++i)
            {
                if (valid_offsets[i])
                {
                    adj_p_bucket.push_back(p + dirs[i]);
                }
            }

            ASSERT(!adj_p_bucket.empty());

            adj_p = rnd::element(adj_p_bucket);
        }
        else // Do not randomize step choices - iterate over offset list
        {
            for (size_t i = 0; i < nr_dirs; ++i)
            {
                if (valid_offsets[i])
                {
                    adj_p = P(p + dirs[i]);
                    break;
                }
            }
        }

        out.push_back(adj_p);

        p = adj_p;

    } // while
}
