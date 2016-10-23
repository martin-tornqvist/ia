#include "mapgen.hpp"

namespace mapgen
{

void merge_regions(Region regions[3][3])
{
    TRACE_FUNC_BEGIN;

    const int nr_merges_to_mk = rnd::range_binom(0, 3, 0.5);

    if (nr_merges_to_mk == 0)
    {
        return;
    }

    const int idx_w_min = 1;
    const int idx_w_max = 2;

    const int idx_h_min = 2;
    const int idx_h_max = 3;

    //
    // Set up merge candidates (we don't care if the regions are free for now,
    // since we'll have to check that later before each merge anyway)
    //

    // The elements of this vector corresponds to sets of region indexes
    std::vector<R> merge_idx_bucket;

    for (int idx_w = idx_w_min; idx_w <= idx_w_max; ++idx_w)
    {
        for (int idx_h = idx_h_min; idx_h <= idx_h_max; ++idx_h)
        {
            // Avoid merging 1x1
            if (idx_w == 1 && idx_h == 1)
            {
                continue;
            }

            const int check_start_x1 = 3 - idx_w;
            const int check_start_y1 = 3 - idx_h;

            for (int start_x = 0; start_x <= check_start_x1; ++start_x)
            {
                for (int start_y = 0; start_y <= check_start_y1; ++start_y)
                {
                    const P start_p(start_x, start_y);

                    const P dim(idx_w, idx_h);

                    const R r(start_p, start_p + dim - 1);

                    ASSERT(r.p1.x < 3 && r.p1.y < 3);

                    merge_idx_bucket.push_back(r);
                }
            }
        }
    }

    //
    // Make merges
    //

    auto is_regions_free = [&regions](const R& r)
    {
        for (int x = r.p0.x; x <= r.p1.x; ++x)
        {
            for (int y = r.p0.y; y <= r.p1.y; ++y)
            {
                if (!regions[x][y].is_free)
                {
                    return false;
                }
            }
        }

        return true;
    };

    for (int merge_nr = 0; merge_nr < nr_merges_to_mk; /* No increment */)
    {
        if (merge_idx_bucket.empty())
        {
            // No more merges possible
            return;
        }

        const size_t bucket_idx =
            rnd::range(0, merge_idx_bucket.size() - 1);

        const R idx_r = merge_idx_bucket[bucket_idx];

        // If the regions are not free (could be because it was blocked before
        // we started, or because we have blocked it with another merge),
        // discard it from the bucket
        if (!is_regions_free(idx_r))
        {
            merge_idx_bucket.erase(
                begin(merge_idx_bucket) + bucket_idx);

            continue;
        }

        //
        // OK, we can make a merge here
        //

        TRACE << "Merging regions: "
              << "(" << idx_r.p0.x << ", " << idx_r.p0.y << ")"
              << " to "
              << "(" << idx_r.p1.x << ", " << idx_r.p1.y << ")"
              << std::endl;

        // NOTE: Region 0 is the top left region, region 1 is the bottom right
        auto& region_0 = regions[idx_r.p0.x][idx_r.p0.y];
        auto& region_1 = regions[idx_r.p1.x][idx_r.p1.y];

        // Expand region 1 over all areas
        region_0.r = R(region_0.r.p0, region_1.r.p1);

        // Set all other regions to blocked, ant to cover no space on the map
        for (int x = idx_r.p0.x; x <= idx_r.p1.x; ++x)
        {
            for (int y = idx_r.p0.y; y <= idx_r.p1.y; ++y)
            {
                const P idx_p(x, y);

                auto& reg = regions[idx_p.x][idx_p.y];

                if (idx_p != idx_r.p0)
                {
                    reg.is_free = false;

                    reg.r = R(-1, -1, -1, -1);
                }
            }
        }

        mk_room(region_0);

        // Merge successful
        ++merge_nr;
    }

    TRACE_FUNC_END;

} // merge_regions

} // mapgen
