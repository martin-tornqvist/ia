#include "rl_utils.hpp"

std::vector<P> pathfind(
        const P& p0,
        const P& p1,
        const Array2<bool>& blocked,
        const bool allow_diagonal,
        const bool randomize_steps)
{
        const auto flood = floodfill(
                p0,
                blocked,
                -1,
                p1,
                allow_diagonal);

        const auto path = pathfind_with_flood(
                p0,
                p1,
                flood,
                allow_diagonal,
                randomize_steps);

        return path;
}

std::vector<P> pathfind_with_flood(
        const P& p0,
        const P& p1,
        const Array2<int>& flood,
        const bool allow_diagonal,
        const bool randomize_steps)
{
        std::vector<P> path;

        if (p0 == p1)
        {
                // Origin and target is same cell
                return path;
        }

        if (flood.at(p1) == 0)
        {
                // No path exists
                return path;
        }

        const std::vector<P>& dirs =
                allow_diagonal
                ? dir_utils::dir_list
                : dir_utils::cardinal_list;

        const size_t nr_dirs = dirs.size();

        // Corresponds to the elements in "dirs"
        std::vector<bool> valid_offsets(nr_dirs, false);

        // The path length will be equal to the flood value at the target cell,
        // so we can reserve that many elements beforehand.
        path.reserve(flood.at(p1));

        // We start at the target cell
        P p(p1);

        path.push_back(p);

        const R rect(P(0, 0), flood.dims() - 1);

        while (true)
        {
                const int current_val = flood.at(p);

                P adj_p;

                // Find valid offsets, and check if origin is reached
                for (size_t i = 0; i < nr_dirs; ++i)
                {
                        const P& d(dirs[i]);

                        adj_p = p + d;

                        if (adj_p == p0)
                        {
                                // Origin reached
                                return path;
                        }

                        if (flood.rect().is_pos_inside(adj_p))
                        {
                                const int adj_val = flood.at(adj_p);

                                // Mark this as a valid travel direction if it
                                // is not blocked, and is fewer steps from the
                                // target than the current cell.
                                valid_offsets[i] =
                                        (adj_val != 0) &&
                                        (adj_val < current_val);
                        }
                }

                // Set the next position to one of the valid offsets - either
                // pick one randomly, or iterate over the list and pick the
                // first valid choice.
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

                path.push_back(adj_p);

                p = adj_p;

        } // while

        return path;
}
