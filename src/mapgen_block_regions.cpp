#include "mapgen.hpp"
#include "map.hpp"

namespace mapgen
{

void randomly_block_regions(Region regions[3][3])
{
    TRACE_FUNC_BEGIN << "Marking some (possibly zero) regions as built, to "
                     << "prevent rooms there (so it's not always 3x3 rooms)"
                     << std:: endl;
    //NOTE: The max number to try can go above the hard limit of regions that
    //could ever be blocked (i.e five regions - blocking is only allowed if
    //no cardinally adjacent region is already blocked). However, this will push
    //the number of attempts towards the upper limit, and increase the chance
    //of a higher number of attempts.
    const int max_nr_to_try_block = std::max(1, map::dlvl / 3);
    const int nr_to_try_block     = std::min(rnd::range(0, max_nr_to_try_block), 5);

    for (int i = 0; i < nr_to_try_block; ++i)
    {
        TRACE_VERBOSE << "Attempting to block region " << i + 1 << "/"
                      << nr_to_try_block << std:: endl;

        std::vector<P> block_bucket;

        for (int x = 0; x < 3; ++x)
        {
            for (int y = 0; y < 3; ++y)
            {
                if (regions[x][y].is_free_)
                {
                    bool is_all_adj_free = true;
                    const P p(x, y);

                    for (const P& d : dir_utils::cardinal_list)
                    {
                        const P p_adj(p + d);

                        if (p_adj >= 0 && p_adj <= 2 && p_adj != p)
                        {
                            if (!regions[p_adj.x][p_adj.y].is_free_)
                            {
                                is_all_adj_free = false;
                                break;
                            }
                        }
                    }

                    if (is_all_adj_free)
                    {
                        block_bucket.push_back(p);
                    }
                }
            }
        }

        if (block_bucket.empty())
        {
            TRACE_VERBOSE << "Failed to find eligible regions to block, after "
                          << "blocking " << i << " regions" << std:: endl;
            break;
        }
        else
        {
            const P& p(block_bucket[rnd::range(0, block_bucket.size() - 1)]);

            TRACE_VERBOSE << "Blocking region at " << p.x << "," << p.y << std:: endl;
            regions[p.x][p.y].is_free_ = false;
        }
    }

    TRACE_FUNC_END;
}

} //mapgen

