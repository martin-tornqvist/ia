#include "populate_items.hpp"

#include <vector>

#include "actor_player.hpp"
#include "map.hpp"
#include "item_factory.hpp"
#include "player_bon.hpp"
#include "map_parsing.hpp"

namespace populate_items
{

void mk_items_on_floor()
{
    int nr_spawns = rnd::range(4, 5);

    if (player_bon::traits[size_t(Trait::treasure_hunter)])
    {
        nr_spawns += 2;
    }

    std::vector<Item_id> item_bucket;
    item_bucket.clear();

    for (int i = 0; i < int(Item_id::END); ++i)
    {
        const Item_data_t& data = item_data::data[i];

        //The following items are NOT allowed to spawn on the floor:
        // * Intrinsic items
        // * Items with a dlvl range not matching current dlvl
        // * Items forbidden to spawn (e.g. unique items already spawned)

        if (
            int(data.type) < int(Item_type::END_OF_EXTR_ITEMS)      &&
            data.spawn_std_range.is_in_range(map::dlvl)             &&
            data.allow_spawn                                        &&
            rnd::percent(data.chance_to_incl_in_floor_spawn_list))
        {
            item_bucket.push_back(Item_id(i));
        }
    }

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::Blocks_items(), blocked);

    std::vector<P> free_cells;
    to_vec((bool*)blocked, false, map_w, map_h, free_cells);

    for (int i = 0; i < nr_spawns; ++i)
    {
        if (free_cells.empty() || item_bucket.empty())
        {
            break;
        }

        const size_t    cell_idx    = rnd::range(0, free_cells.size() - 1);
        const P&        pos         = free_cells[cell_idx];
        const size_t    item_idx    = rnd::range(0, item_bucket.size() - 1);
        const Item_id   id          = item_bucket[item_idx];

        if (item_data::data[size_t(id)].allow_spawn)
        {
            item_factory::mk_item_on_floor(id, pos);

            free_cells.erase(begin(free_cells) + cell_idx);
        }
        else
        {
            item_bucket.erase(begin(item_bucket) + item_idx);
        }
    }
}

} //populate_items
